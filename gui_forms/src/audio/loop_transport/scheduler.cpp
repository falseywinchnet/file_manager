#include "scheduler.hpp"
#include <algorithm>
#include <cmath>
#include <limits>

namespace gui_forms::audio_experiment {
namespace {
constexpr std::uint64_t maximum_frame = std::numeric_limits<std::uint64_t>::max();
bool terminal(RequestPhase phase) noexcept {
    const bool result = phase != RequestPhase::queued && phase != RequestPhase::admitted;
    return result;
}
bool valid_clip(ClipShape clip) noexcept {
    const bool valid = clip.frames > 0 && clip.frames <= 28800000 &&
        clip.bar_frames > 0 && clip.bar_frames <= clip.frames;
    return valid;
}
}
BoundaryResult next_boundary(std::uint64_t render_frame, std::uint64_t current_position,
    ClipShape clip, std::uint64_t cutoff, std::uint64_t available) noexcept {
    const bool valid = valid_clip(clip);
    if (!valid || current_position >= clip.frames) { return {ScheduleStatus::invalid, 0}; }
    if (cutoff == maximum_frame) { return {ScheduleStatus::overflow, 0}; }
    const std::uint64_t lower = std::max({render_frame, available, cutoff + 1});
    const std::uint64_t offset = (lower - render_frame) % clip.frames;
    const std::uint64_t position = (current_position + offset) % clip.frames;
    std::uint64_t delta{};
    const std::uint64_t bar_position = position % clip.bar_frames;
    if (bar_position != 0) { delta = std::min(clip.bar_frames - bar_position, clip.frames - position); }
    if (delta > maximum_frame - lower) { return {ScheduleStatus::overflow, 0}; }
    const BoundaryResult result{ScheduleStatus::ok, lower + delta};
    return result;
}
double outgoing_weight(std::uint64_t index, std::uint64_t frames) noexcept {
    if (frames <= 1 || index >= frames) { return 0; }
    const double weight = 1.0 - static_cast<double>(index) / static_cast<double>(frames - 1);
    return weight;
}
LoopScheduler::LoopScheduler(std::uint64_t epoch, std::uint64_t initial_frame,
                             std::uint64_t first_id) noexcept
    : epoch_(epoch), frame_(initial_frame), next_id_(first_id), closed_(epoch == 0 || first_id == 0) {}
RequestResult LoopScheduler::change(ClipShape clip, ChangeTiming timing) noexcept {
    const bool valid = valid_clip(clip);
    if (!valid || timing.lead_frames > 480000 || timing.fade_frames > 48000) {
        return {ScheduleStatus::invalid, 0};
    }
    std::size_t payload{};
    while (payload < payload_capacity && payloads_[payload].phase != PayloadPhase::free) { ++payload; }
    if (payload == payload_capacity) { return {ScheduleStatus::full, 0}; }
    Command item{};
    item.kind = CommandKind::change;
    item.payload = payload;
    item.timing = timing;
    const RequestResult result = enqueue(item);
    if (result.status == ScheduleStatus::ok) { payloads_[payload] = {PayloadPhase::retained, clip}; }
    return result;
}
RequestResult LoopScheduler::command(CommandKind kind, double gain, std::uint64_t target) noexcept {
    const bool known = kind == CommandKind::pause || kind == CommandKind::resume ||
        kind == CommandKind::stop || kind == CommandKind::gain || kind == CommandKind::cancel;
    if (!known || !std::isfinite(gain) || gain < 0 || gain > 1) {
        return {ScheduleStatus::invalid, 0};
    }
    Command item{};
    item.kind = kind;
    item.gain = gain;
    item.target = target;
    const RequestResult result = enqueue(item);
    return result;
}
RequestResult LoopScheduler::enqueue(Command item) noexcept {
    if (closed_) { return {ScheduleStatus::closed, 0}; }
    if (next_id_ == 0) { return {ScheduleStatus::overflow, 0}; }
    if (queued_ == command_capacity) { return {ScheduleStatus::full, 0}; }
    std::size_t receipt = receipt_capacity;
    for (std::size_t i = 0; i < receipt_capacity; ++i) {
        const std::size_t candidate = (receipt_cursor_ + i) % receipt_capacity;
        const bool reusable = terminal(receipts_[candidate].phase);
        if (reusable) { receipt = candidate; break; }
    }
    if (receipt == receipt_capacity) { return {ScheduleStatus::full, 0}; }
    item.id = next_id_;
    next_id_ = next_id_ == maximum_frame ? 0 : next_id_ + 1;
    receipts_[receipt] = {ScheduleStatus::ok, item.id, epoch_, 0, 0, RequestPhase::queued};
    receipt_cursor_ = (receipt + 1) % receipt_capacity;
    const std::size_t tail = (head_ + queued_) % command_capacity;
    commands_[tail] = item;
    ++queued_;
    const RequestResult result{ScheduleStatus::ok, item.id};
    return result;
}
Receipt LoopScheduler::poll(std::uint64_t id) const noexcept {
    if (id == 0) { return {}; }
    for (const Receipt& receipt : receipts_) {
        if (receipt.id == id) { return receipt; }
    }
    return {};
}
void LoopScheduler::finish(std::uint64_t id, RequestPhase phase) noexcept {
    for (Receipt& receipt : receipts_) {
        if (receipt.id == id) {
            receipt.phase = phase;
            receipt.application_frame = frame_;
            return;
        }
    }
}
void LoopScheduler::retire(std::size_t index) noexcept {
    if (index < payload_capacity) { payloads_[index].phase = PayloadPhase::retired; }
}
void LoopScheduler::cancel_pending(RequestPhase phase) noexcept {
    if (pending_ == SamplePlan::no_payload) { return; }
    retire(pending_);
    finish(pending_id_, phase);
    pending_ = SamplePlan::no_payload;
    pending_id_ = 0;
}
void LoopScheduler::apply(Command item) noexcept {
    if (item.kind == CommandKind::change) {
        std::uint64_t boundary = frame_;
        if (current_ != SamplePlan::no_payload) {
            const std::uint64_t remaining = outgoing_ == SamplePlan::no_payload ? 0 : fade_frames_ - fade_index_;
            if (item.timing.lead_frames > maximum_frame - frame_ || remaining > maximum_frame - frame_) {
                retire(item.payload); finish(item.id, RequestPhase::rejected); return;
            }
            const BoundaryResult next = next_boundary(frame_, current_position_, payloads_[current_].clip,
                frame_ + item.timing.lead_frames, frame_ + remaining);
            if (next.status != ScheduleStatus::ok || item.timing.fade_frames > maximum_frame - next.frame) {
                retire(item.payload); finish(item.id, RequestPhase::rejected); return;
            }
            boundary = next.frame;
        }
        cancel_pending(RequestPhase::replaced);
        pending_ = item.payload;
        pending_id_ = item.id;
        pending_boundary_ = boundary;
        pending_fade_ = item.timing.fade_frames;
        return;
    }
    if (item.kind == CommandKind::pause) { paused_ = true; }
    else if (item.kind == CommandKind::resume) { paused_ = false; }
    else if (item.kind == CommandKind::gain) { gain_ = item.gain; }
    else if (item.kind == CommandKind::stop) {
        cancel_pending(RequestPhase::cancelled);
        retire(current_); retire(outgoing_);
        current_ = outgoing_ = SamplePlan::no_payload;
        current_position_ = outgoing_position_ = 0;
    } else if (item.kind == CommandKind::cancel) {
        if (pending_ != SamplePlan::no_payload && pending_id_ == item.target) {
            cancel_pending(RequestPhase::cancelled);
        } else {
            const Receipt target = poll(item.target);
            finish(item.id, target.status == ScheduleStatus::expired ? RequestPhase::expired_target : RequestPhase::too_late);
            return;
        }
    }
    finish(item.id, RequestPhase::applied);
}
void LoopScheduler::ingest() noexcept {
    const std::size_t count = queued_;
    for (std::size_t i = 0; i < count; ++i) {
        const Command item = commands_[head_];
        head_ = (head_ + 1) % command_capacity;
        --queued_;
        for (Receipt& receipt : receipts_) {
            if (receipt.id == item.id) {
                receipt.phase = RequestPhase::admitted;
                receipt.admission_frame = frame_;
                break;
            }
        }
        apply(item);
    }
}
SamplePlan LoopScheduler::next_sample() noexcept {
    SamplePlan result{};
    result.frame = frame_;
    if (closed_ || paused_) { return result; }
    if (frame_ == maximum_frame) { shutdown(); return result; }
    // The previous SamplePlan has been consumed before this next model step.
    if (outgoing_ != SamplePlan::no_payload && fade_index_ == fade_frames_) {
        retire(outgoing_);
        outgoing_ = SamplePlan::no_payload;
    }
    if (pending_ != SamplePlan::no_payload && frame_ == pending_boundary_) {
        outgoing_ = current_;
        outgoing_position_ = current_position_;
        fade_index_ = 0;
        fade_frames_ = pending_fade_;
        current_ = pending_;
        current_position_ = 0;
        finish(pending_id_, RequestPhase::applied);
        pending_ = SamplePlan::no_payload;
        pending_id_ = 0;
        if (fade_frames_ <= 1) { retire(outgoing_); outgoing_ = SamplePlan::no_payload; }
    }
    if (current_ != SamplePlan::no_payload) {
        result.current = current_;
        result.current_position = current_position_;
        result.current_gain = gain_;
        current_position_ = (current_position_ + 1) % payloads_[current_].clip.frames;
    }
    if (outgoing_ != SamplePlan::no_payload) {
        result.outgoing = outgoing_;
        result.outgoing_position = outgoing_position_;
        const double weight = outgoing_weight(fade_index_, fade_frames_);
        result.outgoing_gain = gain_ * weight;
        outgoing_position_ = (outgoing_position_ + 1) % payloads_[outgoing_].clip.frames;
        ++fade_index_;
    }
    ++frame_;
    return result;
}
std::size_t LoopScheduler::collect_retired() noexcept {
    std::size_t count{};
    for (Payload& payload : payloads_) {
        if (payload.phase == PayloadPhase::retired) { payload = {}; ++count; }
    }
    return count;
}
void LoopScheduler::shutdown() noexcept {
    closed_ = true;
    queued_ = 0;
    current_ = outgoing_ = pending_ = SamplePlan::no_payload;
    for (Payload& payload : payloads_) {
        if (payload.phase == PayloadPhase::retained) { payload.phase = PayloadPhase::retired; }
    }
    for (Receipt& receipt : receipts_) {
        const bool complete = terminal(receipt.phase);
        if (!complete) { receipt.phase = RequestPhase::closed; receipt.application_frame = frame_; }
    }
}
PayloadPhase LoopScheduler::payload_phase(std::size_t index) const noexcept {
    if (index >= payload_capacity) { return PayloadPhase::free; }
    return payloads_[index].phase;
}
std::size_t LoopScheduler::retained_payloads() const noexcept {
    std::size_t count{};
    for (const Payload& payload : payloads_) {
        if (payload.phase != PayloadPhase::free) { ++count; }
    }
    return count;
}
}
