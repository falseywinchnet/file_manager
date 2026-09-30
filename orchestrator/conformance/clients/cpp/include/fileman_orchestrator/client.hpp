#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace fileman::orchestrator {

inline constexpr std::uint16_t local_wire_major = 1;
inline constexpr std::uint16_t local_wire_minor = 0;
inline constexpr std::uint32_t local_wire_max_frame_bytes = 1'048'576;
inline constexpr std::size_t local_wire_max_client_name_bytes = 128;
inline constexpr std::uint16_t frontend_contract_major = 1;
inline constexpr std::uint16_t frontend_contract_minor = 0;
inline constexpr std::uint16_t source_api_major = 1;
inline constexpr std::uint16_t source_api_minor = 0;

[[nodiscard]] std::filesystem::path default_runtime_directory();

class ClientError final : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

struct SessionInfo {
    std::string instance_id;
    std::uint64_t lifecycle_generation{};
    std::uint32_t max_frame_bytes{};
};

struct VersionInfo {
    std::string component;
    std::string build_version;
    std::string protocol_family;
    std::uint16_t protocol_major{};
    std::uint16_t protocol_minor{};
    std::string local_wire_family;
    std::uint16_t local_wire_major{};
    std::uint16_t local_wire_minor{};
};

struct ReleaseRequirement {
    std::string id;
    std::string state;
    std::string evidence;
};

struct ReleaseProvenanceInfo {
    std::string algorithm;
    std::string scope;
    std::string digest;
    bool signed_manifest{};
    std::uint64_t embedded_inputs{};
};

struct ReleaseInfo {
    std::string profile;
    std::string target_version;
    std::string build_version;
    std::string first_platform;
    std::string state;
    bool ready{};
    std::vector<std::string> required_contracts;
    std::vector<ReleaseRequirement> requirements;
    ReleaseProvenanceInfo provenance;
};

struct AvailabilitySummary {
    std::uint64_t available{};
    std::uint64_t degraded{};
    std::uint64_t unavailable{};
    std::uint64_t negotiating{};
    std::uint64_t deferred{};
    std::uint64_t stubbed{};
};

struct StatusInfo {
    std::string component;
    std::string scope;
    std::string lifecycle_state;
    std::uint64_t lifecycle_generation{};
    std::string core_release_state;
    std::string core_target_version;
    bool core_ready{};
    bool lazy{};
    bool has_gui{};
    std::string engine_scope;
    std::string normal_integration_route;
    bool degraded_engine_fallback{};
    AvailabilitySummary availability;
};

struct ContractInfo {
    std::string id;
    std::string name;
    std::string provider;
    std::string stage;
    bool executable{};
};

struct AvailabilityInfo {
    std::string id;
    std::string provider;
    std::string state;
    std::string reason;
    bool required{};
};

struct SnapshotInfo {
    std::string kind;
    std::uint64_t lifecycle_generation{};
    std::uint64_t configuration_generation{};
};

struct DirectEngineFallbackInfo {
    bool registered_route{};
    bool eligible{};
    std::string state;
    std::string reason;
};

struct RoutingInfo {
    std::string normal_integration_route;
    std::string engine_scope;
    DirectEngineFallbackInfo direct_engine_fallback;
};

struct ServiceControlsInfo {
    bool shutdown_eligible{};
    bool restart_eligible{};
    std::string restart_strategy;
    std::string restart_effect;
    std::string diagnostics_state;
    std::optional<std::string> diagnostics_locator;
};

struct FrontendGateBlockerInfo {
    std::string kind;
    std::string id;
    std::string state;
    std::string reason;
};

struct FrontendOrchestratorGateInfo {
    std::string authority;
    std::string state;
    bool satisfied{};
    std::vector<FrontendGateBlockerInfo> blockers;
};

struct FrontendExternalGateInfo {
    std::string authority;
    std::optional<std::string> evidence_capability_id;
    std::string state;
    std::optional<bool> satisfied;
};

struct FrontendOpeningPolicyInfo {
    bool live_snapshot_required{};
    bool separately_gated_provider_absence_blocks_opening{};
    std::string stale_snapshot_authority;
};

struct FrontendOpeningInfo {
    FrontendOrchestratorGateInfo orchestrator_gate;
    FrontendExternalGateInfo gui_forms_gate;
    FrontendExternalGateInfo architect_direction_gate;
    FrontendOpeningPolicyInfo policy;
};

struct BootstrapSnapshot {
    SessionInfo session;
    std::string schema_family;
    std::uint16_t schema_major{};
    std::uint16_t schema_minor{};
    SnapshotInfo snapshot;
    VersionInfo version;
    ReleaseInfo release;
    StatusInfo status;
    std::vector<ContractInfo> contracts;
    std::vector<AvailabilityInfo> availability;
    RoutingInfo routing;
    ServiceControlsInfo service_controls;
    FrontendOpeningInfo frontend_opening;

    [[nodiscard]] bool orchestrator_gate_ready() const noexcept;
};

struct SearchResultInfo {
    std::string name;
    std::filesystem::path path;
    std::string kind;
    std::uint64_t size{};
    bool unavailable{};
};

struct SearchCursorInfo {
    std::string source;
    std::string value;
};

struct SearchExactFilter {
    std::string field;
    std::string value;
};

struct SearchPageInfo {
    std::string terminal;
    std::string source;
    bool complete{};
    std::optional<std::uint64_t> generation;
    std::optional<SearchCursorInfo> cursor;
    std::vector<SearchResultInfo> results;
    std::vector<std::string> names;
};

using SettingValue = std::variant<bool, std::uint64_t, std::string>;

struct SettingSchemaFieldInfo {
    std::string id;
    std::string name_space;
    std::string presentation_tab;
    std::string label_key;
    std::string value_type;
    SettingValue default_value;
    std::optional<std::uint64_t> minimum;
    std::optional<std::uint64_t> maximum;
    std::vector<std::string> choices;
    std::string restart_effect;
    std::string availability;
    std::string availability_reason;
};

struct SettingsSchemaInfo {
    std::string schema_revision;
    std::vector<SettingSchemaFieldInfo> fields;
};

struct SettingValueInfo {
    std::string id;
    SettingValue value;
};

struct SettingsSnapshotInfo {
    std::string schema_revision;
    std::uint64_t revision{};
    std::string recovery_provenance;
    std::vector<SettingValueInfo> values;

    [[nodiscard]] const SettingValue* find(std::string_view id) const noexcept;
};

struct SettingsCommitInfo {
    std::vector<std::string> changed_fields;
    bool restart_required{};
    std::string audit_id;
    SettingsSnapshotInfo snapshot;
};

struct SettingChange {
    std::string id;
    SettingValue value;
};

struct ServiceCommandInfo {
    std::string id;
    std::string title;
    bool available{};
    std::string effect;
};

struct ServiceInfo {
    std::string id;
    std::string title;
    std::string state;
    bool ready{};
    std::optional<std::string> instance_id;
    std::optional<std::uint64_t> generation;
    std::string transport;
    std::optional<std::string> currentness;
    std::vector<std::string> roots;
    std::optional<std::string> reason;
    std::vector<ServiceCommandInfo> commands;
};

struct ServicesSnapshotInfo {
    std::uint16_t schema_major{};
    std::uint16_t schema_minor{};
    std::string snapshot_kind;
    std::vector<ServiceInfo> services;

    [[nodiscard]] const ServiceInfo* find(std::string_view id) const noexcept;
};

struct ServiceCommandResultInfo {
    std::string service_id;
    std::string command_id;
    std::string terminal;
    std::string effect;
};

class Client final {
public:
    static Client connect(const std::filesystem::path& runtime_directory,
                          std::string client_name = "fileman-cpp-conformance");
    static Client connect_default(std::string client_name = "fileman-cpp-conformance");

    Client(Client&& other) noexcept;
    Client& operator=(Client&& other) noexcept;
    Client(const Client&) = delete;
    Client& operator=(const Client&) = delete;
    ~Client();

    [[nodiscard]] const SessionInfo& session() const noexcept;
    [[nodiscard]] VersionInfo version();
    [[nodiscard]] ReleaseInfo release();
    [[nodiscard]] StatusInfo status();
    [[nodiscard]] std::vector<ContractInfo> contracts();
    [[nodiscard]] std::vector<AvailabilityInfo> availability();
    [[nodiscard]] BootstrapSnapshot bootstrap();
    [[nodiscard]] SearchPageInfo search(std::string root_id, std::string text);
    [[nodiscard]] SearchPageInfo search_subtree(
        std::string root_id,
        std::optional<std::string> relative_path,
        std::string text,
        std::uint32_t maximum_results = 128,
        std::optional<SearchCursorInfo> cursor = std::nullopt,
        std::vector<SearchExactFilter> filters = {});
    [[nodiscard]] SettingsSchemaInfo settings_schema();
    [[nodiscard]] SettingsSnapshotInfo settings_snapshot();
    [[nodiscard]] SettingsCommitInfo apply_setting(std::uint64_t expected_revision,
                                                   std::string id,
                                                   SettingValue value);
    [[nodiscard]] SettingsCommitInfo apply_settings(
        std::uint64_t expected_revision,
        std::vector<SettingChange> changes);
    [[nodiscard]] SettingsCommitInfo reset_setting(std::uint64_t expected_revision,
                                                   std::string id);
    [[nodiscard]] ServicesSnapshotInfo services_snapshot();
    [[nodiscard]] ServiceCommandResultInfo service_command(
        std::string service_id,
        std::string command_id,
        std::optional<std::string> expected_instance_id = std::nullopt,
        std::optional<std::uint64_t> expected_generation = std::nullopt,
        std::optional<std::string> root_id = std::nullopt);
    void shutdown();

private:
    explicit Client(std::intptr_t socket, SessionInfo session) noexcept;
    static Client connect_once(const std::filesystem::path& runtime_directory,
                               std::string client_name);
    [[nodiscard]] std::string call(std::string_view method,
                                   std::string_view contract_id,
                                   std::uint16_t contract_major,
                                   std::uint16_t contract_minor,
                                   std::string_view params_json = "{}",
                                   bool allow_partial = false);

    std::intptr_t socket_{-1};
    std::uint64_t next_request_id_{1};
    SessionInfo session_;
};

}  // namespace fileman::orchestrator
