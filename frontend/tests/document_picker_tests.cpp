#include "fixture_links.hpp"
#include "file_manager/document_picker.hpp"

#include <cstdlib>
#include <array>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <string_view>
#include <stdexcept>
#include <unistd.h>

namespace {

void write_fixture(const std::filesystem::path& path, const std::string_view contents,
                   const std::ios::openmode mode = std::ios::out) {
    std::ofstream stream(path, mode);
    stream << contents;
    if (!stream)
        throw std::runtime_error("fixture write failed");
}

class TestRoot final {
  public:
    TestRoot() {
        const std::filesystem::path parent =
            std::filesystem::canonical(std::filesystem::temp_directory_path());
        const std::string name = "file-manager-picker-" + std::to_string(::getpid());
        path_ = parent / name;
        const bool acquired = std::filesystem::create_directory(path_);
        if (!acquired) throw std::runtime_error("picker fixture directory already exists");
        try {
            std::filesystem::create_directories(path_ / "Folder");
            write_fixture(path_ / "alpha.txt", "alpha");
            write_fixture(path_ / "beta.txt", "beta");
            write_fixture(path_ / "image.png", "png");
            write_fixture(path_ / ".hidden.txt", "hidden");
            write_fixture(path_ / ".config", "hidden extensionless");
            write_fixture(path_ / "README", "extensionless");
            write_fixture(path_ / "UPPER.TXT", "case");
            write_fixture(path_ / "multi.part.txt", "suffix");
            (void)create_fixture_link("alpha.txt", path_ / "linked.txt");
        } catch (...) {
            cleanup();
            throw;
        }
    }
    TestRoot(const TestRoot&) = delete;
    TestRoot& operator=(const TestRoot&) = delete;
    ~TestRoot() {
        cleanup();
    }
    [[nodiscard]] const std::filesystem::path& path() const noexcept {
        return path_;
    }

  private:
    void cleanup() noexcept {
        // This owner exclusively created this direct child of canonical temp.
        std::error_code ignored{};
        std::filesystem::remove_all(path_, ignored);
    }
    std::filesystem::path path_{};
};

void require(const bool condition, const std::string_view message) {
    if (!condition) {
        const std::string diagnostic(message);
        throw std::runtime_error(diagnostic);
    }
}

file_manager::DocumentPickerRequest request(const TestRoot& root,
                                            const file_manager::DocumentPickerProfile profile) {
    file_manager::DocumentPickerRequest value{};
    value.profile = profile;
    value.protected_root = root.path();
    value.initial_location = root.path();
    value.orchestrator_session_valid = true; // Explicit fixture session.
    value.owner_application_id = "picker-test-consumer";
    value.filters = {{"text", "Text", {"txt"}}, {"images", "Images", {"png"}}};
    value.active_filter_id = "text";
    return value;
}

const file_manager::DirectoryEntry* find(const file_manager::FileSelectionController& picker,
                                         const std::string_view name) {
    for (const file_manager::DirectoryEntry& entry : picker.browser().entries) {
        if (entry.name == name)
            return &entry;
    }
    return nullptr;
}

void remove_directory_link_fixture(const std::filesystem::path& path) {
    // Only the three directory links created by trusted_directory_links enter
    // here. RemoveDirectoryW unlinks a directory reparse point, not its target.
#if defined(_WIN32)
    const BOOL removed = RemoveDirectoryW(path.c_str());
    if (!removed) {
        const DWORD error = GetLastError();
        throw std::system_error(static_cast<int>(error), std::system_category(),
                                "remove picker directory-link fixture");
    }
#else
    const bool removed = std::filesystem::remove(path);
    require(removed, "directory-link fixture must be removed");
#endif
}

void trusted_directory_links(const TestRoot& root) {
    const std::filesystem::path target = root.path() / "Folder";
    const std::filesystem::path alias = root.path() / "folder-link";
    const bool available = create_fixture_link(target, alias, true);
    if (!available) return;
    const bool outside_available = create_fixture_link(root.path().parent_path(), root.path() / "outside-link", true);
    const bool broken_available = create_fixture_link(root.path() / "missing", root.path() / "broken-link", true);
    require(outside_available && broken_available, "link navigation fixtures created");
    file_manager::DocumentPickerRequest policy = request(root, file_manager::DocumentPickerProfile::open_file);
    file_manager::FileSelectionController contained(policy);
    const bool contained_entered = contained.navigate(alias);
    require(!contained_entered, "session picker retains no-link navigation policy");
    policy.authority = file_manager::DocumentPickerAuthority::trusted_local_host;
    file_manager::FileSelectionController local(policy);
    const file_manager::DirectoryEntry* row = find(local, "folder-link");
    require(row != nullptr && (*row).directory, "directory link survives filename/type filtering for navigation");
    const bool selected = local.set_selection({(*row).stable_id});
    const file_manager::DocumentPickerResult alias_result = local.accept();
    require(selected && !alias_result.accepted(), "link alias can be selected for navigation but not accepted as a file");
    const bool entered = local.navigate(alias);
    if (!entered || local.browser().location != target) {
        const std::filesystem::path canonical_alias = std::filesystem::canonical(alias);
        const std::filesystem::path canonical_target = std::filesystem::canonical(target);
        std::cerr << "link navigation: entered=" << entered
                  << " actual=" << local.browser().location << " expected=" << target
                  << " canonical_alias=" << canonical_alias << " canonical_target=" << canonical_target
                  << " root=" << local.browser().root << " error=" << local.last_error() << '\n';
    }
    require(entered && local.browser().location == target, "trusted directory link enters canonical target");
    const bool returned = local.navigate(root.path());
    require(returned, "return from canonical target");
    const bool outside = local.navigate(root.path() / "outside-link");
    const bool broken = local.navigate(root.path() / "broken-link");
    const bool file_link = local.navigate(root.path() / "linked.txt");
    require(!outside && !broken && !file_link && local.browser().location == root.path(),
            "outside, broken and file links preserve current navigation");
    local.set_authority_valid(false);
    const bool revoked = local.navigate(alias);
    require(!revoked, "revoked grant cannot follow a directory link");

    policy.profile = file_manager::DocumentPickerProfile::select_folder;
    policy.initial_location = alias;
    file_manager::FileSelectionController folder(policy);
    const file_manager::DocumentPickerResult folder_result = folder.accept();
    require(folder_result.accepted() && folder_result.selections.front().path == target &&
                folder_result.selections.front().identity.type == std::filesystem::file_type::directory,
            "initial link route returns freshly observed canonical directory identity");
    policy.profile = file_manager::DocumentPickerProfile::save_as;
    policy.suggested_name = "new.txt";
    file_manager::FileSelectionController save(policy);
    const file_manager::DocumentPickerResult destination = save.accept();
    require(destination.accepted() && destination.selections.front().path == target / "new.txt",
            "save in navigated link target returns canonical non-link destination");
    const bool leaf_available = create_fixture_link(root.path() / "alpha.txt", target / "leaf.txt");
    require(leaf_available, "save link leaf fixture");
    const bool named = save.set_filename("leaf.txt");
    const file_manager::DocumentPickerResult leaf = save.accept();
    require(named && !leaf.accepted(), "directory navigation does not permit following save link leaf");
    const std::array<std::filesystem::path, 3> directory_links{
        alias, root.path() / "outside-link", root.path() / "broken-link"};
    for (const std::filesystem::path& link : directory_links) {
        remove_directory_link_fixture(link);
    }
    require(std::filesystem::is_directory(target) && std::filesystem::is_directory(root.path().parent_path()),
            "directory-link cleanup preserves both target directories");
}

bool select_named(file_manager::FileSelectionController& picker, const std::string_view name) {
    const file_manager::DirectoryEntry* row = find(picker, name);
    require(row != nullptr, "file-alias fixture row is visible");
    const bool selected = picker.set_selection({(*row).stable_id});
    return selected;
}

void trusted_file_links(const TestRoot& root) {
    const std::filesystem::path folder = root.path() / "Folder";
    const std::filesystem::path other = root.path() / "Other";
    std::filesystem::create_directory(other);
    write_fixture(folder / "target.txt", "local target");
    write_fixture(other / "target.bin", "cross-root target");
    const bool available = create_fixture_link(folder / "target.txt", folder / "local.txt");
    if (!available) return;
    require(create_fixture_link(other / "target.bin", folder / "cross.txt"), "cross-root link fixture");
    require(create_fixture_link(root.path() / "alpha.txt", folder / "outside.txt"), "outside link fixture");
    require(create_fixture_link(folder / "missing", folder / "broken.txt"), "broken link fixture");
    require(create_fixture_link("cycle.txt", folder / "cycle.txt"), "cyclic link fixture");
    require(create_fixture_link(folder / "target.txt", folder / "changed.txt"), "changed link fixture");
    const std::array<file_manager::DocumentPickerProfile, 3> read_profiles{
        file_manager::DocumentPickerProfile::open_file,
        file_manager::DocumentPickerProfile::open_files,
        file_manager::DocumentPickerProfile::import_files};
    file_manager::DocumentPickerRequest policy = request(root, read_profiles.front());
    policy.protected_root = folder;
    policy.initial_location = folder;
    policy.admitted_roots = {folder, other};
    policy.authority = file_manager::DocumentPickerAuthority::trusted_local_host;
    for (const file_manager::DocumentPickerProfile profile : read_profiles) {
        policy.profile = profile;
        file_manager::FileSelectionController picker(policy);
        const bool local_selected = select_named(picker, "local.txt");
        const file_manager::DocumentPickerResult local = picker.accept();
        const file_manager::ObjectIdentity local_identity =
            file_manager::observe_identity(folder / "target.txt");
        require(local_selected && local.accepted() && local.selections.size() == 1 &&
                    local.selections.front().path == folder / "target.txt" &&
                    local.selections.front().identity.same_revision(local_identity),
                "all read profiles return canonical target and fresh exact identity");
        const bool cross_selected = select_named(picker, "cross.txt");
        const file_manager::DocumentPickerResult cross = picker.accept();
        require(cross_selected && cross.accepted() && cross.selections.front().path == other / "target.bin",
                "explicit additional root permits target; visible alias supplies filename filter");
        const std::array<std::string_view, 3> refused_names{"outside.txt", "broken.txt", "cycle.txt"};
        for (const std::string_view name : refused_names) {
            const bool selected = select_named(picker, name);
            const file_manager::DocumentPickerResult refused = picker.accept();
            require(selected && !refused.accepted() && refused.selections.empty(),
                    "outside, broken and cyclic aliases refuse without partial selection");
        }
        const bool selected_before_revoke = select_named(picker, "local.txt");
        picker.set_authority_valid(false);
        const file_manager::DocumentPickerResult revoked = picker.accept();
        require(selected_before_revoke && !revoked.accepted(), "revoked authority cannot accept file alias");
    }
    policy.profile = file_manager::DocumentPickerProfile::open_files;
    policy.maximum_selection = 2;
    file_manager::FileSelectionController mixed(policy);
    const file_manager::DirectoryEntry* good = find(mixed, "local.txt");
    const file_manager::DirectoryEntry* bad = find(mixed, "outside.txt");
    require(good != nullptr && bad != nullptr, "mixed selection rows exist");
    const bool mixed_selected = mixed.set_selection({(*good).stable_id, (*bad).stable_id});
    const file_manager::DocumentPickerResult mixed_result = mixed.accept();
    require(mixed_selected && !mixed_result.accepted() && mixed_result.selections.empty(),
            "one refused alias prevents partial multi-selection acceptance");

    policy.profile = file_manager::DocumentPickerProfile::open_file;
    policy.maximum_selection = 1;
    file_manager::FileSelectionController changed(policy);
    const bool changed_selected = select_named(changed, "changed.txt");
    std::filesystem::rename(folder / "changed.txt", folder / "retired.txt");
    require(create_fixture_link(other / "target.bin", folder / "changed.txt"), "replacement alias fixture");
    const file_manager::DocumentPickerResult changed_result = changed.accept();
    require(changed_selected && !changed_result.accepted(), "alias replacement since snapshot refuses");

    policy.authority = file_manager::DocumentPickerAuthority::orchestrator_session;
    file_manager::FileSelectionController session(policy);
    const bool session_selected = select_named(session, "local.txt");
    require(!session_selected, "daemon-session selection retains no-follow policy");
    policy.authority = file_manager::DocumentPickerAuthority::trusted_local_host;
    const std::array<file_manager::DocumentPickerProfile, 2> write_profiles{
        file_manager::DocumentPickerProfile::save_as, file_manager::DocumentPickerProfile::export_file};
    for (const file_manager::DocumentPickerProfile profile : write_profiles) {
        policy.profile = profile;
        file_manager::FileSelectionController save(policy);
        const bool selected = select_named(save, "local.txt");
        const bool named = save.set_filename("local.txt");
        const file_manager::DocumentPickerResult result = save.accept(true);
        require(!selected && named && !result.accepted(), "Save and Export retain no-follow file leaves");
    }
}

} // namespace

int run_tests() {
    {
        TestRoot link_root{};
        trusted_directory_links(link_root);
        trusted_file_links(link_root);
    }
    TestRoot root{};

    file_manager::DocumentPickerRequest open_request =
        request(root, file_manager::DocumentPickerProfile::open_file);
    file_manager::FileSelectionController open(open_request);
    require(open.browser().available() && find(open, "alpha.txt") && !find(open, "image.png") &&
                !find(open, ".hidden.txt"),
            "open profile must browse without Engine and apply type/hidden policy");
    const bool open_selected = open.set_selection({(*find(open, "alpha.txt")).stable_id});
    const file_manager::DocumentPickerResult open_result = open.accept();
    require(open_selected && open_result.accepted() &&
                open_result.selections.front().path.filename() == "alpha.txt",
            "one ordinary visible file must revalidate and accept");
    const std::filesystem::path retained_location = open.browser().location;
    const std::vector<std::string> retained_selection = open.selected_ids();
    const bool outside_navigation = open.navigate(root.path().parent_path());
    require(!outside_navigation && open.browser().available() &&
                open.browser().location == retained_location &&
                open.selected_ids() == retained_selection && !open.last_error().empty(),
            "refused navigation must retain the last usable directory and selection");

    const bool hidden_enabled = open.set_show_hidden(true);
    require(hidden_enabled && find(open, ".hidden.txt"),
            "app-scoped hidden policy must refresh the same browser model");
    const bool images_selected = open.set_active_filter("images");
    require(images_selected && find(open, "image.png") && !find(open, "alpha.txt") &&
                !find(open, ".hidden.txt"),
            "type filter changes must not leak filtered hidden files");

    file_manager::DocumentPickerRequest multi_request =
        request(root, file_manager::DocumentPickerProfile::open_files);
    multi_request.maximum_selection = 2;
    file_manager::FileSelectionController multi(multi_request);
    const bool multiple_selected = multi.set_selection(
        {(*find(multi, "alpha.txt")).stable_id, (*find(multi, "beta.txt")).stable_id});
    const file_manager::DocumentPickerResult multiple_result = multi.accept();
    require(multiple_selected && multiple_result.selections.size() == 2U,
            "bounded multi-open must accept exactly the admitted observations");
    const bool excess_selection = multi.set_selection({(*find(multi, "alpha.txt")).stable_id,
                                                       (*find(multi, "beta.txt")).stable_id,
                                                       (*find(multi, "Folder")).stable_id});
    require(!excess_selection, "multi-open must enforce its cardinality before acceptance");

    file_manager::DocumentPickerRequest folder_request =
        request(root, file_manager::DocumentPickerProfile::select_folder);
    folder_request.filters.clear();
    folder_request.active_filter_id.clear();
    file_manager::FileSelectionController folder(folder_request);
    const file_manager::DocumentPickerResult current_folder_result = folder.accept();
    require(!find(folder, "alpha.txt") && find(folder, "Folder") &&
                current_folder_result.accepted() &&
                current_folder_result.selections.front().path ==
                    std::filesystem::canonical(root.path()),
            "folder profile must hide files and allow the current folder");
    const bool child_selected = folder.set_selection({(*find(folder, "Folder")).stable_id});
    const file_manager::DocumentPickerResult child_result = folder.accept();
    require(child_selected && child_result.accepted() &&
                child_result.selections.front().path.filename() == "Folder",
            "folder profile must accept one selected child folder");

    file_manager::DocumentPickerRequest save_request =
        request(root, file_manager::DocumentPickerProfile::save_as);
    save_request.suggested_name = "draft";
    save_request.default_extension = "txt";
    file_manager::FileSelectionController save(save_request);
    file_manager::DocumentPickerResult result = save.accept();
    require(result.accepted() && result.selections.front().path.filename() == "draft.txt" &&
                !result.selections.front().existing,
            "save-as must correct the extension and return a non-writing destination observation");
    const bool existing_name_set = save.set_filename("alpha.txt");
    require(existing_name_set, "save profile must accept a bounded basename edit");
    result = save.accept();
    require(result.terminal ==
                    file_manager::DocumentPickerTerminal::overwrite_confirmation_required &&
                result.selections.front().existing,
            "existing save destination must require explicit overwrite confirmation");
    const file_manager::DocumentPickerResult overwrite_result = save.accept(true);
    require(overwrite_result.accepted(),
            "confirmed overwrite must return the observed existing destination");
    const bool escape_name_set = save.set_filename("../escape");
    const file_manager::DocumentPickerResult invalid_name_result = save.accept();
    require(escape_name_set && invalid_name_result.code == "invalid-filename",
            "save acceptance must reject path-bearing filenames");

    file_manager::DocumentPickerRequest export_request =
        request(root, file_manager::DocumentPickerProfile::export_file);
    export_request.default_extension = ".txt";
    export_request.suggested_name = "report.txt";
    export_request.allow_native_fallback = true;
    file_manager::FileSelectionController export_picker(export_request);
    export_picker.set_orchestrator_session_valid(false);
    result = export_picker.accept();
    require(result.terminal == file_manager::DocumentPickerTerminal::unavailable &&
                result.code == "session-unavailable" && result.native_fallback_permitted,
            "lost Orchestrator policy must block acceptance and retain fallback policy");
    const file_manager::DocumentPickerResult cancelled_result = export_picker.cancel();
    require(cancelled_result.terminal == file_manager::DocumentPickerTerminal::cancelled,
            "cancel must return no observations");

    file_manager::DocumentPickerRequest stale_request =
        request(root, file_manager::DocumentPickerProfile::open_file);
    file_manager::FileSelectionController stale(stale_request);
    const std::string stale_id = (*find(stale, "beta.txt")).stable_id;
    const bool stale_selected = stale.set_selection({stale_id});
    require(stale_selected, "stale fixture must select beta");
    std::filesystem::remove(root.path() / "beta.txt");
    write_fixture(root.path() / "beta.txt", "replacement");
    const file_manager::DocumentPickerResult stale_result = stale.accept();
    require(stale_result.code == "selection-changed",
            "replacement between selection and acceptance must fail closed");

    file_manager::DocumentPickerRequest offline_request =
        request(root, file_manager::DocumentPickerProfile::open_file);
    offline_request.orchestrator_session_valid = false;
    file_manager::FileSelectionController unavailable(offline_request);
    const file_manager::DocumentPickerResult unavailable_result = unavailable.accept();
    require(unavailable_result.code == "session-unavailable", "default must not invent authority");
    offline_request.authority = file_manager::DocumentPickerAuthority::trusted_local_host;
    offline_request.home_location = root.path();
    file_manager::FileSelectionController local(offline_request);
    const bool navigation_selected = local.set_selection({(*find(local, "Folder")).stable_id});
    const file_manager::DocumentPickerResult folder_as_file_result = local.accept();
    require(navigation_selected && !folder_as_file_result.accepted(),
            "folders are selectable for navigation, never accepted as open files");
    const bool entered_child = local.navigate("Folder");
    const bool returned_parent = local.navigate("..");
    require(entered_child && returned_parent, "relative navigation is based on current location");
    const bool glob_applied = local.set_name_filter("a*.TXT");
    require(glob_applied && find(local, "alpha.txt") && !find(local, "beta.txt") &&
                find(local, "Folder"),
            "filename glob combines with type and leaves folders navigable");
    const bool local_selected = local.set_selection({(*find(local, "alpha.txt")).stable_id});
    const file_manager::DocumentPickerResult local_result = local.accept();
    require(local_selected && local_result.accepted(),
            "explicit trusted host grant accepts without Engine or synthetic daemon");
    local.set_authority_valid(false);
    const file_manager::DocumentPickerResult revoked_result = local.accept();
    require(revoked_result.code == "session-unavailable", "host revocation blocks acceptance");
    file_manager::DocumentPickerRequest hidden_request = offline_request;
    hidden_request.allow_hidden_toggle = false;
    file_manager::FileSelectionController hidden_locked(hidden_request);
    const bool hidden_override = hidden_locked.set_show_hidden(true);
    require(!hidden_override, "host can prohibit session hidden toggle");
    const bool confirmation_name_set = save.set_filename("alpha.txt");
    const file_manager::DocumentPickerResult confirmation_result = save.accept();
    require(confirmation_name_set &&
                confirmation_result.terminal ==
                    file_manager::DocumentPickerTerminal::overwrite_confirmation_required,
            "capture overwrite observation");
    write_fixture(root.path() / "alpha.txt", "changed", std::ios::app);
    const file_manager::DocumentPickerResult changed_destination_result = save.accept(true);
    require(changed_destination_result.code == "destination-changed",
            "overwrite confirmation binds to displayed revision");
    const bool image_filter_set = save.set_active_filter("images");
    const bool new_name_set = save.set_filename("new");
    const file_manager::DocumentPickerResult image_save_result = save.accept();
    require(image_filter_set && new_name_set &&
                image_save_result.selections.front().path.filename() == "new.png",
            "save extension follows selected type");
    const bool hidden_name_set = save.set_filename(".hidden");
    const file_manager::DocumentPickerResult hidden_destination_result = save.accept();
    require(hidden_name_set && hidden_destination_result.code == "hidden-destination",
            "hidden save path must obey same visibility policy");
    // Guard the allocation-free suffix matcher against the filesystem extension
    // rules it replaced, and exercise both wildcard restart and single-byte '?' .
    file_manager::DocumentPickerRequest suffix_request = open_request;
    suffix_request.show_hidden = true;
    suffix_request.filters = {{"none", "No extension", {""}}, {"text", "Text", {"txt"}}};
    suffix_request.active_filter_id = "none";
    file_manager::FileSelectionController suffix(suffix_request);
    require(find(suffix, "README") != nullptr && find(suffix, ".config") != nullptr &&
                find(suffix, "UPPER.TXT") == nullptr,
            "leading-dot and extensionless basenames preserve extension matching");
    const bool text_filter = suffix.set_active_filter("text");
    require(text_filter && find(suffix, "UPPER.TXT") != nullptr &&
                find(suffix, "multi.part.txt") != nullptr,
            "case folding and final-dot suffix semantics remain unchanged");
    const bool wildcard_filter = suffix.set_name_filter("*part.???");
    require(wildcard_filter && find(suffix, "multi.part.txt") != nullptr &&
                find(suffix, "UPPER.TXT") == nullptr,
            "star backtracking and question-mark matching compose with type filtering");
    std::cout << "document picker tests passed\n";
    return 0;
}

int main() {
    try {
        const int status = run_tests();
        return status;
    } catch (const std::exception& error) {
        std::cerr << "document picker test failed: " << error.what() << '\n';
        return 1;
    }
}
