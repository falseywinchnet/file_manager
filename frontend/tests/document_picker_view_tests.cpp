#include "file_manager/document_picker_view.hpp"
#include "fixture_links.hpp"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <string_view>
#include <stdexcept>
#include <unistd.h>

namespace {

class FixtureDirectory final {
public:
    explicit FixtureDirectory(const std::filesystem::path& path) : path_(path) {
        const bool acquired = std::filesystem::create_directory(path_);
        if (!acquired) throw std::runtime_error("picker view fixture directory already exists");
    }
    FixtureDirectory(const FixtureDirectory&) = delete;
    FixtureDirectory& operator=(const FixtureDirectory&) = delete;
    ~FixtureDirectory() {
        std::error_code ignored{};
        std::filesystem::remove_all(path_, ignored);
    }
private:
    // Borrows the stable canonical-temp child through this test call; owns its
    // newly created directory. The path outlives this guard and is never changed.
    const std::filesystem::path& path_;
};

struct ResultRecorder final {
    file_manager::DocumentPickerResult result{};
    bool completed{};
    void receive(const file_manager::DocumentPickerResult& value) {
        result = value;
        completed = true;
    }
};

bool is_open_file(const file_manager::DirectoryEntry& entry) {
    const bool matches = entry.name == "open.txt";
    return matches;
}

bool is_directory(const file_manager::DirectoryEntry& entry) {
    return entry.directory;
}

void require(const bool condition, const std::string_view message) {
    if (!condition) {
        const std::string diagnostic(message);
        throw std::runtime_error(diagnostic);
    }
}

// The window retains controls after the view has revoked its borrowed delegates.
// This checks the boundary rather than merely counting named callback methods.
void verify_disconnected_controls(const file_manager::DocumentPickerRequest& request) {
    std::unique_ptr<file_manager::DocumentPickerView> picker =
        std::make_unique<file_manager::DocumentPickerView>(request);
    gui_forms::Window window((*picker).root_control(), {760, 560});
    window.perform_layout();
    const gui_forms::Control::Ptr control = window.find("file-manager.picker.cancel");
    const std::shared_ptr<gui_forms::Button> cancel =
        std::dynamic_pointer_cast<gui_forms::Button>(control);
    require(cancel != nullptr, "teardown fixture requires Cancel");
    ResultRecorder recorder{};
    const gui_forms::Delegate<const file_manager::DocumentPickerResult&> callback =
        gui_forms::Delegate<const file_manager::DocumentPickerResult&>::bind<
            ResultRecorder, &ResultRecorder::receive>(recorder);
    gui_forms::SubscriptionToken connection = (*picker).completed().subscribe(callback);
    picker.reset();
    require(!connection.connected(), "view destruction revokes completion observers");
    const gui_forms::EventStatistics before = (*cancel).clicked().statistics();
    const bool clicked = (*cancel).perform_click();
    const gui_forms::EventStatistics after = (*cancel).clicked().statistics();
    require(clicked && !recorder.completed && before.callbacks_emitted == after.callbacks_emitted,
            "retained controls cannot call a destroyed picker view");
}

struct DisposingHost final {
    std::unique_ptr<file_manager::DocumentPickerView> picker{};
    unsigned int completion_count{};
    void completed(const file_manager::DocumentPickerResult&) {
        ++completion_count;
        picker.reset();
    }
};

void verify_teardown_during_completion(const file_manager::DocumentPickerRequest& request) {
    DisposingHost host{};
    host.picker = std::make_unique<file_manager::DocumentPickerView>(request);
    gui_forms::Window window((*host.picker).root_control(), {760, 560});
    window.perform_layout();
    const gui_forms::Control::Ptr control = window.find("file-manager.picker.cancel");
    const std::shared_ptr<gui_forms::Button> cancel =
        std::dynamic_pointer_cast<gui_forms::Button>(control);
    require(cancel != nullptr, "completion teardown fixture requires Cancel");
    const gui_forms::Delegate<const file_manager::DocumentPickerResult&> callback =
        gui_forms::Delegate<const file_manager::DocumentPickerResult&>::bind<
            DisposingHost, &DisposingHost::completed>(host);
    gui_forms::SubscriptionToken connection = (*host.picker).completed().subscribe(callback);
    const bool first_click = (*cancel).perform_click();
    require(first_click && host.picker == nullptr && !connection.connected(),
            "completion may tear down view while its controls remain retained");
    const bool second_click = (*cancel).perform_click();
    require(second_click && host.completion_count == 1U,
            "teardown during emission revokes future callbacks");
}

struct CancelDuringLocationUpdate final {
    DisposingHost& host;
    bool survived_update{};
    void changed(const std::string&) {
        if (host.picker == nullptr)
            return;
        (*host.picker).cancel();
        survived_update = host.picker != nullptr && host.completion_count == 0U;
    }
};

void verify_cancel_during_reload(const file_manager::DocumentPickerRequest& request) {
    DisposingHost host{};
    host.picker = std::make_unique<file_manager::DocumentPickerView>(request);
    gui_forms::Window window((*host.picker).root_control(), {760, 560});
    window.perform_layout();
    const gui_forms::Delegate<const file_manager::DocumentPickerResult&> completion =
        gui_forms::Delegate<const file_manager::DocumentPickerResult&>::bind<
            DisposingHost, &DisposingHost::completed>(host);
    gui_forms::SubscriptionToken completion_connection =
        (*host.picker).completed().subscribe(completion);
    CancelDuringLocationUpdate listener{.host = host};
    const gui_forms::Delegate<const std::string&> callback =
        gui_forms::Delegate<const std::string&>::bind<CancelDuringLocationUpdate,
                                                      &CancelDuringLocationUpdate::changed>(
            listener);
    const gui_forms::Control::Ptr control = window.find("file-manager.picker.path");
    const std::shared_ptr<gui_forms::TextBox> path =
        std::dynamic_pointer_cast<gui_forms::TextBox>(control);
    require(path != nullptr, "reload fixture requires path control");
    gui_forms::SubscriptionToken change_connection = (*path).text_changed().subscribe(callback);
    const std::filesystem::path child = request.protected_root / "Folder";
    (*host.picker).present(child);
    require(listener.survived_update && host.picker == nullptr && host.completion_count == 1U,
            "cancel during reload emits exactly once after the outer update releases its borrows");
}

struct NestedLocationUpdate final {
    file_manager::DocumentPickerView& view;
    const std::filesystem::path& destination;
    bool nested{};
    void changed(const std::string&) {
        if (nested)
            return;
        nested = true;
        view.present(destination);
    }
};

void verify_nested_reload(const file_manager::DocumentPickerRequest& request) {
    file_manager::DocumentPickerView view(request);
    gui_forms::Window window(view.root_control(), {760, 560});
    window.perform_layout();
    NestedLocationUpdate listener{.view = view, .destination = request.protected_root};
    const gui_forms::Delegate<const std::string&> callback =
        gui_forms::Delegate<const std::string&>::bind<NestedLocationUpdate,
                                                      &NestedLocationUpdate::changed>(listener);
    const gui_forms::Control::Ptr path_control = window.find("file-manager.picker.path");
    const std::shared_ptr<gui_forms::TextBox> path =
        std::dynamic_pointer_cast<gui_forms::TextBox>(path_control);
    require(path != nullptr, "nested fixture requires path control");
    gui_forms::SubscriptionToken connection = (*path).text_changed().subscribe(callback);
    const std::filesystem::path child = request.protected_root / "Folder";
    view.present(child);
    const std::filesystem::path canonical_root = std::filesystem::canonical(request.protected_root);
    require(listener.nested && view.controller().browser().location == canonical_root,
            "nested reload preserves the last explicitly requested location");
    const gui_forms::Control::Ptr hidden_control = window.find("file-manager.picker.hidden");
    const std::shared_ptr<gui_forms::CheckBox> hidden =
        std::dynamic_pointer_cast<gui_forms::CheckBox>(hidden_control);
    require(hidden != nullptr, "nested fixture requires hidden control");
    (*hidden).set_checked(true);
    require(view.controller().show_hidden(), "nested reload restores the outer update flag");
}

struct ThrowingLocationUpdate final {
    file_manager::DocumentPickerView* cancel_before_throw{};
    bool threw{};
    void changed(const std::string&) {
        if (threw)
            return;
        threw = true;
        if (cancel_before_throw != nullptr)
            (*cancel_before_throw).cancel();
        throw std::runtime_error("injected picker subscriber failure");
    }
};

void verify_exception_reset(const file_manager::DocumentPickerRequest& request,
                            const bool cancel_first) {
    file_manager::DocumentPickerView view(request);
    gui_forms::Window window(view.root_control(), {760, 560});
    window.perform_layout();
    ResultRecorder recorder{};
    const gui_forms::Delegate<const file_manager::DocumentPickerResult&> completion =
        gui_forms::Delegate<const file_manager::DocumentPickerResult&>::bind<
            ResultRecorder, &ResultRecorder::receive>(recorder);
    gui_forms::SubscriptionToken completion_connection = view.completed().subscribe(completion);
    ThrowingLocationUpdate listener{};
    if (cancel_first)
        listener.cancel_before_throw = &view;
    const gui_forms::Delegate<const std::string&> callback =
        gui_forms::Delegate<const std::string&>::bind<ThrowingLocationUpdate,
                                                      &ThrowingLocationUpdate::changed>(listener);
    const gui_forms::Control::Ptr path_control = window.find("file-manager.picker.path");
    const std::shared_ptr<gui_forms::TextBox> path =
        std::dynamic_pointer_cast<gui_forms::TextBox>(path_control);
    require(path != nullptr, "exception fixture requires path control");
    gui_forms::SubscriptionToken change_connection = (*path).text_changed().subscribe(callback);
    bool caught = false;
    try {
        const std::filesystem::path child = request.protected_root / "Folder";
        view.present(child);
    } catch (const std::runtime_error&) {
        caught = true;
    }
    require(caught && listener.threw && !recorder.completed,
            "exception unwinding restores flags without emitting a terminal result");
    const gui_forms::Control::Ptr hidden_control = window.find("file-manager.picker.hidden");
    const std::shared_ptr<gui_forms::CheckBox> hidden =
        std::dynamic_pointer_cast<gui_forms::CheckBox>(hidden_control);
    require(hidden != nullptr, "exception fixture requires hidden control");
    (*hidden).set_checked(true);
    require(view.controller().show_hidden(), "control callbacks still run after failed reload");
    view.cancel();
    require(recorder.completed &&
                recorder.result.terminal == file_manager::DocumentPickerTerminal::cancelled,
            "host can cancel normally after failed reload");
}

} // namespace

int run_tests() {
    const std::filesystem::path parent =
        std::filesystem::canonical(std::filesystem::temp_directory_path());
    const std::string name_prefix = "file-manager-picker-view-" + std::to_string(::getpid());
    const std::filesystem::path root = parent / name_prefix;
    const FixtureDirectory fixture(root);
    std::filesystem::create_directories(root / "Folder");
    {
        std::ofstream stream(root / "open.txt");
        stream << "open";
        require(static_cast<bool>(stream), "fixture write must succeed");
    }

    file_manager::DocumentPickerRequest request{};
    request.profile = file_manager::DocumentPickerProfile::open_file;
    request.protected_root = root;
    request.initial_location = root;
    request.orchestrator_session_valid = true; // Explicit fixture session.
    request.owner_application_id = "picker-view-test";
    request.filters = {{"text", "Text files", {"txt"}}};
    request.active_filter_id = "text";
    file_manager::DocumentPickerView view(std::move(request));
    gui_forms::Window window(view.root_control(), {760, 560});
    window.perform_layout();
    view.attach_dialog(window);

    const gui_forms::Control::Ptr picker_title = window.find("file-manager.picker.title");
    require(
        picker_title && (*picker_title).authored_surface_material() &&
            (*picker_title).committed_arranged_bounds().height == 40.0 &&
            window.find("file-manager.picker.title.name") &&
            window.find("file-manager.picker.navigation") &&
            window.find("file-manager.picker.path") && window.find("file-manager.picker.objects") &&
            window.find("file-manager.picker.accept") && window.find("file-manager.picker.cancel"),
        "installed view must compose the bounded File Manager DNA control set");
    require(view.controller().browser().entries.size() == 2U,
            "view must expose direct-filesystem folder and filtered file rows");

    const std::vector<file_manager::DirectoryEntry>::const_iterator selected =
        std::find_if(view.controller().browser().entries.begin(),
                     view.controller().browser().entries.end(), is_open_file);
    require(selected != view.controller().browser().entries.end(), "fixture open.txt must exist");
    const bool selected_file = view.controller().set_selection({(*selected).stable_id});
    require(selected_file, "view fixture must select an admitted file");
    ResultRecorder recorder{};
    file_manager::DocumentPickerResult& result = recorder.result;
    bool& completed = recorder.completed;
    const gui_forms::Delegate<const file_manager::DocumentPickerResult&> completion =
        gui_forms::Delegate<const file_manager::DocumentPickerResult&>::bind<
            ResultRecorder, &ResultRecorder::receive>(recorder);
    gui_forms::SubscriptionToken subscription = view.completed().subscribe(completion);
    const std::shared_ptr<gui_forms::Button> accept =
        std::dynamic_pointer_cast<gui_forms::Button>(window.find("file-manager.picker.accept"));
    require(accept != nullptr, "fixture Accept button must exist");
    const bool accepted_click = (*accept).perform_click();
    require(accept && accepted_click && completed && result.accepted() &&
                result.selections.front().path.filename() == "open.txt",
            "GUI.Forms Accept command must emit the revalidated selection result");

    view.set_orchestrator_session_valid(false);
    completed = false;
    const bool unavailable_click = (*accept).perform_click();
    require(!(*accept).enabled() && !unavailable_click && !completed,
            "lost Orchestrator session must disable acceptance while browsing remains");

    view.present(root);
    require(!(*accept).enabled(), "reload must retain authority loss");
    view.set_orchestrator_session_valid(true);
    const std::shared_ptr<gui_forms::ObjectView> objects =
        std::dynamic_pointer_cast<gui_forms::ObjectView>(
            window.find("file-manager.picker.objects"));
    const std::vector<file_manager::DirectoryEntry>::const_iterator file_for_reload =
        std::find_if(view.controller().browser().entries.begin(),
                     view.controller().browser().entries.end(), is_open_file);
    (*objects).set_selected_ids({(*file_for_reload).stable_id});
    const std::shared_ptr<gui_forms::CheckBox> hidden =
        std::dynamic_pointer_cast<gui_forms::CheckBox>(window.find("file-manager.picker.hidden"));
    (*hidden).set_checked(true);
    require((*objects).selected_ids().empty() && view.controller().selected_ids().empty(),
            "refresh clears visible and semantic selection together");
    const std::vector<file_manager::DirectoryEntry>::const_iterator folder =
        std::find_if(view.controller().browser().entries.begin(),
                     view.controller().browser().entries.end(), is_directory);
    const bool folder_selected = view.controller().set_selection({(*folder).stable_id});
    const bool folder_clicked = (*accept).perform_click();
    if (!folder_selected || !folder_clicked ||
        view.controller().browser().location.filename() != "Folder")
        std::cerr << "folder selected=" << folder_selected << " clicked=" << folder_clicked
                  << " location=" << view.controller().browser().location
                  << " error=" << view.controller().last_error() << '\n';
    require(folder_selected && folder_clicked &&
                view.controller().browser().location.filename() == "Folder",
            "Open button enters selected folder");
    window.dispatch_key(
        {gui_forms::KeyAction::down, gui_forms::PhysicalKey::l, gui_forms::Modifier::control});
    require(window.focused_control() == window.find("file-manager.picker.path"),
            "Ctrl+L focuses location");
    window.dispatch_key(
        {gui_forms::KeyAction::down, gui_forms::PhysicalKey::up, gui_forms::Modifier::alt});
    require(view.controller().browser().location == std::filesystem::canonical(root),
            "Alt+Up enters parent");
    window.request_focus(objects);
    completed = false;
    window.dispatch_key({gui_forms::KeyAction::down, gui_forms::PhysicalKey::escape});
    require(completed && result.terminal == file_manager::DocumentPickerTerminal::cancelled,
            "Escape cancels picker");
    completed = false;
    view.cancel();
    require(!completed, "terminal completion is emitted once per presentation");
    file_manager::DocumentPickerRequest save_request{};
    save_request.protected_root = root;
    save_request.owner_application_id = "save-host";
    save_request.authority = file_manager::DocumentPickerAuthority::trusted_local_host;
    save_request.profile = file_manager::DocumentPickerProfile::save_as;
    file_manager::DocumentPickerView save(save_request);
    gui_forms::Window save_window(save.root_control(), {580, 420});
    save_window.perform_layout();
    save.attach_dialog(save_window);
    const gui_forms::Control::Ptr name = save_window.find("file-manager.picker.filename");
    require(save_window.focused_control() == name &&
                (*name).committed_arranged_bounds().height == 32,
            "compact save layout retains usable filename height and initial focus");
    const std::shared_ptr<gui_forms::ObjectView> save_objects =
        std::dynamic_pointer_cast<gui_forms::ObjectView>(
            save_window.find("file-manager.picker.objects"));
    const std::vector<file_manager::DirectoryEntry>::const_iterator save_file =
        std::find_if(save.controller().browser().entries.begin(),
                     save.controller().browser().entries.end(), is_open_file);
    (*save_objects).set_selected_ids({(*save_file).stable_id});
    const std::shared_ptr<gui_forms::TextBox> filename_box =
        std::dynamic_pointer_cast<gui_forms::TextBox>(name);
    require((*filename_box).text() == "open.txt", "save selection updates visible filename");
    verify_cancel_during_reload(save_request);
    verify_nested_reload(save_request);
    verify_exception_reset(save_request, false);
    verify_exception_reset(save_request, true);
    verify_disconnected_controls(save_request);
    verify_teardown_during_completion(save_request);
    const bool file_link_available = create_fixture_link(root / "open.txt", root / "file-link.txt");
    if (file_link_available) {
        file_manager::DocumentPickerRequest file_request{};
        file_request.protected_root = root;
        file_request.owner_application_id = "file-link-view-test";
        file_request.authority = file_manager::DocumentPickerAuthority::trusted_local_host;
        ResultRecorder file_recorder{};
        file_manager::DocumentPickerView file_view(file_request);
        gui_forms::Window file_window(file_view.root_control(), {760, 560});
        file_window.perform_layout();
        file_view.attach_dialog(file_window);
        const gui_forms::Delegate<const file_manager::DocumentPickerResult&> file_callback =
            gui_forms::Delegate<const file_manager::DocumentPickerResult&>::bind<
                ResultRecorder, &ResultRecorder::receive>(file_recorder);
        gui_forms::SubscriptionToken file_connection = file_view.completed().subscribe(file_callback);
        const std::shared_ptr<gui_forms::ObjectView> file_objects =
            std::dynamic_pointer_cast<gui_forms::ObjectView>(file_window.find("file-manager.picker.objects"));
        std::string file_id{};
        for (const file_manager::DirectoryEntry& entry : file_view.controller().browser().entries) {
            if (entry.name == "file-link.txt") file_id = entry.stable_id;
        }
        require(!file_id.empty(), "file alias row is visible");
        (*file_objects).set_selected_ids({file_id});
        const std::shared_ptr<gui_forms::Button> file_open =
            std::dynamic_pointer_cast<gui_forms::Button>(file_window.find("file-manager.picker.accept"));
        const bool file_clicked = (*file_open).perform_click();
        const file_manager::ObjectIdentity target_identity = file_manager::observe_identity(root / "open.txt");
        require(file_clicked && file_recorder.completed && file_recorder.result.accepted() &&
                    file_recorder.result.selections.front().path == root / "open.txt" &&
                    file_recorder.result.selections.front().identity.same_revision(target_identity),
                "Open button accepts file alias as canonical target with matching identity");
    }
    const bool link_available = create_fixture_link(root / "Folder", root / "folder-link", true);
    if (link_available) {
        file_manager::DocumentPickerRequest link_request{};
        link_request.protected_root = root;
        link_request.owner_application_id = "link-view-test";
        link_request.authority = file_manager::DocumentPickerAuthority::trusted_local_host;
        link_request.filters = {{"text", "Text files", {"txt"}}};
        file_manager::DocumentPickerView linked(link_request);
        gui_forms::Window linked_window(linked.root_control(), {760, 560});
        linked_window.perform_layout();
        linked.attach_dialog(linked_window);
        const std::shared_ptr<gui_forms::ObjectView> linked_objects =
            std::dynamic_pointer_cast<gui_forms::ObjectView>(linked_window.find("file-manager.picker.objects"));
        std::string link_id{};
        for (const file_manager::DirectoryEntry& entry : linked.controller().browser().entries) {
            if (entry.name == "folder-link") link_id = entry.stable_id;
        }
        require(!link_id.empty(), "folder link stays visible through file filter");
        (*linked_objects).set_selected_ids({link_id});
        const bool link_selected = linked.controller().selected_ids().size() == 1;
        const std::shared_ptr<gui_forms::Button> linked_open =
            std::dynamic_pointer_cast<gui_forms::Button>(linked_window.find("file-manager.picker.accept"));
        const bool link_clicked = (*linked_open).perform_click();
        if (!link_selected || !link_clicked || linked.controller().browser().location != root / "Folder") {
            const std::filesystem::path canonical_alias = std::filesystem::canonical(root / "folder-link");
            const std::filesystem::path canonical_target = std::filesystem::canonical(root / "Folder");
            std::cerr << "link view navigation: selected=" << link_selected << " clicked=" << link_clicked
                      << " actual=" << linked.controller().browser().location << " expected=" << root / "Folder"
                      << " canonical_alias=" << canonical_alias << " canonical_target=" << canonical_target
                      << " root=" << linked.controller().browser().root
                      << " error=" << linked.controller().last_error() << '\n';
        }
        require(link_selected && link_clicked && linked.controller().browser().location == root / "Folder",
                "picker Open enters selected folder link through canonical navigation");
    }
    std::cout << "document picker view tests passed\n";
    return 0;
}

int main() {
    try {
        const int status = run_tests();
        return status;
    } catch (const std::exception& error) {
        std::cerr << "document picker view test failed: " << error.what() << '\n';
        return 1;
    }
}
