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
    request.owner_application_id = "picker-view-test";
    request.filters = {{"text", "Text files", {"txt"}}};
    request.active_filter_id = "text";
    file_manager::DocumentPickerView view(std::move(request));
    gui_forms::Window window(view.root_control(), {760, 520});
    window.perform_layout();

    require(window.find("file-manager.picker.path") &&
                window.find("file-manager.picker.objects") &&
                window.find("file-manager.picker.accept") &&
                window.find("file-manager.picker.cancel"),
            "installed view must compose the bounded GUI.Forms control set");
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

    std::error_code ignored;
    std::filesystem::remove_all(root, ignored);
    std::cout << "document picker view tests passed\n";
    return 0;
}
