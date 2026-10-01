#pragma once
#include <array>
#include <cstddef>
#include <cstdint>

// Serial development model only: no audio node, concurrent publication, clip
// ownership or public API. Indices stand in for separately owned clip payloads.
namespace gui_forms::audio_experiment {
enum class ScheduleStatus { ok, invalid, full, expired, overflow, closed };
enum class RequestPhase { empty, queued, admitted, applied, cancelled, replaced, too_late, expired_target, rejected, closed };
enum class CommandKind { change, pause, resume, stop, gain, cancel };
enum class PayloadPhase { free, retained, retired };
struct ClipShape final {
    std::uint64_t frames{};
    std::uint64_t bar_frames{};
};
struct ChangeTiming final {
    std::uint64_t lead_frames{3840};
    std::uint64_t fade_frames{2400};
};
struct RequestResult final { ScheduleStatus status{}; std::uint64_t id{}; std::size_t payload{32}; };
struct Receipt final {
    ScheduleStatus status{ScheduleStatus::expired};
    std::uint64_t id{}, epoch{}, admission_frame{}, application_frame{};
    RequestPhase phase{RequestPhase::empty};
    ScheduleStatus reason{ScheduleStatus::ok};
};
struct BoundaryResult final { ScheduleStatus status{}; std::uint64_t frame{}; };
struct SamplePlan final {
    static constexpr std::size_t no_payload = 32;
    std::uint64_t frame{};
    std::size_t current{no_payload}, outgoing{no_payload};
    std::uint64_t current_position{}, outgoing_position{};
    double current_gain{}, outgoing_gain{};
};
// First boundary >= available and strictly > cutoff. current_position is the
// source frame which would be read at render_frame. Loop-end is a boundary even
// when the loop length is not a multiple of bar_frames.
[[nodiscard]] BoundaryResult next_boundary(std::uint64_t render_frame,
    std::uint64_t current_position, ClipShape clip, std::uint64_t cutoff,
    std::uint64_t available) noexcept;
[[nodiscard]] double outgoing_weight(std::uint64_t index, std::uint64_t frames) noexcept;

class LoopScheduler final {
public:
    static constexpr std::size_t command_capacity = 16, payload_capacity = 32, receipt_capacity = 64;
    explicit LoopScheduler(std::uint64_t epoch = 1, std::uint64_t initial_frame = 0,
                           std::uint64_t first_id = 1) noexcept;
    [[nodiscard]] RequestResult change(ClipShape clip, ChangeTiming timing) noexcept;
    [[nodiscard]] RequestResult command(CommandKind kind, double gain = 1,
                                        std::uint64_t target = 0) noexcept;
    // Admission point: at most 16 commands, in order, at the current frame.
    void ingest() noexcept;
    [[nodiscard]] SamplePlan next_sample() noexcept;
    [[nodiscard]] Receipt poll(std::uint64_t id) const noexcept;
    // Models the producer observing a retirement acknowledgment after last use.
    std::size_t collect_retired() noexcept;
    void shutdown() noexcept;
    std::uint64_t frame() const noexcept { return frame_; }
    bool paused() const noexcept { return paused_; }
    bool closed() const noexcept { return closed_; }
    PayloadPhase payload_phase(std::size_t index) const noexcept;
    std::size_t retained_payloads() const noexcept;
private:
    struct Payload final { PayloadPhase phase{}; ClipShape clip{}; };
    struct Command final {
        CommandKind kind{};
        std::uint64_t id{}, target{};
        std::size_t payload{SamplePlan::no_payload};
        ChangeTiming timing{};
        double gain{1};
    };
    std::array<Command, command_capacity> commands_{};
    std::array<Payload, payload_capacity> payloads_{};
    std::array<Receipt, receipt_capacity> receipts_{};
    std::size_t head_{}, queued_{}, receipt_cursor_{};
    std::size_t current_{SamplePlan::no_payload}, outgoing_{SamplePlan::no_payload}, pending_{SamplePlan::no_payload};
    std::uint64_t current_position_{}, outgoing_position_{}, pending_id_{}, pending_boundary_{};
    std::uint64_t pending_fade_{}, fade_index_{}, fade_frames_{};
    std::uint64_t epoch_{}, frame_{}, next_id_{};
    double gain_{1};
    bool paused_{}, closed_{};
    [[nodiscard]] RequestResult enqueue(Command item) noexcept;
    void apply(Command item) noexcept;
    void finish(std::uint64_t id, RequestPhase phase) noexcept;
    void retire(std::size_t index) noexcept;
    void cancel_pending(RequestPhase phase) noexcept;
};
}
