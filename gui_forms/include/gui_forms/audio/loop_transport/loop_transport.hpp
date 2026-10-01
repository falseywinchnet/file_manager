#pragma once
#include <cstdint>
#include <memory>

namespace gui_forms {
class AudioClip;
class AudioEngine;
struct AudioLoopTransportState;
enum class AudioLoopStatus { ok, closed, wrong_thread, invalid_value, quota_exceeded, overflow, backend_error, expired, busy, unsupported };
enum class AudioLoopPhase { queued, admitted, applied, cancelled, replaced, too_late, expired_target, rejected, closed };
struct AudioLoopChange final {
    std::uint64_t bar_frames{}; // Incoming clip's grid; the current clip determines this transition.
    std::uint64_t lead_frames{3840};
    std::uint64_t fade_frames{2400};
};
struct AudioLoopCommand final { AudioLoopStatus status{AudioLoopStatus::closed}; std::uint64_t id{}; };
struct AudioLoopReceipt final {
    AudioLoopStatus status{AudioLoopStatus::expired};
    AudioLoopStatus reason{AudioLoopStatus::ok};
    AudioLoopPhase phase{AudioLoopPhase::queued};
    std::uint64_t id{}, epoch{}, admission_frame{}, application_frame{};
};
// Development profile: fixed-rate stereo 48 kHz loops, no seek or pitch.
// One control thread creates, commands, polls and destroys this owner. Commands
// are enqueued, then admitted at an audio read boundary; poll distinguishes them.
// The current source grid selects the first boundary strictly after admission+
// lead, including its actual loop end. A new source starts at frame zero there.
// Pause freezes transport time, but commands still process. Empty unpaused time
// advances. Stop preserves pause/time; only a new engine transport gets an epoch.
// 16 commands, 32 retained clip slots, 64 bounded receipts. Full admission leaves
// prior work unchanged. Poll/command calls reclaim retired clips on this thread.
// Clip lengths are 1..28,800,000 frames; bar is 1..incoming clip length, lead
// 0..480,000 and fade 0..48,000. For N>=2 the old track gains 1-j/(N-1) on
// samples j=0..N-1 while the new track has full gain. N=0/1 cuts immediately.
// Mixing overlap can clip. A change during a fade replaces only the pending
// request, preserves its original admission+lead cutoff and waits for fade end.
// Poll observes engine failure for unfinished receipts without rewriting their
// phase or timing. Device loss/interruption and mixer errors report backend_error;
// closed remains distinct. Already-terminal receipts preserve their outcome.
// Applied is terminal at the first incoming sample; cancellation then is too
// late. Expired receipt history gives expired_target, not an inferred result.
// Destruction and move assignment require the creating control thread. No
// public offline AudioEngine::render may overlap control or device operations.
class AudioLoopTransport final {
public:
    AudioLoopTransport();
    ~AudioLoopTransport();
    AudioLoopTransport(AudioLoopTransport&&) noexcept;
    AudioLoopTransport& operator=(AudioLoopTransport&&) noexcept;
    AudioLoopTransport(const AudioLoopTransport&) = delete;
    AudioLoopTransport& operator=(const AudioLoopTransport&) = delete;
    [[nodiscard]] AudioLoopCommand change(std::shared_ptr<const AudioClip> clip, AudioLoopChange timing);
    [[nodiscard]] AudioLoopCommand pause();
    [[nodiscard]] AudioLoopCommand resume();
    [[nodiscard]] AudioLoopCommand stop();
    [[nodiscard]] AudioLoopCommand set_gain(double gain);
    [[nodiscard]] AudioLoopCommand cancel(std::uint64_t id);
    [[nodiscard]] AudioLoopReceipt poll(std::uint64_t id);
    [[nodiscard]] AudioLoopStatus close();
private:
    friend class AudioEngine;
    friend struct AudioLoopTransportTestAccess;
    std::unique_ptr<AudioLoopTransportState> state_{};
};
}
