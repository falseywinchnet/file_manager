#include "fixture_links.hpp"
#include "file_manager/document_picker.hpp"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string_view>
#include <unistd.h>

namespace {

class TestRoot final {
public:
    TestRoot() {
        path_ = std::filesystem::temp_directory_path() /
            ("file-manager-picker-" + std::to_string(::getpid()));
        std::filesystem::create_directories(path_ / "Folder");
        std::ofstream(path_ / "alpha.txt") << "alpha";
        std::ofstream(path_ / "beta.txt") << "beta";
        std::ofstream(path_ / "image.png") << "png";
        std::ofstream(path_ / ".hidden.txt") << "hidden";
        (void)create_fixture_link("alpha.txt", path_ / "linked.txt");
    }
    ~TestRoot() {
        std::error_code ignored;
        std::filesystem::remove_all(path_, ignored);
    }
    [[nodiscard]] const std::filesystem::path& path() const noexcept {
        return path_;
    }
private:
    std::filesystem::path path_;
};

void require(const bool condition, const std::string_view message) {
    if (!condition) {
        std::cerr << "document picker test failed: " << message << '\n';
        std::exit(1);
    }
}

file_manager::DocumentPickerRequest request(
    const TestRoot& root, const file_manager::DocumentPickerProfile profile) {
    file_manager::DocumentPickerRequest value;
    value.profile = profile;
    value.protected_root = root.path();
    value.initial_location = root.path();
    value.orchestrator_session_valid = true; // Explicit fixture session.
    value.owner_application_id = "picker-test-consumer";
    value.filters = {{"text", "Text", {"txt"}},
                     {"images", "Images", {"png"}}};
    value.active_filter_id = "text";
    return value;
}

const file_manager::DirectoryEntry* find(
    const file_manager::FileSelectionController& picker,
    const std::string_view name) {
    for (const auto& entry : picker.browser().entries) {
        if (entry.name == name) return &entry;
    }
    return nullptr;
}

} // namespace

int main() {
    TestRoot root;

    auto open_request = request(root, file_manager::DocumentPickerProfile::open_file);
    file_manager::FileSelectionController open(open_request);
    require(open.browser().available() && find(open, "alpha.txt") &&
                !find(open, "image.png") && !find(open, ".hidden.txt"),
            "open profile must browse without Engine and apply type/hidden policy");
    require(open.set_selection({find(open, "alpha.txt")->stable_id}) &&
                open.accept().accepted() &&
                open.accept().selections.front().path.filename() == "alpha.txt",
            "one ordinary visible file must revalidate and accept");
    const auto retained_location = open.browser().location;
    const auto retained_selection = open.selected_ids();
    require(!open.navigate(root.path().parent_path()) &&
                open.browser().available() &&
                open.browser().location == retained_location &&
                open.selected_ids() == retained_selection &&
                !open.last_error().empty(),
            "refused navigation must retain the last usable directory and selection");

    require(open.set_show_hidden(true) && find(open, ".hidden.txt"),
            "app-scoped hidden policy must refresh the same browser model");
    require(open.set_active_filter("images") && find(open, "image.png") &&
                !find(open, "alpha.txt") && !find(open, ".hidden.txt"),
            "type filter changes must not leak filtered hidden files");

    auto multi_request = request(
        root, file_manager::DocumentPickerProfile::open_files);
    multi_request.maximum_selection = 2;
    file_manager::FileSelectionController multi(multi_request);
    require(multi.set_selection({find(multi, "alpha.txt")->stable_id,
                                 find(multi, "beta.txt")->stable_id}) &&
                multi.accept().selections.size() == 2U,
            "bounded multi-open must accept exactly the admitted observations");
    require(!multi.set_selection({find(multi, "alpha.txt")->stable_id,
                                  find(multi, "beta.txt")->stable_id,
                                  find(multi, "Folder")->stable_id}),
            "multi-open must enforce its cardinality before acceptance");

    auto folder_request = request(
        root, file_manager::DocumentPickerProfile::select_folder);
    folder_request.filters.clear();
    folder_request.active_filter_id.clear();
    file_manager::FileSelectionController folder(folder_request);
    require(!find(folder, "alpha.txt") && find(folder, "Folder") &&
                folder.accept().accepted() &&
                folder.accept().selections.front().path ==
                    std::filesystem::canonical(root.path()),
            "folder profile must hide files and allow the current folder");
    require(folder.set_selection({find(folder, "Folder")->stable_id}) &&
                folder.accept().accepted() &&
                folder.accept().selections.front().path.filename() == "Folder",
            "folder profile must accept one selected child folder");

    auto save_request = request(root, file_manager::DocumentPickerProfile::save_as);
    save_request.suggested_name = "draft";
    save_request.default_extension = "txt";
    file_manager::FileSelectionController save(save_request);
    auto result = save.accept();
    require(result.accepted() &&
                result.selections.front().path.filename() == "draft.txt" &&
                !result.selections.front().existing,
            "save-as must correct the extension and return a non-writing destination observation");
    require(save.set_filename("alpha.txt"),
            "save profile must accept a bounded basename edit");
    result = save.accept();
    require(result.terminal ==
                file_manager::DocumentPickerTerminal::overwrite_confirmation_required &&
                result.selections.front().existing,
            "existing save destination must require explicit overwrite confirmation");
    require(save.accept(true).accepted(),
            "confirmed overwrite must return the observed existing destination");
    require(save.set_filename("../escape") &&
                save.accept().code == "invalid-filename",
            "save acceptance must reject path-bearing filenames");

    auto export_request = request(
        root, file_manager::DocumentPickerProfile::export_file);
    export_request.default_extension = ".txt";
    export_request.suggested_name = "report.txt";
    export_request.allow_native_fallback = true;
    file_manager::FileSelectionController export_picker(export_request);
    export_picker.set_orchestrator_session_valid(false);
    result = export_picker.accept();
    require(result.terminal == file_manager::DocumentPickerTerminal::unavailable &&
                result.code == "session-unavailable" &&
                result.native_fallback_permitted,
            "lost Orchestrator policy must block acceptance and retain fallback policy");
    require(export_picker.cancel().terminal ==
                file_manager::DocumentPickerTerminal::cancelled,
            "cancel must return no observations");

    auto stale_request = request(root, file_manager::DocumentPickerProfile::open_file);
    file_manager::FileSelectionController stale(stale_request);
    const auto stale_id = find(stale, "beta.txt")->stable_id;
    require(stale.set_selection({stale_id}), "stale fixture must select beta");
    std::filesystem::remove(root.path() / "beta.txt");
    std::ofstream(root.path() / "beta.txt") << "replacement";
    require(stale.accept().code == "selection-changed",
            "replacement between selection and acceptance must fail closed");

    auto offline_request = request(root, file_manager::DocumentPickerProfile::open_file);
    offline_request.orchestrator_session_valid = false;
    file_manager::FileSelectionController unavailable(offline_request);
    require(unavailable.accept().code == "session-unavailable", "default must not invent authority");
    offline_request.authority = file_manager::DocumentPickerAuthority::trusted_local_host;
    offline_request.home_location = root.path();
    file_manager::FileSelectionController local(offline_request);
    require(local.set_selection({find(local, "Folder")->stable_id}) && !local.accept().accepted(),
            "folders are selectable for navigation, never accepted as open files");
    require(local.navigate("Folder") && local.navigate(".."), "relative navigation is based on current location");
    require(local.set_name_filter("a*.TXT") && find(local, "alpha.txt") && !find(local, "beta.txt") && find(local, "Folder"),
            "filename glob combines with type and leaves folders navigable");
    require(local.set_selection({find(local, "alpha.txt")->stable_id}) && local.accept().accepted(),
            "explicit trusted host grant accepts without Engine or synthetic daemon");
    local.set_authority_valid(false);
    require(local.accept().code == "session-unavailable", "host revocation blocks acceptance");
    auto hidden_request = offline_request;
    hidden_request.allow_hidden_toggle = false;
    file_manager::FileSelectionController hidden_locked(hidden_request);
    require(!hidden_locked.set_show_hidden(true), "host can prohibit session hidden toggle");
    require(save.set_filename("alpha.txt") && save.accept().terminal == file_manager::DocumentPickerTerminal::overwrite_confirmation_required,
            "capture overwrite observation");
    std::ofstream(root.path() / "alpha.txt", std::ios::app) << "changed";
    require(save.accept(true).code == "destination-changed", "overwrite confirmation binds to displayed revision");
    require(save.set_active_filter("images") && save.set_filename("new") && save.accept().selections.front().path.filename() == "new.png",
            "save extension follows selected type");
    require(save.set_filename(".hidden") && save.accept().code == "hidden-destination",
            "hidden save path must obey same visibility policy");
    std::cout << "document picker tests passed\n";
    return 0;
}
