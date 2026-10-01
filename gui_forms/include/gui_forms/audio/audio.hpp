#pragma once
#include <cstdint>
#include <filesystem>
#include <memory>
#include <span>
#include <vector>

namespace gui_forms {
enum class AudioStatus {
    ok, invalid_format, invalid_value, file_error, allocation_failed,
    quota_exceeded, device_unavailable, backend_error, closed
};
class AudioClip;
struct AudioClipResult final {
    std::shared_ptr<const AudioClip> clip{};
    AudioStatus status{AudioStatus::ok};
};
// Immutable interleaved L,R samples, 48 kHz. Up to 600 seconds per clip;
// aggregate live clip payload is limited to 512 MiB, including external owners.
class AudioClip final {
public:
    ~AudioClip();
    AudioClip(const AudioClip&) = delete;
    AudioClip& operator=(const AudioClip&) = delete;
    [[nodiscard]] static AudioClipResult copy(std::span<const float> stereo);
    // Synchronous bounded file I/O: never call on the device callback or a UI
    // latency-sensitive path. Little-endian RIFF PCM16/float32 stereo 48 kHz only.
    [[nodiscard]] static AudioClipResult load_wav(const std::filesystem::path& path,
                                  std::uint64_t trim_frames = 0);
    std::span<const float> samples() const noexcept;
    std::uint64_t frames() const noexcept;
private:
    AudioClip() = default;
    std::unique_ptr<float[]> samples_{};
    std::size_t sample_count_{};
    std::uint64_t charged_bytes_{};
};

struct AudioEngineState;
struct AudioVoiceState;
class AudioVoice final {
public:
    AudioVoice();
    ~AudioVoice();
    AudioVoice(AudioVoice&&) noexcept;
    AudioVoice& operator=(AudioVoice&&) noexcept;
    AudioVoice(const AudioVoice&) = delete;
    AudioVoice& operator=(const AudioVoice&) = delete;
    AudioStatus play(); // Resume; at EOF seek to zero before replaying.
    AudioStatus pause(); // Retain cursor.
    AudioStatus stop(); // Pause and rewind to frame zero.
    AudioStatus set_gain(double gain); // Finite [0,1]. Mix clips to [-1,1].
    AudioStatus set_rate(double rate); // Finite [.25,4], linear interpolation.
    bool playing() const;
private:
    friend class AudioEngine;
    std::unique_ptr<AudioVoiceState> state_{};
};

// All public engine/voice operations belong to one control thread. The device
// callback is private. Offline render is exclusive with controls and device use.
// At most 64 live voice objects, including paused/stopped voices. No streaming.
class AudioEngine final {
public:
    AudioEngine();
    ~AudioEngine();
    AudioEngine(const AudioEngine&) = delete;
    AudioEngine& operator=(const AudioEngine&) = delete;
    AudioStatus open(bool offline = false);
    // Pitch-enabled voices use a linear interpolator with one input-frame latency.
    // Fixed-rate voices bypass it, preserving exact PCM sample positions.
    AudioStatus voice(std::shared_ptr<const AudioClip> clip, bool loop, AudioVoice& output,
                      bool pitch_enabled = false);
    AudioStatus render(std::span<float> stereo);
    AudioStatus status() const noexcept;
    // Immediately stop output, quiesce device callback, then release backend
    // voices. Held voice handles become closed. No audible drain or auto-reopen.
    void shutdown();
private:
    std::shared_ptr<AudioEngineState> state_{};
};
}
