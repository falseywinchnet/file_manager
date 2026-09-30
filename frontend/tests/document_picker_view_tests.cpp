#include "file_manager/document_picker_view.hpp"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string_view>
#include <unistd.h>

namespace {

void require(const bool condition, const std::string_view message) {
    if (!condition) {
        std::cerr << "document picker view test failed: " << message << '\n';
        std::exit(1);
    }
}

} // namespace

int main() {
    const auto root = std::filesystem::temp_directory_path() /
        ("file-manager-picker-view-" + std::to_string(::getpid()));
    std::filesystem::create_directories(root / "Folder");
    std::ofstream(root / "open.txt") << "open";

    file_manager::DocumentPickerRequest request;
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

    const auto picker_title = window.find("file-manager.picker.title");
    require(picker_title && picker_title->authored_surface_material() &&
                picker_title->committed_arranged_bounds().height == 40.0 &&
                window.find("file-manager.picker.title.name") &&
                window.find("file-manager.picker.navigation") &&
                window.find("file-manager.picker.path") &&
                window.find("file-manager.picker.objects") &&
                window.find("file-manager.picker.accept") &&
                window.find("file-manager.picker.cancel"),
            "installed view must compose the bounded File Manager DNA control set");
    require(view.controller().browser().entries.size() == 2U,
            "view must expose direct-filesystem folder and filtered file rows");

    const auto selected = std::find_if(
        view.controller().browser().entries.begin(),
        view.controller().browser().entries.end(),
        [](const file_manager::DirectoryEntry& entry) {
            return entry.name == "open.txt";
        });
    require(selected != view.controller().browser().entries.end() &&
                view.controller().set_selection({selected->stable_id}),
            "view fixture must select an admitted file");
    file_manager::DocumentPickerResult result;
    bool completed{};
    auto subscription = view.completed().subscribe(
        [&result, &completed](const file_manager::DocumentPickerResult& value) {
            result = value;
            completed = true;
        });
    const auto accept = std::dynamic_pointer_cast<gui_forms::Button>(
        window.find("file-manager.picker.accept"));
    require(accept && accept->perform_click() && completed && result.accepted() &&
                result.selections.front().path.filename() == "open.txt",
            "GUI.Forms Accept command must emit the revalidated selection result");

    view.set_orchestrator_session_valid(false);
    completed = false;
    require(!accept->enabled() && !accept->perform_click() && !completed,
            "lost Orchestrator session must disable acceptance while browsing remains");

    view.present(root);
    require(!accept->enabled(), "reload must retain authority loss");
    view.set_orchestrator_session_valid(true);
    const auto objects = std::dynamic_pointer_cast<gui_forms::ObjectView>(window.find("file-manager.picker.objects"));
    const auto file_for_reload = std::find_if(view.controller().browser().entries.begin(), view.controller().browser().entries.end(),
        [](const auto& entry) { return entry.name == "open.txt"; });
    objects->set_selected_ids({file_for_reload->stable_id});
    const auto hidden = std::dynamic_pointer_cast<gui_forms::CheckBox>(window.find("file-manager.picker.hidden"));
    hidden->set_checked(true);
    require(objects->selected_ids().empty() && view.controller().selected_ids().empty(),
        "refresh clears visible and semantic selection together");
    const auto folder = std::find_if(view.controller().browser().entries.begin(), view.controller().browser().entries.end(),
        [](const auto& entry) { return entry.directory; });
    const bool folder_selected = view.controller().set_selection({folder->stable_id});
    const bool folder_clicked = accept->perform_click();
    if (!folder_selected || !folder_clicked || view.controller().browser().location.filename() != "Folder")
        std::cerr << "folder selected=" << folder_selected << " clicked=" << folder_clicked << " location=" << view.controller().browser().location << " error=" << view.controller().last_error() << '\n';
    require(folder_selected && folder_clicked && view.controller().browser().location.filename() == "Folder", "Open button enters selected folder");
    window.dispatch_key({gui_forms::KeyAction::down, gui_forms::PhysicalKey::l, gui_forms::Modifier::control});
    require(window.focused_control() == window.find("file-manager.picker.path"), "Ctrl+L focuses location");
    window.dispatch_key({gui_forms::KeyAction::down, gui_forms::PhysicalKey::up, gui_forms::Modifier::alt});
    require(view.controller().browser().location == std::filesystem::canonical(root), "Alt+Up enters parent");
    window.request_focus(objects);
    completed = false;
    window.dispatch_key({gui_forms::KeyAction::down, gui_forms::PhysicalKey::escape});
    require(completed && result.terminal == file_manager::DocumentPickerTerminal::cancelled, "Escape cancels picker");
    completed = false;
    view.cancel();
    require(!completed, "terminal completion is emitted once per presentation");
    file_manager::DocumentPickerRequest save_request;
    save_request.protected_root = root;
    save_request.owner_application_id = "save-host";
    save_request.authority = file_manager::DocumentPickerAuthority::trusted_local_host;
    save_request.profile = file_manager::DocumentPickerProfile::save_as;
    file_manager::DocumentPickerView save(save_request);
    gui_forms::Window save_window(save.root_control(), {580, 420});
    save_window.perform_layout();
    save.attach_dialog(save_window);
    const auto name = save_window.find("file-manager.picker.filename");
    require(save_window.focused_control() == name && name->committed_arranged_bounds().height == 32,
        "compact save layout retains usable filename height and initial focus");
    const auto save_objects = std::dynamic_pointer_cast<gui_forms::ObjectView>(save_window.find("file-manager.picker.objects"));
    const auto save_file = std::find_if(save.controller().browser().entries.begin(), save.controller().browser().entries.end(),
        [](const auto& entry) { return entry.name == "open.txt"; });
    save_objects->set_selected_ids({save_file->stable_id});
    require(std::dynamic_pointer_cast<gui_forms::TextBox>(name)->text() == "open.txt", "save selection updates visible filename");
    std::error_code ignored;
    std::filesystem::remove_all(root, ignored);
    std::cout << "document picker view tests passed\n";
    return 0;
}
