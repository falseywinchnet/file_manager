#include "gui_forms/audio/audio.hpp"
#include <array>
#include <cmath>
#include <chrono>
#include <fstream>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <vector>

#ifdef GUI_FORMS_AUDIO_TESTING
namespace gui_forms { AudioStatus audio_test_revoked_callback_status(); }
#endif

namespace {
void require(bool condition, const char* message) {
    if (!condition) { throw std::runtime_error(message); }
}
void controls_and_lifetime() {
    using gui_forms::AudioStatus;
    const std::array<float, 8> source{.1f, -.1f, .2f, -.2f, .3f, -.3f, .4f, -.4f};
    gui_forms::AudioClipResult clip = gui_forms::AudioClip::copy(source);
    require(clip.status == AudioStatus::ok, "clip admitted");
    gui_forms::AudioEngine engine{};
    const gui_forms::AudioStatus open_engine = engine.open(true);
    require(open_engine == AudioStatus::ok, "offline engine");
    gui_forms::AudioVoice voice{};
    const gui_forms::AudioStatus admit_voice = engine.voice(clip.clip, true, voice);
    require(admit_voice == AudioStatus::ok, "voice admitted");
    const gui_forms::AudioStatus start_voice = voice.play();
    require(start_voice == AudioStatus::ok, "start");
    std::array<float, 40> result{};
    const gui_forms::AudioStatus render_loop = engine.render(result);
    require(render_loop == AudioStatus::ok, "render");
    for (std::size_t i = 0; i < result.size(); ++i) {
        if (std::abs(result[i] - source[i % source.size()]) >= 1e-6f) {
            std::cerr << "sample " << i << ": " << result[i] << " expected " << source[i % source.size()] << '\n';
        }
        require(std::abs(result[i] - source[i % source.size()]) < 1e-6f, "exact loop seam");
    }
    const gui_forms::AudioStatus pause_voice = voice.pause();
    require(pause_voice == AudioStatus::ok, "pause");
    const gui_forms::AudioStatus render_silence = engine.render(result);
    require(render_silence == AudioStatus::ok, "paused render");
    for (float sample : result) { require(sample == 0, "paused silence"); }
    std::array<float, 2> one_frame{};
    const gui_forms::AudioStatus resume_voice = voice.play();
    require(resume_voice == AudioStatus::ok, "resume before partial frame check");
    const gui_forms::AudioStatus first_frame = engine.render(one_frame);
    require(first_frame == AudioStatus::ok && one_frame[0] == source[0], "first frame");
    const gui_forms::AudioStatus pause_partial = voice.pause();
    require(pause_partial == AudioStatus::ok, "pause partway through loop");
    const gui_forms::AudioStatus render_paused = engine.render(result);
    require(render_paused == AudioStatus::ok, "time passes while paused");
    const gui_forms::AudioStatus resume_partial = voice.play();
    require(resume_partial == AudioStatus::ok, "resume retains position");
    const gui_forms::AudioStatus retained_frame = engine.render(one_frame);
    require(retained_frame == AudioStatus::ok && one_frame[0] == source[2], "paused cursor held");
    const gui_forms::AudioStatus rewind_voice = voice.stop();
    const gui_forms::AudioStatus restart_voice = voice.play();
    require(rewind_voice == AudioStatus::ok && restart_voice == AudioStatus::ok, "rewind and resume");
    const gui_forms::AudioStatus render_rewound = engine.render(result);
    require(render_rewound == AudioStatus::ok, "rewound render");
    require(std::abs(result[0] - source[0]) < 1e-6f, "rewind is frame zero");
    const gui_forms::AudioStatus negative_gain = voice.set_gain(-.1);
    require(negative_gain == AudioStatus::invalid_value, "negative gain rejected");
    const gui_forms::AudioStatus nan_rate = voice.set_rate(std::numeric_limits<double>::quiet_NaN());
    require(nan_rate == AudioStatus::invalid_value,
            "NaN rate rejected");
    const gui_forms::AudioStatus fixed_rate = voice.set_rate(2);
    require(fixed_rate == AudioStatus::invalid_value, "fixed-rate voice preserves sample positions");
    gui_forms::AudioVoice pitched{};
    const gui_forms::AudioStatus admit_pitched = engine.voice(clip.clip, true, pitched, true);
    require(admit_pitched == AudioStatus::ok, "pitch voice");
    const gui_forms::AudioStatus set_pitch = pitched.set_rate(2);
    const gui_forms::AudioStatus start_pitched = pitched.play();
    require(set_pitch == AudioStatus::ok && start_pitched == AudioStatus::ok, "pitch ratio admitted");
    const gui_forms::AudioStatus render_pitched = engine.render(result);
    require(render_pitched == AudioStatus::ok, "pitch render");
    for (float sample : result) { require(std::isfinite(sample) && std::abs(sample) <= 1, "bounded pitch output"); }
    std::array<float, 3> odd{};
    const gui_forms::AudioStatus odd_render = engine.render(odd);
    require(odd_render == AudioStatus::invalid_value, "odd sample span rejected");
    engine.shutdown();
    const gui_forms::AudioStatus revoked_play = voice.play();
    require(revoked_play == AudioStatus::closed, "retained voice revoked");
    const gui_forms::AudioStatus closed_render = engine.render(result);
    require(closed_render == AudioStatus::closed, "closed engine rejects render");
    const gui_forms::AudioStatus reopen_engine = engine.open(true);
    require(reopen_engine == AudioStatus::ok, "explicit reopen");
    const gui_forms::AudioStatus old_voice_play = voice.play();
    require(old_voice_play == AudioStatus::closed, "old handle remains revoked after reopen");
}
void quotas_and_values() {
    using gui_forms::AudioStatus;
    const std::array<float, 2> source{.75f, .75f};
    gui_forms::AudioClipResult clip = gui_forms::AudioClip::copy(source);
    gui_forms::AudioEngine engine{};
    const gui_forms::AudioStatus quota_engine = engine.open(true);
    require(quota_engine == AudioStatus::ok, "quota engine");
    std::array<gui_forms::AudioVoice, 65> voices{};
    for (std::size_t i = 0; i < 64; ++i) {
        const gui_forms::AudioStatus admitted_slot = engine.voice(clip.clip, true, voices[i]);
        require(admitted_slot == AudioStatus::ok, "slot admitted");
    }
    const gui_forms::AudioStatus excess_slot = engine.voice(clip.clip, true, voices[64]);
    require(excess_slot == AudioStatus::quota_exceeded,
            "stopped voices count toward quota");
    const gui_forms::AudioStatus first_mix_voice = voices[0].play();
    const gui_forms::AudioStatus second_mix_voice = voices[1].play();
    require(first_mix_voice == AudioStatus::ok && second_mix_voice == AudioStatus::ok, "mix start");
    std::array<float, 512> result{};
    const gui_forms::AudioStatus render_mix = engine.render(result);
    require(render_mix == AudioStatus::ok, "mix render");
    for (float sample : result) { require(sample == 1, "mix clips at unity"); }
    voices[0] = gui_forms::AudioVoice{};
    const gui_forms::AudioStatus reused_slot = engine.voice(clip.clip, true, voices[64]);
    require(reused_slot == AudioStatus::ok, "released slot reusable");
    const std::array<float, 2> invalid{0, std::numeric_limits<float>::infinity()};
    const gui_forms::AudioClipResult invalid_pcm = gui_forms::AudioClip::copy(invalid);
    require(invalid_pcm.status == AudioStatus::invalid_value, "infinite PCM rejected");
    const gui_forms::AudioClipResult empty_pcm = gui_forms::AudioClip::copy({});
    require(empty_pcm.status == AudioStatus::invalid_format, "empty PCM rejected");
}
void write_bytes(std::ostream& stream, std::uint32_t value, int count) {
    for (int i = 0; i < count; ++i) { stream.put(static_cast<char>((value >> (i * 8)) & 255)); }
}
void wav_loader() {
    std::chrono::steady_clock::duration stamp = std::chrono::steady_clock::now().time_since_epoch();
    std::filesystem::path root = std::filesystem::temp_directory_path() /
                                 ("gui-forms-audio-" + std::to_string(stamp.count()));
    const bool directory_created = std::filesystem::create_directory(root);
    require(directory_created, "isolated temporary directory");
    std::filesystem::path path = root / "clip.wav";
    {
        std::ofstream file(path, std::ios::binary);
        file.write("RIFF", 4); write_bytes(file, 44, 4); file.write("WAVEfmt ", 8);
        write_bytes(file, 16, 4); write_bytes(file, 1, 2); write_bytes(file, 2, 2);
        write_bytes(file, 48000, 4); write_bytes(file, 192000, 4);
        write_bytes(file, 4, 2); write_bytes(file, 16, 2);
        file.write("data", 4); write_bytes(file, 8, 4);
        write_bytes(file, 16384, 2); write_bytes(file, 49152, 2);
        write_bytes(file, 8192, 2); write_bytes(file, 57344, 2);
    }
    gui_forms::AudioClipResult clip = gui_forms::AudioClip::load_wav(path, 1);
    require(clip.status == gui_forms::AudioStatus::ok && (*clip.clip).frames() == 1, "WAV exact trim");
    require((*clip.clip).samples()[0] == .5f && (*clip.clip).samples()[1] == -.5f, "little endian PCM16");
    const gui_forms::AudioClipResult excess_trim = gui_forms::AudioClip::load_wav(path, 3);
    require(excess_trim.status == gui_forms::AudioStatus::invalid_format,
            "trim cannot exceed payload");
    std::filesystem::resize_file(path, 47);
    const gui_forms::AudioClipResult truncated_wav = gui_forms::AudioClip::load_wav(path);
    require(truncated_wav.status == gui_forms::AudioStatus::invalid_format,
            "truncated RIFF rejected");
    {
        std::ofstream file(path, std::ios::binary | std::ios::trunc);
        file.write("RIFF", 4); write_bytes(file, 44, 4); file.write("WAVEfmt ", 8);
        write_bytes(file, 16, 4); write_bytes(file, 3, 2); write_bytes(file, 2, 2);
        write_bytes(file, 48000, 4); write_bytes(file, 384000, 4);
        write_bytes(file, 8, 2); write_bytes(file, 32, 2);
        file.write("data", 4); write_bytes(file, 8, 4);
        write_bytes(file, 0x3f000000, 4); write_bytes(file, 0xbf000000, 4);
    }
    clip = gui_forms::AudioClip::load_wav(path);
    require(clip.status == gui_forms::AudioStatus::ok, "IEEE float32 WAV admitted");
    require((*clip.clip).samples()[0] == .5f && (*clip.clip).samples()[1] == -.5f, "float endian values");
    {
        std::fstream file(path, std::ios::binary | std::ios::in | std::ios::out);
        file.seekp(44); write_bytes(file, 0x7fc00000, 4);
    }
    const gui_forms::AudioClipResult nan_wav = gui_forms::AudioClip::load_wav(path);
    require(nan_wav.status == gui_forms::AudioStatus::invalid_value,
            "NaN WAV payload rejected");
    std::filesystem::remove(path);
    std::filesystem::remove(root);
}
}
int main() {
    try {
#ifdef GUI_FORMS_AUDIO_TESTING
        const gui_forms::AudioStatus revoked = gui_forms::audio_test_revoked_callback_status();
        require(revoked == gui_forms::AudioStatus::closed, "late callback cannot replace closed status");
#endif
        controls_and_lifetime();
        quotas_and_values();
        wav_loader();
        std::cout << "PCM validation, exact loops, controls, quotas and shutdown pass.\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
