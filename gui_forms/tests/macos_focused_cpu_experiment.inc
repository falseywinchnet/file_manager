// Opt-in native deferred-paint diagnostic. No change to the static cost probe.
namespace {

constexpr std::size_t focused_window_count = 9U;
constexpr std::size_t focused_interval_count = 4U;
constexpr std::chrono::seconds focused_settle(3);
constexpr std::chrono::seconds focused_interval(10);

struct FocusedCpuProbe final {
    // Models belong to the synchronous native run; one deferred callback borrows
    // this stack owner at a time. Native views are borrowed only inside callbacks.
    std::array<gui_forms::Window*, focused_window_count> models{};
    std::array<std::shared_ptr<gui_forms::TextBox>, focused_window_count> editors{};
    std::array<std::string, focused_window_count> before{};
    std::array<NSRect, focused_window_count> bounds{};
    std::array<double, focused_window_count> scales{};
    std::array<double, 4> expected_extents{};
    std::uint64_t expected_source_bytes{0};
    std::uint64_t expected_focused_draws{0};
    std::uint64_t process_start{0};
    std::uint64_t thread_start{0};
    std::chrono::steady_clock::time_point wall_start{};
    std::size_t phase{0};
    std::size_t closed{0};
    bool measuring{false};
    bool completed{false};
    bool have_geometry{false};
    std::string failure{};
};

void focused_require(const bool condition, const char* const message) {
    if (!condition) throw std::runtime_error(message);
}

bool focused_mode(const std::size_t phase) noexcept {
    const bool focused = phase == 0U || phase == 3U;
    return focused;
}

NSString* focused_title(const std::size_t index) {
    NSString* title = [NSString stringWithFormat:@"Focused CPU window %zu", index];
    return title;
}

NSWindow* focused_window(const std::size_t index) {
    NSString* title = focused_title(index);
    NSWindow* window = find_window(title);
    focused_require(window != nil, "focused CPU window missing");
    return window;
}

std::uint64_t focused_cpu_clock(const clockid_t clock_id) {
    timespec value{};
    const int status = clock_gettime(clock_id, &value);
    focused_require(status == 0 && value.tv_sec >= 0 && value.tv_nsec >= 0 &&
        value.tv_nsec < 1000000000L, "CPU clock read failed");
    const std::uint64_t seconds = static_cast<std::uint64_t>(value.tv_sec);
    const std::uint64_t fraction = static_cast<std::uint64_t>(value.tv_nsec);
    const std::uint64_t limit = std::numeric_limits<std::uint64_t>::max();
    focused_require(seconds <= (limit - fraction) / 1000000000U,
                    "CPU clock conversion overflow");
    const std::uint64_t nanoseconds = seconds * 1000000000U + fraction;
    return nanoseconds;
}

std::uint64_t focused_delta(const std::string& before, const std::string& after,
                            const std::string& key) {
    const std::uint64_t first = counter(before, key);
    const std::uint64_t last = counter(after, key);
    focused_require(last >= first, "host counter regressed");
    const std::uint64_t delta = last - first;
    return delta;
}

void advance_focused_cpu(void* const context);

void queue_focused_cpu(FocusedCpuProbe& probe, const std::chrono::seconds delay) {
    const std::chrono::nanoseconds duration = delay;
    const std::int64_t nanoseconds = duration.count();
    const dispatch_time_t deadline = dispatch_time(DISPATCH_TIME_NOW, nanoseconds);
    dispatch_after_f(deadline, dispatch_get_main_queue(), &probe, advance_focused_cpu);
}

void prepare_focused_phase(FocusedCpuProbe& probe) {
    gui_forms::Window& model = *probe.models[0];
    bool changed{false};
    if (focused_mode(probe.phase)) changed = model.request_focus(probe.editors[0]);
    else changed = model.request_focus({});
    focused_require(changed, "could not set requested model focus");
    // Focus invalidation and native transactions settle outside the CPU interval.
    queue_focused_cpu(probe, focused_settle);
}

void check_focused_state(FocusedCpuProbe& probe, const std::size_t index) {
    NSWindow* window = focused_window(index);
    gui_forms::Window& model = *probe.models[index];
    const bool primary = index == 0U;
    focused_require(static_cast<bool>(window.isVisible) == primary &&
        model.occluded() == !primary, "visibility or occlusion changed");
    const bool expect_focus = primary && focused_mode(probe.phase);
    focused_require(static_cast<bool>(model.focused_control()) == expect_focus,
                    "model focus changed");
    focused_require(model.next_wake().has_value() == expect_focus,
                    "unexpected autonomous deadline state");
    if (primary) {
        focused_require(window.isKeyWindow && NSApp.isActive,
                        "native key window or application activation changed");
    }
    const NSRect bounds = window.contentView.bounds;
    const double scale = window.backingScaleFactor;
    if (!probe.have_geometry) {
        probe.bounds[index] = bounds;
        probe.scales[index] = scale;
    } else {
        focused_require(NSEqualRects(bounds, probe.bounds[index]) &&
            scale == probe.scales[index], "view extent or backing scale changed");
    }
}

void begin_focused_interval(FocusedCpuProbe& probe) {
    for (std::size_t index = 0; index < focused_window_count; ++index) {
        check_focused_state(probe, index);
        NSWindow* window = focused_window(index);
        const gui_forms::MetricsSnapshot metrics = (*probe.models[index]).metrics_snapshot();
        focused_require(metrics.renderer_name.find("bundled fonts (optional Unicode fallback)") !=
            std::string::npos, "bundled fonts not ready; CPU interval rejected");
        probe.before[index] = [window.contentView hostJSON];
        (*probe.models[index]).reset_activity_metrics();
        const BOOL enabled = [window.contentView setCpuAttributionEnabled:YES];
        focused_require(enabled == YES, "could not enable private CPU diagnostic");
    }
    probe.have_geometry = true;
    // Sequential CPU readings use the same clocks at each boundary. Their small
    // skew is not interpreted as other-thread CPU. Logging/snapshots stay outside.
    probe.process_start = focused_cpu_clock(CLOCK_PROCESS_CPUTIME_ID);
    probe.thread_start = focused_cpu_clock(CLOCK_THREAD_CPUTIME_ID);
    probe.wall_start = std::chrono::steady_clock::now();
    probe.measuring = true;
    queue_focused_cpu(probe, focused_interval);
}

void verify_focused_window(FocusedCpuProbe& probe, const std::size_t index,
                           std::uint64_t& draw_cpu, std::uint64_t& collect_cpu) {
    check_focused_state(probe, index);
    NSWindow* window = focused_window(index);
    const BOOL disabled = [window.contentView setCpuAttributionEnabled:NO];
    focused_require(disabled == YES, "could not disable private CPU diagnostic");
    const std::array<std::uint64_t, 13> cpu = [window.contentView cpuAttributionSnapshot];
    const std::string after = [window.contentView hostJSON];
    const gui_forms::MetricsSnapshot metrics = (*probe.models[index]).metrics_snapshot();
    const bool active = index == 0U && focused_mode(probe.phase);
    const std::uint64_t draws = focused_delta(probe.before[index], after, "native_draw_count");
    const std::uint64_t wakes = focused_delta(probe.before[index], after, "scheduled_wake_count");
    focused_require(cpu[4] == 0U && cpu[5] == 0U && cpu[6] == 0U && cpu[7] == 0U,
                    "CPU read, overlap, saturation or extent diagnostic failure");
    focused_require(cpu[0] == draws && cpu[12] == draws,
                    "draw attribution count mismatch");
    const std::uint64_t collections = focused_delta(probe.before[index], after, "damage_collection_count");
    focused_require(cpu[2] == collections, "damage collection attribution count mismatch");
    focused_require(counter(after, "native_callback_faults") == 0U &&
        focused_delta(probe.before[index], after, "display_tick_count") == 0U,
        "native fault or unexpected display-link work");
    focused_require(metrics.measure_passes == 0U && metrics.arrange_passes == 0U &&
        metrics.focus_transitions == 0U && metrics.input_events == 0U &&
        metrics.occlusion_suspensions == 0U && metrics.occlusion_resumes == 0U,
        "layout, focus, input or occlusion changed in measured interval");
    if (active) {
        // armWakeTimer floors to milliseconds, so an early wake may legitimately
        // rearm once before the same caret deadline. Do not equate wakes to paints.
        focused_require(draws >= 18U && draws <= 19U && wakes >= draws &&
            wakes <= 2U * draws + 1U &&
            metrics.frame_deadlines_fired == draws && metrics.paint_passes == draws,
            "normal ten-second caret counts diverged");
        focused_require(cpu[10] >= 430000000U && cpu[11] <= 630000000U,
                        "caret draw cadence outside predeclared bounds");
        if (probe.phase == 0U) probe.expected_focused_draws = draws;
        else focused_require(draws == probe.expected_focused_draws,
                             "focused intervals have different draw counts");
        const std::array<double, 4> current{
            extent(after, "last_native_dirty_area"), extent(after, "last_frame_damage_bounds_area"),
            extent(after, "last_cg_destination_area"), extent(after, "last_cg_clip_bounds_area")};
        const std::uint64_t bytes = counter(after, "last_cg_source_bytes");
        for (const double area : current) focused_require(area > 0.0, "empty caret presentation extent");
        focused_require(bytes > 0U, "empty presentation source");
        if (probe.phase == 0U) {
            probe.expected_extents = current;
            probe.expected_source_bytes = bytes;
        } else {
            focused_require(current == probe.expected_extents && bytes == probe.expected_source_bytes,
                            "focused interval presentation extents differ");
        }
        const double expected_area = current[1] * static_cast<double>(draws);
        focused_require(std::abs(metrics.painted_damage_area - expected_area) < 1.0e-6,
                        "aggregate caret damage differs from observed extents");
    } else {
        focused_require(draws == 0U && wakes == 0U && collections == 0U &&
            metrics.paint_passes == 0U && metrics.frame_deadlines_fired == 0U &&
            cpu[1] == 0U && cpu[3] == 0U, "cleared or hidden window was not quiet");
    }
    const std::array<std::string_view, 6> phases{
        "raster_prepare", "retained", "raster_finish", "cg_setup", "cg_draw", "cg_release"};
    for (const std::string_view phase : phases) {
        const std::string prefix = "paint_" + std::string(phase);
        const std::string flag = "\"" + prefix + "_saturated\":false";
        focused_require(after.find(flag) != std::string::npos, "wall phase saturated");
        const std::uint64_t calls = focused_delta(probe.before[index], after, prefix + "_calls");
        focused_require(calls == draws, "synchronous wall phase count differs");
    }
    // Only the primary is permitted measured work; hidden values were proven zero.
    if (index == 0U) {
        draw_cpu = cpu[1];
        collect_cpu = cpu[3];
    }
    const std::string metrics_json = metrics.to_json();
    std::printf("focused-cpu-window|interval=%zu|window=%zu|draw_calls=%llu|draw_cpu_ns=%llu|collect_calls=%llu|collect_cpu_ns=%llu|clock_failures=%llu|overlaps=%llu|saturated=%llu|extent_mismatches=%llu|min_draw_gap_ns=%llu|max_draw_gap_ns=%llu|metrics=%s|host=%s\n",
        probe.phase, index, static_cast<unsigned long long>(cpu[0]),
        static_cast<unsigned long long>(cpu[1]), static_cast<unsigned long long>(cpu[2]),
        static_cast<unsigned long long>(cpu[3]), static_cast<unsigned long long>(cpu[4]),
        static_cast<unsigned long long>(cpu[5]), static_cast<unsigned long long>(cpu[6]),
        static_cast<unsigned long long>(cpu[7]), static_cast<unsigned long long>(cpu[10]),
        static_cast<unsigned long long>(cpu[11]), metrics_json.c_str(), after.c_str());
}

void end_focused_interval(FocusedCpuProbe& probe) {
    const std::chrono::steady_clock::time_point wall_end = std::chrono::steady_clock::now();
    const std::uint64_t thread_end = focused_cpu_clock(CLOCK_THREAD_CPUTIME_ID);
    const std::uint64_t process_end = focused_cpu_clock(CLOCK_PROCESS_CPUTIME_ID);
    probe.measuring = false;
    const std::chrono::duration<double> wall = wall_end - probe.wall_start;
    focused_require(wall.count() >= 10.0 && wall.count() <= 10.1,
                    "ten-second interval exceeded declared duration bound");
    focused_require(thread_end >= probe.thread_start && process_end >= probe.process_start,
                    "interval CPU clock regressed");
    const std::uint64_t thread_cpu = thread_end - probe.thread_start;
    const std::uint64_t process_cpu = process_end - probe.process_start;
    std::uint64_t draw_cpu{0};
    std::uint64_t collect_cpu{0};
    for (std::size_t index = 0; index < focused_window_count; ++index) {
        verify_focused_window(probe, index, draw_cpu, collect_cpu);
    }
    focused_require(draw_cpu <= thread_cpu && collect_cpu <= thread_cpu - draw_cpu,
                    "attributed CPU exceeds same-clock main-thread interval");
    const std::uint64_t rest_cpu = thread_cpu - draw_cpu - collect_cpu;
    const double process_percent = static_cast<double>(process_cpu) / 1.0e7 / wall.count();
    const char* const mode = focused_mode(probe.phase) ? "focused" : "cleared";
    std::printf("focused-cpu-interval|interval=%zu|mode=%s|wall_seconds=%.9f|process_cpu_ns=%llu|main_thread_cpu_ns=%llu|draw_cpu_ns=%llu|collect_cpu_ns=%llu|remaining_main_cpu_ns=%llu|one_core_percent=%.9f|remaining_is_not_CA_attribution=true\n",
        probe.phase, mode, wall.count(), static_cast<unsigned long long>(process_cpu),
        static_cast<unsigned long long>(thread_cpu), static_cast<unsigned long long>(draw_cpu),
        static_cast<unsigned long long>(collect_cpu), static_cast<unsigned long long>(rest_cpu),
        process_percent);
    std::fflush(stdout);
    ++probe.phase;
    if (probe.phase == focused_interval_count) {
        probe.completed = true;
        NSWindow* primary = focused_window(0U);
        [primary performClose:nil];
    } else {
        prepare_focused_phase(probe);
    }
}

void advance_focused_cpu(void* const context) {
    FocusedCpuProbe& probe = *static_cast<FocusedCpuProbe*>(context);
    try {
        if (probe.measuring) end_focused_interval(probe);
        else begin_focused_interval(probe);
    } catch (const std::exception& error) {
        probe.failure = error.what();
        std::fprintf(stderr, "focused-cpu-rejected|interval=%zu|reason=%s\n",
                     probe.phase, probe.failure.c_str());
        std::fflush(stdout);
        // Disable diagnostics before teardown even when validation failed midway.
        for (std::size_t index = 0; index < focused_window_count; ++index) {
            NSWindow* window = find_window(focused_title(index));
            if (window != nil) [window.contentView setCpuAttributionEnabled:NO];
        }
        // Close owned children before the primary stops the native run loop.
        for (std::size_t remaining = focused_window_count; remaining > 1U; --remaining) {
            const std::size_t index = remaining - 1U;
            NSWindow* child = find_window(focused_title(index));
            if (child != nil) [child performClose:nil];
        }
        NSWindow* primary = find_window(focused_title(0U));
        if (primary != nil) [primary performClose:nil];
    }
}

class StartFocusedCpu final {
public:
    explicit StartFocusedCpu(FocusedCpuProbe& probe) noexcept : probe_(probe) {}
    void operator()(const std::function<void()>, const std::function<void()>) const {
        prepare_focused_phase(probe_);
    }
private:
    FocusedCpuProbe& probe_;
};

class CloseFocusedCpu final {
public:
    explicit CloseFocusedCpu(FocusedCpuProbe& probe) noexcept : probe_(probe) {}
    void operator()() const { ++probe_.closed; }
private:
    FocusedCpuProbe& probe_;
};

int run_focused_cpu_experiment() {
    FocusedCpuProbe probe{};
    std::vector<gui_forms::host::MacApplicationWindow> windows{};
    windows.reserve(focused_window_count);
    for (std::size_t index = 0; index < focused_window_count; ++index) {
        gui_forms::host::MacApplicationWindow entry{};
        entry.stable_id = "focused.cpu." + std::to_string(index);
        if (index != 0U) entry.owner_id = "focused.cpu.0";
        const gui_forms::StableId editor_id("focused.editor." + std::to_string(index));
        probe.editors[index] = std::make_shared<gui_forms::TextBox>(editor_id);
        (*probe.editors[index]).set_multiline(true);
        entry.model = std::make_unique<gui_forms::Window>(probe.editors[index], gui_forms::Size{640.0, 480.0});
        probe.models[index] = entry.model.get();
        const bool cleared = (*entry.model).request_focus({});
        focused_require(cleared, "could not initialize cleared focus");
        entry.options.title = "Focused CPU window " + std::to_string(index);
        entry.options.initial_size = {640.0, 480.0};
        entry.options.initially_visible = index == 0U;
        entry.options.print_metrics_on_close = false;
        entry.options.closed = CloseFocusedCpu(probe);
        if (index == 0U) entry.options.visibility_ready = StartFocusedCpu(probe);
        windows.push_back(std::move(entry));
    }
    std::printf("focused-cpu-protocol|order=focused,cleared,cleared,focused|interval_seconds=10|settle_seconds=3|duration_max_seconds=10.1|caret_draws=18..19_equal_between_focused_intervals|native_wakes=draws..2*draws+1|draw_gap_ms=430..630|native_deferred_paint=true|clock_overhead_included=true|performance_threshold=none\n");
    const int status = gui_forms::host::run_macos_application(std::move(windows));
    if (status != 0 || !probe.completed || probe.closed != focused_window_count || !probe.failure.empty()) return 1;
    std::printf("focused-cpu-attribution=accepted-comparable-intervals|CA_causation=unmeasured\n");
    return 0;
}

} // namespace
