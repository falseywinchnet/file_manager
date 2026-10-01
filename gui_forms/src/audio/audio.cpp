#include "gui_forms/audio/audio.hpp"
#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"
#include <algorithm>
#include <array>
#include <atomic>
#include <bit>
#include <cmath>
#include <fstream>
#include <limits>

namespace gui_forms {
namespace {
constexpr std::uint64_t maximum_frames = 28800000;
constexpr std::uint64_t maximum_clip_bytes = maximum_frames * 2 * sizeof(float);
constexpr std::uint64_t clip_budget = 512ULL * 1024 * 1024;
constexpr std::uint64_t file_limit = 256ULL * 1024 * 1024;
std::atomic<std::uint64_t> clip_bytes{0};
bool charge_clip(std::uint64_t bytes) {
    std::uint64_t current = clip_bytes.load();
    while (current <= clip_budget && bytes <= clip_budget - current) {
        const bool accepted = clip_bytes.compare_exchange_weak(current, current + bytes);
        if (accepted) {
            return true;
        }
    }
    return false;
}
std::uint32_t little32(const unsigned char* bytes) {
    const std::uint32_t value = std::uint32_t(bytes[0]) | (std::uint32_t(bytes[1]) << 8) |
           (std::uint32_t(bytes[2]) << 16) | (std::uint32_t(bytes[3]) << 24);
    return value;
}
std::uint16_t little16(const unsigned char* bytes) {
    const std::uint16_t value = std::uint16_t(std::uint16_t(bytes[0]) | (std::uint16_t(bytes[1]) << 8));
    return value;
}
bool tag(const unsigned char* bytes, const char* text) {
    const bool matches = bytes[0] == text[0] && bytes[1] == text[1] && bytes[2] == text[2] && bytes[3] == text[3];
    return matches;
}
bool decode_pcm16(const unsigned char* bytes, std::span<float> output) {
    for (std::size_t i = 0; i < output.size(); ++i) {
        const std::uint16_t encoded = little16(bytes + i * 2);
        const std::int16_t sample = std::bit_cast<std::int16_t>(encoded);
        output[i] = static_cast<float>(sample) / 32768.0f;
    }
    return true;
}
bool decode_float32(const unsigned char* bytes, std::span<float> output) {
    for (std::size_t i = 0; i < output.size(); ++i) {
        const std::uint32_t encoded = little32(bytes + i * 4);
        const float sample = std::bit_cast<float>(encoded);
        if (!std::isfinite(sample) || sample < -1 || sample > 1) { return false; }
        output[i] = sample;
    }
    return true;
}
}

AudioClip::~AudioClip() { clip_bytes.fetch_sub(charged_bytes_); }
std::span<const float> AudioClip::samples() const noexcept {
    const std::span<const float> view(samples_.get(), sample_count_);
    return view;
}
std::uint64_t AudioClip::frames() const noexcept {
    const std::uint64_t count = sample_count_ / 2;
    return count;
}
AudioClipResult AudioClip::copy(std::span<const float> stereo) {
    if (stereo.empty() || stereo.size() % 2 != 0 || stereo.size_bytes() > maximum_clip_bytes) {
        return {{}, AudioStatus::invalid_format};
    }
    for (float sample : stereo) {
        if (!std::isfinite(sample) || sample < -1 || sample > 1) {
            return {{}, AudioStatus::invalid_value};
        }
    }
    std::uint64_t bytes = stereo.size_bytes();
    const bool accepted = charge_clip(bytes);
    if (!accepted) {
        return {{}, AudioStatus::quota_exceeded};
    }
    bool owned_charge = false;
    try {
        std::shared_ptr<AudioClip> clip(new AudioClip);
        AudioClip& owned = *clip;
        owned.charged_bytes_ = bytes;
        owned_charge = true;
        owned.samples_ = std::make_unique<float[]>(stereo.size());
        owned.sample_count_ = stereo.size();
        std::copy(stereo.begin(), stereo.end(), owned.samples_.get());
        AudioClipResult result{std::move(clip), AudioStatus::ok};
        return result;
    } catch (const std::bad_alloc&) {
        if (!owned_charge) {
            clip_bytes.fetch_sub(bytes);
        }
        return {{}, AudioStatus::allocation_failed};
    }
}

AudioClipResult AudioClip::load_wav(const std::filesystem::path& path, std::uint64_t trim_frames) {
    try {
        std::error_code error{};
        std::uint64_t file_bytes = std::filesystem::file_size(path, error);
        if (error) { return {{}, AudioStatus::file_error}; }
        if (file_bytes < 44 || file_bytes > file_limit) { return {{}, AudioStatus::invalid_format}; }
        std::ifstream file(path, std::ios::binary);
        std::array<unsigned char, 64> header{};
        file.read(reinterpret_cast<char*>(header.data()), 12);
        if (!file || !tag(header.data(), "RIFF") || !tag(header.data() + 8, "WAVE") ||
            std::uint64_t(little32(header.data() + 4)) + 8 != file_bytes) {
            return {{}, AudioStatus::invalid_format};
        }
        std::uint64_t position = 12, data_offset = 0, data_bytes = 0;
        unsigned sample_bytes = 0;
        bool format_seen = false;
        while (position + 8 <= file_bytes) {
            file.seekg(static_cast<std::streamoff>(position));
            file.read(reinterpret_cast<char*>(header.data()), 8);
            std::uint64_t chunk_bytes = little32(header.data() + 4);
            if (!file || chunk_bytes > file_bytes - position - 8) {
                return {{}, AudioStatus::invalid_format};
            }
            if (tag(header.data(), "fmt ")) {
                if (format_seen || chunk_bytes < 16 || chunk_bytes > header.size()) {
                    return {{}, AudioStatus::invalid_format};
                }
                file.read(reinterpret_cast<char*>(header.data()), static_cast<std::streamsize>(chunk_bytes));
                unsigned encoding = little16(header.data());
                unsigned bits = little16(header.data() + 14);
                sample_bytes = bits / 8;
                if (!file || little16(header.data() + 2) != 2 || little32(header.data() + 4) != 48000 ||
                    !((encoding == 1 && bits == 16) || (encoding == 3 && bits == 32)) ||
                    little16(header.data() + 12) != sample_bytes * 2 ||
                    little32(header.data() + 8) != sample_bytes * 2 * 48000) {
                    return {{}, AudioStatus::invalid_format};
                }
                format_seen = true;
            } else if (tag(header.data(), "data")) {
                if (data_offset != 0) { return {{}, AudioStatus::invalid_format}; }
                data_offset = position + 8;
                data_bytes = chunk_bytes;
            }
            position += 8 + chunk_bytes + (chunk_bytes & 1);
        }
        if (!format_seen || data_offset == 0 || data_bytes == 0 || position != file_bytes ||
            data_bytes % (sample_bytes * 2) != 0) { return {{}, AudioStatus::invalid_format}; }
        std::uint64_t frames = data_bytes / (sample_bytes * 2);
        if (frames > maximum_frames || trim_frames > frames) { return {{}, AudioStatus::invalid_format}; }
        // One bounded conversion workspace, independent of the live-clip budget.
        // No whole encoded file allocation; extra workspace <=230,400,000 bytes.
        std::vector<float> decoded(static_cast<std::size_t>(frames * 2));
        file.seekg(static_cast<std::streamoff>(data_offset));
        std::array<unsigned char, 8192> block{};
        using DecodeKernel = bool (*)(const unsigned char*, std::span<float>);
        const DecodeKernel decode = sample_bytes == 2 ? &decode_pcm16 : &decode_float32;
        std::size_t sample_index = 0;
        while (sample_index < decoded.size()) {
            std::size_t count = std::min(decoded.size() - sample_index, block.size() / sample_bytes);
            file.read(reinterpret_cast<char*>(block.data()), static_cast<std::streamsize>(count * sample_bytes));
            if (!file) { return {{}, AudioStatus::file_error}; }
            std::span<float> destination(decoded.data() + sample_index, count);
            const bool valid = decode(block.data(), destination);
            if (!valid) { return {{}, AudioStatus::invalid_value}; }
            sample_index += count;
        }
        std::size_t count = static_cast<std::size_t>((trim_frames == 0 ? frames : trim_frames) * 2);
        const std::span<const float> trimmed(decoded.data(), count);
        AudioClipResult result = copy(trimmed);
        return result;
    } catch (const std::bad_alloc&) {
        return {{}, AudioStatus::allocation_failed};
    } catch (const std::filesystem::filesystem_error&) {
        return {{}, AudioStatus::file_error};
    } catch (const std::ios_base::failure&) {
        return {{}, AudioStatus::file_error};
    }
}

struct AudioEngineState final {
    ma_engine engine{};
    ma_device device{};
    std::array<AudioVoiceState*, 64> voices{}; // Non-owning, control-thread only.
    std::atomic<AudioStatus> status{AudioStatus::closed};
    bool initialized{false}, device_initialized{false}, offline{false};
    void callback_failure(AudioStatus failure) {
        AudioStatus expected = AudioStatus::ok;
        status.compare_exchange_strong(expected, failure);
    }
    void mix(std::span<float> stereo) {
        std::size_t offset = 0;
        while (offset < stereo.size()) {
            std::size_t count = std::min<std::size_t>(8192, stereo.size() - offset);
            ma_result result = ma_engine_read_pcm_frames(&engine, stereo.data() + offset, count / 2, nullptr);
            if (result != MA_SUCCESS) {
                callback_failure(AudioStatus::backend_error);
                std::fill(stereo.begin() + static_cast<std::ptrdiff_t>(offset), stereo.end(), 0);
                return;
            }
            for (std::size_t i = offset; i < offset + count; ++i) {
                stereo[i] = std::clamp(stereo[i], -1.0f, 1.0f);
            }
            offset += count;
        }
    }
    static void output(ma_device* device, void* output, const void*, ma_uint32 frames) {
        AudioEngineState& state = *static_cast<AudioEngineState*>((*device).pUserData);
        std::span<float> samples(static_cast<float*>(output), static_cast<std::size_t>(frames) * 2);
        // The backend fixed-size intermediary is configured to 256 frames.
        // Graph work is capped at 4096 frames. An unexpected larger native buffer
        // still requires a complete silence fill proportional to its supplied size.
        if (frames > 4096 || state.status.load() != AudioStatus::ok) {
            std::fill(samples.begin(), samples.end(), 0);
            if (frames > 4096) { state.callback_failure(AudioStatus::backend_error); }
            return;
        }
        state.mix(samples);
    }
    static void notification(const ma_device_notification* event) {
        AudioEngineState& state = *static_cast<AudioEngineState*>((*(*event).pDevice).pUserData);
        if ((*event).type == ma_device_notification_type_stopped ||
            (*event).type == ma_device_notification_type_interruption_began) {
            state.callback_failure(AudioStatus::device_unavailable);
        }
    }
};
struct AudioVoiceState final {
    std::shared_ptr<AudioEngineState> engine{};
    std::shared_ptr<const AudioClip> clip{};
    ma_audio_buffer buffer{};
    ma_sound sound{};
    std::size_t slot{};
    bool initialized{false};
    bool pitch_enabled{false};
    void close() {
        if (initialized) {
            // miniaudio detaches on the control thread and waits for graph readers.
            ma_sound_uninit(&sound);
            ma_audio_buffer_uninit(&buffer);
            initialized = false;
        }
        if (engine) { (*engine).voices[slot] = nullptr; }
    }
    ~AudioVoiceState() { close(); }
    bool usable() const {
        const bool available = initialized && (*engine).status.load() == AudioStatus::ok;
        return available;
    }
};

#ifdef GUI_FORMS_AUDIO_TESTING
// Private regression seam: model callback errors arriving after revocation.
AudioStatus audio_test_revoked_callback_status() {
    AudioEngineState state{};
    state.status.store(AudioStatus::closed);
    state.callback_failure(AudioStatus::backend_error);
    state.callback_failure(AudioStatus::device_unavailable);
    const AudioStatus result = state.status.load();
    return result;
}
#endif

AudioVoice::AudioVoice() = default;
AudioVoice::~AudioVoice() = default;
AudioVoice::AudioVoice(AudioVoice&&) noexcept = default;
AudioVoice& AudioVoice::operator=(AudioVoice&&) noexcept = default;
AudioStatus AudioVoice::play() {
    if (!state_ || !(*state_).usable()) { return AudioStatus::closed; }
    ma_result result = ma_sound_start(&(*state_).sound);
    const AudioStatus status = result == MA_SUCCESS ? AudioStatus::ok : AudioStatus::backend_error;
    return status;
}
AudioStatus AudioVoice::pause() {
    if (!state_ || !(*state_).usable()) { return AudioStatus::closed; }
    ma_result result = ma_sound_stop(&(*state_).sound);
    const AudioStatus status = result == MA_SUCCESS ? AudioStatus::ok : AudioStatus::backend_error;
    return status;
}
AudioStatus AudioVoice::stop() {
    AudioStatus result = pause();
    if (result != AudioStatus::ok) { return result; }
    ma_result seek = ma_sound_seek_to_pcm_frame(&(*state_).sound, 0);
    const AudioStatus status = seek == MA_SUCCESS ? AudioStatus::ok : AudioStatus::backend_error;
    return status;
}
AudioStatus AudioVoice::set_gain(double gain) {
    if (!std::isfinite(gain) || gain < 0 || gain > 1) { return AudioStatus::invalid_value; }
    if (!state_ || !(*state_).usable()) { return AudioStatus::closed; }
    ma_sound_set_volume(&(*state_).sound, static_cast<float>(gain));
    return AudioStatus::ok;
}
AudioStatus AudioVoice::set_rate(double rate) {
    if (!std::isfinite(rate) || rate < .25 || rate > 4) { return AudioStatus::invalid_value; }
    if (!state_ || !(*state_).usable()) { return AudioStatus::closed; }
    if (!(*state_).pitch_enabled && rate != 1) { return AudioStatus::invalid_value; }
    ma_sound_set_pitch(&(*state_).sound, static_cast<float>(rate));
    return AudioStatus::ok;
}
bool AudioVoice::playing() const {
    if (!state_) { return false; }
    const AudioVoiceState& state = *state_;
    const bool active = state.usable() && ma_sound_is_playing(&state.sound);
    return active;
}
AudioEngine::AudioEngine() = default;
AudioEngine::~AudioEngine() { shutdown(); }
AudioStatus AudioEngine::open(bool offline) {
    if (state_ && (*state_).initialized) { return AudioStatus::invalid_value; }
    try { state_ = std::make_shared<AudioEngineState>(); }
    catch (const std::bad_alloc&) { return AudioStatus::allocation_failed; }
    AudioEngineState& state = *state_;
    state.offline = offline;
    ma_engine_config config = ma_engine_config_init();
    config.noDevice = MA_TRUE;
    config.channels = 2;
    config.sampleRate = 48000;
    // No graph look-ahead cache: offline controls apply at the next render call.
    // The physical device separately supplies fixed 256-frame callback blocks.
    config.periodSizeInFrames = 0;
    const ma_result engine_result = ma_engine_init(&config, &state.engine);
    if (engine_result != MA_SUCCESS) {
        state.status.store(AudioStatus::backend_error);
        return AudioStatus::backend_error;
    }
    state.initialized = true;
    state.status.store(AudioStatus::ok);
    if (!offline) {
        ma_device_config device = ma_device_config_init(ma_device_type_playback);
        device.playback.format = ma_format_f32;
        device.playback.channels = 2;
        device.sampleRate = 48000;
        device.periodSizeInFrames = 256;
        device.noFixedSizedCallback = MA_FALSE;
        device.dataCallback = &AudioEngineState::output;
        device.notificationCallback = &AudioEngineState::notification;
        device.pUserData = &state;
        const ma_result device_result = ma_device_init(nullptr, &device, &state.device);
        if (device_result != MA_SUCCESS) {
            shutdown();
            state.status.store(AudioStatus::device_unavailable);
            return AudioStatus::device_unavailable;
        }
        state.device_initialized = true;
        const ma_result start_result = ma_device_start(&state.device);
        if (start_result != MA_SUCCESS) {
            shutdown();
            state.status.store(AudioStatus::device_unavailable);
        }
    }
    const AudioStatus result = state.status.load();
    return result;
}
AudioStatus AudioEngine::voice(std::shared_ptr<const AudioClip> clip, bool loop, AudioVoice& output,
                               bool pitch_enabled) {
    if (!state_ || status() != AudioStatus::ok) { return AudioStatus::closed; }
    if (!clip || (*clip).frames() == 0) { return AudioStatus::invalid_value; }
    AudioEngineState& engine = *state_;
    std::size_t slot = 0;
    while (slot < engine.voices.size() && engine.voices[slot] != nullptr) { ++slot; }
    if (slot == engine.voices.size()) { return AudioStatus::quota_exceeded; }
    try {
        std::unique_ptr<AudioVoiceState> candidate = std::make_unique<AudioVoiceState>();
        AudioVoiceState& state = *candidate;
        state.engine = state_;
        state.clip = std::move(clip);
        state.slot = slot;
        state.pitch_enabled = pitch_enabled;
        ma_audio_buffer_config config = ma_audio_buffer_config_init(ma_format_f32, 2, (*state.clip).frames(),
                                                                    (*state.clip).samples().data(), nullptr);
        const ma_result buffer_result = ma_audio_buffer_init(&config, &state.buffer);
        if (buffer_result != MA_SUCCESS) { return AudioStatus::backend_error; }
        ma_uint32 flags = MA_SOUND_FLAG_NO_SPATIALIZATION;
        if (!pitch_enabled) { flags |= MA_SOUND_FLAG_NO_PITCH; }
        const ma_result sound_result = ma_sound_init_from_data_source(&engine.engine, &state.buffer, flags, nullptr, &state.sound);
        if (sound_result != MA_SUCCESS) {
            ma_audio_buffer_uninit(&state.buffer);
            return AudioStatus::backend_error;
        }
        state.initialized = true;
        ma_sound_set_looping(&state.sound, loop ? MA_TRUE : MA_FALSE);
        output.state_ = std::move(candidate);
        engine.voices[slot] = &(*output.state_);
        return AudioStatus::ok;
    } catch (const std::bad_alloc&) { return AudioStatus::allocation_failed; }
}
AudioStatus AudioEngine::render(std::span<float> stereo) {
    if (!state_ || status() != AudioStatus::ok) { return AudioStatus::closed; }
    if (!(*state_).offline || stereo.size() % 2 != 0) { return AudioStatus::invalid_value; }
    (*state_).mix(stereo);
    const AudioStatus result = status();
    return result;
}
AudioStatus AudioEngine::status() const noexcept {
    const AudioStatus result = state_ ? (*state_).status.load() : AudioStatus::closed;
    return result;
}
void AudioEngine::shutdown() {
    if (!state_) { return; }
    AudioEngineState& state = *state_;
    state.status.store(AudioStatus::closed);
    if (state.device_initialized) {
        ma_device_uninit(&state.device);
        state.device_initialized = false;
    }
    for (AudioVoiceState* voice : state.voices) {
        if (voice != nullptr) { (*voice).close(); }
    }
    if (state.initialized) {
        ma_engine_uninit(&state.engine);
        state.initialized = false;
    }
    state.status.store(AudioStatus::closed);
}
}
