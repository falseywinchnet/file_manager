#include "scheduler.hpp"
#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>

namespace {
using namespace gui_forms::audio_experiment;
void require(bool condition, const char* message) {
    if (!condition) { throw std::runtime_error(message); }
}
void advance(LoopScheduler& scheduler, std::uint64_t count) {
    for (std::uint64_t i = 0; i < count; ++i) {
        const SamplePlan sample = scheduler.next_sample();
        static_cast<void>(sample);
    }
}
std::uint64_t brute_boundary(std::uint64_t frame, std::uint64_t position, ClipShape clip,
                            std::uint64_t cutoff, std::uint64_t available) {
    std::uint64_t time = frame;
    for (;;) {
        if (time > cutoff && time >= available && (position == 0 || position % clip.bar_frames == 0)) {
            return time;
        }
        ++time;
        position = (position + 1) % clip.frames;
    }
}
void boundary_math() {
    std::uint64_t cases{};
    for (std::uint64_t length = 1; length <= 23; ++length) {
        for (std::uint64_t bar = 1; bar <= length; ++bar) {
            const ClipShape clip{length, bar};
            for (std::uint64_t position = 0; position < length; ++position) {
                for (std::uint64_t lead = 0; lead <= 2 * length; ++lead) {
                    for (std::uint64_t availability = 0; availability <= 2 * length; availability += length) {
                        const BoundaryResult actual = next_boundary(100, position, clip, 100 + lead, 100 + availability);
                        const std::uint64_t expected = brute_boundary(100, position, clip, 100 + lead, 100 + availability);
                        require(actual.status == ScheduleStatus::ok && actual.frame == expected,
                                "boundary math agrees with independent frame-by-frame oracle");
                        ++cases;
                    }
                }
            }
        }
    }
    const ClipShape tier1{3177931, 99310};
    const BoundaryResult tail = next_boundary(0, 3177920, tier1, 0, 0);
    const BoundaryResult before = next_boundary(0, 0, {100, 20}, 19, 0);
    const BoundaryResult equal = next_boundary(0, 0, {100, 20}, 20, 0);
    const BoundaryResult after = next_boundary(0, 0, {100, 20}, 21, 0);
    require(tail.frame == 11, "actual loop end retains eleven-frame residual");
    require(before.frame == 20 && equal.frame == 40 && after.frame == 40, "strict cutoff equality");
    const std::uint64_t maximum = std::numeric_limits<std::uint64_t>::max();
    const BoundaryResult overflow = next_boundary(maximum - 3, 1, {100, 20}, maximum - 2, 0);
    const BoundaryResult invalid = next_boundary(0, 100, {100, 20}, 0, 0);
    require(overflow.status == ScheduleStatus::overflow && invalid.status == ScheduleStatus::invalid,
            "overflow and malformed shape rejected");
    std::cout << "Independent boundary oracle: " << cases << " cases.\n";
}
void switch_and_fade() {
    for (std::uint64_t fade = 0; fade <= 4; ++fade) {
        LoopScheduler scheduler{};
        const RequestResult first = scheduler.change({100, 20}, {10, 0});
        require(first.status == ScheduleStatus::ok && scheduler.poll(first.id).phase == RequestPhase::queued,
                "enqueue does not claim application");
        scheduler.ingest();
        require(scheduler.poll(first.id).phase == RequestPhase::admitted, "admission is separate");
        const SamplePlan initial = scheduler.next_sample();
        require(initial.frame == 0 && initial.current_position == 0 && initial.current_gain == 1,
                "empty transport begins immediately ignoring lead");
        advance(scheduler, 6);
        const RequestResult second = scheduler.change({71, 13}, {13, fade});
        scheduler.ingest();
        const Receipt admitted = scheduler.poll(second.id);
        require(admitted.admission_frame == 7, "actual ingestion frame recorded");
        advance(scheduler, 33); // cutoff20 excludes source boundary20, selecting40.
        require(scheduler.poll(second.id).phase == RequestPhase::admitted, "not applied before first incoming sample");
        const SamplePlan switched = scheduler.next_sample();
        require(switched.frame == 40 && switched.current_position == 0 && switched.current != initial.current,
                "switch starts frame zero at the strict source boundary");
        const Receipt applied = scheduler.poll(second.id);
        require(applied.phase == RequestPhase::applied && applied.application_frame == 40, "applied receipt at first sample");
        if (fade <= 1) { require(switched.outgoing == SamplePlan::no_payload, "zero and one frame fade cut immediately"); }
        else {
            require(switched.outgoing == initial.current && switched.outgoing_gain == 1, "fade first gain is one");
            for (std::uint64_t index = 1; index < fade; ++index) {
                const SamplePlan sample = scheduler.next_sample();
                const double expected = 1.0 - static_cast<double>(index) / static_cast<double>(fade - 1);
                require(sample.outgoing_gain == expected && sample.current_position == index, "exact linear fade endpoints");
            }
            require(scheduler.payload_phase(initial.current) == PayloadPhase::retained,
                    "last returned plan still retains outgoing index");
            const SamplePlan after = scheduler.next_sample();
            require(after.outgoing == SamplePlan::no_payload && scheduler.payload_phase(initial.current) == PayloadPhase::retired,
                    "outgoing retired only after last plan consumption");
        }
        const RequestResult late = scheduler.command(CommandKind::cancel, 1, second.id);
        scheduler.ingest();
        require(late.status == ScheduleStatus::ok && scheduler.poll(late.id).phase == RequestPhase::too_late,
                "cancelling applied request is explicitly too late");
    }
}
void pause_replace_stop() {
    LoopScheduler scheduler{};
    advance(scheduler, 9);
    require(scheduler.frame() == 9, "empty unpaused output advances timeline");
    const RequestResult paused = scheduler.command(CommandKind::pause);
    const RequestResult first = scheduler.change({100, 20}, {0, 0});
    scheduler.ingest();
    advance(scheduler, 200);
    require(paused.status == ScheduleStatus::ok && scheduler.frame() == 9 &&
            scheduler.poll(first.id).phase == RequestPhase::admitted, "paused admission freezes samples and pending");
    const RequestResult resume = scheduler.command(CommandKind::resume);
    scheduler.ingest();
    const SamplePlan start = scheduler.next_sample();
    require(resume.status == ScheduleStatus::ok && start.frame == 9, "resume starts at same transport frame");
    const RequestResult replaced = scheduler.change({80, 10}, {1, 20});
    const RequestResult replacement = scheduler.change({60, 15}, {1, 20});
    scheduler.ingest();
    require(scheduler.poll(replaced.id).phase == RequestPhase::replaced, "last admitted change replaces pending");
    advance(scheduler, 19);
    const SamplePlan begin_fade = scheduler.next_sample();
    require(begin_fade.frame == 29 && scheduler.poll(replacement.id).phase == RequestPhase::applied, "source grid unaffected by incoming grid");
    const RequestResult after_fade = scheduler.change({50, 5}, {10, 0});
    scheduler.ingest(); // frame30, fade ends49, incoming grid boundary59 selected.
    advance(scheduler, 29);
    const SamplePlan delayed = scheduler.next_sample();
    require(delayed.frame == 59 && delayed.current_position == 0 &&
            scheduler.poll(after_fade.id).application_frame == 59, "pending during fade keeps original cutoff and uses current grid");
    const RequestResult pending = scheduler.change({90, 30}, {100, 0});
    const RequestResult cancel = scheduler.command(CommandKind::cancel, 1, pending.id);
    scheduler.ingest();
    require(scheduler.poll(pending.id).phase == RequestPhase::cancelled &&
            scheduler.poll(cancel.id).phase == RequestPhase::applied, "cancellation linearizes at ingestion");
    const RequestResult pause = scheduler.command(CommandKind::pause);
    const RequestResult stop = scheduler.command(CommandKind::stop);
    scheduler.ingest();
    const std::uint64_t stopped_frame = scheduler.frame();
    advance(scheduler, 10);
    require(pause.status == ScheduleStatus::ok && stop.status == ScheduleStatus::ok && scheduler.paused() &&
            scheduler.frame() == stopped_frame, "stop preserves pause and monotonic time");
    const std::size_t collected = scheduler.collect_retired();
    require(collected > 0 && scheduler.retained_payloads() == 0, "producer acknowledges all stopped payloads");
}
void bounds_and_receipts() {
    LoopScheduler scheduler{};
    const RequestResult first = scheduler.change({100, 20}, {0, 0});
    scheduler.ingest();
    advance(scheduler, 1);
    const RequestResult pending = scheduler.change({100, 20}, {100, 0});
    scheduler.ingest();
    for (std::size_t i = 0; i < LoopScheduler::command_capacity; ++i) {
        const RequestResult item = scheduler.command(CommandKind::gain, .5);
        require(item.status == ScheduleStatus::ok, "bounded command admission");
    }
    const RequestResult refused = scheduler.command(CommandKind::cancel, 1, pending.id);
    require(refused.status == ScheduleStatus::full && scheduler.poll(pending.id).phase == RequestPhase::admitted,
            "full queue does not claim or perform cancellation");
    scheduler.ingest();
    const RequestResult invalid = scheduler.command(CommandKind::gain, std::numeric_limits<double>::quiet_NaN());
    const SamplePlan half = scheduler.next_sample();
    require(invalid.status == ScheduleStatus::invalid && half.current_gain == .5, "invalid gain preserves prior gain");
    for (std::size_t i = 0; i < 100; ++i) {
        const RequestResult item = scheduler.command(CommandKind::gain, .5);
        require(item.status == ScheduleStatus::ok, "terminal receipt eviction admits work");
        scheduler.ingest();
    }
    require(scheduler.poll(first.id).status == ScheduleStatus::expired && scheduler.poll(pending.id).phase == RequestPhase::admitted,
            "terminal history expires; pending history never evicted");
    const RequestResult expired_cancel = scheduler.command(CommandKind::cancel, 1, first.id);
    scheduler.ingest();
    require(scheduler.poll(expired_cancel.id).phase == RequestPhase::expired_target, "unknown history is not inferred too late");
    scheduler.shutdown();
    require(scheduler.poll(pending.id).phase == RequestPhase::closed, "shutdown closes pending receipt");
    const std::size_t shutdown_collected = scheduler.collect_retired();
    require(shutdown_collected == 2 && scheduler.retained_payloads() == 0, "shutdown retires slots for producer collection");

    LoopScheduler payloads{};
    for (std::size_t i = 0; i < LoopScheduler::payload_capacity; ++i) {
        const RequestResult item = payloads.change({100, 20}, {0, 0});
        require(item.status == ScheduleStatus::ok, "retired but uncollected slots remain charged");
        payloads.ingest();
    }
    const RequestResult no_payload = payloads.change({100, 20}, {0, 0});
    require(no_payload.status == ScheduleStatus::full, "payload backpressure independent of empty command queue");
    const std::size_t reclaimed = payloads.collect_retired();
    const RequestResult retry = payloads.change({100, 20}, {0, 0});
    require(reclaimed == 31 && retry.status == ScheduleStatus::ok, "reuse only after producer observes retirement");
    const std::uint64_t maximum = std::numeric_limits<std::uint64_t>::max();
    LoopScheduler ids(1, 0, maximum);
    const RequestResult last = ids.command(CommandKind::pause);
    const RequestResult overflow = ids.command(CommandKind::resume);
    require(last.id == maximum && overflow.status == ScheduleStatus::overflow, "request IDs never wrap");
    LoopScheduler clock(1, maximum - 1);
    advance(clock, 2);
    require(clock.closed() && clock.frame() == maximum, "render frame overflow closes before wrapping");
}
void rejected_change_and_admission() {
    const std::uint64_t maximum = std::numeric_limits<std::uint64_t>::max();
    LoopScheduler near_end(2, maximum - 30);
    const RequestResult first = near_end.change({20, 4}, {0, 0});
    near_end.ingest();
    advance(near_end, 1);
    const RequestResult preserved = near_end.change({20, 4}, {0, 0});
    near_end.ingest();
    const RequestResult bad = near_end.change({20, 4}, {100, 0});
    near_end.ingest();
    require(first.status == ScheduleStatus::ok && near_end.poll(bad.id).phase == RequestPhase::rejected &&
            near_end.poll(preserved.id).phase == RequestPhase::admitted, "arithmetic refusal preserves previously pending change");
    advance(near_end, 4);
    require(near_end.poll(preserved.id).phase == RequestPhase::applied, "preserved change still applies");

    LoopScheduler early{};
    LoopScheduler late{};
    const RequestResult early_first = early.change({100, 20}, {0, 0});
    const RequestResult late_first = late.change({100, 20}, {0, 0});
    early.ingest(); late.ingest();
    advance(early, 1); advance(late, 1);
    const RequestResult early_change = early.change({100, 20}, {0, 0});
    const RequestResult late_change = late.change({100, 20}, {0, 0});
    early.ingest();
    advance(late, 20);
    late.ingest();
    advance(early, 40); advance(late, 20);
    require(early_first.status == ScheduleStatus::ok && late_first.status == ScheduleStatus::ok &&
            early.poll(early_change.id).application_frame == 20 && late.poll(late_change.id).application_frame == 40,
            "different callback admission frames legitimately choose different boundaries");
    LoopScheduler unpaused_stop{};
    const RequestResult stop = unpaused_stop.command(CommandKind::stop);
    unpaused_stop.ingest();
    advance(unpaused_stop, 5);
    require(stop.status == ScheduleStatus::ok && unpaused_stop.frame() == 5, "unpaused stopped output advances");
}
}
int main() {
    try {
        boundary_math();
        switch_and_fade();
        pause_replace_stop();
        bounds_and_receipts();
        rejected_change_and_admission();
        std::cout << "Serial loop scheduler timing, capacity and retirement model pass.\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
