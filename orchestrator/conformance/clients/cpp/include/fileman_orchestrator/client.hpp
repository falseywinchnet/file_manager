#pragma once

#include <cstdint>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <vector>

namespace fileman::orchestrator {

inline constexpr std::uint16_t local_wire_major = 0;
inline constexpr std::uint16_t local_wire_minor = 1;
inline constexpr std::uint32_t local_wire_max_frame_bytes = 1'048'576;

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
};

struct ReleaseRequirement {
    std::string id;
    std::string state;
    std::string evidence;
};

struct ReleaseInfo {
    std::string profile;
    std::string target_version;
    std::string build_version;
    std::string state;
    bool ready{};
    std::vector<std::string> required_contracts;
    std::vector<ReleaseRequirement> requirements;
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
    bool degraded_engine_fallback{};
    AvailabilitySummary availability;
};

struct AvailabilityInfo {
    std::string id;
    std::string provider;
    std::string state;
    std::string reason;
    bool required{};
};

struct BootstrapSnapshot {
    SessionInfo session;
    VersionInfo version;
    ReleaseInfo release;
    StatusInfo status;
    std::vector<AvailabilityInfo> availability;
};

class Client final {
public:
    static Client connect(const std::filesystem::path& runtime_directory,
                          std::string client_name = "fileman-cpp-conformance");

    Client(Client&& other) noexcept;
    Client& operator=(Client&& other) noexcept;
    Client(const Client&) = delete;
    Client& operator=(const Client&) = delete;
    ~Client();

    [[nodiscard]] const SessionInfo& session() const noexcept;
    [[nodiscard]] VersionInfo version();
    [[nodiscard]] ReleaseInfo release();
    [[nodiscard]] StatusInfo status();
    [[nodiscard]] std::vector<AvailabilityInfo> availability();
    [[nodiscard]] BootstrapSnapshot bootstrap();
    void shutdown();

private:
    explicit Client(int socket, SessionInfo session) noexcept;
    [[nodiscard]] std::string call(std::string_view method);

    int socket_{-1};
    std::uint64_t next_request_id_{1};
    SessionInfo session_;
};

}  // namespace fileman::orchestrator
