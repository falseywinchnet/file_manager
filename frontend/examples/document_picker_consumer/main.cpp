#include <file_manager/document_picker_view.hpp>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>

int main() {
    const auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
    const auto root = std::filesystem::temp_directory_path() /
        ("picker-public-consumer-" + std::to_string(stamp));
    std::filesystem::create_directories(root);
    struct Cleanup {
        std::filesystem::path path;
        ~Cleanup() { std::error_code ec; std::filesystem::remove_all(path, ec); }
    } cleanup{root};
    std::ofstream(root / "document") << "independent consumer fixture";
    file_manager::DocumentPickerRequest request;
    request.protected_root = root;
    request.home_location = root;
    request.owner_application_id = "public-picker-consumer";
    request.authority = file_manager::DocumentPickerAuthority::trusted_local_host;
    file_manager::DocumentPickerView picker(request);
    gui_forms::Window window(picker.root_control(), {760, 560});
    window.perform_layout();
    picker.attach_dialog(window);
    file_manager::DocumentPickerResult result;
    unsigned terminals = 0;
    auto subscription = picker.completed().subscribe([&](const auto& value) {
        result = value;
        if (value.accepted() || value.terminal == file_manager::DocumentPickerTerminal::cancelled) ++terminals;
    });
    const auto& entry = picker.controller().browser().entries.at(0);
    if (!picker.controller().set_selection({entry.stable_id})) return 1;
    const auto accept = std::dynamic_pointer_cast<gui_forms::Button>(
        window.find("file-manager.picker.accept"));
    if (!accept->perform_click() || !result.accepted() || terminals != 1 ||
        result.selections.at(0).path.filename() != "document") return 2;
    // The host still checks the exact observation immediately before its own I/O.
    if (!result.selections.at(0).identity.same_revision(
        file_manager::observe_identity(result.selections.at(0).path))) return 3;
    picker.set_authority_valid(true);
    picker.present(root);
    window.dispatch_key({gui_forms::KeyAction::down, gui_forms::PhysicalKey::escape});
    if (result.terminal != file_manager::DocumentPickerTerminal::cancelled || terminals != 2) return 4;
    picker.cancel();
    if (terminals != 2) return 5;
    std::cout << "Installed public picker: extensionless open, host revalidation, reopen and keyboard cancel passed\n";
}
