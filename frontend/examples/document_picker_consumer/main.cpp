#include <file_manager/document_picker_view.hpp>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>

namespace {

class FixtureDirectory final {
  public:
    explicit FixtureDirectory(const std::filesystem::path& path) : path_(path) {
        std::filesystem::create_directories(path_);
    }
    ~FixtureDirectory() {
        std::error_code error{};
        std::filesystem::remove_all(path_, error);
    }
    FixtureDirectory(const FixtureDirectory&) = delete;
    FixtureDirectory& operator=(const FixtureDirectory&) = delete;

  private:
    std::filesystem::path path_{};
};

struct CompletionRecorder final {
    file_manager::DocumentPickerResult result{};
    unsigned int terminal_count{};

    void receive(const file_manager::DocumentPickerResult& value) {
        result = value;
        if (value.accepted() || value.terminal == file_manager::DocumentPickerTerminal::cancelled) {
            ++terminal_count;
        }
    }
};

} // namespace

int main() {
    const std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
    const std::chrono::steady_clock::duration elapsed = now.time_since_epoch();
    const std::chrono::steady_clock::rep stamp = elapsed.count();
    const std::string fixture_name = "picker-public-consumer-" + std::to_string(stamp);
    const std::filesystem::path root = std::filesystem::temp_directory_path() / fixture_name;
    FixtureDirectory fixture(root);
    const std::filesystem::path document = root / "document";
    {
        std::ofstream stream(document);
        stream << "independent consumer fixture";
        if (!stream)
            throw std::runtime_error("fixture write failed");
    }
    file_manager::DocumentPickerRequest request{
        .protected_root = root,
        .owner_application_id = "public-picker-consumer",
        .home_location = root,
        .authority = file_manager::DocumentPickerAuthority::trusted_local_host,
    };
    file_manager::DocumentPickerView picker(request);
    gui_forms::Window window(picker.root_control(), {760, 560});
    window.perform_layout();
    picker.attach_dialog(window);
    CompletionRecorder recorder{};
    const gui_forms::Delegate<const file_manager::DocumentPickerResult&> callback =
        gui_forms::Delegate<const file_manager::DocumentPickerResult&>::bind<
            CompletionRecorder, &CompletionRecorder::receive>(recorder);
    // Recorder outlives the token; the token disconnects before its borrowed target ends.
    gui_forms::SubscriptionToken subscription = picker.completed().subscribe(callback);
    const file_manager::DirectoryEntry& entry = picker.controller().browser().entries.at(0);
    const bool selected = picker.controller().set_selection({entry.stable_id});
    if (!selected)
        return 1;
    const gui_forms::Control::Ptr accept_control = window.find("file-manager.picker.accept");
    const std::shared_ptr<gui_forms::Button> accept =
        std::dynamic_pointer_cast<gui_forms::Button>(accept_control);
    if (accept == nullptr)
        return 2;
    const bool clicked = (*accept).perform_click();
    if (!clicked || !recorder.result.accepted() || recorder.terminal_count != 1U)
        return 2;
    const file_manager::DocumentSelectionObservation& observation =
        recorder.result.selections.at(0);
    if (observation.path.filename() != "document")
        return 2;
    // The host still checks the exact observation immediately before its own I/O.
    const file_manager::ObjectIdentity current = file_manager::observe_identity(observation.path);
    const bool unchanged = observation.identity.same_revision(current);
    if (!unchanged)
        return 3;
    picker.set_authority_valid(true);
    picker.present(root);
    window.dispatch_key({gui_forms::KeyAction::down, gui_forms::PhysicalKey::escape});
    if (recorder.result.terminal != file_manager::DocumentPickerTerminal::cancelled ||
        recorder.terminal_count != 2U)
        return 4;
    picker.cancel();
    if (recorder.terminal_count != 2U)
        return 5;
    std::cout << "Installed public picker: extensionless open, host revalidation, reopen and "
                 "keyboard cancel passed\n";
    return 0;
}
