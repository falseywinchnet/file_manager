#include "gui_forms/audio/audio.hpp"
#include <array>
#include <atomic>
#include <cmath>
#include <chrono>
#include <iostream>
#include <stdexcept>
#include <thread>
#include <vector>

namespace gui_forms {
AudioLoopStatus audio_loop_test_native_render(AudioLoopTransport& transport, std::span<float> samples);
}
namespace {
using namespace gui_forms;
void require(bool condition, const char* message) {
    if (!condition) { throw std::runtime_error(message); }
}
std::shared_ptr<const AudioClip> make_clip(std::size_t frames, float base) {
    std::vector<float> samples(frames * 2);
    for (std::size_t i = 0; i < frames; ++i) {
        samples[i * 2] = base + static_cast<float>(i) * .001f;
        samples[i * 2 + 1] = -samples[i * 2];
    }
    AudioClipResult made = AudioClip::copy(samples);
    require(made.status == AudioStatus::ok, "PCM fixture admitted");
    return made.clip;
}
void render(AudioEngine& engine, std::span<float> samples) {
    const AudioStatus result = engine.render(samples);
    require(result == AudioStatus::ok, "offline PCM render");
}
void exact_mix(std::size_t chunk, std::uint64_t fade = 4) {
    AudioEngine engine{};
    const AudioStatus opened = engine.open(true);
    require(opened == AudioStatus::ok, "offline engine");
    AudioLoopTransport transport{};
    const AudioLoopStatus created = engine.loop_transport(transport);
    require(created == AudioLoopStatus::ok, "transport admitted");
    const std::shared_ptr<const AudioClip> first = make_clip(11, .1f);
    const std::shared_ptr<const AudioClip> second = make_clip(7, .3f);
    const AudioLoopCommand start = transport.change(first, {4, 0, 0});
    std::array<float, 10> prefix{};
    render(engine, prefix); // Render samples0..4. All variants admit at frame5.
    const AudioLoopCommand change = transport.change(second, {3, 3, fade});
    const AudioLoopReceipt queued = transport.poll(change.id);
    require(start.status == AudioLoopStatus::ok && queued.phase == AudioLoopPhase::queued, "queued phase is honest");
    std::vector<float> output(30 * 2);
    std::size_t position{};
    while (position < 30) {
        const std::size_t frames = std::min(chunk, 30 - position);
        render(engine, std::span<float>(output).subspan(position * 2, frames * 2));
        position += frames;
    }
    const AudioLoopReceipt applied = transport.poll(change.id);
    require(applied.phase == AudioLoopPhase::applied && applied.admission_frame == 5 && applied.application_frame == 11,
            "strict cutoff8 skips bar8, actual loop end11 selects switch");
    const std::span<const float> old_samples = (*first).samples();
    const std::span<const float> new_samples = (*second).samples();
    for (std::size_t index = 0; index < 30; ++index) {
        const std::size_t frame = index + 5;
        double expected{};
        if (frame < 11) { expected = old_samples[(frame % 11) * 2]; }
        else {
            expected = new_samples[((frame - 11) % 7) * 2];
            if (fade > 1 && frame < 11 + fade) {
                expected += static_cast<double>(old_samples[(frame % 11) * 2]) *
                    (1.0 - static_cast<double>(frame - 11) / static_cast<double>(fade - 1));
            }
        }
        require(std::abs(static_cast<double>(output[index * 2]) - expected) < 0.0000001,
                "actual PCM matches independent mixed sample oracle");
        require(output[index * 2 + 1] == -output[index * 2], "stereo channel layout preserved");
    }
    const AudioLoopCommand cancel = transport.cancel(change.id);
    std::array<float, 2> one{};
    render(engine, one);
    const AudioLoopReceipt late = transport.poll(cancel.id);
    require(late.phase == AudioLoopPhase::too_late, "late cancel typed");
    const AudioLoopCommand paused = transport.pause();
    render(engine, one);
    require(paused.status == AudioLoopStatus::ok && one[0] == 0, "pause produces silence");
    const AudioLoopCommand pending = transport.change(first, {4, 0, 0});
    render(engine, one);
    const AudioLoopReceipt pending_receipt = transport.poll(pending.id);
    require(pending_receipt.phase == AudioLoopPhase::admitted, "paused change remains pending");
    const AudioLoopCommand stopped = transport.stop();
    render(engine, one);
    require(stopped.status == AudioLoopStatus::ok && transport.poll(pending.id).phase == AudioLoopPhase::cancelled,
            "stop cancels while paused");
    engine.shutdown();
    const AudioLoopCommand closed = transport.resume();
    require(closed.status == AudioLoopStatus::closed && (*first).frames() == 11, "shutdown revokes transport and preserves external clip");
}
void quota_and_history() {
    AudioEngine engine{};
    const AudioStatus opened = engine.open(true);
    require(opened == AudioStatus::ok, "quota engine");
    const std::shared_ptr<const AudioClip> clip = make_clip(32, .1f);
    std::array<AudioVoice, 63> voices{};
    for (AudioVoice& voice : voices) {
        const AudioStatus admitted = engine.voice(clip, true, voice);
        require(admitted == AudioStatus::ok, "voice quota admission");
    }
    AudioLoopTransport transport{};
    const AudioLoopStatus admitted = engine.loop_transport(transport);
    AudioLoopTransport extra{};
    const AudioLoopStatus full = engine.loop_transport(extra);
    require(admitted == AudioLoopStatus::ok && full == AudioLoopStatus::quota_exceeded, "voices and transports share64quota");
    AudioVoice denied_voice{};
    const AudioStatus voice_full = engine.voice(clip, true, denied_voice);
    require(voice_full == AudioStatus::quota_exceeded, "transport consumes a voice slot");
    const AudioLoopCommand start = transport.change(clip, {8, 0, 0});
    std::array<float, 2> sample{};
    render(engine, sample);
    const AudioLoopCommand pending = transport.change(clip, {8, 480000, 0});
    render(engine, sample);
    for (unsigned i = 0; i < 16; ++i) {
        const AudioLoopCommand item = transport.set_gain(.5);
        require(item.status == AudioLoopStatus::ok, "command ring accepts sixteen");
    }
    const AudioLoopCommand refused = transport.cancel(pending.id);
    require(refused.status == AudioLoopStatus::quota_exceeded, "full ring does not fabricate cancel");
    render(engine, sample);
    for (unsigned i = 0; i < 100; ++i) {
        const AudioLoopCommand item = transport.set_gain(.5);
        require(item.status == AudioLoopStatus::ok, "history can recycle terminal cells");
        render(engine, sample);
    }
    require(transport.poll(start.id).status == AudioLoopStatus::expired &&
            transport.poll(pending.id).phase == AudioLoopPhase::admitted, "bounded history preserves pending request");
    const AudioLoopCommand expired = transport.cancel(start.id);
    render(engine, sample);
    require(transport.poll(expired.id).phase == AudioLoopPhase::expired_target, "expired cancellation history explicit");
    const AudioLoopStatus closed = transport.close();
    const AudioLoopStatus retry = engine.loop_transport(extra);
    require(closed == AudioLoopStatus::ok && retry == AudioLoopStatus::ok, "closed transport returns shared quota");
    const AudioLoopStatus again = transport.close();
    AudioVoice still_full{};
    const AudioStatus after_repeat = engine.voice(clip, true, still_full);
    require(again == AudioLoopStatus::ok && after_repeat == AudioStatus::quota_exceeded,
            "repeated close cannot revoke a reused slot");
}
struct ClipOwner final {
    std::shared_ptr<const AudioClip> clip{};
    std::thread::id control{};
    std::atomic<bool>* destroyed{};
    std::atomic<bool>* wrong_thread{};
    ~ClipOwner() {
        (*wrong_thread).store(std::this_thread::get_id() != control);
        (*destroyed).store(true);
    }
};
struct RenderWorker final {
    AudioLoopTransport* transport{};
    std::atomic<unsigned> blocks{};
    std::atomic<bool> failed{};
    static void run(std::stop_token stop, RenderWorker* context) {
        RenderWorker& worker = *context;
        std::array<float, 512> output{};
        while (!stop.stop_requested()) {
            const AudioLoopStatus result = audio_loop_test_native_render(*worker.transport, output);
            if (result != AudioLoopStatus::ok) { worker.failed.store(true); }
            worker.blocks.fetch_add(1);
        }
    }
};
struct ForeignControl final {
    AudioLoopTransport* transport{};
    AudioLoopStatus result{};
    void run() { const AudioLoopCommand request = (*transport).pause(); result = request.status; }
};
void concurrent_retirement() {
    std::atomic<bool> destroyed{}, wrong_thread{}; // Outlive transport even on a failed check.
    AudioEngine engine{};
    const AudioStatus opened = engine.open(true);
    require(opened == AudioStatus::ok, "concurrency engine");
    AudioLoopTransport transport{};
    const AudioLoopStatus admitted = engine.loop_transport(transport);
    require(admitted == AudioLoopStatus::ok, "concurrency transport");
    std::shared_ptr<ClipOwner> owner = std::make_shared<ClipOwner>();
    (*owner).clip = make_clip(32, .1f);
    (*owner).control = std::this_thread::get_id();
    (*owner).destroyed = &destroyed;
    (*owner).wrong_thread = &wrong_thread;
    std::shared_ptr<const AudioClip> alias(owner, (*owner).clip.get());
    owner.reset();
    const AudioLoopCommand start = transport.change(alias, {8, 0, 0});
    alias.reset();
    ForeignControl foreign{&transport};
    std::thread wrong(&ForeignControl::run, &foreign);
    wrong.join();
    require(foreign.result == AudioLoopStatus::wrong_thread, "wrong control thread refused");
    RenderWorker worker{};
    worker.transport = &transport;
    std::jthread renderer(&RenderWorker::run, &worker);
    unsigned accepted{};
    const std::chrono::steady_clock::time_point deadline = std::chrono::steady_clock::now() + std::chrono::seconds(10);
    while (accepted < 1000) {
        const std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
        require(now < deadline, "concurrent producer makes bounded-time progress");
        AudioLoopCommand request{};
        if (accepted % 8 == 0) {
            const std::shared_ptr<const AudioClip> replacement = make_clip(32, .2f);
            request = transport.change(replacement, {8, 0, 4});
        } else {
            request = transport.set_gain(.25);
        }
        if (request.status == AudioLoopStatus::ok) { ++accepted; }
        else { require(request.status == AudioLoopStatus::quota_exceeded, "only bounded backpressure during concurrent enqueue"); }
        const AudioLoopReceipt receipt = transport.poll(start.id);
        require(receipt.status == AudioLoopStatus::ok || receipt.status == AudioLoopStatus::expired ||
                receipt.status == AudioLoopStatus::busy, "bounded coherent polling during rendering");
        std::this_thread::yield();
    }
    AudioLoopCommand stopped{};
    while (stopped.status != AudioLoopStatus::ok) {
        const std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
        require(now < deadline, "stop admission makes progress");
        stopped = transport.stop();
        require(stopped.status == AudioLoopStatus::ok || stopped.status == AudioLoopStatus::quota_exceeded,
                "stop has typed backpressure");
        std::this_thread::yield();
    }
    bool stop_applied{};
    while (!destroyed.load() || !stop_applied) {
        const std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
        require(now < deadline, "retired clip collected within deadline");
        const AudioLoopReceipt receipt = transport.poll(stopped.id);
        require(receipt.status == AudioLoopStatus::ok || receipt.status == AudioLoopStatus::busy,
                "stop receipt stays retained");
        stop_applied = receipt.status == AudioLoopStatus::ok && receipt.phase == AudioLoopPhase::applied;
        std::this_thread::yield();
    }
    require(!wrong_thread.load(), "callback retirement leaves last-owner destruction to control collection");
    // Keep the engine alive and privately render after detaching the transport.
    // This exercises miniaudio graph-reader quiescence, not public offline use.
    const AudioLoopStatus closed = transport.close();
    renderer.request_stop();
    renderer.join();
    require(closed == AudioLoopStatus::ok && worker.blocks.load() > 0 && !worker.failed.load(),
            "native graph detach while private reader runs");
    require(destroyed.load() && !wrong_thread.load(), "last clip owner released on control thread after detach");
    engine.shutdown();
}
}
int main() {
    try {
        exact_mix(1); exact_mix(7); exact_mix(30);
        exact_mix(7, 0); exact_mix(7, 1); exact_mix(7, 2);
        quota_and_history();
        concurrent_retirement();
        std::cout << "Actual PCM loop transitions, command receipts, shared quota and concurrent retirement pass.\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
