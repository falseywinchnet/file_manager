#include "application.hpp"
#include "application_jobs.hpp"

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <mutex>
#include <limits>
#include <new>
#include <random>
#include <stdexcept>
#include <string>
#include <thread>

namespace progress_allocation_fixture {
// Only the calling test thread is armed, around synchronous progress setup.
// Reject once, then permit normal failure handling and all other allocations.
thread_local std::size_t permitted = std::numeric_limits<std::size_t>::max();
thread_local bool rejected{};
}

void* operator new(const std::size_t bytes) {
    if (progress_allocation_fixture::permitted != std::numeric_limits<std::size_t>::max()) {
        if (progress_allocation_fixture::permitted == 0U) {
            progress_allocation_fixture::permitted = std::numeric_limits<std::size_t>::max();
            progress_allocation_fixture::rejected = true;
            throw std::bad_alloc{};
        }
        --progress_allocation_fixture::permitted;
    }
    const std::size_t allocation_size = bytes == 0U ? 1U : bytes;
    void* const memory = std::malloc(allocation_size);
    if (memory == nullptr) throw std::bad_alloc{};
    return memory;
}

void operator delete(void* const memory) noexcept { std::free(memory); }
void operator delete(void* const memory, const std::size_t) noexcept { std::free(memory); }

namespace file_manager {
class ApplicationTransferProbe final {
public:
    static void post_worker(Application& application, std::function<void()> work) {
        application.post_worker(std::move(work));
    }
    static void post_operation(Application& application, std::function<void()> work) {
        application.post_operation(std::move(work));
    }
    static void request_undo(Application& application) {
        application.request_undo();
    }
    static bool stopping(const Application& application) {
        const bool stopped = application.stopping_.load();
        return stopped;
    }
    static std::size_t pending_operations(Application& application) {
        const std::lock_guard<std::mutex> lock(application.operation_mutex_);
        const std::size_t count = application.operation_queue_.size();
        return count;
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

    static bool progress_coalesces() {
        Application::TransferProgressState state{};
        CopyProgress progress{};
        progress.current_source = "source.txt";
        progress.total_bytes = 10000U;
        std::size_t notifications{};
        for (std::uint64_t bytes = 0U; bytes <= 10000U; ++bytes) {
            progress.copied_bytes = bytes;
            const bool notify = state.offer(progress);
            if (notify) ++notifications;
        }
        const CopyProgress latest = state.take();
        const bool rearmed = state.offer(progress);
        const bool correct = notifications == 1U && latest.copied_bytes == 10000U &&
            latest.total_bytes == 10000U && latest.current_source == "source.txt" && rearmed;
        return correct;
    }

    static bool progress_setup_failure_falls_back(const std::shared_ptr<Application>& application) {
        for (std::size_t permitted = 0U; permitted <= 1U; ++permitted) {
            progress_allocation_fixture::rejected = false;
            progress_allocation_fixture::permitted = permitted;
            const CopyProgressObserver observer = Application::TransferProgressReport::prepare(application, 7U);
            progress_allocation_fixture::permitted = std::numeric_limits<std::size_t>::max();
            if (!progress_allocation_fixture::rejected || observer) return false;
        }
        const CopyProgressObserver recovered = Application::TransferProgressReport::prepare(application, 7U);
        const bool available = static_cast<bool>(recovered);
        return available;
    }

    static bool progress_respects_terminal_state(const std::shared_ptr<Application>& application) {
        Application& model = *application;
        const std::shared_ptr<Application::TransferProgressState> state =
            std::make_shared<Application::TransferProgressState>();
        CopyProgress progress{};
        progress.current_source = "source.txt";
        progress.copied_bytes = 1024U;
        progress.total_bytes = 2048U;
        const bool offered = (*state).offer(progress);
        if (!offered) return false;
        model.transfer_generation_.store(7U);
        model.cancelled_transfer_generation_.store(7U);
        model.transfer_in_flight_ = true;
        model.set_status("Cancellation sentinel", "Keep terminal state");
        const Application::TransferProgressReady current{application, state, 7U};
        current();
        const gui_forms::Label& status = *model.form_.file_manager_app_shell_status_ready;
        if (status.text() != "Cancellation sentinel") return false;
        model.cancelled_transfer_generation_.store(0U);
        const Application::TransferProgressReady obsolete{application, state, 6U};
        obsolete();
        if (status.text() != "Cancellation sentinel") return false;
        model.transfer_in_flight_ = false;
        current();
        if (status.text() != "Cancellation sentinel") return false;
        model.transfer_in_flight_ = true;
        current();
        const bool current_visible = status.text() == "Copying source.txt";
        model.transfer_in_flight_ = false;
        return current_visible;
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

struct DirectoryCreated final {
    const fs::path& path;
    bool operator()() const {
        const bool created = fs::is_directory(path);
        return created;
    }
};

struct OperationCompleted final {
    std::atomic_bool completed{};
    void mark() { completed.store(true); }
    bool ready() const {
        const bool value = completed.load();
        return value;
    }
};

struct ReleaseOnStop final {
    std::shared_ptr<file_manager::Application> application{};
    std::shared_ptr<WorkerGate> gate{};
    std::atomic_bool& observed_stop;

    void operator()() const {
        const Clock::time_point deadline = Clock::now() + std::chrono::seconds(5);
        while (!Probe::stopping(*application) && Clock::now() < deadline) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        observed_stop.store(Probe::stopping(*application));
        (*gate).release();
    }
};

void test_operation_order_and_shutdown() {
    Fixture fixture{};
    fixture.create();
    const std::shared_ptr<file_manager::Application> application =
        std::make_shared<file_manager::Application>(fixture.root(), fixture.quarantine(), true, "");
    const std::unique_ptr<gui_forms::Window> window = (*application).make_window();
    StopApplication stop(*application);
    (*application).bind_host(noop, noop);
    const std::shared_ptr<gui_forms::ObjectView> objects = std::dynamic_pointer_cast<gui_forms::ObjectView>(
        (*window).find("fm.objects.current-folder"));
    const std::shared_ptr<gui_forms::Command> create = Probe::command(*application, "file.new-folder");
    require(objects && create, "operation-order fixture controls absent");
    wait_for(*application, SourceListed{*objects}, "operation-order source listing timed out");
    const std::shared_ptr<WorkerGate> gate = std::make_shared<WorkerGate>();
    ReleaseGate release(gate);
    Probe::post_operation(*application, GateWork{gate});
    wait_for(*application, GateStarted{*gate}, "operation-order gate did not start");
    const bool first = (*create).execute("fixture.operation-order.first-create");
    Probe::request_undo(*application);
    const bool second = (*create).execute("fixture.operation-order.second-create");
    require(first && second && Probe::pending_operations(*application) == 3U,
        "Create, Undo and Create must share one FIFO operation queue");
    std::atomic_bool observed_stop{};
    // jthread joins before the borrowed observation and fixture can be destroyed.
    // The helper releases the queue only after stop has revoked admission.
    std::jthread release_thread(ReleaseOnStop{application, gate, observed_stop});
    (*application).stop();
    release_thread.join();
    const fs::path created = fixture.root() / "New folder";
    const fs::path duplicate = fixture.root() / "New folder 2";
    require(observed_stop.load() && !(*gate).expired() && fs::is_directory(created) && !fs::exists(duplicate),
        "shutdown must drain admitted mutations and Undo in order before returning");
    Probe::request_undo(*application);
    (*application).drain_ui();
    require(Probe::pending_operations(*application) == 0U && fs::is_directory(created),
        "post-stop Undo must neither retain a job nor mutate the completed result");
    fixture.cleanup();
}

void test_operation_completion_preserves_pending_navigation() {
    Fixture fixture{};
    fixture.create();
    const std::shared_ptr<file_manager::Application> application =
        std::make_shared<file_manager::Application>(fixture.root(), fixture.quarantine(), true, "");
    const std::unique_ptr<gui_forms::Window> window = (*application).make_window();
    StopApplication stop(*application);
    (*application).bind_host(noop, noop);
    const std::shared_ptr<gui_forms::ObjectView> objects = std::dynamic_pointer_cast<gui_forms::ObjectView>(
        (*window).find("fm.objects.current-folder"));
    const std::shared_ptr<gui_forms::TextBox> rename = std::dynamic_pointer_cast<gui_forms::TextBox>(
        (*window).find("fm.operations.rename"));
    const std::shared_ptr<gui_forms::Command> create = Probe::command(*application, "file.new-folder");
    const std::shared_ptr<gui_forms::Command> undo = Probe::command(*application, "edit.undo");
    require(objects && rename && create && undo, "pending-navigation fixture controls absent");
    wait_for(*application, SourceListed{*objects}, "pending-navigation source listing timed out");
    const bool created = (*create).execute("fixture.pending-navigation.create");
    require(created, "pending-navigation fixture creation refused");
    struct NamingReady final {
        const gui_forms::TextBox& rename;
        const gui_forms::Command& undo;
        bool operator()() const {
            const bool ready = rename.visible() && undo.state().enabled;
            return ready;
        }
    };
    wait_for(*application, NamingReady{*rename, *undo}, "created folder naming did not become ready");
    const bool naming_cancelled = (*window).dispatch_key(
        {gui_forms::KeyAction::down, gui_forms::PhysicalKey::escape});
    require(naming_cancelled, "default folder naming must cancel before navigating");
    const std::shared_ptr<WorkerGate> gate = std::make_shared<WorkerGate>();
    ReleaseGate release(gate);
    Probe::post_worker(*application, GateWork{gate});
    wait_for(*application, GateStarted{*gate}, "pending-navigation read gate did not start");
    Probe::navigate(*application, fixture.destination());
    const bool undone = (*undo).execute("fixture.pending-navigation.undo");
    require(undone, "Undo must execute while navigation is held");
    const std::shared_ptr<OperationCompleted> completion = std::make_shared<OperationCompleted>();
    Probe::post_operation(*application, std::bind_front(&OperationCompleted::mark, completion));
    wait_for(*application, std::bind_front(&OperationCompleted::ready, completion),
        "Undo must complete independently of the held navigation");
    (*application).drain_ui();
    const fs::path removed = fixture.root() / "New folder";
    require(!fs::exists(removed) && Probe::location(*application) == fixture.root(),
        "Undo must commit while the old location is still displayed");
    (*gate).release();
    struct RequestedLocationReady final {
        const file_manager::Application& application;
        const fs::path& requested;
        bool operator()() const {
            const bool ready = Probe::location(application) == requested;
            return ready;
        }
    };
    wait_for(*application, RequestedLocationReady{*application, fixture.destination()},
        "operation completion must preserve the pending user navigation destination");
    require(!(*gate).expired(), "navigation preservation must not depend on gate timeout");
    const std::shared_ptr<gui_forms::Command> back = Probe::command(*application, "go.back");
    const std::shared_ptr<gui_forms::Command> forward = Probe::command(*application, "go.forward");
    require(back && forward && (*back).state().enabled && !(*forward).state().enabled,
        "operation refresh must retain the destination's history entry");
    const bool went_back = (*back).execute("fixture.pending-navigation.back");
    require(went_back, "Back must return from the operation-refreshed destination");
    wait_for(*application, RequestedLocationReady{*application, fixture.root()},
        "Back lost the preceding location after operation refresh");
    require(!(*back).state().enabled && (*forward).state().enabled,
        "operation refresh must not duplicate the destination in history");
    const bool went_forward = (*forward).execute("fixture.pending-navigation.forward");
    require(went_forward, "Forward must retain the operation-refreshed destination");
    wait_for(*application, RequestedLocationReady{*application, fixture.destination()},
        "Forward lost the operation-refreshed destination");
    (*application).stop();
    fixture.cleanup();
}

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
    require(Probe::progress_setup_failure_falls_back(application),
        "mailbox and observer allocation failure must fall back before copying without escaping setup");
    require(Probe::progress_respects_terminal_state(application),
        "obsolete, cancelled and completed copies must reject late progress while active copies display it");
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
    Probe::post_operation(*application, GateWork{gate});
    wait_for(*application, GateStarted{*gate}, "worker gate did not start");
    const bool pasted = (*paste).execute("fixture.transfer.paste");
    require(pasted && Probe::active(*application), "Paste must remain active while queued");
    const std::uint64_t generation = Probe::generation(*application);
    const fs::path destination = fixture.destination() / "source.txt";
    Probe::navigate(*application, fixture.root());
    wait_for(*application, SourceListed{*objects},
        "navigation must complete while the operation queue remains blocked");
    const bool preview_selected = (*window).perform_semantic_action(source_id, gui_forms::SemanticAction::select);
    require(preview_selected, "source must remain selectable during a queued transfer");
    const std::shared_ptr<gui_forms::Label> preview = std::dynamic_pointer_cast<gui_forms::Label>(
        (*window).find("fm.inspector.preview.text"));
    require(preview != nullptr, "preview control must exist during a queued transfer");
    struct PreviewReady final {
        const gui_forms::Label& preview;
        bool operator()() const {
            const bool ready = preview.visible() && preview.text() == "copy fixture\n";
            return ready;
        }
    };
    wait_for(*application, PreviewReady{*preview},
        "preview must complete while the operation queue remains blocked");
    require(Probe::active(*application) && !(*gate).expired() && !fs::exists(destination),
        "read completion must not release the queued operation or fabricate its terminal result");
    Probe::navigate(*application, fixture.destination());
    struct DestinationReady final {
        const file_manager::Application& application;
        const fs::path& path;
        bool operator()() const {
            const bool ready = Probe::location(application) == path;
            return ready;
        }
    };
    wait_for(*application, DestinationReady{*application, fixture.destination()},
        "return navigation must complete before cancelling the queued transfer");
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

enum class NewFolderCase {
    commit_name, cancel_name, navigate_queued, navigate_after_create,
    edit_location_after_create, settings_queued
};

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
    if (scenario == NewFolderCase::navigate_queued || scenario == NewFolderCase::settings_queued) {
        Probe::post_operation(*application, GateWork{gate});
        wait_for(*application, GateStarted{*gate}, "new-folder gate did not start");
    }
    const bool hold_refresh = scenario == NewFolderCase::navigate_after_create ||
        scenario == NewFolderCase::edit_location_after_create;
    if (hold_refresh) {
        // Hold reads before mutation starts. Creation can proceed independently;
        // its authoritative refreshed listing must remain behind this barrier.
        Probe::post_worker(*application, GateWork{gate});
        wait_for(*application, GateStarted{*gate}, "refresh gate did not start");
    }
    const bool requested = (*create).execute("fixture.new-folder");
    require(requested, "New folder command refused");
    const fs::path created = fixture.root() / "New folder 2";
    if (hold_refresh) {
        wait_for(*application, DirectoryCreated{created}, "creation must complete while reads are held");
        (*application).drain_ui();
        require(!(*rename).visible(),
            "created folder must await the authoritative refreshed listing");
    }
    if (scenario == NewFolderCase::settings_queued) {
        const std::shared_ptr<gui_forms::Command> settings = Probe::command(*application, "file.settings");
        const gui_forms::Control::Ptr settings_surface = (*window).find("file-manager-app.shell.settings");
        require(settings && settings_surface, "settings completion fixture controls absent");
        const bool opened = (*settings).execute("fixture.new-folder.settings");
        require(opened && (*settings_surface).visible(), "Settings must open while creation is queued");
        const std::shared_ptr<OperationCompleted> completion = std::make_shared<OperationCompleted>();
        Probe::post_operation(*application, std::bind_front(&OperationCompleted::mark, completion));
        (*gate).release();
        wait_for(*application, std::bind_front(&OperationCompleted::ready, completion),
            "creation must finish while Settings is open");
        (*application).drain_ui();
        require(fs::is_directory(created) && (*settings_surface).visible() && !(*rename).visible(),
            "creation must preserve the Settings surface without opening a name editor");
        const bool returned = (*settings).execute("fixture.new-folder.files");
        require(returned && !(*settings_surface).visible(), "Back to files must close Settings");
        wait_for(*application, ObjectNamed{*objects, "New folder 2"},
            "Back to files must refresh a creation completed while Settings was open");
        require(!(*rename).visible() && !(*gate).expired(),
            "deferred refresh must not reopen the abandoned name editor");
    } else if (navigating) {
        Probe::navigate(*application, fixture.destination());
        (*gate).release();
        wait_for(*application, LocationReady{*application, fixture.destination()}, "newer navigation was lost");
        wait_for(*application, DirectoryCreated{created}, "queued creation must complete after its operation gate releases");
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
        require(Probe::progress_coalesces(), "copy progress must retain one notification and the latest snapshot");
        test_operation_order_and_shutdown();
        test_operation_completion_preserves_pending_navigation();
        test_transfer_cancellation(false);
        test_transfer_cancellation(true);
        test_new_folder_naming(NewFolderCase::commit_name);
        test_new_folder_naming(NewFolderCase::cancel_name);
        test_new_folder_naming(NewFolderCase::navigate_queued);
        test_new_folder_naming(NewFolderCase::navigate_after_create);
        test_new_folder_naming(NewFolderCase::edit_location_after_create);
        test_new_folder_naming(NewFolderCase::settings_queued);
        std::cout << "Application transfer and new-folder tests passed\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
