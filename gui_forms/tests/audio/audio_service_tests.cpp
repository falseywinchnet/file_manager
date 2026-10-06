#include "gui_forms/audio/audio.hpp"
#include <array>
#include <memory>
#include <span>
#include <cmath>
#include <chrono>
#include <fstream>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <vector>
#include <thread>
#include <barrier>

#ifdef GUI_FORMS_AUDIO_TESTING
namespace gui_forms {
AudioStatus audio_test_revoked_callback_status();
void audio_test_decode_limits(std::size_t arena_bytes, std::uint64_t budget, std::stop_source* cancel_after_chunk);
std::uint64_t audio_test_clip_bytes();
}
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
// A ramp source: each frame is its index / 10000 on both channels, and it counts
// how many frames the mixer has pulled.
class RampGenerator final : public gui_forms::AudioGenerator {
public:
    std::uint64_t pulled{};
    void render(std::span<float> stereo) noexcept override {
        for (std::size_t i = 0; i + 1 < stereo.size(); i += 2) {
            const float value = static_cast<float>(pulled % 5000U) / 10000.0f;
            stereo[i] = value;
            stereo[i + 1] = -value;
            ++pulled;
        }
    }
};
void generator_voices() {
    using gui_forms::AudioStatus;
    gui_forms::AudioEngine engine{};
    require(engine.open(true) == AudioStatus::ok, "offline engine for generators");
    gui_forms::AudioVoice rejected{};
    require(engine.generator(nullptr, rejected) == AudioStatus::invalid_value, "a generator is required");
    std::shared_ptr<RampGenerator> ramp = std::make_shared<RampGenerator>();
    {
        gui_forms::AudioVoice voice{};
        require(engine.generator(ramp, voice) == AudioStatus::ok, "generator admitted");
        std::array<float, 16> result{};
        require(engine.render(result) == AudioStatus::ok, "render before play");
        require((*ramp).pulled == 0, "a voice that is not playing is not pulled");
        for (float sample : result) { require(sample == 0, "silent before play"); }
        require(voice.play() == AudioStatus::ok, "generator plays");
        require(engine.render(result) == AudioStatus::ok, "generator render");
        require((*ramp).pulled == 8, "pulled frame for frame");
        for (std::size_t i = 0; i < result.size(); i += 2) {
            const float expected = static_cast<float>(i / 2) / 10000.0f;
            require(std::abs(result[i] - expected) < 1e-6f && std::abs(result[i + 1] + expected) < 1e-6f, "generator samples pass through");
        }
        require(voice.set_gain(0.5) == AudioStatus::ok, "generator gain");
        require(engine.render(result) == AudioStatus::ok, "render at half gain");
        require(std::abs(result[0] - 0.5f * 8.0f / 10000.0f) < 1e-6f, "gain scales a generator");
        require(voice.pause() == AudioStatus::ok, "generator pauses");
        const std::uint64_t before_pause = (*ramp).pulled;
        require(engine.render(result) == AudioStatus::ok, "paused generator render");
        require((*ramp).pulled == before_pause, "a paused generator is frozen");
        for (float sample : result) { require(sample == 0, "paused generator is silent"); }
        require(voice.stop() == AudioStatus::ok && voice.play() == AudioStatus::ok, "stop only pauses a generator");
        require(engine.render(result) == AudioStatus::ok, "resumed generator render");
        require(std::abs(result[0] - 0.5f * static_cast<float>(before_pause) / 10000.0f) < 1e-6f, "a generator resumes where it stopped");
    }
    require(ramp.use_count() == 1, "a destroyed voice releases its generator");
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
// Author-generated 440 Hz stereo tone, 5760 frames at 48 kHz, encoded with
// libvorbis quality 2. Embedded so runtime tests need neither FFmpeg nor assets.
constexpr char vorbis_fixture_hex[] =
    "4f676753000200000000000000009af590c500000000df44ee8a011e01766f72626973000000000280bb000000000000"
    "0077010000000000b8014f676753000000000000000000009af590c5010000006a20a259103fffffffffffffffffffff"
    "ffffffffe203766f726269730c0000004c61766636312e372e313030010000001f000000656e636f6465723d4c617663"
    "36312e31392e313030206c6962766f726269730105766f726269732542435601004000002473182a46a5731684101a42"
    "5019e31c42ce6bec19424c11821c324c5bcb25739021a4a042885b2881d09055000040000087417814848a4108218425"
    "3d589283273d082184883978148469410821841042082184104208218445396892832741081d84e330380c83e538f81c"
    "8445395810832741e820840f42b89a83ac3908218424354850830639e81c84c22c288a82c430b816840435288c82e430"
    "c8d4830b42889a834935f81a846741781684694108218424414890830641c8188446415892830639b81484cb41a81a84"
    "2a39081f842034641500900000a0a2288aa2280a101ab20a00c8000010405114c7711cc9911cc9b11c0b080d59050000"
    "0100080000a0488aa4488ee44892245992255992255992e689aa2ccbb22ccbb22ccb32101ab20a0048000050510c4571"
    "1407080d59050064000008a0388aa5588aa5688ae7888e088486ac0200800000040000103443533c479444cf5455d7b6"
    "6ddbb66ddbb66ddbb66ddbb66d5b966519080d59050040000010d26966a9068830031906424356010008000080118a30"
    "c480d09055000040000080184a0ea209ad39df9ce3a0590e9a4ab1391d9c48b579929b8ab939e79c73cec9e69c31ce39"
    "e79ca29c590c9a09ad39e79cc4a0590a9a09ad39e79c27b179d09a2aad39e79c71cee9609c11c639e79c26ad79909a8d"
    "b539e79c05ad698e9a4bb139e79c48b979529b4bb539e79c73ce39e79c73ce39e79ceac5e91c9c13ce39e79ca8bdb996"
    "9bd0c539e79c4fc6e9de9c10ce39e79c73ce39e79c73ce39e79c20346415000004004010868d61dc2908d2e768204611"
    "621a32e941f7e830091a839c42ead1e868a4943a08259571524a27080d59050000020040082185145248218514524821"
    "8514628821861872ca29a7a0824a2aa9a8a28c32cb2cb3cc32cb2cb3cc3aecacb30e3b0c31c410432badc452536d35d6"
    "586bee39e79a83b4565a6badb5524a29a594520a42435601002000000442061964905148218514628829a79c720a2aa8"
    "80d090550000200080000000004ff21cd1111dd1111dd1111dd1111dd1f11ccf112551122551122dd33235d353455575"
    "65d7967559b77d5bd8855df77dddf77dddf8756158966559966559966559966559966559962034641500000200002084"
    "104248218514524829c61873cc39e8249410080d59050000020008000000701447711cc9911c49b2244bd224cdd22c4f"
    "f3344f133d511445d33455d1155d51376d513665d3355d53365d55566d57966d5bb675db9765dbf77ddff77ddff77ddf"
    "f77ddff77d5d0742435601001200003a92232992222992e3388e24494068c82a004006004000008ae2288ee338922449"
    "92256992677996a8999ae9999e2aaa4068c82a00001000400000000000008aa6788aa9788aa8788ee88892689996a8a9"
    "9a2bcaa6ecbaaeebbaaeebbaaeebbaaeebbaaeebbaaeebbaaeebbaaeebbaaeebbaaeebbaaeebbaae0b8486ac02002400"
    "007424477224475224455224477280d0905500800c008000001cc3312445722ccbd2344ff3344f133dd1133dd3534557"
    "7481d0905500002000800000000000000cc9b014cbd11c4d1225d5522d55532dd55245d5535555555555555555555555"
    "555555555555555555555555555555555555555555554dd3344d13080d590900900100a0105b4badc5dc096a1c62d272"
    "cc24744e6210aab1082247b5b7ca31a51cc59e1a889451127baa28638a49cc31b4d02927add6523a8514a498530a1552"
    "0e5a2034648500109a01e0701c40b22c40b2340000000000000090340dd03c0fb03c0f00000000000000244d032c4f03"
    "34cf03000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
    "0000000000000000000000000000000000000000000040d23440f33c40f33c00000000000000d03c0ff04411f0441100"
    "0000000000002ccf033cd1033c5104000000000000000000000000000000000000000000000000000000000000000000"
    "00000000000000000000000000000000000000000000000000000000000000000000c0d13440f33c40f33c0000000000"
    "0000b03c0ff04411f03c110000000000000034cf033c51043c5104000000000000000000000000000000000000000000"
    "000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
    "000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
    "000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
    "000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
    "000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
    "000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
    "000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
    "000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
    "000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
    "000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
    "000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
    "000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
    "000000000000000000000000000000000010000010e00000106021141ab2220088130030380e340d9a06cf03389605cf"
    "83e74114018e65c1f3e07910450000000000000000000034cf83aa4255e1aa00cdf360aa5055a82e0000000000000000"
    "000096e74155a1aa705d80e5793055982a5415000000000000000000004f14a1ba505db82ac03345b82a5c15aa0b0000"
    "0000000000000000000000000000000000000000080000187000000830a10c141ab2220088130070388a65010080e338"
    "960500008ee3581600005896258a000060599a2802000000000000000000000000000000000000000000000000000000"
    "00000000000000000000000000000000000000000000000000000000000000000000000000080000187000000830a10c"
    "141ab2120088020030288a6501cbb22c6059960534cdb2009606d03c80e7014411000800002870000008b04153627180"
    "42435602005100000645b12c4d13459aa6699a268a344dd3344d14799ea6799e6942d33ccf34218a9e679a1045cf334d"
    "98a628aa2a10455515000050e00000106083a6c4e2008586ac04004202000c8e62599e278aa2288aa6a9aa344dd33c4f"
    "1445d13455d555699aa6799e288aa269aaaaeaf23c4d1345d31445d35455d785a689a2699aa269aaaaebc2f344d1344d"
    "535555d575e179a2689aa6a9aaaeebba104551344dd35455d7755d208aa6699aaaeabab20c44d1345555555d57968128"
    "9aa6aaaaaaebca32304dd35455d7955d590698a6aabaae2ccb3240555dd7756559b601aaeabaae2bcbb20d705dd79565"
    "59b66d00ae2bcbb26cdb0200000e1c0000028ca0938c2a8bb0d1840b0f40a1212b0280280000c018a61453ca302621a4"
    "101ac624841242262595944aaa20a4525229158454522a25a392526a295510522929950a422aa595540000d8810300d8"
    "81855068c84a00200f0080204629c618630c32a61463ce390795528a31e79c938c31c69873ce49291963cc39e7a4948c"
    "39e79c73524ae69c73ce3929a573ce39e79c94524ae79c734e4a292584ce3927a594d239e79c13000054e000001060a3"
    "c8e60423418586ac04005201000c8e63599aa6699e278a9624699ae7799e289aa666499ae6799e278aa6c9f33c4f1445"
    "d1345595e7799e288aa269aa2ad71545d3344d555555b22c8aa6699aaaeaba304dd35455d7756598a669aaaaebba2e6c"
    "db5455d5756519b6ad9aaa2abbb20c5c577565d7b681ebbaaeecdab60000f0040700a0021b564738291a0b2c34642500"
    "9001004018838c4208218510420a2184945208090000187000000830a10c141ab212004805000090b1d65a6badb5d640"
    "4729a594524aa9708c524a29a594524a29a594524a29a5944a4a29a594524a29a594524a29a594524a29a594524a29a5"
    "94524a29a594524a29a594524a29a594524a29a594524a29a594524a29a594524a29a594524a29a594524a29a594524a"
    "05002e553800e83ed8b03ac249d15860a1212b0180540000c018a598724e422915428c392621a5162b8418734e4a4a31"
    "16cf3907a194d65a2c9e730e4229adc55854ea9c94945a8aada814322929a5d66210c294945a6ba5b520842aa9c4965a"
    "6b41085d536a2996d88210b6b692528c3106e1838fb195586a0c3ef8205b2b31d55a00006683030044820dab239c148d"
    "05161ab21200080900208c518a31c61873ce39e724638c31e69c73104208a1648c31e79c730e42082194ce39e79c7310"
    "42082184524ac79c730e420821845052ea9c73104208a184104a2a9d730e42082184524a49a573104208a18450424925"
    "a5d43908218410422929a5944208218412422825a59452082184104228a1a494520a2184524208a59494524a2985104a"
    "08a59492524929a5124a09218452524929a51442082594524a2a29a5944a09a184524a29a5a494524a21945042290500"
    "001c3800000418412719551661a309171e80424356020064000094b2524a28ad554022a518a4da424799831473892c73"
    "0c5acda5620e2906ad86ca31a518b416320899524c4a0925754c29272dc5984ae79ca498738da5731000000041008080"
    "9000000304053300c0e000e17310740204471b00802044668844c3427078500910115301406282422e0054585ca45d5c"
    "4097012ee8e2ae0321042108412c0ea080041c9c70c3136f78c20d4ed0292a752000000000000b00f00000905c001111"
    "d1cc6164686c7074787c8084888c9008000000000016007c00002425404444347318191a1b1c1d1e1f20212223240100"
    "8000020000000020800004040400000000000200000004044f676753000480160000000000009af590c5020000002bfc"
    "466808255a414141427e5354e1abba2ae52a7c5557a5bc2bea555081002046c4117fa6e6d5abd8486c24128944229108"
    "5ad8bd0c5d05b0e7de5d07dea951532cec5e86ae02d873efae03efd4a8297a02db0300400a0000000000000000000000"
    "0000000000e60b067cae2fe0f70595750100e2303cd4661a6a988669b15a66ce9839633a990ee330a6295ee8dd5a5701"
    "b8ddbbcf09dfec3fd4140bbd5beb2a00b77bf739e19bfd879a220403000000000000000000000000000000000084aa04"
    "0048a5dc6d5687450500005ee8dd5a5701b83dbbcf09dfdc0ad4140bbd5beb2a00b767f739e19b5b819a220403000000"
    "0000000000000000000000000000a43a010022eae1a1aea6a90000005ee8ddda5701b83fbb8f0ddff81750532cf46eed"
    "ab00dc9fddc7866ffc0ba82942300000000000000000000000000000000000808c0a00d0167a161c9e620300007ee8bd"
    "a85701a43dbbcf09dff84fa0a678e8bda85701a43dbbcf09dff84fa0a608c10600000000000000000000000000000000"
    "0044d40000f4f58eb66038c40e0000fea7dd0ebbd2de67f7eb07de5053fcd36e875d69efb3fbf5036fa829ea16d15803"
    "40ca7f180661000000000000c0458dba23a88ac1a491216b9a2d23cd94441b9d0ce398c1d8a49585d4d7da44ea3d975a"
    "ae89d47ba4966b1319ea55525feb68a4de5b48fd5abf44867a0ba9d4da44ea3d974a9d89684304680e14e00c00fe46dd"
    "5c29020271ed1e3fe8de50537ca36eae140181b8768f1f746fa8291ab31740860100000000000000000000000000f0c9"
    "eb7af13bf2e231af3c104d2349d32432651b674e26d37192b183d12002f41c0a";
std::vector<unsigned char> vorbis_fixture() {
    std::vector<unsigned char> bytes((sizeof(vorbis_fixture_hex) - 1) / 2);
    for (std::size_t i = 0; i < bytes.size(); ++i) {
        const char high = vorbis_fixture_hex[i * 2];
        const char low = vorbis_fixture_hex[i * 2 + 1];
        const unsigned a = high <= '9' ? static_cast<unsigned>(high - '0') : static_cast<unsigned>(high - 'a' + 10);
        const unsigned b = low <= '9' ? static_cast<unsigned>(low - '0') : static_cast<unsigned>(low - 'a' + 10);
        bytes[i] = static_cast<unsigned char>(a * 16 + b);
    }
    return bytes;
}
void save_fixture(const std::filesystem::path& path, std::span<const unsigned char> bytes) {
    std::ofstream file(path, std::ios::binary | std::ios::trunc);
    file.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    require(static_cast<bool>(file), "write isolated fixture");
}
void repair_page_checksum(std::vector<unsigned char>& bytes, std::size_t page) {
    std::size_t size = 27 + bytes[page + 26];
    for (std::size_t i = 0; i < bytes[page + 26]; ++i) { size += bytes[page + 27 + i]; }
    for (std::size_t i = 22; i < 26; ++i) { bytes[page + i] = 0; }
    std::uint32_t crc = 0;
    for (std::size_t i = 0; i < size; ++i) {
        crc ^= static_cast<std::uint32_t>(bytes[page + i]) << 24;
        for (unsigned bit = 0; bit < 8; ++bit) {
            crc = (crc & 0x80000000U) != 0 ? (crc << 1) ^ 0x04c11db7U : crc << 1;
        }
    }
    for (unsigned i = 0; i < 4; ++i) { bytes[page + 22 + i] = static_cast<unsigned char>(crc >> (8 * i)); }
}
struct ConcurrentDecode final {
    const std::filesystem::path* path{};
    std::barrier<>* start{};
    std::span<const float> expected{};
    bool passed{true};
    void run() {
        (*start).arrive_and_wait();
        for (unsigned iteration = 0; iteration < 12; ++iteration) {
            const gui_forms::AudioClipResult result = gui_forms::AudioClip::load_ogg(*path);
            if (result.status != gui_forms::AudioStatus::ok || (*result.clip).samples().size() != expected.size()) {
                passed = false; return;
            }
            const std::span<const float> samples = (*result.clip).samples();
            for (std::size_t i = 0; i < samples.size(); ++i) {
                if (samples[i] != expected[i]) { passed = false; return; }
            }
        }
    }
};
void vorbis_loader() {
    using gui_forms::AudioStatus;
    const std::chrono::steady_clock::duration stamp = std::chrono::steady_clock::now().time_since_epoch();
    const std::filesystem::path root = std::filesystem::temp_directory_path() /
        ("gui-forms-vorbis-" + std::to_string(stamp.count()));
    const bool created = std::filesystem::create_directory(root);
    require(created, "isolated Vorbis directory");
    const std::filesystem::path path = root / "tone.ogg";
    const std::vector<unsigned char> fixture = vorbis_fixture();
    save_fixture(path, fixture);
    gui_forms::AudioClipResult original = gui_forms::AudioClip::load_ogg(path);
    require(original.status == AudioStatus::ok && (*original.clip).frames() == 5760, "Vorbis exact sample count");
    double energy = 0;
    for (float sample : (*original.clip).samples()) {
        require(std::isfinite(sample) && std::abs(sample) <= 1, "finite bounded decoded tone");
        energy += static_cast<double>(sample) * sample;
    }
    require(energy > 1, "tone was decoded, not replaced with silence");
    std::barrier<> start(4);
    std::array<ConcurrentDecode, 4> jobs{};
    std::array<std::thread, 4> workers{};
    for (std::size_t i = 0; i < jobs.size(); ++i) {
        jobs[i] = {&path, &start, (*original.clip).samples(), true};
        workers[i] = std::thread(&ConcurrentDecode::run, &jobs[i]);
    }
    for (std::thread& worker : workers) { worker.join(); }
    for (const ConcurrentDecode& job : jobs) { require(job.passed, "concurrent opens and decode are deterministic"); }
    std::stop_source cancelled{};
    cancelled.request_stop();
    const gui_forms::AudioClipResult stopped = gui_forms::AudioClip::load_ogg(path, cancelled.get_token());
    require(stopped.status == AudioStatus::cancelled && !stopped.clip, "pre-cancel publishes nothing");
#ifdef GUI_FORMS_AUDIO_TESTING
    const std::uint64_t held_bytes = gui_forms::audio_test_clip_bytes();
    gui_forms::audio_test_decode_limits(64, 512ULL * 1024 * 1024, nullptr);
    const gui_forms::AudioClipResult exhausted = gui_forms::AudioClip::load_ogg(path);
    require(exhausted.status == AudioStatus::allocation_failed && !exhausted.clip, "codec arena exhaustion is reported");
    gui_forms::audio_test_decode_limits(16 * 1024 * 1024, held_bytes, nullptr);
    const gui_forms::AudioClipResult quota = gui_forms::AudioClip::load_ogg(path);
    require(quota.status == AudioStatus::quota_exceeded && !quota.clip, "aggregate quota checked before PCM allocation");
    std::stop_source halfway{};
    gui_forms::audio_test_decode_limits(16 * 1024 * 1024, 512ULL * 1024 * 1024, &halfway);
    const gui_forms::AudioClipResult interrupted = gui_forms::AudioClip::load_ogg(path, halfway.get_token());
    require(interrupted.status == AudioStatus::cancelled && !interrupted.clip, "between-chunk cancellation publishes nothing");
    gui_forms::audio_test_decode_limits(16 * 1024 * 1024, 512ULL * 1024 * 1024, nullptr);
    const std::uint64_t after_failure = gui_forms::audio_test_clip_bytes();
    require(after_failure == held_bytes, "failure and cancellation release reservations");
    require((*original.clip).frames() == 5760 && (*original.clip).samples()[100] != 0, "existing owner survives failed load");
#endif
    const gui_forms::AudioClipResult retry = gui_forms::AudioClip::load_ogg(path);
    require(retry.status == AudioStatus::ok, "retry after failure succeeds");
    std::vector<unsigned char> broken = fixture;
    broken.back() ^= 1;
    save_fixture(path, broken);
    const gui_forms::AudioClipResult checksum = gui_forms::AudioClip::load_ogg(path);
    require(checksum.status == AudioStatus::invalid_format && !checksum.clip, "corrupted checksum rejected");
    for (std::size_t length : std::array<std::size_t, 4>{0, 27, 58, fixture.size() - 1}) {
        save_fixture(path, std::span<const unsigned char>(fixture.data(), length));
        const gui_forms::AudioClipResult truncated = gui_forms::AudioClip::load_ogg(path);
        require(truncated.status == AudioStatus::invalid_format && !truncated.clip, "truncated file rejected");
    }
    broken = fixture;
    broken[39] = 1; // Vorbis identification channel count, with valid Ogg CRC.
    repair_page_checksum(broken, 0);
    save_fixture(path, broken);
    const gui_forms::AudioClipResult mono = gui_forms::AudioClip::load_ogg(path);
    require(mono.status == AudioStatus::invalid_format && !mono.clip, "unsupported mono rejected");
    broken = fixture;
    broken[40] = 0x44; broken[41] = 0xac; // 44100 Hz.
    repair_page_checksum(broken, 0);
    save_fixture(path, broken);
    const gui_forms::AudioClipResult rate = gui_forms::AudioClip::load_ogg(path);
    require(rate.status == AudioStatus::invalid_format && !rate.clip, "unsupported rate rejected");
    std::size_t last_page = 0;
    for (std::size_t page = 0; page < fixture.size();) {
        last_page = page;
        std::size_t page_bytes = 27 + fixture[page + 26];
        for (std::size_t i = 0; i < fixture[page + 26]; ++i) { page_bytes += fixture[page + 27 + i]; }
        page += page_bytes;
    }
    broken = fixture;
    broken[last_page + 5] &= 3; // Complete pages but no end-of-stream marker.
    repair_page_checksum(broken, last_page);
    save_fixture(path, broken);
    const gui_forms::AudioClipResult missing_end = gui_forms::AudioClip::load_ogg(path);
    require(missing_end.status == AudioStatus::invalid_format && !missing_end.clip, "missing EOS rejected");
    broken = fixture;
    const std::uint64_t oversized_frames = 28800001;
    for (unsigned i = 0; i < 8; ++i) { broken[last_page + 6 + i] = static_cast<unsigned char>(oversized_frames >> (i * 8)); }
    repair_page_checksum(broken, last_page);
    save_fixture(path, broken);
    const gui_forms::AudioClipResult long_clip = gui_forms::AudioClip::load_ogg(path);
    require(long_clip.status == AudioStatus::invalid_format && !long_clip.clip, "frame limit rejects oversized granule");
    broken = fixture;
    // A valid CRC and supported identification do not make a setup packet valid.
    const std::size_t setup_page = 58;
    const std::size_t setup_data = setup_page + 27 + broken[setup_page + 26];
    broken[setup_data] = 0;
    repair_page_checksum(broken, setup_page);
    save_fixture(path, broken);
    const gui_forms::AudioClipResult malformed = gui_forms::AudioClip::load_ogg(path);
    require(malformed.status == AudioStatus::invalid_format && !malformed.clip, "malformed packet rejected beyond CRC");
    broken = fixture;
    broken.insert(broken.end(), fixture.begin(), fixture.end());
    save_fixture(path, broken);
    const gui_forms::AudioClipResult chained = gui_forms::AudioClip::load_ogg(path);
    require(chained.status == AudioStatus::invalid_format && !chained.clip, "chained streams rejected");
    save_fixture(path, fixture);
    std::filesystem::resize_file(path, 64ULL * 1024 * 1024 + 1);
    const gui_forms::AudioClipResult oversized = gui_forms::AudioClip::load_ogg(path);
    require(oversized.status == AudioStatus::invalid_format && !oversized.clip, "encoded limit checked before read");
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
        generator_voices();
        wav_loader();
        vorbis_loader();
        std::cout << "PCM validation, exact loops, controls, quotas, generators and shutdown pass.\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
