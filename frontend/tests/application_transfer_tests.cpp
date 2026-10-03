#include "application.hpp"
#include "application_jobs.hpp"

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <mutex>
#include <random>
#include <stdexcept>
#include <string>
#include <thread>

namespace file_manager {
class ApplicationTransferProbe final {
public:
    static void post_worker(Application& application, std::function<void()> work) {
        application.post_worker(std::move(work));
    }
    static void navigate(Application& application, const std::filesystem::path& path) {
        application.request_navigation(path, true);
    }
    static const std::filesystem::path& location(const Application& application) {
        return application.location_;
    }
    static std::shared_ptr<gui_forms::Command> command(const Application& application,
                                                       const std::string_view id) {
        for (const std::shared_ptr<gui_forms::Command>& command : application.commands_) {
            if ((*command).stable_id() == id) return command;
        }
        return {};
    }
    static bool active(const Application& application) { return application.transfer_in_flight_; }
    static std::uint64_t generation(const Application& application) {
        const std::uint64_t result = application.transfer_generation_.load();
        return result;
    }
    static bool cancelled(const std::shared_ptr<Application>& application,
                          const std::uint64_t generation, const bool drop) {
        if (drop) {
            const Application::InternalDropCancelled check{application, generation};
            const bool result = check();
            return result;
        }
        const Application::TransferCancelled check{application, generation};
        const bool result = check();
        return result;
    }
};
}

namespace {
using Clock = std::chrono::steady_clock;
using Probe = file_manager::ApplicationTransferProbe;
namespace fs = std::filesystem;

void require(const bool value, const char* const message) {
    if (!value) throw std::runtime_error(message);
}
void noop() {}

class Fixture final {
public:
    Fixture() = default;
    Fixture(const Fixture&) = delete;
    Fixture& operator=(const Fixture&) = delete;
    ~Fixture() {
        if (!owned_) return;
        try { cleanup(); }
        catch (...) { std::cerr << "Transfer fixture cleanup failed; owned path retained\n"; }
    }
    void create() {
        parent_ = fs::canonical(fs::temp_directory_path());
        std::random_device random{};
        for (std::size_t attempt = 0; attempt < 64U; ++attempt) {
            const unsigned int first = random();
            const unsigned int second = random();
            const std::string name = "file-manager-transfer-" + std::to_string(first) + "-" + std::to_string(second);
            base_ = parent_ / name;
            std::error_code error{};
            const bool created = fs::create_directory(base_, error);
            if (!created) {
                if (error && error != std::errc::file_exists) throw std::system_error(error);
                continue;
            }
            identity_ = file_manager::observe_identity(base_);
            owned_ = true;
            require(identity_.available(), "fixture root identity unavailable");
            root_ = base_ / "files";
            quarantine_ = base_ / "quarantine";
            destination_ = root_ / "Destination";
            fs::create_directories(destination_);
            fs::create_directory(quarantine_);
            const fs::path source = root_ / "source.txt";
            std::ofstream stream(source, std::ios::binary);
            stream << "copy fixture\n";
            stream.close();
            require(static_cast<bool>(stream), "fixture write failed");
            return;
        }
        throw std::runtime_error("exclusive fixture creation exhausted");
    }
    void cleanup() {
        require(owned_ && base_.is_absolute() && base_.parent_path() == parent_ &&
            fs::canonical(base_) == base_ && file_manager::observe_identity(base_) == identity_,
            "fixture cleanup authority changed");
        std::error_code error{};
        fs::remove_all(base_, error);
        if (error) throw std::system_error(error);
        owned_ = false;
    }
    const fs::path& root() const noexcept { return root_; }
    const fs::path& quarantine() const noexcept { return quarantine_; }
    const fs::path& destination() const noexcept { return destination_; }
private:
    fs::path parent_{};
    fs::path base_{};
    fs::path root_{};
    fs::path quarantine_{};
    fs::path destination_{};
    file_manager::ObjectIdentity identity_{};
    bool owned_{};
};

class StopApplication final {
public:
    explicit StopApplication(file_manager::Application& application) : application_(application) {}
    ~StopApplication() { application_.stop(); }
    StopApplication(const StopApplication&) = delete;
    StopApplication& operator=(const StopApplication&) = delete;
private:
    file_manager::Application& application_;
};

// A named, owned barrier makes cancellation-before-dequeue deterministic.
// Its release guard dies before Application joins the worker, including failures.
class WorkerGate final {
public:
    void wait() {
        std::unique_lock<std::mutex> lock(mutex_);
        started_.store(true);
        const Clock::time_point deadline = Clock::now() + std::chrono::seconds(10);
        while (!released_) {
            if (condition_.wait_until(lock, deadline) == std::cv_status::timeout && !released_) {
                expired_.store(true);
                return;
            }
        }
    }
    void release() {
        {
            const std::lock_guard<std::mutex> lock(mutex_);
            released_ = true;
        }
        condition_.notify_all();
    }
    bool started() const noexcept {
        const bool value = started_.load();
        return value;
    }
    bool expired() const noexcept {
        const bool value = expired_.load();
        return value;
    }
private:
    std::mutex mutex_{};
    std::condition_variable condition_{};
    std::atomic_bool started_{};
    std::atomic_bool expired_{};
    bool released_{};
};
struct GateWork final {
    std::shared_ptr<WorkerGate> gate{};
    void operator()() const { (*gate).wait(); }
};
class ReleaseGate final {
public:
    explicit ReleaseGate(std::shared_ptr<WorkerGate> gate) : gate_(std::move(gate)) {}
    ~ReleaseGate() { (*gate_).release(); }
    ReleaseGate(const ReleaseGate&) = delete;
    ReleaseGate& operator=(const ReleaseGate&) = delete;
private:
    std::shared_ptr<WorkerGate> gate_{};
};
struct GateStarted final {
    const WorkerGate& gate;
    bool operator()() const { return gate.started(); }
};
struct TransferFinished final {
    const file_manager::Application& application;
    bool operator()() const {
        const bool finished = !Probe::active(application);
        return finished;
    }
};
struct CommandEnabled final {
    const gui_forms::Command& command;
    bool operator()() const { return command.state().enabled; }
};

template<class Predicate>
void wait_for(file_manager::Application& application, const Predicate& predicate, const char* const message) {
    const Clock::time_point deadline = Clock::now() + std::chrono::seconds(5);
    while (!predicate()) {
        application.drain_ui();
        require(Clock::now() < deadline, message);
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

struct SourceListed final {
    const gui_forms::ObjectView& objects;
    bool operator()() const {
        for (const gui_forms::ObjectViewItem& item : objects.items()) {
            if (item.name == "source.txt") return true;
        }
        return false;
    }
};

void test_transfer_cancellation(const bool move) {
    Fixture fixture{};
    fixture.create();
    const std::shared_ptr<file_manager::Application> application =
        std::make_shared<file_manager::Application>(fixture.root(), fixture.quarantine(), true, "");
    const std::unique_ptr<gui_forms::Window> window = (*application).make_window();
    StopApplication stop(*application);
    (*application).bind_host(noop, noop);
    (*window).resize({800.0, 600.0});
    const std::shared_ptr<gui_forms::ObjectView> objects =
        std::dynamic_pointer_cast<gui_forms::ObjectView>((*window).find("fm.objects.current-folder"));
    const std::shared_ptr<gui_forms::Button> cancel = std::dynamic_pointer_cast<gui_forms::Button>(
        (*window).find("file-manager-app.shell.status.cancel-copy"));
    const std::shared_ptr<gui_forms::Label> status = std::dynamic_pointer_cast<gui_forms::Label>(
        (*window).find("file-manager-app.shell.status.ready"));
    const std::shared_ptr<gui_forms::Command> capture = Probe::command(*application, move ? "selection.move" : "selection.copy");
    const std::shared_ptr<gui_forms::Command> paste = Probe::command(*application, "selection.paste");
    const std::shared_ptr<gui_forms::Command> cancel_command = Probe::command(*application, "transfer.cancel");
    require(objects && cancel && status && capture && paste && cancel_command, "transfer controls absent");
    require(!(*cancel).visible() && !(*cancel_command).state().enabled, "idle must not offer copy cancellation");
    wait_for(*application, SourceListed{*objects}, "source listing timed out");
    std::string source_id{};
    for (const gui_forms::ObjectViewItem& item : (*objects).items()) {
        if (item.name == "source.txt") source_id = item.stable_id;
    }
    const bool selected = (*window).perform_semantic_action(source_id, gui_forms::SemanticAction::select);
    require(selected, "source selection failed");
    const bool captured = (*capture).execute("fixture.transfer.capture");
    require(captured, "source capture failed");
    Probe::navigate(*application, fixture.destination());
    wait_for(*application, CommandEnabled{*paste}, "destination navigation timed out");

    const std::shared_ptr<WorkerGate> gate = std::make_shared<WorkerGate>();
    ReleaseGate release(gate);
    Probe::post_worker(*application, GateWork{gate});
    wait_for(*application, GateStarted{*gate}, "worker gate did not start");
    const bool pasted = (*paste).execute("fixture.transfer.paste");
    require(pasted && Probe::active(*application), "Paste must remain active while queued");
    const std::uint64_t generation = Probe::generation(*application);
    const fs::path destination = fixture.destination() / "source.txt";
    if (move) {
        require(!(*cancel).visible() && !(*cancel_command).state().enabled,
            "atomic move must not advertise cancellation");
        const bool cancelled = (*cancel_command).execute("fixture.transfer.move-cancel");
        require(!cancelled && !Probe::cancelled(application, generation, false), "disabled cancel must not cancel a move");
    } else {
        (*window).perform_layout();
        const gui_forms::Rect bounds = (*cancel).absolute_bounds();
        require((*cancel).effectively_visible() && (*cancel).enabled() && bounds.width >= 90.0 &&
            bounds.height >= 20.0 && bounds.x >= 0.0 && bounds.x + bounds.width <= 800.0 &&
            bounds.y >= 0.0 && bounds.y + bounds.height <= 600.0, "active Cancel copy must fit the footer");
        const bool cancelled = (*window).perform_semantic_action(
            "file-manager-app.shell.status.cancel-copy", gui_forms::SemanticAction::press);
        require(cancelled && Probe::active(*application), "cancel request must retain busy state until terminal result");
        require(Probe::generation(*application) == generation && Probe::cancelled(application, generation, false) &&
            Probe::cancelled(application, generation, true), "request must revoke both copy routes without changing job identity");
        require((*cancel).text() == "Stopping…" && !(*cancel).enabled() && !(*paste).state().enabled,
            "pending cancellation must not offer another request or Paste");
        require(!fs::exists(destination), "queued cancelled copy must not publish a destination");
    }
    (*gate).release();
    wait_for(*application, TransferFinished{*application}, "transfer terminal result timed out");
    require(!(*gate).expired() && !(*cancel).visible(), "terminal result must retire cancellation controls");
    if (move) {
        require(fs::exists(destination), "uncancelled move must publish normally");
    } else {
        require(!fs::exists(destination) && (*status).text() == "Copy cancelled" && (*paste).state().enabled,
            "cancelled copy must leave no destination and retain capture for retry");
        const bool retried = (*paste).execute("fixture.transfer.retry");
        require(retried, "copy retry must remain available");
        // Wait for real destination publication without letting UI observe the result.
        const Clock::time_point deadline = Clock::now() + std::chrono::seconds(5);
        while (!fs::exists(destination)) {
            require(Clock::now() < deadline, "retry commit timed out");
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        const bool late_cancel = (*cancel_command).execute("fixture.transfer.after-commit");
        require(late_cancel, "UI may request cancellation before observing a queued successful result");
        wait_for(*application, TransferFinished{*application}, "committed reply timed out");
        require(fs::exists(destination) && !(*paste).state().enabled && !(*cancel).visible(),
            "a committed copy must remain success after a late cancellation request");
    }
    (*application).stop();
    fixture.cleanup();
}

enum class NewFolderCase { commit_name, cancel_name, navigate_queued, navigate_after_create, edit_location_after_create };

struct RenameVisible final {
    const gui_forms::TextBox& editor;
    bool operator()() const { return editor.visible(); }
};
struct LocationReady final {
    const file_manager::Application& application;
    const fs::path& expected;
    bool operator()() const {
        const bool ready = Probe::location(application) == expected;
        return ready;
    }
};
struct ObjectNamed final {
    const gui_forms::ObjectView& objects;
    std::string name{};
    bool operator()() const {
        for (const gui_forms::ObjectViewItem& item : objects.items()) {
            if (item.name == name) return true;
        }
        return false;
    }
};

void test_new_folder_naming(const NewFolderCase scenario) {
    Fixture fixture{};
    fixture.create();
    const fs::path occupied = fixture.root() / "New folder";
    fs::create_directory(occupied);
    const file_manager::ObjectIdentity occupied_identity = file_manager::observe_identity(occupied);
    const std::shared_ptr<file_manager::Application> application =
        std::make_shared<file_manager::Application>(fixture.root(), fixture.quarantine(), true, "");
    const std::unique_ptr<gui_forms::Window> window = (*application).make_window();
    StopApplication stop(*application);
    (*application).bind_host(noop, noop);
    (*window).resize({800.0, 600.0});
    const std::shared_ptr<gui_forms::ObjectView> objects = std::dynamic_pointer_cast<gui_forms::ObjectView>(
        (*window).find("fm.objects.current-folder"));
    const std::shared_ptr<gui_forms::TextBox> rename = std::dynamic_pointer_cast<gui_forms::TextBox>(
        (*window).find("fm.operations.rename"));
    const std::shared_ptr<gui_forms::Command> create = Probe::command(*application, "file.new-folder");
    const std::shared_ptr<gui_forms::Command> undo = Probe::command(*application, "edit.undo");
    const std::shared_ptr<gui_forms::Command> focus_location = Probe::command(*application, "go.location");
    const std::shared_ptr<gui_forms::TextBox> location = std::dynamic_pointer_cast<gui_forms::TextBox>(
        (*window).find("fm.path.editor"));
    require(objects && rename && create && undo && location && focus_location, "new-folder controls absent");
    wait_for(*application, SourceListed{*objects}, "new-folder listing timed out");
    const bool navigating = scenario == NewFolderCase::navigate_queued ||
        scenario == NewFolderCase::navigate_after_create;
    const std::shared_ptr<WorkerGate> gate = std::make_shared<WorkerGate>();
    ReleaseGate release(gate);
    if (scenario == NewFolderCase::navigate_queued) {
        Probe::post_worker(*application, GateWork{gate});
        wait_for(*application, GateStarted{*gate}, "new-folder gate did not start");
    }
    const bool requested = (*create).execute("fixture.new-folder");
    require(requested, "New folder command refused");
    if (scenario == NewFolderCase::navigate_after_create || scenario == NewFolderCase::edit_location_after_create) {
        // Creation precedes this barrier; its refresh remains behind it.
        Probe::post_worker(*application, GateWork{gate});
        wait_for(*application, GateStarted{*gate}, "created-folder gate did not start");
        (*application).drain_ui();
        require(fs::exists(fixture.root() / "New folder 2") && !(*rename).visible(),
            "created folder must await the authoritative refreshed listing");
    }
    const fs::path created = fixture.root() / "New folder 2";
    if (navigating) {
        Probe::navigate(*application, fixture.destination());
        (*gate).release();
        wait_for(*application, LocationReady{*application, fixture.destination()}, "newer navigation was lost");
        require(fs::is_directory(created) && !(*rename).visible() && !(*gate).expired(),
            "completed creation must preserve newer navigation without opening an editor");
    } else if (scenario == NewFolderCase::edit_location_after_create) {
        const bool focused = (*focus_location).execute("fixture.new-folder.focus-location");
        const bool typed = (*window).dispatch_text({"draft path"});
        require(focused && typed && (*window).focused_control() == location, "location editor did not receive focus");
        (*gate).release();
        wait_for(*application, ObjectNamed{*objects, "New folder 2"}, "new-folder refresh timed out during location editing");
        require(!(*rename).visible() && (*window).focused_control() == location && (*location).text() == "draft path",
            "new-folder naming must not steal an active text editor's focus or contents");
    } else {
        wait_for(*application, RenameVisible{*rename}, "created folder did not open its name editor");
        const file_manager::ObjectIdentity created_identity = file_manager::observe_identity(created);
        require(created_identity.available() && (*rename).text() == "New folder 2" &&
            (*rename).selected_text() == "New folder 2" && (*window).focused_control() == rename &&
            (*objects).selected_ids().size() == 1U, "new folder name must be fully selected and focused");
        if (scenario == NewFolderCase::commit_name) {
            const bool typed = (*window).dispatch_text({"Project notes"});
            const bool committed = (*window).dispatch_key({gui_forms::KeyAction::down, gui_forms::PhysicalKey::enter});
            require(typed && committed, "new folder name entry failed");
            wait_for(*application, ObjectNamed{*objects, "Project notes"}, "named folder refresh timed out");
            const fs::path renamed = fixture.root() / "Project notes";
            require(file_manager::observe_identity(renamed) == created_identity && !fs::exists(created),
                "naming must rename the exact created directory");
        } else {
            const bool cancelled = (*window).dispatch_key({gui_forms::KeyAction::down, gui_forms::PhysicalKey::escape});
            require(cancelled && !(*rename).visible() && (*window).focused_control() == objects &&
                file_manager::observe_identity(created) == created_identity && (*undo).state().enabled,
                "Escape must retain the default-named folder and creation undo");
        }
    }
    require(file_manager::observe_identity(occupied) == occupied_identity,
        "New folder naming must preserve the preexisting directory");
    (*application).stop();
    fixture.cleanup();
}
}

int main() {
    try {
        test_transfer_cancellation(false);
        test_transfer_cancellation(true);
        test_new_folder_naming(NewFolderCase::commit_name);
        test_new_folder_naming(NewFolderCase::cancel_name);
        test_new_folder_naming(NewFolderCase::navigate_queued);
        test_new_folder_naming(NewFolderCase::navigate_after_create);
        test_new_folder_naming(NewFolderCase::edit_location_after_create);
        std::cout << "Application transfer and new-folder tests passed\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
