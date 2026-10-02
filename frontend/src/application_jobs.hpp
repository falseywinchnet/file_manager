#pragma once

#include "application.hpp"

#include "file_manager/platform_paths.hpp"

#include <algorithm>
#include <exception>
#include <utility>

namespace file_manager {

// The named records below own captured state and Application until invoked or discarded.
// The generic worker-error handler separately borrows this under Runtime + stop/join.
// stop() rejects new work and discards queued UI work. drain_ui checks stop before
// each invocation, including reentrant shutdown. Already admitted worker jobs drain
// during join, preserving mutation semantics; cancellable reads observe stop.
// Runtime owns Application and explicitly calls stop before releasing it; queue
// entries retain it through invocation. Subscriptions revoke before owner teardown.
// ChecksumProgressReport borrows last_report only within synchronous checksum_sha256.

struct Application::BootstrapUnavailable final {
    std::shared_ptr<Application> self{};
    std::string message{};
    void operator()() const {
        (*(*self).form_.file_manager_app_shell_title_service).set_text(
            "Orchestrator · unavailable");
        (*(*self).form_.file_manager_app_shell_status_summary).set_text(
            "Base navigation remains available · " + message);
    }
};

struct Application::BootstrapReady final {
    std::shared_ptr<Application> self{};
    std::string service{};
    std::string summary{};
    void operator()() const {
        (*(*self).form_.file_manager_app_shell_title_service).set_text(service);
        (*(*self).form_.file_manager_app_shell_status_summary).set_text(summary);
    }
};

struct Application::BootstrapWork final {
    std::shared_ptr<Application> self{};
    void operator()() const {
        try {
            fileman::orchestrator::Client client = fileman::orchestrator::Client::connect_default(
                "file-manager-frontend-1.0");
            const fileman::orchestrator::BootstrapSnapshot snapshot = client.bootstrap();
            const std::string gui_state = snapshot.frontend_opening.gui_forms_gate.state;
            const std::string service = snapshot.orchestrator_gate_ready()
                ? "Core ready · GUI.Forms " + gui_state
                : "Core blocked · GUI.Forms " + gui_state;
            const std::string summary = "Core " + snapshot.release.target_version +
                " · GUI.Forms " + gui_state;
            (*self).post_ui(BootstrapReady{self, service, summary});
        } catch (const std::exception& error) {
            const std::string message = error.what();
            (*self).post_ui(BootstrapUnavailable{self, message});
        }
    }
};

struct Application::SettingsUnavailable final {
    std::shared_ptr<Application> self{};
    std::string message{};
    void operator()() const {
        (*self).settings_loading_ = false;
        (*(*self).form_.file_manager_app_shell_settings_heading_revision).set_text(
            "service unavailable");
        (*(*self).form_.file_manager_app_shell_settings_actions_status).set_text(
            "Settings unavailable · " + message);
        (*self).update_settings_actions();
    }
};

struct Application::SettingsReady final {
    std::shared_ptr<Application> self{};
    fileman::orchestrator::SettingsSchemaInfo schema{};
    fileman::orchestrator::SettingsSnapshotInfo snapshot{};
    void operator()() {
        (*self).settings_loading_ = false;
        (*self).apply_settings_state(std::move(schema),
                                   std::move(snapshot));
    }
};

struct Application::SettingsWork final {
    std::shared_ptr<Application> self{};
    void operator()() const {
        try {
            fileman::orchestrator::Client client = fileman::orchestrator::Client::connect_default(
                "file-manager-settings-1.0");
            fileman::orchestrator::SettingsSchemaInfo schema = client.settings_schema();
            fileman::orchestrator::SettingsSnapshotInfo snapshot = client.settings_snapshot();
            (*self).post_ui(SettingsReady{self, std::move(schema), std::move(snapshot)});
        } catch (const std::exception& error) {
            const std::string message = error.what();
            (*self).post_ui(SettingsUnavailable{self, message});
        }
    }
};

struct Application::ServicesUnavailable final {
    std::shared_ptr<Application> self{};
    std::string message{};
    void operator()() const {
        (*self).services_loading_ = false;
        (*self).services_snapshot_.reset();
        (*self).service_notice_ = "Services unavailable · " + message;
        if ((*self).settings_open_ && (*self).settings_tab_ == "services") {
            (*self).rebuild_settings_page();
        }
    }
};

struct Application::ServicesReady final {
    std::shared_ptr<Application> self{};
    fileman::orchestrator::ServicesSnapshotInfo snapshot{};
    void operator()() {
        (*self).services_loading_ = false;
        (*self).services_snapshot_ = std::move(snapshot);
        if ((*self).service_notice_.empty()) {
            (*self).service_notice_ =
                "Immutable ORC-UI-001 service observation";
        }
        if ((*self).settings_open_ && (*self).settings_tab_ == "services") {
            (*self).rebuild_settings_page();
        }
    }
};

struct Application::ServicesWork final {
    std::shared_ptr<Application> self{};
    void operator()() const {
        try {
            fileman::orchestrator::Client client = fileman::orchestrator::Client::connect_default(
                "file-manager-services-1.0");
            fileman::orchestrator::ServicesSnapshotInfo snapshot = client.services_snapshot();
            (*self).post_ui(ServicesReady{self, std::move(snapshot)});
        } catch (const std::exception& error) {
            const std::string message = error.what();
            (*self).post_ui(ServicesUnavailable{self, message});
        }
    }
};

struct Application::SuggestionUnavailable final {
    std::shared_ptr<Application> self{};
    std::uint64_t generation{};
    std::string requested_text{};
    std::string preview{};
    std::string notice{};
    void operator()() {
        (*self).apply_path_suggestions(
            generation, std::move(requested_text), std::move(preview),
            {}, notice);
    }
};

struct Application::SuggestionReady final {
    std::shared_ptr<Application> self{};
    std::uint64_t generation{};
    std::string requested_text{};
    std::string preview{};
    std::vector<std::filesystem::path> suggestions{};
    std::string notice{};
    void operator()() {
        (*self).apply_path_suggestions(
            generation, std::move(requested_text), std::move(preview),
            std::move(suggestions), notice);
    }
};

struct Application::SuggestionCancelled final {
    std::shared_ptr<Application> self{};
    std::uint64_t generation{};
    bool operator()() const {
        const bool cancelled = (*self).stopping_.load() ||
            (*self).path_suggestion_generation_.load() != generation;
        return cancelled;
    }
};

struct Application::SuggestionNotAdmitted final {
    std::shared_ptr<Application> self{};
    std::uint64_t generation{};
    std::string requested_text{};
    std::string preview{};
    void operator()() {
        (*self).apply_path_suggestions(
            generation, std::move(requested_text),
            std::move(preview), {},
            "Outside Home, Volumes, and explicit launch roots");
    }
};

struct Application::SuggestionWork final {
    std::shared_ptr<Application> self{};
    std::uint64_t generation{};
    std::string requested_text{};
    std::vector<std::filesystem::path> admitted_roots{};
    std::filesystem::path current_location{};
    std::filesystem::path home_root{};
    std::filesystem::path candidate{};
    bool show_hidden{};
    void operator()() {
        try {
            const std::optional<NavigationTarget> target = resolve_navigation_target(
                admitted_roots, current_location, home_root, candidate);
            if ((*self).stopping_.load() ||
                (*self).path_suggestion_generation_.load() != generation) {
                return;
            }
            if (!target) {
                (*self).post_ui(SuggestionNotAdmitted{self, generation, std::move(requested_text), "Not admitted: " + path_utf8(candidate)});
                return;
            }
            std::filesystem::path root = (*target).root;
            candidate = (*target).path;
            std::error_code type_error{};
            const bool candidate_is_directory =
                std::filesystem::is_directory(candidate, type_error) &&
                !type_error;
            std::filesystem::path parent = candidate_is_directory
                ? candidate : candidate.parent_path();
            const std::string prefix = candidate_is_directory
                ? std::string{} : path_utf8(candidate.filename());
            DirectorySnapshot snapshot = read_directory(
                root, parent, prefix, generation,
                SuggestionCancelled{self, generation}, show_hidden);
            if (snapshot.cancelled) return;
            std::vector<std::filesystem::path> suggestions{};
            suggestions.reserve(
                std::min<std::size_t>(snapshot.entries.size(), 12U));
            for (const DirectoryEntry& entry : snapshot.entries) {
                if (!entry.directory) continue;
                suggestions.push_back(entry.path);
                if (suggestions.size() == 12U) break;
            }
            const std::string notice = snapshot.error.empty()
                ? (suggestions.empty() ? "No matching local folders"
                                       : std::string{})
                : snapshot.error;
            const std::string preview = snapshot.available()
                ? (candidate_is_directory
                    ? "Resolved: " + path_utf8(snapshot.location)
                    : "Proposed: " + path_utf8(candidate))
                : "Unavailable: " + path_utf8(candidate);
            (*self).post_ui(SuggestionReady{self, generation, std::move(requested_text), preview, std::move(suggestions), notice});
        } catch (const std::exception& error) {
            const std::string notice =
                std::string("Suggestion lookup failed: ") + error.what();
            const std::string preview =
                "Unavailable: " + path_utf8(candidate);
            (*self).post_ui(SuggestionUnavailable{self, generation, std::move(requested_text), preview, notice});
        }
    }
};

struct Application::ServiceCommandUnavailable final {
    std::shared_ptr<Application> self{};
    std::string message{};
    void operator()() const {
        (*self).service_command_in_flight_ = false;
        (*self).services_snapshot_.reset();
        (*self).service_notice_ = "Service command refused · " + message;
        (*self).request_services();
    }
};

struct Application::ServiceCommandReady final {
    std::shared_ptr<Application> self{};
    fileman::orchestrator::ServiceCommandResultInfo result{};
    void operator()() {
        (*self).service_command_in_flight_ = false;
        (*self).services_snapshot_.reset();
        (*self).service_notice_ = result.service_id + " " +
            result.command_id + " · " + result.terminal + " · " +
            result.effect;
        if (result.service_id == "orchestrator" &&
            result.command_id == "shutdown") {
            (*self).rebuild_settings_page();
        } else {
            (*self).request_services();
        }
    }
};

struct Application::ServiceCommandWork final {
    std::shared_ptr<Application> self{};
    std::string service_id{};
    std::string command_id{};
    std::optional<std::string> root_id{};
    std::optional<std::string> instance{};
    std::optional<std::uint64_t> generation{};
    void operator()() const {
        try {
            fileman::orchestrator::Client client = fileman::orchestrator::Client::connect_default(
                "file-manager-service-command-1.0");
            fileman::orchestrator::ServiceCommandResultInfo result = client.service_command(service_id, command_id,
                                                 instance, generation, root_id);
            (*self).post_ui(ServiceCommandReady{self, std::move(result)});
        } catch (const std::exception& error) {
            const std::string message = error.what();
            (*self).post_ui(ServiceCommandUnavailable{self, message});
        }
    }
};

struct Application::SettingsRefreshUnavailable final {
    std::shared_ptr<Application> self{};
    std::string message{};
    std::string refresh{};
    void operator()() const {
        (*self).settings_apply_in_flight_ = false;
        (*(*self).form_.file_manager_app_shell_settings_actions_status).set_text(
            "Apply failed · " + message + " · refresh failed · " + refresh);
        (*self).update_settings_actions();
        (*self).rebuild_settings_page();
    }
};

struct Application::SettingsConflictReady final {
    std::shared_ptr<Application> self{};
    fileman::orchestrator::SettingsSchemaInfo schema{};
    fileman::orchestrator::SettingsSnapshotInfo snapshot{};
    std::string message{};
    void operator()() {
        (*self).settings_apply_in_flight_ = false;
        (*self).apply_settings_state(
            std::move(schema), std::move(snapshot),
            "Apply conflict/refusal · refreshed committed values · " +
                message);
    }
};

struct Application::SettingsCommitReady final {
    std::shared_ptr<Application> self{};
    fileman::orchestrator::SettingsCommitInfo commit{};
    void operator()() {
        (*self).apply_settings_commit(std::move(commit));
    }
};

struct Application::SettingsCommitWork final {
    std::shared_ptr<Application> self{};
    std::uint64_t revision{};
    std::vector<fileman::orchestrator::SettingChange> changes{};
    void operator()() {
        try {
            fileman::orchestrator::Client client = fileman::orchestrator::Client::connect_default(
                "file-manager-settings-1.0");
            fileman::orchestrator::SettingsCommitInfo commit = client.apply_settings(revision, std::move(changes));
            (*self).post_ui(SettingsCommitReady{self, std::move(commit)});
        } catch (const std::exception& error) {
            const std::string message = error.what();
            try {
                fileman::orchestrator::Client client = fileman::orchestrator::Client::connect_default(
                    "file-manager-settings-refresh-1.0");
                fileman::orchestrator::SettingsSchemaInfo schema = client.settings_schema();
                fileman::orchestrator::SettingsSnapshotInfo snapshot = client.settings_snapshot();
                (*self).post_ui(SettingsConflictReady{self, std::move(schema), std::move(snapshot), message});
            } catch (const std::exception& refresh_error) {
                const std::string refresh = refresh_error.what();
                (*self).post_ui(SettingsRefreshUnavailable{self, message, refresh});
            }
        }
    }
};

struct Application::NavigationReady final {
    std::shared_ptr<Application> self{};
    DirectorySnapshot snapshot{};
    bool add_history{};
    void operator()() {
        (*self).apply_directory(std::move(snapshot), add_history);
    }
};

struct Application::NavigationCancelled final {
    std::shared_ptr<Application> self{};
    std::uint64_t generation{};
    bool operator()() const {
        const bool cancelled = (*self).stopping_.load() ||
            (*self).requested_generation_.load() != generation;
        return cancelled;
    }
};

struct Application::NavigationWork final {
    std::shared_ptr<Application> self{};
    std::filesystem::path root{};
    std::filesystem::path path{};
    std::uint64_t generation{};
    bool add_history{};
    bool show_hidden{};
    void operator()() const {
        DirectorySnapshot snapshot = read_directory(root, path, {}, generation,
            NavigationCancelled{self, generation}, show_hidden);
        if (snapshot.cancelled) return;
        (*self).post_ui(NavigationReady{self, std::move(snapshot), add_history});
    }
};

struct Application::TreeExpansionReady final {
    std::shared_ptr<Application> self{};
    DirectorySnapshot snapshot{};
    void operator()() {
        (*self).tree_directory_entries_[path_generic_utf8(snapshot.location)] =
            snapshot.entries;
        (*self).rebuild_tree(snapshot);
    }
};

struct Application::TreeExpansionCancelled final {
    std::shared_ptr<Application> self{};
    bool operator()() const {
        const bool cancelled = (*self).stopping_.load();
        return cancelled;
    }
};

struct Application::TreeExpansionWork final {
    std::shared_ptr<Application> self{};
    std::filesystem::path root{};
    std::filesystem::path path{};
    std::uint64_t generation{};
    bool show_hidden{};
    void operator()() const {
        DirectorySnapshot snapshot = read_directory(root, path, {}, generation,
            TreeExpansionCancelled{self}, show_hidden);
        if (snapshot.cancelled || !snapshot.available()) return;
        (*self).post_ui(TreeExpansionReady{self, std::move(snapshot)});
    }
};

struct Application::CriteriaUnavailable final {
    std::shared_ptr<Application> self{};
    std::uint64_t generation{};
    std::string message{};
    void operator()() const {
        if (generation != (*self).search_generation_.load()) return;
        (*self).criteria_loading_ = false;
        (*(*self).form_.file_manager_app_shell_workspace_selection_content_heading_more_results).set_enabled((*self).criteria_cursor_.has_value());
        (*self).update_command_state();
        (*self).set_status("Criteria unavailable", message);
    }
};

struct Application::CriteriaReady final {
    std::shared_ptr<Application> self{};
    fileman::orchestrator::SearchPageInfo page{};
    std::vector<fileman::orchestrator::SearchExactFilter> filters{};
    std::uint64_t generation{};
    bool append{};
    void operator()() {
        (*self).apply_engine_criteria(
            std::move(page), std::move(filters), generation, append);
    }
};

// Query jobs retain Application. Supersession is observed at each synchronous
// service boundary; the transport call itself has no cancellation argument.
struct Application::SearchCancelled final {
    std::shared_ptr<Application> self{};
    std::uint64_t generation{};
    bool operator()() const {
        const bool cancelled = (*self).stopping_.load() ||
            (*self).search_generation_.load() != generation;
        return cancelled;
    }
};

struct Application::CriteriaWork final {
    std::shared_ptr<Application> self{};
    std::string engine_root_id{};
    std::optional<std::string> relative_path{};
    std::vector<fileman::orchestrator::SearchExactFilter> filters{};
    std::uint32_t maximum_results{};
    std::uint64_t generation{};
    std::optional<fileman::orchestrator::SearchCursorInfo> cursor{};
    bool append{};
    void operator()() {
        const SearchCancelled cancelled{self, generation};
        if (cancelled()) return;
        try {
            fileman::orchestrator::Client client = fileman::orchestrator::Client::connect_default(
                "file-manager-criteria-1.0");
            if (cancelled()) return;
            fileman::orchestrator::SearchPageInfo page = client.search_subtree(
                engine_root_id, relative_path, {}, maximum_results, cursor,
                filters);
            if (cancelled()) return;
            (*self).post_ui(CriteriaReady{self, std::move(page), std::move(filters), generation, append});
        } catch (const std::exception& error) {
            if (cancelled()) return;
            const std::string message = error.what();
            (*self).post_ui(CriteriaUnavailable{self, generation, message});
        }
    }
};

struct Application::SearchUnavailable final {
    std::shared_ptr<Application> self{};
    std::uint64_t generation{};
    std::string message{};
    void operator()() const {
        if (generation != (*self).search_generation_.load()) return;
        (*self).search_loading_ = false;
        (*(*self).form_.file_manager_app_shell_workspace_selection_content_heading_more_results).set_enabled((*self).search_cursor_.has_value());
        (*self).update_command_state();
        (*self).set_status("Search unavailable", message);
    }
};

struct Application::SearchReady final {
    std::shared_ptr<Application> self{};
    fileman::orchestrator::SearchPageInfo page{};
    std::string query{};
    std::uint64_t generation{};
    bool append{};
    void operator()() {
        (*self).apply_engine_search(
            std::move(page), std::move(query), generation, append);
    }
};

struct Application::SearchWork final {
    std::shared_ptr<Application> self{};
    std::string engine_root_id{};
    std::optional<std::string> relative_path{};
    std::string query{};
    std::uint32_t maximum_results{};
    std::uint64_t generation{};
    std::optional<fileman::orchestrator::SearchCursorInfo> cursor{};
    bool append{};
    void operator()() const {
        const SearchCancelled cancelled{self, generation};
        if (cancelled()) return;
        try {
            fileman::orchestrator::Client client = fileman::orchestrator::Client::connect_default(
                "file-manager-search-1.0");
            if (cancelled()) return;
            fileman::orchestrator::SearchPageInfo page = client.search_subtree(
                engine_root_id, relative_path, query, maximum_results, cursor);
            if (cancelled()) return;
            (*self).post_ui(SearchReady{self, std::move(page), query, generation, append});
        } catch (const std::exception& error) {
            if (cancelled()) return;
            const std::string message = error.what();
            (*self).post_ui(SearchUnavailable{self, generation, message});
        }
    }
};

struct Application::PreviewReady final {
    std::shared_ptr<Application> self{};
    PreviewResult result{};
    std::string stable_id{};
    std::uint64_t generation{};
    void operator()() {
        (*self).apply_preview(std::move(result), std::move(stable_id),
                            generation);
    }
};

struct Application::PreviewCancelled final {
    std::shared_ptr<Application> self{};
    std::uint64_t generation{};
    bool operator()() const {
        const bool cancelled = (*self).stopping_.load() ||
            (*self).preview_generation_.load() != generation;
        return cancelled;
    }
};

struct Application::PreviewWork final {
    std::shared_ptr<Application> self{};
    std::filesystem::path root{};
    DirectoryEntry entry{};
    std::uint64_t generation{};
    void operator()() const {
        const PreviewCancelled cancelled{self, generation};
        if (cancelled()) return;
        PreviewResult result = load_preview(root, entry.path, entry.identity,
            cancelled);
        // Selection can change during the read. Retire obsolete output here;
        // the UI still revalidates generation in case it changes after enqueue.
        if (cancelled()) return;
        (*self).post_ui(PreviewReady{self, std::move(result), entry.stable_id, generation});
    }
};

struct Application::ChecksumReady final {
    std::shared_ptr<Application> self{};
    ChecksumResult result{};
    std::string expected{};
    std::uint64_t generation{};
    void operator()() {
        (*self).apply_checksum(std::move(result), std::move(expected),
                             generation);
    }
};

struct Application::ChecksumProgressReady final {
    std::shared_ptr<Application> self{};
    std::uint64_t generation{};
    ChecksumProgress progress{};
    void operator()() const {
        if (generation != (*self).checksum_generation_.load()) return;
        (*self).set_status(
            "Computing SHA-256 · " +
                format_bytes(progress.bytes_read) + " / " +
                format_bytes(progress.total_bytes),
            "Cancel hash stops before publishing a digest");
    }
};

struct Application::ChecksumProgressReport final {
    std::shared_ptr<Application> self{};
    std::uint64_t generation{};
    std::uint64_t& last_report;
    void operator()(const ChecksumProgress progress) const {
        constexpr std::uint64_t report_stride = 8U * 1024U * 1024U;
        if (progress.bytes_read != progress.total_bytes &&
            progress.bytes_read - last_report < report_stride) {
            return;
        }
        last_report = progress.bytes_read;
        (*self).post_ui(ChecksumProgressReady{self, generation, progress});
    }
};

struct Application::ChecksumCancelled final {
    std::shared_ptr<Application> self{};
    std::uint64_t generation{};
    bool operator()() const {
        const bool cancelled = (*self).stopping_.load() ||
            (*self).checksum_generation_.load() != generation;
        return cancelled;
    }
};

struct Application::ChecksumWork final {
    std::shared_ptr<Application> self{};
    std::filesystem::path root{};
    DirectoryEntry entry{};
    std::string expected{};
    std::uint64_t generation{};
    void operator()() const {
        std::uint64_t last_report{};
        ChecksumResult result = checksum_sha256(
            root, entry.path, entry.identity,
            ChecksumCancelled{self, generation},
            ChecksumProgressReport{self, generation, last_report});
        (*self).post_ui(ChecksumReady{self, std::move(result), expected, generation});
    }
};

struct Application::PlatformCommandReady final {
    std::shared_ptr<Application> self{};
    PlatformCommandKind kind{};
    PlatformCommandResult result{};
    std::filesystem::path path{};
    void operator()() {
        (*self).apply_platform_command(kind, std::move(result),
                                     std::move(path));
    }
};

struct Application::PlatformCommandWork final {
    std::shared_ptr<Application> self{};
    PlatformCommandKind kind{};
    PlatformCommandPlan plan{};
    std::filesystem::path path{};
    void operator()() {
        PlatformCommandResult result = execute_platform_command(plan);
        (*self).post_ui(PlatformCommandReady{self, kind, std::move(result), std::move(path)});
    }
};

struct Application::CreateFolderReady final {
    std::shared_ptr<Application> self{};
    OperationResult result{};
    void operator()() {
        (*self).apply_operation(std::move(result));
    }
};

struct Application::CreateFolderWork final {
    std::shared_ptr<Application> self{};
    std::filesystem::path parent{};
    void operator()() const {
        OperationResult result = (*(*self).operations_).create_folder(parent);
        (*self).post_ui(CreateFolderReady{self, std::move(result)});
    }
};

struct Application::RenameReady final {
    std::shared_ptr<Application> self{};
    OperationResult result{};
    void operator()() {
        (*self).apply_operation(std::move(result));
    }
};

struct Application::RenameWork final {
    std::shared_ptr<Application> self{};
    DirectoryEntry entry{};
    std::string basename{};
    void operator()() {
        OperationResult result = (*(*self).operations_).rename_object(
            entry.path, entry.identity, basename);
        (*self).post_ui(RenameReady{self, std::move(result)});
    }
};

struct Application::PropertyRenameReady final {
    std::shared_ptr<Application> self{};
    DirectoryEntry entry{};
    OperationResult result{};
    void operator()() {
        (*self).property_rename_in_flight_ = false;
        if (!result.succeeded()) {
            const std::optional<DirectoryEntry> selected = (*self).selected_entry();
            if (selected && (*selected).stable_id == entry.stable_id) {
                (*(*self).property_list_).set_value(
                    "fm.property.name", entry.name);
            }
        }
        (*self).update_mutation_controls();
        (*self).apply_operation(std::move(result));
    }
};

struct Application::PropertyRenameWork final {
    std::shared_ptr<Application> self{};
    DirectoryEntry entry{};
    std::string basename{};
    void operator()() {
        OperationResult result = (*(*self).operations_).rename_object(
            entry.path, entry.identity, basename);
        (*self).post_ui(PropertyRenameReady{self, std::move(entry), std::move(result)});
    }
};

struct Application::TransferReady final {
    std::shared_ptr<Application> self{};
    OperationResult result{};
    std::uint64_t generation{};
    void operator()() {
        (*self).apply_transfer(std::move(result), generation);
    }
};

struct Application::TransferCancelled final {
    std::shared_ptr<Application> self{};
    std::uint64_t generation{};
    bool operator()() const {
        const bool cancelled = (*self).stopping_.load() ||
            (*self).transfer_generation_.load() != generation;
        return cancelled;
    }
};

struct Application::TransferWork final {
    std::shared_ptr<Application> self{};
    PendingTransfer transfer{};
    std::filesystem::path destination{};
    std::uint64_t generation{};
    void operator()() const {
        OperationResult result{};
        if (transfer.move) {
            result = (*(*self).operations_).move_object(
                transfer.entry.path, transfer.entry.identity, destination);
        } else {
            result = (*(*self).operations_).copy_object(
                transfer.entry.path, transfer.entry.identity, destination,
                TransferCancelled{self, generation});
        }
        (*self).post_ui(TransferReady{self, std::move(result), generation});
    }
};

struct Application::InternalDropReady final {
    std::shared_ptr<Application> self{};
    OperationResult result{};
    std::uint64_t generation{};
    void operator()() {
        (*self).apply_transfer(std::move(result), generation);
    }
};

struct Application::InternalDropCancelled final {
    std::shared_ptr<Application> self{};
    std::uint64_t generation{};
    bool operator()() const {
        const bool cancelled = (*self).stopping_.load() ||
            (*self).transfer_generation_.load() != generation;
        return cancelled;
    }
};

struct Application::InternalDropWork final {
    std::shared_ptr<Application> self{};
    DirectoryEntry source{};
    DirectoryEntry destination{};
    bool copy{};
    std::uint64_t generation{};
    void operator()() const {
        OperationResult result{};
        if (copy) {
            result = (*(*self).operations_).copy_object(
                source.path, source.identity, destination.path,
                InternalDropCancelled{self, generation});
        } else {
            result = (*(*self).operations_).move_object(
                source.path, source.identity, destination.path);
        }
        (*self).post_ui(InternalDropReady{self, std::move(result), generation});
    }
};

struct Application::QuarantineReady final {
    std::shared_ptr<Application> self{};
    OperationResult result{};
    void operator()() {
        (*self).apply_operation(std::move(result));
    }
};

struct Application::QuarantineWork final {
    std::shared_ptr<Application> self{};
    DirectoryEntry entry{};
    void operator()() const {
        OperationResult result = (*(*self).operations_).quarantine_object(
            entry.path, entry.identity);
        (*self).post_ui(QuarantineReady{self, std::move(result)});
    }
};

struct Application::UndoReady final {
    std::shared_ptr<Application> self{};
    OperationResult result{};
    void operator()() {
        (*self).apply_operation(std::move(result));
    }
};

struct Application::UndoWork final {
    std::shared_ptr<Application> self{};
    void operator()() const {
        OperationResult result = (*(*self).operations_).undo_last();
        (*self).post_ui(UndoReady{self, std::move(result)});
    }
};

} // namespace file_manager
