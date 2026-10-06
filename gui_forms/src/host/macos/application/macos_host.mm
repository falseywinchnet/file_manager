#import <AppKit/AppKit.h>
#import <CoreGraphics/CoreGraphics.h>
#import <CoreVideo/CoreVideo.h>
#import <UniformTypeIdentifiers/UniformTypeIdentifiers.h>

#include "macos_host.hpp"
#include "live_surface_damage.hpp"
#include "gui_forms/text/types/text_types.hpp"
#include "../../../core/damage/device_damage/device_damage.hpp"
#include "../../../render/skia/raster/skia_raster.hpp"

#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <exception>
#include <limits>
#include <memory>
#include <optional>
#include <time.h>
#include <string>
#include <string_view>
#include <unordered_set>
#include <utility>
#include <vector>

using gui_forms::DamageRegion;
using gui_forms::DragAction;
using gui_forms::DragBinaryData;
using gui_forms::DragDataItem;
using gui_forms::DragEffect;
using gui_forms::DragEvent;
using gui_forms::DragFileListData;
using gui_forms::DragLimits;
using gui_forms::DragTextData;
using gui_forms::HostActivationEvent;
using gui_forms::HostAttachEvent;
using gui_forms::HostCapability;
using gui_forms::HostCloseReason;
using gui_forms::HostCloseRequest;
using gui_forms::HostClipboardTextResult;
using gui_forms::HostClosedEvent;
using gui_forms::HostDispatchResult;
using gui_forms::HostDialogChoice;
using gui_forms::HostDialogOutcome;
using gui_forms::HostDialogRequest;
using gui_forms::HostDialogResult;
using gui_forms::HostColorDialogRequest;
using gui_forms::HostColorDialogResult;
using gui_forms::HostDisplayEvent;
using gui_forms::HostFolderDialogRequest;
using gui_forms::HostMessageButtons;
using gui_forms::HostMessageDialogRequest;
using gui_forms::HostMessageDialogResult;
using gui_forms::HostMessageIcon;
using gui_forms::HostOpenFileDialogRequest;
using gui_forms::HostPathDialogResult;
using gui_forms::HostSaveFileDialogRequest;
using gui_forms::HostEvent;
using gui_forms::HostEventPayload;
using gui_forms::HostOcclusionEvent;
using gui_forms::HostResizeEvent;
using gui_forms::HostScaleEvent;
using gui_forms::HostMonitor;
using gui_forms::HostMonitorResult;
using gui_forms::HostServiceError;
using gui_forms::HostServiceStatus;
using gui_forms::HostSoundCue;
using gui_forms::HostSoundCueRequest;
using gui_forms::HostServices;
using gui_forms::HostSession;
using gui_forms::HostShutdownEvent;
using gui_forms::HostTooltipRequest;
using gui_forms::KeyAction;
using gui_forms::KeyEvent;
using gui_forms::LiveSurfacePresentation;
using gui_forms::Modifier;
using gui_forms::PaintReceipt;
using GFPoint = gui_forms::Point;
using gui_forms::PointerAction;
using gui_forms::PointerButton;
using gui_forms::PointerEvent;
using gui_forms::SemanticAction;
using gui_forms::SemanticNode;
using gui_forms::SemanticRole;
using gui_forms::SemanticState;
using GFRect = gui_forms::Rect;
using GFSize = gui_forms::Size;
using gui_forms::TextInputEvent;
using gui_forms::Window;
using gui_forms::CursorKind;
using gui_forms::render::SkiaRaster;

namespace {
// Private fixture wire image: header then 256 rows, each with 13 uint64 fields.
// Header: version, count, overflow, ambiguity, clock failure, serial exhaustion,
// unpaired draw, unfinished draw, pending deadlines, enabled, reserved x3.
// Row: kind, time ns, arm, desired ns, delay ns, wake, poll, fired, next ns,
// linked poll, draw, pending count, flags. No native pointers cross this boundary.
using MacCausalSnapshot = std::array<std::array<std::uint64_t, 13>, 257>;

enum class MacCausalKind : std::uint64_t { arm = 1, wake, poll, draw_entry, draw_exit };
enum MacCausalFlag : std::uint64_t {
    causal_has_deadline = 1U, causal_has_next = 2U, causal_early_wake = 4U,
    causal_untimed_poll = 8U, causal_ambiguous = 16U, causal_unpaired = 32U,
    causal_clock_invalid = 64U, causal_inherited_arm = 128U,
};
struct MacCausalRow final {
    MacCausalKind kind{MacCausalKind::arm};
    std::uint64_t time_ns{0};
    std::uint64_t arm{0};
    std::uint64_t desired_ns{0};
    std::uint64_t delay_ns{0};
    std::uint64_t wake{0};
    std::uint64_t poll{0};
    std::uint64_t fired{0};
    std::uint64_t next_ns{0};
    std::uint64_t linked_poll{0};
    std::uint64_t draw{0};
    std::uint64_t pending{0};
    std::uint64_t flags{0};
};
struct MacCausalTrace final {
    // Main-executor owned; allocated only when the private diagnostic is enabled.
    // Overflow retains the first 256 rows and permanently invalidates pairing.
    std::array<MacCausalRow, 256> rows{};
    std::size_t count{0};
    bool enabled{false};
    bool overflow{false};
    bool ambiguous{false};
    bool clock_failure{false};
    bool serial_exhausted{false};
    bool unpaired{false};
    bool drawing{false};
    bool has_deadline{false};
    std::uint64_t last_time_ns{0};
    std::uint64_t arm_serial{0};
    std::uint64_t wake_serial{0};
    std::uint64_t poll_serial{0};
    std::uint64_t draw_serial{0};
    std::uint64_t active_wake{0};
    std::uint64_t desired_ns{0};
    std::uint64_t delay_ns{0};
    std::uint64_t pending{0};
    std::uint64_t linked_poll{0};
    std::uint64_t drawing_pending{0};
    std::uint64_t drawing_poll{0};

    [[nodiscard]] std::uint64_t timestamp(const gui_forms::FrameTime value) noexcept {
        // macOS FrameClock is steady_clock with signed nanosecond ticks. Enforce
        // the unit instead of overflowing an unchecked duration conversion.
        static_assert(gui_forms::FrameClock::period::num == 1);
        static_assert(gui_forms::FrameClock::period::den == 1000000000);
        static_assert(std::numeric_limits<gui_forms::FrameClock::duration::rep>::digits <= 64);
        const gui_forms::FrameClock::duration::rep ticks = value.time_since_epoch().count();
        if (ticks < 0) {
            clock_failure = true;
            return 0U;
        }
        const std::uint64_t result = static_cast<std::uint64_t>(ticks);
        return result;
    }
    void advance(std::uint64_t& serial) noexcept {
        if (serial == std::numeric_limits<std::uint64_t>::max()) serial_exhausted = true;
        else ++serial;
    }
    void seed(const std::optional<gui_forms::FrameTime>& deadline) noexcept {
        if (!enabled) return;
        has_deadline = deadline.has_value();
        if (deadline) desired_ns = timestamp(*deadline);
    }
    [[nodiscard]] MacCausalRow make_row(const MacCausalKind kind,
                                       const gui_forms::FrameTime time) noexcept {
        MacCausalRow row{};
        row.kind = kind;
        row.time_ns = timestamp(time);
        row.arm = arm_serial;
        row.desired_ns = desired_ns;
        row.delay_ns = delay_ns;
        row.wake = active_wake;
        row.poll = poll_serial;
        row.draw = draw_serial;
        row.linked_poll = linked_poll;
        row.pending = pending;
        if (has_deadline) row.flags |= causal_has_deadline;
        if (arm_serial == 0U) row.flags |= causal_inherited_arm;
        return row;
    }
    void append(MacCausalRow row) noexcept {
        if (count != 0U && row.time_ns < last_time_ns) clock_failure = true;
        last_time_ns = row.time_ns;
        if (clock_failure) row.flags |= causal_clock_invalid;
        if (count == rows.size()) {
            overflow = true;
            return;
        }
        rows[count] = row;
        ++count;
    }
    void record_arm(const gui_forms::FrameTime now,
                    const std::optional<gui_forms::FrameTime>& deadline,
                    const std::int64_t requested_delay_ns) noexcept {
        if (!enabled) return;
        advance(arm_serial);
        seed(deadline);
        if (!deadline) desired_ns = 0U;
        if (requested_delay_ns < 0) {
            clock_failure = true;
            delay_ns = 0U;
        } else delay_ns = static_cast<std::uint64_t>(requested_delay_ns);
        const MacCausalRow row = make_row(MacCausalKind::arm, now);
        append(row);
    }
    void record_wake(const gui_forms::FrameTime now) noexcept {
        if (!enabled) return;
        const bool nested = active_wake != 0U;
        if (nested) ambiguous = true;
        advance(wake_serial);
        active_wake = wake_serial;
        MacCausalRow row = make_row(MacCausalKind::wake, now);
        if (nested) row.flags |= causal_ambiguous;
        if (has_deadline && row.time_ns < desired_ns) row.flags |= causal_early_wake;
        append(row);
    }
    void record_poll(const gui_forms::FrameTime now, const std::uint64_t fired,
                     const std::optional<gui_forms::FrameTime>& next) noexcept {
        if (!enabled) return;
        advance(poll_serial);
        if (fired != 0U) {
            if (pending != 0U || fired != 1U) ambiguous = true;
            if (fired > std::numeric_limits<std::uint64_t>::max() - pending) {
                serial_exhausted = true;
                pending = std::numeric_limits<std::uint64_t>::max();
            } else pending += fired;
            linked_poll = pending == 1U ? poll_serial : 0U;
        }
        MacCausalRow row = make_row(MacCausalKind::poll, now);
        row.fired = fired;
        if (next) {
            row.next_ns = timestamp(*next);
            row.flags |= causal_has_next;
        }
        if (active_wake == 0U) row.flags |= causal_untimed_poll;
        if (pending > 1U) row.flags |= causal_ambiguous;
        append(row);
    }
    void record_draw_entry(const gui_forms::FrameTime now) noexcept {
        if (!enabled) return;
        if (drawing) ambiguous = true;
        drawing = true;
        advance(draw_serial);
        drawing_pending = pending;
        drawing_poll = linked_poll;
        MacCausalRow row = make_row(MacCausalKind::draw_entry, now);
        if (pending == 0U) {
            unpaired = true;
            row.flags |= causal_unpaired;
        }
        if (pending > 1U) row.flags |= causal_ambiguous;
        append(row);
        pending = 0U;
        linked_poll = 0U;
    }
    void record_draw_exit(const gui_forms::FrameTime now) noexcept {
        if (!enabled) return;
        MacCausalRow row = make_row(MacCausalKind::draw_exit, now);
        row.pending = drawing_pending;
        row.linked_poll = drawing_poll;
        if (!drawing || drawing_pending == 0U) {
            unpaired = true;
            row.flags |= causal_unpaired;
        }
        if (drawing_pending > 1U) row.flags |= causal_ambiguous;
        append(row);
        drawing = false;
    }
    [[nodiscard]] MacCausalSnapshot snapshot() const noexcept {
        MacCausalSnapshot output{};
        output[0] = {1U, static_cast<std::uint64_t>(count), overflow, ambiguous,
            clock_failure, serial_exhausted, unpaired, drawing, pending, enabled, 0U, 0U, 0U};
        for (std::size_t index = 0; index < count; ++index) {
            const MacCausalRow& row = rows[index];
            output[index + 1U] = {static_cast<std::uint64_t>(row.kind), row.time_ns,
                row.arm, row.desired_ns, row.delay_ns, row.wake, row.poll, row.fired,
                row.next_ns, row.linked_poll, row.draw, row.pending, row.flags};
        }
        return output;
    }
};

class MacCausalWakeScope final {
public:
    explicit MacCausalWakeScope(MacCausalTrace& trace) noexcept
        : trace_(trace), previous_wake_(trace.active_wake) {
        if (trace_.enabled) trace_.record_wake(gui_forms::FrameClock::now());
    }
    ~MacCausalWakeScope() { trace_.active_wake = previous_wake_; }
    MacCausalWakeScope(const MacCausalWakeScope&) = delete;
    MacCausalWakeScope& operator=(const MacCausalWakeScope&) = delete;
private:
    MacCausalTrace& trace_; // View-owned trace outlives this callback scope.
    const std::uint64_t previous_wake_;
};

// Private opt-in diagnostics. Array layout is shared only with the native test:
// draw calls/ns, collect calls/ns, clock failures, overlapping scopes, saturation,
// extent mismatches, first/last draw wall ns, min/max draw gap ns, draw count.
// All access is on the main executor. Disabled views never sample a CPU clock.
struct MacCpuAttribution final {
    std::array<std::uint64_t, 13> values{};
    std::array<double, 4> extents{};
    std::size_t source_bytes{0};
    bool enabled{false};
    bool active{false};

    void add(const std::size_t index, const std::uint64_t amount) noexcept {
        const std::uint64_t limit = std::numeric_limits<std::uint64_t>::max();
        if (amount > limit - values[index]) {
            values[index] = limit;
            values[6] = 1U;
        } else {
            values[index] += amount;
        }
    }

    void record_draw(const std::uint64_t wall_ns,
                     const std::array<double, 4>& current,
                     const std::size_t bytes) noexcept {
        if (values[12] == 0U) {
            extents = current;
            source_bytes = bytes;
            values[8] = wall_ns;
        } else {
            if (current != extents || bytes != source_bytes) add(7U, 1U);
            if (wall_ns < values[9]) {
                add(4U, 1U);
            } else {
                const std::uint64_t gap = wall_ns - values[9];
                if (values[12] == 1U) values[10] = gap;
                else values[10] = std::min(values[10], gap);
                values[11] = std::max(values[11], gap);
            }
        }
        values[9] = wall_ns;
        add(12U, 1U);
    }
};

bool read_mac_thread_cpu(std::uint64_t& output) noexcept {
    timespec value{};
    const int status = clock_gettime(CLOCK_THREAD_CPUTIME_ID, &value);
    if (status != 0 || value.tv_sec < 0 || value.tv_nsec < 0 ||
        value.tv_nsec >= 1000000000L) return false;
    const std::uint64_t seconds = static_cast<std::uint64_t>(value.tv_sec);
    const std::uint64_t fraction = static_cast<std::uint64_t>(value.tv_nsec);
    const std::uint64_t limit = std::numeric_limits<std::uint64_t>::max();
    if (seconds > (limit - fraction) / 1000000000U) return false;
    output = seconds * 1000000000U + fraction;
    return true;
}

class MacCpuScope final {
public:
    explicit MacCpuScope(MacCpuAttribution& owner, const std::size_t index) noexcept
        : owner_(owner), index_(index) {
        if (owner_.active) {
            owner_.add(5U, 1U);
            return;
        }
        owner_.active = true;
        owns_scope_ = true;
        valid_ = read_mac_thread_cpu(start_);
        if (!valid_) owner_.add(4U, 1U);
    }
    ~MacCpuScope() {
        if (!owns_scope_) return;
        if (valid_) {
            std::uint64_t end{0};
            const bool read = read_mac_thread_cpu(end);
            if (!read || end < start_) {
                owner_.add(4U, 1U);
            } else {
                const std::uint64_t elapsed = end - start_;
                owner_.add(index_, 1U);
                owner_.add(index_ + 1U, elapsed);
            }
        }
        owner_.active = false;
    }
    MacCpuScope(const MacCpuScope&) = delete;
    MacCpuScope& operator=(const MacCpuScope&) = delete;
private:
    MacCpuAttribution& owner_; // Borrowed view field outlives this stack scope.
    const std::size_t index_;
    std::uint64_t start_{0};
    bool owns_scope_{false};
    bool valid_{false};
};

// Completed synchronous wall phases; not CPU, deferred AppKit work or bytes copied.
struct MacPaintPhase final {
    std::uint64_t calls{0};
    std::uint64_t nanoseconds{0};
    std::uint64_t maximum_nanoseconds{0};
    bool saturated{false};

    void record(const std::chrono::steady_clock::time_point start,
                const std::chrono::steady_clock::time_point end) noexcept {
        const std::chrono::nanoseconds elapsed =
            std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
        const std::uint64_t duration = elapsed.count() > 0
            ? static_cast<std::uint64_t>(elapsed.count()) : 0U;
        const std::uint64_t limit = std::numeric_limits<std::uint64_t>::max();
        if (calls == limit) saturated = true;
        else ++calls;
        if (duration > limit - nanoseconds) {
            nanoseconds = limit;
            saturated = true;
        } else {
            nanoseconds += duration;
        }
        maximum_nanoseconds = std::max(maximum_nanoseconds, duration);
    }

    [[nodiscard]] std::string json_fields(const std::string_view name) const {
        const std::string prefix = ",\"paint_" + std::string(name);
        const std::string result = prefix + "_calls\":" + std::to_string(calls) +
            prefix + "_nanoseconds\":" + std::to_string(nanoseconds) +
            prefix + "_maximum_nanoseconds\":" + std::to_string(maximum_nanoseconds) +
            prefix + "_saturated\":" + (saturated ? "true" : "false");
        return result;
    }
};
} // namespace

static std::uint64_t host_now_nanoseconds() {
    return static_cast<std::uint64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count());
}

static std::uint64_t host_event_nanoseconds(NSEvent* event) {
    if (event == nil || !std::isfinite(event.timestamp) || event.timestamp < 0.0) {
        return host_now_nanoseconds();
    }
    return static_cast<std::uint64_t>(event.timestamp * 1'000'000'000.0);
}

static Modifier modifiers_for(NSEventModifierFlags flags) {
    Modifier result = Modifier::none;
    if ((flags & NSEventModifierFlagShift) != 0) {
        result = result | Modifier::shift;
    }
    if ((flags & NSEventModifierFlagControl) != 0) {
        result = result | Modifier::control;
    }
    if ((flags & NSEventModifierFlagOption) != 0) {
        result = result | Modifier::alt;
    }
    if ((flags & NSEventModifierFlagCommand) != 0) {
        result = result | Modifier::meta;
    }
    return result;
}

static PointerButton button_for(NSInteger button_number) {
    if (button_number == 0) {
        return PointerButton::primary;
    }
    if (button_number == 1) {
        return PointerButton::secondary;
    }
    if (button_number == 2) {
        return PointerButton::middle;
    }
    return PointerButton::none;
}

// AppKit keyCode values describe positions in Apple's hardware vocabulary.
// Translate them at the adapter boundary so retained controls see the same USB
// HID usages on every host. Text remains owned by NSTextInputClient below.
static std::uint32_t physical_key_for(unsigned short key_code) {
    switch (key_code) {
    case 0: return 0x04U;   // A
    case 1: return 0x16U;   // S
    case 2: return 0x07U;   // D
    case 3: return 0x09U;   // F
    case 4: return 0x0BU;   // H
    case 5: return 0x0AU;   // G
    case 6: return 0x1DU;   // Z
    case 7: return 0x1BU;   // X
    case 8: return 0x06U;   // C
    case 9: return 0x19U;   // V
    case 11: return 0x05U;  // B
    case 12: return 0x14U;  // Q
    case 13: return 0x1AU;  // W
    case 14: return 0x08U;  // E
    case 15: return 0x15U;  // R
    case 16: return 0x1CU;  // Y
    case 17: return 0x17U;  // T
    case 18: return 0x1EU;  // 1
    case 19: return 0x1FU;  // 2
    case 20: return 0x20U;  // 3
    case 21: return 0x21U;  // 4
    case 22: return 0x23U;  // 6
    case 23: return 0x22U;  // 5
    case 24: return 0x2EU;  // =
    case 25: return 0x26U;  // 9
    case 26: return 0x24U;  // 7
    case 27: return 0x2DU;  // -
    case 28: return 0x25U;  // 8
    case 29: return 0x27U;  // 0
    case 30: return 0x30U;  // ]
    case 31: return 0x12U;  // O
    case 32: return 0x18U;  // U
    case 33: return 0x2FU;  // [
    case 34: return 0x0CU;  // I
    case 35: return 0x13U;  // P
    case 36: return 0x28U;  // Return
    case 37: return 0x0FU;  // L
    case 38: return 0x0DU;  // J
    case 39: return 0x34U;  // '
    case 40: return 0x0EU;  // K
    case 41: return 0x33U;  // ;
    case 42: return 0x31U;  // backslash
    case 43: return 0x36U;  // comma
    case 44: return 0x38U;  // slash
    case 45: return 0x11U;  // N
    case 46: return 0x10U;  // M
    case 47: return 0x37U;  // period
    case 48: return 0x2BU;  // Tab
    case 49: return 0x2CU;  // Space
    case 50: return 0x35U;  // grave
    case 51: return 0x2AU;  // Backspace
    case 53: return 0x29U;  // Escape
    case 64: return 0x6CU;  // F17
    case 65: return 0x63U;  // Keypad decimal
    case 67: return 0x55U;  // Keypad multiply
    case 69: return 0x57U;  // Keypad plus
    case 71: return 0x53U;  // Keypad clear
    case 75: return 0x54U;  // Keypad divide
    case 76: return 0x58U;  // Keypad enter
    case 78: return 0x56U;  // Keypad minus
    case 81: return 0x67U;  // Keypad equals
    case 82: return 0x62U;  // Keypad 0
    case 83: return 0x59U;  // Keypad 1
    case 84: return 0x5AU;  // Keypad 2
    case 85: return 0x5BU;  // Keypad 3
    case 86: return 0x5CU;  // Keypad 4
    case 87: return 0x5DU;  // Keypad 5
    case 88: return 0x5EU;  // Keypad 6
    case 89: return 0x5FU;  // Keypad 7
    case 91: return 0x60U;  // Keypad 8
    case 92: return 0x61U;  // Keypad 9
    case 96: return 0x3EU;  // F5
    case 97: return 0x3FU;  // F6
    case 98: return 0x40U;  // F7
    case 99: return 0x3CU;  // F3
    case 100: return 0x41U; // F8
    case 101: return 0x42U; // F9
    case 103: return 0x44U; // F11
    case 105: return 0x68U; // F13
    case 106: return 0x6BU; // F16
    case 107: return 0x69U; // F14
    case 109: return 0x43U; // F10
    case 111: return 0x45U; // F12
    case 113: return 0x6AU; // F15
    case 114: return 0x49U; // Help/Insert
    case 115: return 0x4AU; // Home
    case 116: return 0x4BU; // Page Up
    case 117: return 0x4CU; // Forward Delete
    case 118: return 0x3DU; // F4
    case 119: return 0x4DU; // End
    case 120: return 0x3BU; // F2
    case 121: return 0x4EU; // Page Down
    case 122: return 0x3AU; // F1
    case 123: return 0x50U; // Left
    case 124: return 0x4FU; // Right
    case 125: return 0x51U; // Down
    case 126: return 0x52U; // Up
    default: return 0U;
    }
}

static DragEffect drag_effects_for(NSDragOperation operations) {
    DragEffect result = DragEffect::none;
    if ((operations & NSDragOperationCopy) != 0) {
        result = result | DragEffect::copy;
    }
    if ((operations & NSDragOperationMove) != 0) {
        result = result | DragEffect::move;
    }
    if ((operations & NSDragOperationLink) != 0) {
        result = result | DragEffect::link;
    }
    return result;
}

static NSDragOperation native_drag_operation(DragEffect effect) {
    switch (effect) {
    case DragEffect::copy: return NSDragOperationCopy;
    case DragEffect::move: return NSDragOperationMove;
    case DragEffect::link: return NSDragOperationLink;
    case DragEffect::none: return NSDragOperationNone;
    }
    return NSDragOperationNone;
}

static std::vector<DragDataItem> drag_items_for(NSPasteboard* pasteboard) {
    std::vector<DragDataItem> result;
    std::size_t total_bytes = 0;
    NSArray<NSURL*>* urls = [pasteboard
        readObjectsForClasses:@[[NSURL class]]
                    options:@{NSPasteboardURLReadingFileURLsOnlyKey: @YES}];
    DragFileListData files;
    if (urls.count > DragLimits::maximum_paths) {
        return {};
    }
    for (NSURL* url in urls) {
        NSString* native_path = url.path;
        const NSUInteger byte_count =
            [native_path lengthOfBytesUsingEncoding:NSUTF8StringEncoding];
        if (byte_count == 0 || byte_count > DragLimits::maximum_path_bytes ||
            byte_count > DragLimits::maximum_total_bytes - total_bytes) {
            return {};
        }
        const char* path = native_path.UTF8String;
        if (path == nullptr) {
            return {};
        }
        files.paths_utf8.emplace_back(path, byte_count);
        total_bytes += byte_count;
    }
    if (!files.paths_utf8.empty()) {
        result.emplace_back(std::move(files));
    }
    NSString* text = [pasteboard stringForType:NSPasteboardTypeString];
    if (text != nil) {
        const NSUInteger byte_count =
            [text lengthOfBytesUsingEncoding:NSUTF8StringEncoding];
        if (byte_count > DragLimits::maximum_text_bytes ||
            byte_count > DragLimits::maximum_total_bytes - total_bytes) {
            return {};
        }
        const char* bytes = text.UTF8String;
        if (bytes != nullptr) {
            result.emplace_back(DragTextData{std::string(bytes, byte_count)});
            total_bytes += byte_count;
        }
    }
    NSPasteboardType type = [pasteboard availableTypeFromArray:@[@"public.data"]];
    if (type != nil && ![type isEqualToString:NSPasteboardTypeString] &&
        ![type isEqualToString:NSPasteboardTypeFileURL]) {
        NSData* data = [pasteboard dataForType:type];
        const char* media = type.UTF8String;
        if (data != nil && data.length != 0 && media != nullptr &&
            [type lengthOfBytesUsingEncoding:NSUTF8StringEncoding] <=
                DragLimits::maximum_media_type_bytes &&
            data.length <= DragLimits::maximum_total_bytes - total_bytes) {
            const std::uint8_t* begin = static_cast<const std::uint8_t*>(data.bytes);
            result.emplace_back(DragBinaryData{
                media, std::vector<std::uint8_t>(begin, begin + data.length)});
        }
    }
    if (result.size() > DragLimits::maximum_items) {
        return {};
    }
    return result;
}

static std::string utf8_string(id value) {
    NSString* string = nil;
    if ([value isKindOfClass:[NSAttributedString class]]) {
        string = [(NSAttributedString*)value string];
    } else if ([value isKindOfClass:[NSString class]]) {
        string = (NSString*)value;
    }
    if (string == nil) {
        return {};
    }
    const char* bytes = [string UTF8String];
    return bytes == nullptr ? std::string{} : std::string(bytes);
}

static bool register_bundle_typeface(SkiaRaster& raster,
                                     NSString* resource_name,
                                     gui_forms::FontRole role,
                                     std::uint16_t weight,
                                     bool italic = false,
                                     NSString* extension = @"ttf",
                                     bool optional = false) {
    NSURL* url = [[NSBundle mainBundle] URLForResource:resource_name
                                        withExtension:extension
                                         subdirectory:@"fonts"];
    if (url == nil) {
        return optional;
    }
    const bool registered = raster.register_typeface_file(role, weight, italic, url.fileSystemRepresentation);
    return registered;
}

static bool register_bundle_fallback_typeface(SkiaRaster& raster,
                                              NSString* resource_name,
                                              NSString* extension,
                                              std::uint16_t weight = 400,
                                              bool italic = false) {
    NSURL* url = [[NSBundle mainBundle] URLForResource:resource_name
                                        withExtension:extension
                                         subdirectory:@"fonts"];
    if (url == nil) return true;
    const bool registered = raster.register_fallback_typeface_file(weight, italic, url.fileSystemRepresentation);
    return registered;
}

namespace {

NSString* native_string(std::string_view text) {
    return [[NSString alloc] initWithBytes:text.data()
                                    length:text.size()
                                  encoding:NSUTF8StringEncoding];
}

NSString* native_accessibility_role(SemanticRole role) {
    switch (role) {
    case SemanticRole::group: return NSAccessibilityGroupRole;
    case SemanticRole::static_text: return NSAccessibilityStaticTextRole;
    case SemanticRole::button: return NSAccessibilityButtonRole;
    case SemanticRole::check_box: return NSAccessibilityCheckBoxRole;
    case SemanticRole::radio_button: return NSAccessibilityRadioButtonRole;
    case SemanticRole::link: return NSAccessibilityLinkRole;
    case SemanticRole::text_box:
    case SemanticRole::numeric_field: return NSAccessibilityTextFieldRole;
    case SemanticRole::list: return NSAccessibilityListRole;
    case SemanticRole::list_item: return NSAccessibilityRowRole;
    case SemanticRole::check_list_item: return NSAccessibilityCheckBoxRole;
    case SemanticRole::combo_box: return NSAccessibilityComboBoxRole;
    case SemanticRole::slider: return NSAccessibilitySliderRole;
    case SemanticRole::scroll_bar: return NSAccessibilityScrollBarRole;
    case SemanticRole::progress_bar: return NSAccessibilityProgressIndicatorRole;
    case SemanticRole::image: return NSAccessibilityImageRole;
    case SemanticRole::tab_group: return NSAccessibilityTabGroupRole;
    case SemanticRole::tab: return NSAccessibilityRadioButtonRole;
    case SemanticRole::split_pane: return NSAccessibilitySplitGroupRole;
    case SemanticRole::tool_tip: return NSAccessibilityStaticTextRole;
    case SemanticRole::date_picker: return NSAccessibilityComboBoxRole;
    case SemanticRole::calendar: return NSAccessibilityGroupRole;
    case SemanticRole::date_cell: return NSAccessibilityButtonRole;
    case SemanticRole::property_grid: return NSAccessibilityGroupRole;
    case SemanticRole::property_group: return NSAccessibilityGroupRole;
    case SemanticRole::property_row: return NSAccessibilityStaticTextRole;
    case SemanticRole::menu_bar: return NSAccessibilityMenuBarRole;
    case SemanticRole::menu_bar_item: return NSAccessibilityMenuBarItemRole;
    case SemanticRole::menu: return NSAccessibilityMenuRole;
    case SemanticRole::menu_item: return NSAccessibilityMenuItemRole;
    // AppKit exposes no separator role. A visual menu separator is structural,
    // never an adjustable splitter, so retain it as a neutral group.
    case SemanticRole::separator: return NSAccessibilityGroupRole;
    case SemanticRole::generic: return NSAccessibilityGroupRole;
    }
    return NSAccessibilityGroupRole;
}

bool semantic_has_action(const SemanticNode& node, SemanticAction action) {
    return std::find(node.actions.begin(), node.actions.end(), action) !=
           node.actions.end();
}

} // namespace

@class GUIFormsAccessibilityElement;

@interface GUIFormsView : NSView <NSTextInputClient, NSDraggingDestination> {
    std::unique_ptr<Window> _model;
    std::unique_ptr<HostSession> _hostSession;
    std::unique_ptr<HostServices> _hostServices;
    gui_forms::SubscriptionToken _closeRequestSubscription;
    std::uint64_t _nextHostSequence;
    BOOL _hostAttached;
    SkiaRaster _raster;
    BOOL _rasterHasContent;
    DamageRegion _pendingDamage;
    NSMutableAttributedString* _markedText;
    NSRange _selectedRange;
    NSTrackingArea* _trackingArea;
    dispatch_source_t _wakeSource;
    CVDisplayLinkRef _displayLink;
    std::atomic<bool> _displayTickQueued;
    BOOL _hostOccluded;
    std::vector<LiveSurfacePresentation> _pendingLivePresentations;
    // Objective-C++ constructs these vectors empty with the view. Reuse storage
    // across ticks; immutable leases survive producer reconfiguration until
    // this pass has finished consuming their pixels.
    std::vector<gui_forms::LiveSurfaceFrame> _liveFrames;
    std::vector<GFRect> _opaqueLiveClips;
    NSPanel* _tooltipPanel;
    NSTimer* _tooltipTimer;
    std::uint64_t _lastSemanticGeneration;
    std::uint64_t _accessibilityCacheGeneration;
    std::uint64_t _nativeCallbackFaults;
    BOOL _initialHostOccluded;
    std::uint64_t _scheduledWakeCount;
    std::uint64_t _damageCollectionCount;
    std::uint64_t _nativeDrawCount;
    std::uint64_t _displayTickCount;
    MacPaintPhase _rasterPreparePhase;
    MacPaintPhase _retainedPaintPhase;
    MacPaintPhase _rasterFinishPhase;
    MacPaintPhase _cgSetupPhase;
    MacPaintPhase _cgDrawPhase;
    MacPaintPhase _cgReleasePhase;
    MacCpuAttribution _cpuAttribution;
    std::unique_ptr<MacCausalTrace> _causalTrace;
    double _lastNativeDirtyArea;
    double _lastFrameDamageBoundsArea;
    double _lastCGDestinationArea;
    double _lastCGClipBoundsArea;
    std::size_t _lastCGSourceBytes;
    NSArray* _semanticAccessibilityChildren;
    NSMutableDictionary<NSString*, GUIFormsAccessibilityElement*>*
        _semanticAccessibilityElements;
    BOOL _transparentFullSizeContent;
    std::vector<std::string> _windowDragRegionIds;
}
- (instancetype)initWithModel:(std::unique_ptr<Window>)model
       transparentFullSizeContent:(BOOL)transparentFullSizeContent
              windowDragRegionIds:(std::vector<std::string>)windowDragRegionIds;
- (BOOL)initializeHost;
- (void)installCloseRequestHandler:(std::function<void(HostCloseRequest&)>)handler;
- (HostDispatchResult)dispatchHostPayload:(HostEventPayload)payload
                         timestampNanoseconds:(std::uint64_t)timestamp;
- (BOOL)requestCloseHiding:(BOOL)hiding;
- (void)notifyClosed;
- (void)notifyActivation:(BOOL)active;
- (void)notifyOcclusion:(BOOL)occluded;
- (void)notifyScaleChanged;
- (void)notifyDisplaysChanged;
- (void)screenParametersChanged:(NSNotification*)notification;
- (void)collectDamage;
- (void)drainPostedWork;
- (void)armWakeTimer;
- (void)scheduledWake;
- (void)startDisplayLinkIfNeeded;
- (void)stopDisplayLink;
- (void)queueDisplayLinkTick;
- (void)displayLinkTick;
- (void)recordNativeCallbackFault:(const char*)operation
                          message:(const char*)message;
- (void)drawRetainedRect:(NSRect)dirtyRect;
- (void)prepareForShutdown;
- (BOOL)acceptsTitleUpdate;
- (HostDialogResult)showHostDialog:(const HostDialogRequest&)request;
- (HostClipboardTextResult)readHostClipboard;
- (HostServiceStatus)writeHostClipboard:(std::string_view)text;
- (HostServiceStatus)showHostTooltip:(const HostTooltipRequest&)request;
- (void)hideHostTooltip;
- (DragEvent)dragEventFor:(id<NSDraggingInfo>)sender action:(DragAction)action;
- (std::string)metricsJSON;
- (std::string)hostJSON;
- (BOOL)setCpuAttributionEnabled:(const BOOL)enabled;
- (std::array<std::uint64_t, 13>)cpuAttributionSnapshot;
- (MacCausalSnapshot)cpuCausalTraceSnapshot;
- (NSRect)accessibilityFrameForSemanticBounds:(GFRect)bounds;
- (BOOL)performSemanticAction:(SemanticAction)action
                     stableId:(NSString*)stableId
                        value:(NSString*)value;
@end

static CVReturn gui_forms_display_link_callback(
    CVDisplayLinkRef, const CVTimeStamp*, const CVTimeStamp*,
    CVOptionFlags, CVOptionFlags*, void* context) {
    @autoreleasepool {
        GUIFormsView* view = (__bridge GUIFormsView*)context;
        if (view != nil) [view queueDisplayLinkTick];
    }
    return kCVReturnSuccess;
}

@interface GUIFormsAccessibilityElement : NSAccessibilityElement {
    __weak GUIFormsView* _owner;
    SemanticNode _node;
    NSArray* _semanticChildren;
}
- (instancetype)initWithNode:(const SemanticNode&)node
                       owner:(GUIFormsView*)owner
                      parent:(id)parent;
- (void)updateWithNode:(const SemanticNode&)node
                 owner:(GUIFormsView*)owner
                parent:(id)parent;
- (void)setSemanticChildren:(NSArray*)children;
@end

@implementation GUIFormsAccessibilityElement

- (instancetype)initWithNode:(const SemanticNode&)node
                       owner:(GUIFormsView*)owner
                      parent:(id)parent {
    self = [super init];
    if (self != nil) {
        _owner = owner;
        _node = node;
        self.accessibilityParent = parent;
        NSMutableArray* children = [[NSMutableArray alloc]
            initWithCapacity:_node.children.size()];
        for (const SemanticNode& childNode : _node.children) {
            GUIFormsAccessibilityElement* child =
                [[GUIFormsAccessibilityElement alloc] initWithNode:childNode
                                                              owner:owner
                                                             parent:self];
            [children addObject:child];
        }
        _semanticChildren = children;
    }
    return self;
}

- (void)updateWithNode:(const SemanticNode&)node
                 owner:(GUIFormsView*)owner
                parent:(id)parent {
    _owner = owner;
    _node = node;
    self.accessibilityParent = parent;
}

- (void)setSemanticChildren:(NSArray*)children {
    _semanticChildren = [children copy];
}

- (BOOL)isAccessibilityElement { return YES; }
- (NSString*)accessibilityRole { return native_accessibility_role(_node.role); }
- (NSString*)accessibilityLabel { return native_string(_node.name); }
- (NSString*)accessibilityHelp { return native_string(_node.description); }
- (NSString*)accessibilityIdentifier { return native_string(_node.stable_id); }
- (NSArray*)accessibilityChildren { return _semanticChildren; }
- (NSRect)accessibilityFrame {
    return _owner == nil ? NSZeroRect
                         : [_owner accessibilityFrameForSemanticBounds:_node.bounds];
}
- (BOOL)isAccessibilityEnabled {
    return gui_forms::has_semantic_state(_node.states, SemanticState::enabled);
}
- (BOOL)isAccessibilityFocused {
    return gui_forms::has_semantic_state(_node.states, SemanticState::focused);
}
- (BOOL)isAccessibilitySelected {
    return gui_forms::has_semantic_state(_node.states, SemanticState::selected);
}
- (BOOL)isAccessibilityExpanded {
    return gui_forms::has_semantic_state(_node.states, SemanticState::expanded);
}
- (BOOL)isAccessibilityProtectedContent {
    return gui_forms::has_semantic_state(
        _node.states, SemanticState::protected_content);
}
- (id)accessibilityValue {
    if (_node.role == SemanticRole::check_box ||
        _node.role == SemanticRole::check_list_item ||
        _node.role == SemanticRole::radio_button) {
        if (gui_forms::has_semantic_state(_node.states, SemanticState::mixed)) {
            return @2;
        }
        return @(gui_forms::has_semantic_state(_node.states, SemanticState::checked));
    }
    if (_node.numeric_value) return @(*_node.numeric_value);
    return native_string(_node.value);
}
- (id)accessibilityMinValue {
    return _node.minimum_value ? @(*_node.minimum_value) : nil;
}
- (id)accessibilityMaxValue {
    return _node.maximum_value ? @(*_node.maximum_value) : nil;
}
- (BOOL)accessibilityPerformPress {
    if (_owner == nil) return NO;
    SemanticAction action = SemanticAction::press;
    if (!semantic_has_action(_node, action)) {
        if (semantic_has_action(_node, SemanticAction::select)) {
            action = SemanticAction::select;
        } else if (semantic_has_action(_node, SemanticAction::collapse)) {
            action = SemanticAction::collapse;
        } else if (semantic_has_action(_node, SemanticAction::expand)) {
            action = SemanticAction::expand;
        } else {
            return NO;
        }
    }
    return [_owner performSemanticAction:action
                                 stableId:native_string(_node.stable_id)
                                    value:nil];
}
- (void)setAccessibilitySelected:(BOOL)selected {
    if (selected && _owner != nil &&
        semantic_has_action(_node, SemanticAction::select)) {
        [_owner performSemanticAction:SemanticAction::select
                              stableId:native_string(_node.stable_id)
                                 value:nil];
    }
}
- (BOOL)accessibilityPerformShowMenu {
    if (_owner == nil) return NO;
    const SemanticAction action = semantic_has_action(_node, SemanticAction::show_menu)
        ? SemanticAction::show_menu : SemanticAction::expand;
    if (!semantic_has_action(_node, action)) return NO;
    return [_owner performSemanticAction:action
                                 stableId:native_string(_node.stable_id)
                                    value:nil];
}
- (BOOL)accessibilityPerformIncrement {
    if (_owner == nil || !semantic_has_action(_node, SemanticAction::increment)) return NO;
    return [_owner performSemanticAction:SemanticAction::increment
                                 stableId:native_string(_node.stable_id)
                                    value:nil];
}
- (BOOL)accessibilityPerformDecrement {
    if (_owner == nil || !semantic_has_action(_node, SemanticAction::decrement)) return NO;
    return [_owner performSemanticAction:SemanticAction::decrement
                                 stableId:native_string(_node.stable_id)
                                    value:nil];
}
- (void)setAccessibilityFocused:(BOOL)focused {
    if (focused && _owner != nil &&
        semantic_has_action(_node, SemanticAction::focus)) {
        [_owner performSemanticAction:SemanticAction::focus
                              stableId:native_string(_node.stable_id)
                                 value:nil];
    }
}
- (void)setAccessibilityValue:(id)value {
    if (_owner == nil || !semantic_has_action(_node, SemanticAction::set_value)) return;
    NSString* text = [value isKindOfClass:[NSString class]]
        ? (NSString*)value : [value stringValue];
    [_owner performSemanticAction:SemanticAction::set_value
                          stableId:native_string(_node.stable_id)
                             value:text];
}

- (BOOL)isAccessibilitySelectorAllowed:(SEL)selector {
    if (selector == @selector(setAccessibilityFocused:)) {
        return semantic_has_action(_node, SemanticAction::focus);
    }
    if (selector == @selector(setAccessibilityValue:)) {
        return semantic_has_action(_node, SemanticAction::set_value);
    }
    if (selector == @selector(setAccessibilitySelected:)) {
        return semantic_has_action(_node, SemanticAction::select);
    }
    if (selector == @selector(accessibilityPerformPress)) {
        return semantic_has_action(_node, SemanticAction::press) ||
               semantic_has_action(_node, SemanticAction::select) ||
               semantic_has_action(_node, SemanticAction::expand) ||
               semantic_has_action(_node, SemanticAction::collapse);
    }
    if (selector == @selector(accessibilityPerformShowMenu)) {
        return semantic_has_action(_node, SemanticAction::show_menu) ||
               semantic_has_action(_node, SemanticAction::expand);
    }
    if (selector == @selector(accessibilityPerformIncrement)) {
        return semantic_has_action(_node, SemanticAction::increment);
    }
    if (selector == @selector(accessibilityPerformDecrement)) {
        return semantic_has_action(_node, SemanticAction::decrement);
    }
    return [super isAccessibilitySelectorAllowed:selector];
}

- (BOOL)accessibilityIsAttributeSettable:(NSAccessibilityAttributeName)attribute {
    if ([attribute isEqualToString:NSAccessibilityFocusedAttribute]) {
        return semantic_has_action(_node, SemanticAction::focus);
    }
    if ([attribute isEqualToString:NSAccessibilityValueAttribute]) {
        return semantic_has_action(_node, SemanticAction::set_value);
    }
    if ([attribute isEqualToString:NSAccessibilitySelectedAttribute]) {
        return semantic_has_action(_node, SemanticAction::select);
    }
    return [super accessibilityIsAttributeSettable:attribute];
}

@end


static GUIFormsAccessibilityElement* reconcile_accessibility_element(
    const SemanticNode& node,
    GUIFormsView* owner,
    id parent,
    NSDictionary<NSString*, GUIFormsAccessibilityElement*>* previous,
    NSMutableDictionary<NSString*, GUIFormsAccessibilityElement*>* next) {
    NSString* identifier = native_string(node.stable_id);
    GUIFormsAccessibilityElement* element = previous[identifier];
    if (element == nil) {
        element = [[GUIFormsAccessibilityElement alloc] initWithNode:node
                                                               owner:owner
                                                              parent:parent];
    } else {
        [element updateWithNode:node owner:owner parent:parent];
    }
    next[identifier] = element;
    NSMutableArray* children = [[NSMutableArray alloc]
        initWithCapacity:node.children.size()];
    for (const SemanticNode& childNode : node.children) {
        [children addObject:reconcile_accessibility_element(
            childNode, owner, element, previous, next)];
    }
    [element setSemanticChildren:children];
    return element;
}

class DrainPostedWorkWake final {
public:
    explicit DrainPostedWorkWake(GUIFormsView* view) noexcept
        : view_(view) {}

    void operator()() const
    {
        __weak GUIFormsView* weakView = view_;
        dispatch_async(dispatch_get_main_queue(), ^{
            GUIFormsView* strongView = weakView;
            if (strongView != nil) [strongView drainPostedWork];
        });
    }

private:
    __weak GUIFormsView* view_;
};

class CollectDamageWake final {
public:
    explicit CollectDamageWake(GUIFormsView* view) noexcept
        : view_(view) {}

    void operator()() const
    {
        __weak GUIFormsView* weakView = view_;
        dispatch_async(dispatch_get_main_queue(), ^{
            GUIFormsView* strongView = weakView;
            if (strongView != nil) [strongView collectDamage];
        });
    }

private:
    __weak GUIFormsView* view_;
};

@implementation GUIFormsView

- (instancetype)initWithModel:(std::unique_ptr<Window>)model
       transparentFullSizeContent:(BOOL)transparentFullSizeContent
              windowDragRegionIds:(std::vector<std::string>)windowDragRegionIds {
    self = [super initWithFrame:NSMakeRect(0.0, 0.0, 980.0, 680.0)];
    if (self != nil) {
        _model = std::move(model);
        _hostServices = gui_forms::host::make_macos_host_services();
        _hostSession = std::make_unique<HostSession>(
            *_model, gui_forms::host::macos_capabilities(), _hostServices.get());
        _nextHostSequence = 1;
        _hostAttached = NO;
        _lastSemanticGeneration = 0;
        _accessibilityCacheGeneration = 0;
        _nativeCallbackFaults = 0;
        _initialHostOccluded = YES;
        _scheduledWakeCount = 0;
        _damageCollectionCount = 0;
        _nativeDrawCount = 0;
        _displayTickCount = 0;
        _rasterPreparePhase = {};
        _retainedPaintPhase = {};
        _rasterFinishPhase = {};
        _cgSetupPhase = {};
        _cgDrawPhase = {};
        _cgReleasePhase = {};
        _cpuAttribution = {};
        _causalTrace = nullptr;
        _lastNativeDirtyArea = 0.0;
        _lastFrameDamageBoundsArea = 0.0;
        _lastCGDestinationArea = 0.0;
        _lastCGClipBoundsArea = 0.0;
        _lastCGSourceBytes = 0U;
        _transparentFullSizeContent = transparentFullSizeContent;
        _windowDragRegionIds = std::move(windowDragRegionIds);
        _semanticAccessibilityChildren = nil;
        _semanticAccessibilityElements = [[NSMutableDictionary alloc] init];
        _markedText = [[NSMutableAttributedString alloc] init];
        _selectedRange = NSMakeRange(NSNotFound, 0);
        _displayLink = nullptr;
        _displayTickQueued.store(false, std::memory_order_relaxed);
        _hostOccluded = NO;
        if (CVDisplayLinkCreateWithActiveCGDisplays(&_displayLink) ==
            kCVReturnSuccess) {
            CVDisplayLinkSetOutputCallback(
                _displayLink, gui_forms_display_link_callback,
                (__bridge void*)self);
        } else {
            _displayLink = nullptr;
        }
        _wakeSource = dispatch_source_create(
            DISPATCH_SOURCE_TYPE_TIMER, 0, 0, dispatch_get_main_queue());
        __weak GUIFormsView* weakSelf = self;
        dispatch_source_set_event_handler(_wakeSource, ^{
            GUIFormsView* strongSelf = weakSelf;
            if (strongSelf != nil) {
                [strongSelf scheduledWake];
            }
        });
        dispatch_resume(_wakeSource);
        dispatch_source_set_timer(_wakeSource, DISPATCH_TIME_FOREVER,
                                  DISPATCH_TIME_FOREVER, 0);
        (*_model).set_dispatch_wake_handler(DrainPostedWorkWake(self));
        (*_model).set_paint_wake_handler(CollectDamageWake(self));
        [self setWantsLayer:NO];
        [self registerForDraggedTypes:@[
            NSPasteboardTypeString, NSPasteboardTypeFileURL, @"public.data"]];
        const bool fonts_ready =
            register_bundle_typeface(_raster, @"PortsmouthRapids",
                                     gui_forms::FontRole::control, 400) &&
            register_bundle_typeface(_raster, @"PortsmouthRapids-Bold",
                                     gui_forms::FontRole::control, 700) &&
            register_bundle_typeface(_raster, @"Carlito-Regular",
                                     gui_forms::FontRole::content, 400) &&
            register_bundle_typeface(_raster, @"Carlito-Bold",
                                     gui_forms::FontRole::content, 700) &&
            register_bundle_typeface(_raster, @"Carlito-Italic",
                                     gui_forms::FontRole::content, 400, true) &&
            register_bundle_typeface(_raster, @"Carlito-BoldItalic",
                                     gui_forms::FontRole::content, 700, true) &&
            register_bundle_typeface(_raster, @"Cousine-Regular",
                                     gui_forms::FontRole::monospace, 400) &&
            register_bundle_typeface(_raster, @"Cousine-Bold",
                                     gui_forms::FontRole::monospace, 700, false, @"ttf", true) &&
            register_bundle_typeface(_raster, @"Cousine-Italic",
                                     gui_forms::FontRole::monospace, 400, true, @"ttf", true) &&
            register_bundle_typeface(_raster, @"Cousine-BoldItalic",
                                     gui_forms::FontRole::monospace, 700, true, @"ttf", true) &&
            register_bundle_fallback_typeface(
                _raster, @"NotoSansCJKjp-Regular", @"otf") &&
            register_bundle_fallback_typeface(
                _raster, @"NotoSansArabic-Regular", @"ttf") &&
            register_bundle_fallback_typeface(
                _raster, @"NotoSansHebrew-Regular", @"ttf") &&
            register_bundle_fallback_typeface(
                _raster, @"NotoSansDevanagari-Regular", @"ttf") &&
            register_bundle_fallback_typeface(
                _raster, @"NotoSansBengali-Regular", @"ttf") &&
            register_bundle_fallback_typeface(
                _raster, @"NotoSansGurmukhi-Regular", @"ttf") &&
            register_bundle_fallback_typeface(
                _raster, @"NotoEmoji-Regular", @"ttf");
        (*_model).set_text_metrics_provider(fonts_ready ? &_raster : nullptr);
        (*_model).metrics().set_renderer(
            fonts_ready
                ? "Skia CPU m152 · HarfBuzz 14.2.1 · FreeType 2.14.2 · bundled fonts (optional Unicode fallback)"
                : "Skia CPU m152 · incomplete bundled font pack",
            true);
        [[NSNotificationCenter defaultCenter]
            addObserver:self
               selector:@selector(screenParametersChanged:)
                   name:NSApplicationDidChangeScreenParametersNotification
                 object:nil];
    }
    return self;
}

- (BOOL)initializeHost {
    if (_hostAttached == YES) return YES;
    const NSRect bounds = self.bounds;
    const double scale = self.window == nil ? 1.0 : self.window.backingScaleFactor;
    const HostDispatchResult result = [self dispatchHostPayload:
        HostAttachEvent{GFSize{std::max(1.0, bounds.size.width),
                               std::max(1.0, bounds.size.height)}, scale}
                               timestampNanoseconds:host_now_nanoseconds()];
    if (!result.accepted()) return NO;
    _hostAttached = YES;
    // Initial ordering can precede attachment, when occlusion notifications
    // cannot enter the host session. Establish native state before scheduling.
    NSWindow* nativeWindow = self.window;
    const bool nativeVisible = nativeWindow != nil && nativeWindow.isVisible &&
        (nativeWindow.occlusionState & NSWindowOcclusionStateVisible) != 0;
    _initialHostOccluded = nativeVisible ? NO : YES;
    [self notifyOcclusion:_initialHostOccluded];
    [self notifyDisplaysChanged];
    [self collectDamage];
    [self startDisplayLinkIfNeeded];
    return YES;
}

- (void)installCloseRequestHandler:(std::function<void(HostCloseRequest&)>)handler {
    _closeRequestSubscription.disconnect();
    if (_hostSession != nullptr && handler) {
        _closeRequestSubscription = (*_hostSession).closing().subscribe(std::move(handler));
    }
}

- (HostDispatchResult)dispatchHostPayload:(HostEventPayload)payload
                         timestampNanoseconds:(std::uint64_t)timestamp {
    if (_hostSession == nullptr) {
        HostDispatchResult result;
        result.error = gui_forms::HostDispatchError::after_shutdown;
        return result;
    }
    HostEvent event;
    event.sequence = _nextHostSequence++;
    event.timestamp_nanoseconds = timestamp;
    event.payload = std::move(payload);
    return (*_hostSession).dispatch(std::move(event));
}

- (BOOL)requestCloseHiding:(BOOL)hiding {
    if (_hostAttached == NO) return NO;
    const HostDispatchResult result = [self dispatchHostPayload:
        HostCloseRequest{HostCloseReason::user, false, hiding == YES}
                                           timestampNanoseconds:host_now_nanoseconds()];
    return result.accepted() && result.close_allowed;
}

- (void)notifyClosed {
    if (_hostAttached == NO) return;
    static_cast<void>([self dispatchHostPayload:HostClosedEvent{HostCloseReason::user}
                               timestampNanoseconds:host_now_nanoseconds()]);
}

- (void)notifyActivation:(BOOL)active {
    if (_hostAttached == NO) return;
    static_cast<void>([self dispatchHostPayload:HostActivationEvent{active == YES}
                               timestampNanoseconds:host_now_nanoseconds()]);
    [self collectDamage];
    if (active == YES) [self setNeedsDisplay:YES];
}

- (void)notifyOcclusion:(BOOL)occluded {
    if (_hostAttached == NO) return;
    static_cast<void>([self dispatchHostPayload:HostOcclusionEvent{occluded == YES}
                               timestampNanoseconds:host_now_nanoseconds()]);
    _hostOccluded = occluded;
    if (occluded == YES) {
        [self stopDisplayLink];
        [self armWakeTimer];
    } else {
        _pendingDamage.add(GFRect{0.0, 0.0, self.bounds.size.width, self.bounds.size.height});
        [self collectDamage];
        // AppKit may consume a display request while our model is still hidden.
        // Rearm native presentation even when no new model damage was produced.
        [self setNeedsDisplay:YES];
        [self startDisplayLinkIfNeeded];
    }
}

- (void)notifyScaleChanged {
    if (_hostAttached == NO) return;
    const double scale = self.window == nil ? 1.0 : self.window.backingScaleFactor;
    static_cast<void>([self dispatchHostPayload:HostScaleEvent{scale}
                               timestampNanoseconds:host_now_nanoseconds()]);
    [self collectDamage];
}

- (void)notifyDisplaysChanged {
    if (_hostAttached == NO || _hostServices == nullptr) {
        return;
    }
    HostMonitorResult monitors = (*_hostServices).query_monitors();
    if (!monitors.status.accepted()) {
        return;
    }
    static_cast<void>([self dispatchHostPayload:
        HostDisplayEvent{std::move(monitors.monitors)}
                               timestampNanoseconds:host_now_nanoseconds()]);
}

- (void)screenParametersChanged:(NSNotification*)notification {
    [self notifyDisplaysChanged];
}

- (BOOL)isFlipped {
    return YES;
}

- (BOOL)acceptsFirstResponder {
    return YES;
}

- (BOOL)becomeFirstResponder {
    return YES;
}

- (BOOL)resignFirstResponder {
    return YES;
}

- (BOOL)isAccessibilityElement { return YES; }
- (NSString*)accessibilityRole { return NSAccessibilityGroupRole; }
- (NSString*)accessibilityLabel {
    return @"GUI.Forms retained surface";
}

- (NSArray*)accessibilityChildren {
    if (!_model) return @[];
    const gui_forms::SemanticSnapshot snapshot = (*_model).semantic_snapshot();
    if (_semanticAccessibilityChildren != nil &&
        _accessibilityCacheGeneration == snapshot.generation) {
        return _semanticAccessibilityChildren;
    }
    NSMutableArray* result = [[NSMutableArray alloc]
        initWithCapacity:snapshot.roots.size()];
    NSMutableDictionary<NSString*, GUIFormsAccessibilityElement*>* next =
        [[NSMutableDictionary alloc] initWithCapacity:snapshot.node_count];
    for (const SemanticNode& node : snapshot.roots) {
        GUIFormsAccessibilityElement* element = reconcile_accessibility_element(
            node, self, self, _semanticAccessibilityElements, next);
        [result addObject:element];
    }
    _semanticAccessibilityElements = next;
    _semanticAccessibilityChildren = [result copy];
    _accessibilityCacheGeneration = snapshot.generation;
    return _semanticAccessibilityChildren;
}

- (NSArray*)accessibilityChildrenInNavigationOrder {
    return [self accessibilityChildren];
}

- (NSArray*)accessibilityVisibleChildren {
    return [self accessibilityChildren];
}

- (NSRect)accessibilityFrameForSemanticBounds:(GFRect)bounds {
    if (self.window == nil) return NSZeroRect;
    const NSRect local = NSMakeRect(bounds.x, bounds.y, bounds.width, bounds.height);
    const NSRect inWindow = [self convertRect:local toView:nil];
    return [self.window convertRectToScreen:inWindow];
}

- (BOOL)performSemanticAction:(SemanticAction)action
                     stableId:(NSString*)stableId
                        value:(NSString*)value {
    if (!_model || stableId == nil) return NO;
    const std::string identifier = utf8_string(stableId);
    const std::string actionValue = value == nil ? std::string{} : utf8_string(value);
    if (action == SemanticAction::focus) {
        [self.window makeFirstResponder:self];
    }
    const bool handled = (*_model).perform_semantic_action(identifier, action, actionValue);
    if (handled) {
        [self collectDamage];
        NSAccessibilityPostNotification(self, NSAccessibilityValueChangedNotification);
    }
    return handled ? YES : NO;
}

- (void)viewDidMoveToWindow {
    [super viewDidMoveToWindow];
    [self notifyScaleChanged];
    [self notifyDisplaysChanged];
    [self collectDamage];
    [self startDisplayLinkIfNeeded];
}

- (void)viewWillMoveToWindow:(NSWindow*)newWindow {
    if (newWindow == nil && _wakeSource != nil) {
        dispatch_source_set_timer(_wakeSource, DISPATCH_TIME_FOREVER,
                                  DISPATCH_TIME_FOREVER, 0);
    }
    if (newWindow == nil) [self stopDisplayLink];
    [super viewWillMoveToWindow:newWindow];
}

- (void)dealloc {
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    [self stopDisplayLink];
    if (_displayLink != nullptr) {
        CVDisplayLinkRelease(_displayLink);
        _displayLink = nullptr;
    }
    if (_wakeSource != nil) {
        dispatch_source_cancel(_wakeSource);
        _wakeSource = nil;
    }
    [self hideHostTooltip];
    if (_hostSession != nullptr) {
        (*_hostSession).shutdown();
        _hostSession.reset();
    }
    if (_hostServices != nullptr) {
        (*_hostServices).shutdown();
        _hostServices.reset();
    }
    if (_model != nullptr) {
        (*_model).set_text_metrics_provider(nullptr);
    }
    _model.reset();
}

- (HostDialogResult)showHostDialog:(const HostDialogRequest&)request {
    if (_hostServices == nullptr) {
        return {{HostServiceError::after_shutdown}, request.request_id,
                HostPathDialogResult{}};
    }
    [self hideHostTooltip];
    return (*_hostServices).show_dialog(request);
}

- (HostClipboardTextResult)readHostClipboard {
    if (_hostServices == nullptr) {
        return {{HostServiceError::after_shutdown}, {}, 0, false};
    }
    return (*_hostServices).read_clipboard_text();
}

- (HostServiceStatus)writeHostClipboard:(std::string_view)text {
    if (_hostServices == nullptr) return {HostServiceError::after_shutdown};
    return (*_hostServices).write_clipboard_text(text);
}

- (HostServiceStatus)showHostTooltip:(const HostTooltipRequest&)request {
    [self hideHostTooltip];
    if (request.text.empty()) return {};
    NSString* text = native_string(request.text);
    if (text == nil) return {HostServiceError::invalid_utf8};
    NSFont* font = [NSFont fontWithName:@"Lucida Grande" size:12.0];
    if (font == nil) font = [NSFont systemFontOfSize:12.0];
    NSDictionary* attributes = @{NSFontAttributeName: font};
    const NSRect measured = [text boundingRectWithSize:NSMakeSize(360.0, CGFLOAT_MAX)
                                               options:NSStringDrawingUsesLineFragmentOrigin |
                                                       NSStringDrawingUsesFontLeading
                                            attributes:attributes];
    const CGFloat width = std::clamp(std::ceil(measured.size.width) + 16.0,
                                     40.0, 376.0);
    const CGFloat height = std::max(24.0, std::ceil(measured.size.height) + 10.0);
    NSTextField* label = [[NSTextField alloc]
        initWithFrame:NSMakeRect(8.0, 5.0, width - 16.0, height - 10.0)];
    label.stringValue = text;
    label.font = font;
    label.textColor = [NSColor colorWithSRGBRed:31.0 / 255.0
                                          green:37.0 / 255.0
                                           blue:44.0 / 255.0 alpha:1.0];
    label.bordered = NO;
    label.editable = NO;
    label.selectable = NO;
    label.drawsBackground = NO;
    label.lineBreakMode = NSLineBreakByWordWrapping;
    label.usesSingleLineMode = NO;
    NSPanel* panel = [[NSPanel alloc]
        initWithContentRect:NSMakeRect(0.0, 0.0, width, height)
                  styleMask:NSWindowStyleMaskBorderless |
                            NSWindowStyleMaskNonactivatingPanel
                    backing:NSBackingStoreBuffered defer:NO];
    panel.releasedWhenClosed = NO;
    panel.opaque = YES;
    panel.backgroundColor = [NSColor colorWithSRGBRed:1.0 green:1.0
                                                 blue:240.0 / 255.0 alpha:1.0];
    panel.hasShadow = YES;
    panel.level = NSStatusWindowLevel;
    panel.ignoresMouseEvents = YES;
    panel.contentView = label;
    NSPoint anchor = [self convertPoint:NSMakePoint(request.anchor.x,
                                                    request.anchor.y)
                                  toView:nil];
    anchor = [self.window convertPointToScreen:anchor];
    NSRect work = self.window.screen.visibleFrame;
    CGFloat x = std::clamp(anchor.x, NSMinX(work),
                           std::max(NSMinX(work), NSMaxX(work) - width));
    CGFloat y = std::clamp(anchor.y - height, NSMinY(work),
                           std::max(NSMinY(work), NSMaxY(work) - height));
    [panel setFrameOrigin:NSMakePoint(x, y)];
    [self.window addChildWindow:panel ordered:NSWindowAbove];
    [panel orderFrontRegardless];
    _tooltipPanel = panel;
    if (request.duration_milliseconds != 0U) {
        _tooltipTimer = [NSTimer scheduledTimerWithTimeInterval:
            request.duration_milliseconds / 1000.0 repeats:NO
            block:^(NSTimer*) { [self hideHostTooltip]; }];
    }
    return {};
}

- (void)hideHostTooltip {
    [_tooltipTimer invalidate];
    _tooltipTimer = nil;
    if (_tooltipPanel != nil) {
        [self.window removeChildWindow:_tooltipPanel];
        [_tooltipPanel orderOut:nil];
        _tooltipPanel = nil;
    }
}

- (void)setFrameSize:(NSSize)newSize {
    [super setFrameSize:newSize];
    if (_hostSession) {
        static_cast<void>([self dispatchHostPayload:
            HostResizeEvent{GFSize{newSize.width, newSize.height}}
                                   timestampNanoseconds:host_now_nanoseconds()]);
        [self collectDamage];
    }
}

- (void)updateTrackingAreas {
    if (_trackingArea != nil) {
        [self removeTrackingArea:_trackingArea];
    }
    const NSTrackingAreaOptions options = NSTrackingMouseEnteredAndExited |
        NSTrackingMouseMoved | NSTrackingCursorUpdate | NSTrackingActiveInKeyWindow | NSTrackingInVisibleRect;
    _trackingArea = [[NSTrackingArea alloc] initWithRect:NSZeroRect
                                                 options:options
                                                   owner:self
                                                userInfo:nil];
    [self addTrackingArea:_trackingArea];
    [super updateTrackingAreas];
}

- (void)collectDamage {
    std::optional<MacCpuScope> cpuScope{};
    if (_cpuAttribution.enabled) cpuScope.emplace(_cpuAttribution, 2U);
    ++_damageCollectionCount;
    try {
        if (!_model) {
            return;
        }
        const gui_forms::FrameTime pollTime = std::chrono::steady_clock::now();
        const gui_forms::FramePollResult pollResult = (*_model).poll_frame_schedule(pollTime);
        if (_causalTrace && (*_causalTrace).enabled) {
            (*_causalTrace).record_poll(pollTime, pollResult.deadlines_fired, pollResult.next_wake);
        }
        const std::uint64_t semanticGeneration = (*_model).semantic_generation();
        if (semanticGeneration != _lastSemanticGeneration) {
            _lastSemanticGeneration = semanticGeneration;
            _semanticAccessibilityChildren = nil;
            _accessibilityCacheGeneration = 0;
            NSAccessibilityPostNotification(
                self, NSAccessibilityLayoutChangedNotification);
        }
        DamageRegion damage = (*_model).take_damage();
        const double scale =
            self.window == nil ? 1.0 : self.window.backingScaleFactor;
        for (GFRect rect : damage.rectangles()) {
            rect = gui_forms::detail::align_damage_outward(rect, scale);
            _pendingDamage.add(rect);
            [self setNeedsDisplayInRect:
                NSMakeRect(rect.x, rect.y, rect.width, rect.height)];
        }
        [self startDisplayLinkIfNeeded];
        [self armWakeTimer];
    } catch (const std::exception& error) {
        [self recordNativeCallbackFault:"collect-damage" message:error.what()];
    } catch (...) {
        [self recordNativeCallbackFault:"collect-damage" message:"unknown"];
    }
}

- (void)startDisplayLinkIfNeeded {
    if (_displayLink == nullptr || !_model || self.window == nil ||
        _hostOccluded == YES || !(*_model).has_live_surface_presentations() ||
        CVDisplayLinkIsRunning(_displayLink)) {
        return;
    }
    CVDisplayLinkStart(_displayLink);
}

- (void)stopDisplayLink {
    if (_displayLink != nullptr && CVDisplayLinkIsRunning(_displayLink)) {
        CVDisplayLinkStop(_displayLink);
    }
    _displayTickQueued.store(false, std::memory_order_release);
}

- (void)queueDisplayLinkTick {
    if (_displayTickQueued.exchange(true, std::memory_order_acq_rel)) return;
    __weak GUIFormsView* weakSelf = self;
    dispatch_async(dispatch_get_main_queue(), ^{
        GUIFormsView* strongSelf = weakSelf;
        if (strongSelf == nil) return;
        strongSelf->_displayTickQueued.store(false, std::memory_order_release);
        [strongSelf displayLinkTick];
    });
}

- (void)displayLinkTick {
    ++_displayTickCount;
    if (!_model || self.window == nil || _hostOccluded == YES) return;
    std::vector<LiveSurfacePresentation> updates =
        (*_model).take_live_surface_presentations();
    if (!(*_model).has_live_surface_presentations()) {
        [self stopDisplayLink];
        return;
    }
    if (updates.empty()) return;

    _pendingLivePresentations = std::move(updates);
    const double scale = self.window.backingScaleFactor;
    for (const LiveSurfacePresentation& update : _pendingLivePresentations) {
        const GFRect clip = gui_forms::detail::align_damage_outward(
            update.clip, scale);
        [self setNeedsDisplayInRect:
            NSMakeRect(clip.x, clip.y, clip.width, clip.height)];
    }
    // This is the terminal display-clock release. Ticks coalesce before the
    // main queue, and each release samples only the newest published frame.
    [self displayIfNeeded];
}

- (void)drainPostedWork {
    try {
        if (!_model) return;
        static_cast<void>((*_model).drain_posted_work());
        [self collectDamage];
    } catch (const std::exception& error) {
        [self recordNativeCallbackFault:"posted-work" message:error.what()];
    } catch (...) {
        [self recordNativeCallbackFault:"posted-work" message:"unknown"];
    }
}

- (void)armWakeTimer {
    if (!_model || _wakeSource == nil) {
        return;
    }
    const std::optional<gui_forms::FrameTime> wake = (*_model).next_wake();
    if (!wake) {
        if (_causalTrace && (*_causalTrace).enabled) {
            (*_causalTrace).record_arm(gui_forms::FrameClock::now(), wake, 0);
        }
        dispatch_source_set_timer(_wakeSource, DISPATCH_TIME_FOREVER,
                                  DISPATCH_TIME_FOREVER, 0);
        return;
    }
    const std::chrono::steady_clock::time_point now =
        std::chrono::steady_clock::now();
    const std::chrono::duration<long long, std::ratio<1, 1000>> delay = std::max(std::chrono::milliseconds(1),
                                std::chrono::duration_cast<std::chrono::milliseconds>(
                                    *wake - now));
    const std::chrono::nanoseconds::rep nanoseconds =
        std::chrono::duration_cast<std::chrono::nanoseconds>(delay).count();
    dispatch_source_set_timer(
        _wakeSource,
        dispatch_time(DISPATCH_TIME_NOW, static_cast<std::int64_t>(nanoseconds)),
        DISPATCH_TIME_FOREVER,
        static_cast<std::uint64_t>(std::chrono::microseconds(250).count() * 1000));
    if (_causalTrace && (*_causalTrace).enabled) {
        (*_causalTrace).record_arm(now, wake, static_cast<std::int64_t>(nanoseconds));
    }
}

- (void)scheduledWake {
    std::optional<MacCausalWakeScope> causalScope{};
    if (_causalTrace && (*_causalTrace).enabled) causalScope.emplace(*_causalTrace);
    ++_scheduledWakeCount;
    // Objective-C exceptions do not participate in C++ exception handling.
    // A raised AppKit exception escaping a libdispatch source terminates the
    // process, so contain it at the native callback boundary and preserve its
    // exact name/reason in the same structured fault channel as C++ failures.
    @try {
        [self collectDamage];
    } @catch (NSException* exception) {
        NSString* name = exception.name == nil ? @"NSException" : exception.name;
        NSString* reason = exception.reason == nil ? @"no reason" : exception.reason;
        NSString* diagnostic = [NSString stringWithFormat:@"%@: %@", name, reason];
        [self recordNativeCallbackFault:"scheduled-wake"
                                 message:diagnostic.UTF8String];
    }
}

- (void)recordNativeCallbackFault:(const char*)operation
                          message:(const char*)message {
    ++_nativeCallbackFaults;
    if (_wakeSource != nil) {
        dispatch_source_set_timer(_wakeSource, DISPATCH_TIME_FOREVER,
                                  DISPATCH_TIME_FOREVER, 0);
    }
    std::fprintf(stderr, "gui-forms-host=%s-fault|what:%s\n",
                 operation == nullptr ? "native-callback" : operation,
                 message == nullptr ? "unknown" : message);
    std::fflush(stderr);
}

- (BOOL)acceptsTitleUpdate {
    if (_hostSession == nullptr) return NO;
    const gui_forms::HostLifecyclePhase phase = (*_hostSession).snapshot().phase;
    const bool live = phase == gui_forms::HostLifecyclePhase::attached ||
                      phase == gui_forms::HostLifecyclePhase::close_authorized;
    const BOOL result = live ? YES : NO;
    return result;
}
- (void)prepareForShutdown {
    [self stopDisplayLink];
    if (_wakeSource != nil) {
        dispatch_source_set_timer(_wakeSource, DISPATCH_TIME_FOREVER,
                                  DISPATCH_TIME_FOREVER, 0);
    }
    if (_model) {
        (*_model).set_paint_wake_handler({});
        (*_model).shutdown_dispatcher();
    }
    if (_hostSession) {
        static_cast<void>([self dispatchHostPayload:HostShutdownEvent{}
                                   timestampNanoseconds:host_now_nanoseconds()]);
    }
}

- (DragEvent)dragEventFor:(id<NSDraggingInfo>)sender action:(DragAction)action {
    DragEvent event;
    event.action = action;
    event.session_id = std::max<std::uint64_t>(
        1, static_cast<std::uint64_t>(sender.draggingSequenceNumber));
    const NSPoint point = [self convertPoint:sender.draggingLocation fromView:nil];
    event.position = {point.x, point.y};
    event.allowed_effects = drag_effects_for(sender.draggingSourceOperationMask);
    if (action != DragAction::leave) {
        event.items = drag_items_for(sender.draggingPasteboard);
    }
    return event;
}

- (NSDragOperation)draggingEntered:(id<NSDraggingInfo>)sender {
    DragEvent event = [self dragEventFor:sender action:DragAction::enter];
    const HostDispatchResult result = [self dispatchHostPayload:std::move(event)
        timestampNanoseconds:host_now_nanoseconds()];
    [self collectDamage];
    return native_drag_operation(result.drag_effect);
}

- (NSDragOperation)draggingUpdated:(id<NSDraggingInfo>)sender {
    DragEvent event = [self dragEventFor:sender action:DragAction::over];
    const HostDispatchResult result = [self dispatchHostPayload:std::move(event)
        timestampNanoseconds:host_now_nanoseconds()];
    [self collectDamage];
    return native_drag_operation(result.drag_effect);
}

- (void)draggingExited:(id<NSDraggingInfo>)sender {
    DragEvent event = [self dragEventFor:sender action:DragAction::leave];
    static_cast<void>([self dispatchHostPayload:std::move(event)
        timestampNanoseconds:host_now_nanoseconds()]);
    [self collectDamage];
}

- (BOOL)prepareForDragOperation:(id<NSDraggingInfo>)sender {
    return _hostSession != nullptr;
}

- (BOOL)performDragOperation:(id<NSDraggingInfo>)sender {
    DragEvent event = [self dragEventFor:sender action:DragAction::drop];
    const HostDispatchResult result = [self dispatchHostPayload:std::move(event)
        timestampNanoseconds:host_now_nanoseconds()];
    [self collectDamage];
    return result.accepted() && result.handled;
}

- (void)drawRect:(NSRect)dirtyRect {
    ++_nativeDrawCount;
    try {
        [self drawRetainedRect:dirtyRect];
    } catch (const std::exception& error) {
        _raster.end_frame();
        _liveFrames.clear();
#if defined(GUI_FORMS_PREPARED_TEXT)
        _raster.abort_prepared_frame();
#endif
        [self recordNativeCallbackFault:"draw" message:error.what()];
    } catch (...) {
        _raster.end_frame();
        _liveFrames.clear();
#if defined(GUI_FORMS_PREPARED_TEXT)
        _raster.abort_prepared_frame();
#endif
        [self recordNativeCallbackFault:"draw" message:"unknown"];
    }
}

- (void)drawRetainedRect:(NSRect)dirtyRect {
    if (!_model) {
        return;
    }
    std::optional<MacCpuScope> cpuScope{};
    if (_cpuAttribution.enabled) cpuScope.emplace(_cpuAttribution, 0U);
    const std::chrono::steady_clock::time_point started =
        std::chrono::steady_clock::now();
    if (_causalTrace && (*_causalTrace).enabled) (*_causalTrace).record_draw_entry(started);
    const double scale = self.window == nil ? 1.0 : self.window.backingScaleFactor;
    const GFSize logicalSize{self.bounds.size.width, self.bounds.size.height};
    _lastNativeDirtyArea = std::max(0.0, dirtyRect.size.width) *
        std::max(0.0, dirtyRect.size.height);
    _lastFrameDamageBoundsArea = 0.0;
    _lastCGDestinationArea = 0.0;
    _lastCGClipBoundsArea = 0.0;
    _lastCGSourceBytes = 0U;
#if !defined(GUI_FORMS_PREPARED_TEXT)
    if (_raster.resize(logicalSize, scale)) {
        _rasterHasContent = NO;
        _pendingDamage.add(GFRect{0.0, 0.0, logicalSize.width, logicalSize.height});
    }
#endif
    _liveFrames.clear();
    _opaqueLiveClips.clear();
    _liveFrames.reserve(_pendingLivePresentations.size());
    _opaqueLiveClips.reserve(_pendingLivePresentations.size());
    for (const LiveSurfacePresentation& update : _pendingLivePresentations) {
        gui_forms::LiveSurfaceFrame frame{};
        if (update.surface) frame = (*update.surface).acquire_latest();
        if (frame && frame.opaque()) {
            const GFRect covered = gui_forms::detail::opaque_live_clip(
                GFRect::intersection(update.clip, update.destination), scale);
            if (!covered.empty()) _opaqueLiveClips.push_back(covered);
        }
        _liveFrames.push_back(std::move(frame));
    }
    const GFRect nativeDamage{dirtyRect.origin.x, dirtyRect.origin.y,
                              dirtyRect.size.width, dirtyRect.size.height};
    const bool modelClean = (*_model).paint_lease_snapshot().state ==
        gui_forms::PaintLeaseState::clean;
    bool reusableRaster = _rasterHasContent == YES && _hostOccluded == NO && modelClean;
#if defined(GUI_FORMS_PREPARED_TEXT)
    reusableRaster = reusableRaster && _raster.prepared_front_matches(logicalSize, scale);
    const PaintReceipt previousReceipt = _raster.prepared_front_receipt();
#endif
    bool liveOnly = gui_forms::detail::opaque_live_damage_covers(
        nativeDamage, scale, _opaqueLiveClips, reusableRaster, !_pendingDamage.empty());
    if (!liveOnly) _pendingDamage.add(nativeDamage);

#if defined(GUI_FORMS_PREPARED_TEXT)
    const bool imagesReady = _raster.synchronize_images((*_model).image_resources());
#else
    static_cast<void>(_raster.synchronize_images((*_model).image_resources()));
#endif
    DamageRegion frameDamage = _pendingDamage;
    for (const LiveSurfacePresentation& update : _pendingLivePresentations) {
        frameDamage.add(update.clip);
    }
    bool frameReady{true};
#if defined(GUI_FORMS_PREPARED_TEXT)
    const gui_forms::PreparedTextStatus admission = imagesReady
        ? _raster.begin_prepared_frame(logicalSize, scale, frameDamage)
        : gui_forms::PreparedTextStatus::resource_failure;
    frameReady = admission == gui_forms::PreparedTextStatus::success;
    if (frameReady) {
        // Admission can require a full repaint after resize or revocation.
        // Keep that damage until this exact candidate reaches native presentation.
        if (liveOnly) {
            liveOnly = gui_forms::detail::opaque_live_damage_covers(
                frameDamage.bounds(), scale, _opaqueLiveClips, reusableRaster, false);
        }
        if (!liveOnly) {
            for (const GFRect rectangle : frameDamage.rectangles()) _pendingDamage.add(rectangle);
        }
    }
#else
    _raster.begin_frame(frameDamage);
#endif
    _lastFrameDamageBoundsArea = frameDamage.bounds().area();
    const std::chrono::steady_clock::time_point preparedAt =
        std::chrono::steady_clock::now();
    _rasterPreparePhase.record(started, preparedAt);
    std::optional<PaintReceipt> receipt{};
    if (frameReady && !_pendingDamage.empty()) {
        const std::chrono::steady_clock::time_point paintStarted =
            std::chrono::steady_clock::now();
        receipt = (*_model).paint(_raster, _pendingDamage.bounds());
        const std::chrono::steady_clock::time_point paintEnded =
            std::chrono::steady_clock::now();
        _retainedPaintPhase.record(paintStarted, paintEnded);
    }
    const std::chrono::steady_clock::time_point finishStarted =
        std::chrono::steady_clock::now();
    if (frameReady) {
        for (std::size_t index = 0; index < _pendingLivePresentations.size(); ++index) {
            const LiveSurfacePresentation& update = _pendingLivePresentations[index];
            _raster.save();
            _raster.clip_rect(update.clip);
            _raster.draw_live_surface_frame(_liveFrames[index], update.destination, 1.0);
            _raster.restore();
        }
        _raster.end_frame();
    }
#if defined(GUI_FORMS_PREPARED_TEXT)
    bool committed{false};
    if (frameReady && (receipt || liveOnly)) {
        // Live-only pixels retain the same coherent retained-tree revision.
        // Do not manufacture or acknowledge another model paint transaction.
        const PaintReceipt publicationReceipt = receipt ? *receipt : previousReceipt;
        const gui_forms::PreparedTextStatus publication = _raster.commit_prepared_frame(publicationReceipt);
        committed = publication == gui_forms::PreparedTextStatus::success;
    }
    if (!committed) {
        _raster.abort_prepared_frame();
        receipt.reset();
    }
#endif
    _liveFrames.clear();
    const std::chrono::steady_clock::time_point finishEnded =
        std::chrono::steady_clock::now();
    _rasterFinishPhase.record(finishStarted, finishEnded);

    if (receipt) _rasterHasContent = YES;
    GFSize presentationSize = logicalSize;
#if defined(GUI_FORMS_PREPARED_TEXT)
    _rasterHasContent = _raster.prepared_front_receipt() ? YES : NO;
    if (_rasterHasContent == YES) {
        const double frontScale = _raster.prepared_front_scale();
        presentationSize = {
            static_cast<double>(_raster.pixel_width()) / frontScale,
            static_cast<double>(_raster.pixel_height()) / frontScale};
    }
#endif
    bool presented = false;
    const void* pixels = _raster.pixels();
    // Native exposure/snapshot callbacks can precede unocclusion. The model
    // intentionally declines painting then; the last complete raster is still
    // valid and must be copied into AppKit's newly supplied backing context.
    if (_rasterHasContent == YES && pixels != nullptr) {
        const std::chrono::steady_clock::time_point setupStarted =
            std::chrono::steady_clock::now();
        CGDataProviderRef provider = CGDataProviderCreateWithData(
            nullptr, pixels, _raster.byte_size(), nullptr);
        CGColorSpaceRef colorSpace = CGColorSpaceCreateWithName(kCGColorSpaceSRGB);
        const CGBitmapInfo bitmapInfo = static_cast<CGBitmapInfo>(
            static_cast<CGBitmapInfo>(kCGImageAlphaPremultipliedLast) |
            static_cast<CGBitmapInfo>(kCGBitmapByteOrder32Big));
        CGImageRef image = CGImageCreate(
            _raster.pixel_width(), _raster.pixel_height(), 8, 32,
            _raster.row_bytes(), colorSpace, bitmapInfo, provider,
            nullptr, false, kCGRenderingIntentDefault);
        const std::chrono::steady_clock::time_point setupEnded =
            std::chrono::steady_clock::now();
        _cgSetupPhase.record(setupStarted, setupEnded);
        if (image != nullptr) {
            CGContextRef context = NSGraphicsContext.currentContext.CGContext;
            const CGRect clipBounds = CGContextGetClipBoundingBox(context);
            _lastCGClipBoundsArea = std::max(0.0, clipBounds.size.width) *
                std::max(0.0, clipBounds.size.height);
            _lastCGDestinationArea = presentationSize.width * presentationSize.height;
            _lastCGSourceBytes = _raster.byte_size();
            const std::chrono::steady_clock::time_point drawStarted =
                std::chrono::steady_clock::now();
            CGContextSaveGState(context);
            CGContextSetBlendMode(context, kCGBlendModeCopy);
            CGContextTranslateCTM(context, 0.0, presentationSize.height);
            CGContextScaleCTM(context, 1.0, -1.0);
            CGContextDrawImage(context,
                               CGRectMake(0.0, 0.0, presentationSize.width, presentationSize.height),
                               image);
            CGContextRestoreGState(context);
            const std::chrono::steady_clock::time_point drawEnded =
                std::chrono::steady_clock::now();
            _cgDrawPhase.record(drawStarted, drawEnded);
            presented = true;
        }
        const std::chrono::steady_clock::time_point releaseStarted =
            std::chrono::steady_clock::now();
        if (image != nullptr) CGImageRelease(image);
        CGColorSpaceRelease(colorSpace);
        CGDataProviderRelease(provider);
        const std::chrono::steady_clock::time_point releaseEnded =
            std::chrono::steady_clock::now();
        _cgReleasePhase.record(releaseStarted, releaseEnded);
    }
    const std::chrono::duration<long long, std::ratio<1, 1000000000>> elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::steady_clock::now() - started);
    if (presented) {
#if defined(GUI_FORMS_PREPARED_TEXT)
        if (committed) _pendingLivePresentations.clear();
#else
        _pendingLivePresentations.clear();
#endif
        if (receipt &&
            (*_model).notify_presented(*receipt,
                static_cast<std::uint64_t>(elapsed.count()))) {
            _pendingDamage.clear();
        }
    }
    // Do not collect the next model invalidation from inside AppKit's paint
    // transaction. setNeedsDisplayInRect: may leave the view dirty without
    // enqueueing a second draw when invoked synchronously by drawRect:, which
    // lets animation callbacks advance while the last raster remains frozen.
    // The retained deadline source owns the next poll; arm it after presenting
    // and collect damage from its main-queue callback.
    [self armWakeTimer];
    if (_cpuAttribution.enabled) {
        const std::array<double, 4> extents{
            _lastNativeDirtyArea, _lastFrameDamageBoundsArea,
            _lastCGDestinationArea, _lastCGClipBoundsArea};
        const std::uint64_t wallNs = host_now_nanoseconds();
        _cpuAttribution.record_draw(wallNs, extents, _lastCGSourceBytes);
    }
    if (_causalTrace && (*_causalTrace).enabled) {
        (*_causalTrace).record_draw_exit(gui_forms::FrameClock::now());
    }
}

- (GFPoint)modelPointForEvent:(NSEvent*)event {
    const NSPoint point = [self convertPoint:event.locationInWindow fromView:nil];
    return GFPoint{point.x, point.y};
}

- (void)dispatchPointerEvent:(NSEvent*)event action:(PointerAction)action {
    PointerEvent input;
    input.action = action;
    input.button = button_for(event.buttonNumber);
    input.position = [self modelPointForEvent:event];
    input.modifiers = modifiers_for(event.modifierFlags);
    input.pointer_id = 1;
    input.click_count = static_cast<std::uint32_t>(
        std::max<NSInteger>(1, event.clickCount));
    const GFPoint position = input.position;
    static_cast<void>([self dispatchHostPayload:std::move(input)
                               timestampNanoseconds:host_event_nanoseconds(event)]);
    [self updateCursorAtPoint:position];
    [self collectDamage];
}

- (void)updateCursorAtPoint:(GFPoint)position {
    if (_hostServices != nullptr) {
        const gui_forms::Control::Ptr captured = (*_model).captured_control();
        const gui_forms::Control::Ptr target = captured ? captured : (*_model).hit_test(position);
        static_cast<void>((*_hostServices).set_custom_cursor(
            target ? (*target).effective_cursor_images() : gui_forms::CursorImagesPtr{},
            self.window.backingScaleFactor,
            target ? (*target).effective_cursor() : CursorKind::arrow));
    }
}

- (void)cursorUpdate:(NSEvent*)event {
    [self updateCursorAtPoint:[self modelPointForEvent:event]];
}

- (void)mouseExited:(NSEvent*)event {
    if (_model && (*_model).captured_control()) return;
    if (_hostServices != nullptr) {
        static_cast<void>((*_hostServices).set_cursor(CursorKind::arrow));
    }
}

- (void)mouseDown:(NSEvent*)event {
    [self.window makeFirstResponder:self];
    if (event.buttonNumber == 0 && !_windowDragRegionIds.empty() && _model) {
        const GFPoint point = [self modelPointForEvent:event];
        const gui_forms::WindowChromeHit hit =
            gui_forms::resolve_window_chrome_hit(
                *_model, point, _windowDragRegionIds);
        if (hit.begins_native_drag()) {
            [self.window performWindowDragWithEvent:event];
            return;
        }
    }
    [self dispatchPointerEvent:event action:PointerAction::down];
}

- (void)mouseUp:(NSEvent*)event {
    [self dispatchPointerEvent:event action:PointerAction::up];
}

- (void)rightMouseDown:(NSEvent*)event {
    [self dispatchPointerEvent:event action:PointerAction::down];
}

- (void)rightMouseUp:(NSEvent*)event {
    [self dispatchPointerEvent:event action:PointerAction::up];
}

- (void)otherMouseDown:(NSEvent*)event {
    [self dispatchPointerEvent:event action:PointerAction::down];
}

- (void)otherMouseUp:(NSEvent*)event {
    [self dispatchPointerEvent:event action:PointerAction::up];
}

- (void)mouseMoved:(NSEvent*)event {
    [self dispatchPointerEvent:event action:PointerAction::move];
}

- (void)mouseDragged:(NSEvent*)event {
    [self dispatchPointerEvent:event action:PointerAction::move];
}

- (void)rightMouseDragged:(NSEvent*)event {
    [self dispatchPointerEvent:event action:PointerAction::move];
}

- (void)otherMouseDragged:(NSEvent*)event {
    [self dispatchPointerEvent:event action:PointerAction::move];
}

- (void)scrollWheel:(NSEvent*)event {
    PointerEvent input;
    input.action = PointerAction::wheel;
    input.position = [self modelPointForEvent:event];
    input.wheel_delta = GFPoint{event.scrollingDeltaX, event.scrollingDeltaY};
    input.modifiers = modifiers_for(event.modifierFlags);
    input.pointer_id = 1;
    static_cast<void>([self dispatchHostPayload:std::move(input)
                               timestampNanoseconds:host_event_nanoseconds(event)]);
    [self collectDamage];
}

- (void)keyDown:(NSEvent*)event {
    KeyEvent input;
    input.action = KeyAction::down;
    input.physical_key = physical_key_for(event.keyCode);
    input.modifiers = modifiers_for(event.modifierFlags);
    input.repeat = event.isARepeat;
    const bool handled = [self dispatchHostPayload:std::move(input)
                               timestampNanoseconds:host_event_nanoseconds(event)].handled;
    if (!handled) {
        [self interpretKeyEvents:@[event]];
    }
    [self collectDamage];
}

- (void)keyUp:(NSEvent*)event {
    KeyEvent input;
    input.action = KeyAction::up;
    input.physical_key = physical_key_for(event.keyCode);
    input.modifiers = modifiers_for(event.modifierFlags);
    static_cast<void>([self dispatchHostPayload:std::move(input)
                               timestampNanoseconds:host_event_nanoseconds(event)]);
    [self collectDamage];
}

- (void)insertText:(id)string replacementRange:(NSRange)replacementRange {
    TextInputEvent input;
    input.text_utf8 = utf8_string(string);
    input.composing = false;
    if (replacementRange.location != NSNotFound) {
        input.replacement_start = static_cast<std::int32_t>(replacementRange.location);
        input.replacement_length = static_cast<std::int32_t>(replacementRange.length);
    }
    static_cast<void>([self dispatchHostPayload:std::move(input)
                               timestampNanoseconds:host_now_nanoseconds()]);
    [_markedText setAttributedString:[[NSAttributedString alloc] initWithString:@""]];
    [self collectDamage];
}

- (void)setMarkedText:(id)string
         selectedRange:(NSRange)selectedRange
       replacementRange:(NSRange)replacementRange {
    NSString* plain = [string isKindOfClass:[NSAttributedString class]]
        ? [(NSAttributedString*)string string]
        : (NSString*)string;
    [_markedText setAttributedString:[[NSAttributedString alloc]
        initWithString:(plain == nil ? @"" : plain)]];
    _selectedRange = selectedRange;
    TextInputEvent input;
    input.text_utf8 = utf8_string(string);
    input.composing = true;
    if (replacementRange.location != NSNotFound) {
        input.replacement_start = static_cast<std::int32_t>(replacementRange.location);
        input.replacement_length = static_cast<std::int32_t>(replacementRange.length);
    }
    static_cast<void>([self dispatchHostPayload:std::move(input)
                               timestampNanoseconds:host_now_nanoseconds()]);
    [self collectDamage];
}

- (void)unmarkText {
    [_markedText setAttributedString:[[NSAttributedString alloc] initWithString:@""]];
}

- (BOOL)hasMarkedText {
    return _markedText.length != 0;
}

- (NSRange)markedRange {
    return [self hasMarkedText] ? NSMakeRange(0, _markedText.length)
                                : NSMakeRange(NSNotFound, 0);
}

- (NSRange)selectedRange {
    return _selectedRange;
}

- (nullable NSAttributedString*)attributedSubstringForProposedRange:(NSRange)range
                                                        actualRange:(nullable NSRangePointer)actualRange {
    if (actualRange != nullptr) {
        *actualRange = NSMakeRange(NSNotFound, 0);
    }
    return nil;
}

- (NSArray<NSAttributedStringKey>*)validAttributesForMarkedText {
    return @[];
}

- (NSRect)firstRectForCharacterRange:(NSRange)range
                          actualRange:(nullable NSRangePointer)actualRange {
    if (actualRange != nullptr) {
        *actualRange = range;
    }
    NSRect caret = NSMakeRect(0.0, 0.0, 1.0, 18.0);
    if (const gui_forms::Control::Ptr focused = (*_model).focused_control()) {
        const GFRect bounds = (*focused).absolute_bounds();
        caret = NSMakeRect(bounds.x, bounds.y, 1.0, bounds.height);
    }
    const NSRect inWindow = [self convertRect:caret toView:nil];
    return [self.window convertRectToScreen:inWindow];
}

- (NSUInteger)characterIndexForPoint:(NSPoint)point {
    return NSNotFound;
}

- (void)doCommandBySelector:(SEL)selector {
    // Physical command keys are handled by the retained control first. The
    // initial text milestone intentionally leaves unimplemented Cocoa editing
    // selectors inert instead of mutating an invisible native text field.
}

- (std::string)metricsJSON {
    return _model ? (*_model).metrics_snapshot().to_json() : std::string("{}");
}

- (BOOL)setCpuAttributionEnabled:(const BOOL)enabled {
    if (![NSThread isMainThread] || _cpuAttribution.active) return NO;
    if (enabled == YES) {
        std::unique_ptr<MacCausalTrace> replacement = std::make_unique<MacCausalTrace>();
        (*replacement).enabled = true;
        if (_model) (*replacement).seed((*_model).next_wake());
        _causalTrace = std::move(replacement);
        _cpuAttribution = {};
    } else if (_causalTrace) (*_causalTrace).enabled = false;
    _cpuAttribution.enabled = enabled == YES;
    return YES;
}

- (std::array<std::uint64_t, 13>)cpuAttributionSnapshot {
    return _cpuAttribution.values;
}

- (MacCausalSnapshot)cpuCausalTraceSnapshot {
    MacCausalSnapshot result{};
    if (_causalTrace) result = (*_causalTrace).snapshot();
    return result;
}

- (std::string)hostJSON {
    const std::string session = _hostSession
        ? (*_hostSession).snapshot().to_json() : std::string("{}");
    const std::string services = _hostServices
        ? (*_hostServices).snapshot().to_json() : std::string("{}");
    const NSWindow* nativeWindow = self.window;
    const bool fullSizeContent = nativeWindow != nil &&
        ((nativeWindow.styleMask & NSWindowStyleMaskFullSizeContentView) != 0);
    const bool titleHidden = nativeWindow != nil &&
        nativeWindow.titleVisibility == NSWindowTitleHidden;
    const bool titlebarTransparent = nativeWindow != nil &&
        nativeWindow.titlebarAppearsTransparent;
    const bool closeButtonPresent = nativeWindow != nil &&
        [nativeWindow standardWindowButton:NSWindowCloseButton] != nil;
    const bool minimizeButtonPresent = nativeWindow != nil &&
        [nativeWindow standardWindowButton:NSWindowMiniaturizeButton] != nil;
    const bool zoomButtonPresent = nativeWindow != nil &&
        [nativeWindow standardWindowButton:NSWindowZoomButton] != nil;
    const bool nativeTitlePresent = nativeWindow != nil &&
        nativeWindow.title.length != 0;
    const bool dragRegionConfigured = !_windowDragRegionIds.empty();
    std::size_t resolvedDragRegions{};
    if (_model) {
        for (const std::string& regionId : _windowDragRegionIds) {
            resolvedDragRegions +=
                static_cast<bool>((*_model).find(regionId)) ? 1U : 0U;
        }
    }
    const bool dragRegionResolved =
        dragRegionConfigured && resolvedDragRegions == _windowDragRegionIds.size();
    return "{\"session\":" + session + ",\"services\":" + services +
        ",\"titlebar_presentation\":\"" +
        (_transparentFullSizeContent
             ? std::string("transparent_full_size_content")
             : std::string("standard")) + "\"" +
        ",\"native_full_size_content\":" +
        (fullSizeContent ? "true" : "false") +
        ",\"native_title_hidden\":" + (titleHidden ? "true" : "false") +
        ",\"native_titlebar_transparent\":" +
        (titlebarTransparent ? "true" : "false") +
        ",\"native_close_button_present\":" +
        (closeButtonPresent ? "true" : "false") +
        ",\"native_minimize_button_present\":" +
        (minimizeButtonPresent ? "true" : "false") +
        ",\"native_zoom_button_present\":" +
        (zoomButtonPresent ? "true" : "false") +
        ",\"native_system_title_present\":" +
        (nativeTitlePresent ? "true" : "false") +
        ",\"window_drag_region_configured\":" +
        (dragRegionConfigured ? "true" : "false") +
        ",\"window_drag_region_resolved\":" +
        (dragRegionResolved ? "true" : "false") +
        ",\"window_drag_region_count\":" +
        std::to_string(_windowDragRegionIds.size()) +
        ",\"window_drag_region_resolved_count\":" +
        std::to_string(resolvedDragRegions) +
        ",\"initial_host_occluded\":" + (_initialHostOccluded ? "true" : "false") +
        ",\"host_occluded\":" + (_hostOccluded ? "true" : "false") +
        ",\"scheduled_wake_count\":" + std::to_string(_scheduledWakeCount) +
        ",\"damage_collection_count\":" + std::to_string(_damageCollectionCount) +
        ",\"native_draw_count\":" + std::to_string(_nativeDrawCount) +
        ",\"display_tick_count\":" + std::to_string(_displayTickCount) +
        _rasterPreparePhase.json_fields("raster_prepare") +
        _retainedPaintPhase.json_fields("retained") +
        _rasterFinishPhase.json_fields("raster_finish") +
        _cgSetupPhase.json_fields("cg_setup") +
        _cgDrawPhase.json_fields("cg_draw") +
        _cgReleasePhase.json_fields("cg_release") +
        ",\"last_native_dirty_area\":" + std::to_string(_lastNativeDirtyArea) +
        ",\"last_frame_damage_bounds_area\":" + std::to_string(_lastFrameDamageBoundsArea) +
        ",\"last_cg_destination_area\":" + std::to_string(_lastCGDestinationArea) +
        ",\"last_cg_clip_bounds_area\":" + std::to_string(_lastCGClipBoundsArea) +
        ",\"last_cg_source_bytes\":" + std::to_string(_lastCGSourceBytes) +
        ",\"native_callback_faults\":" +
        std::to_string(_nativeCallbackFaults) + "}";
}

@end

@interface GUIFormsWindowDelegate : NSObject <NSWindowDelegate> {
    __weak GUIFormsView* _view;
    BOOL _stopsApplicationOnClose;
    BOOL _hideOnClose;
    std::function<void()> _closedHandler;
}
- (instancetype)initWithView:(GUIFormsView*)view
      stopsApplicationOnClose:(BOOL)stopsApplicationOnClose
                  hideOnClose:(BOOL)hideOnClose
                 closedHandler:(std::function<void()>)closedHandler;
- (void)prepareForApplicationShutdown;
@end

@implementation GUIFormsWindowDelegate
- (instancetype)initWithView:(GUIFormsView*)view
      stopsApplicationOnClose:(BOOL)stopsApplicationOnClose
                  hideOnClose:(BOOL)hideOnClose
                 closedHandler:(std::function<void()>)closedHandler {
    self = [super init];
    if (self != nil) {
        _view = view;
        _stopsApplicationOnClose = stopsApplicationOnClose;
        _hideOnClose = hideOnClose;
        _closedHandler = std::move(closedHandler);
    }
    return self;
}
- (BOOL)windowShouldClose:(NSWindow*)sender {
    if (_view != nil && ![_view requestCloseHiding:_hideOnClose]) return NO;
    if (_hideOnClose) {
        [sender orderOut:nil];
        return NO;
    }
    return YES;
}
- (void)prepareForApplicationShutdown {
    _hideOnClose = NO;
}
- (void)windowWillClose:(NSNotification*)notification {
    [_view notifyClosed];
    if (_closedHandler) {
        std::function<void()> callback = std::move(_closedHandler);
        _closedHandler = {};
        callback();
    }
    if (!_stopsApplicationOnClose) return;
    [NSApp stop:nil];
    NSEvent* wake = [NSEvent otherEventWithType:NSEventTypeApplicationDefined
                                        location:NSZeroPoint
                                   modifierFlags:0
                                       timestamp:0
                                    windowNumber:0
                                         context:nil
                                         subtype:0
                                           data1:0
                                           data2:0];
    [NSApp postEvent:wake atStart:NO];
}
- (void)windowDidBecomeKey:(NSNotification*)notification {
    [_view notifyActivation:YES];
}
- (void)windowDidResignKey:(NSNotification*)notification {
    [_view notifyActivation:NO];
}
- (void)windowDidChangeOcclusionState:(NSNotification*)notification {
    NSWindow* window = notification.object;
    const BOOL visible = (window.occlusionState & NSWindowOcclusionStateVisible) != 0;
    [_view notifyOcclusion:!visible];
}
- (void)windowDidDeminiaturize:(NSNotification*)notification {
    [_view setNeedsDisplay:YES];
}
- (void)windowDidChangeBackingProperties:(NSNotification*)notification {
    [_view notifyScaleChanged];
}
@end

static void install_application_menu() {
    NSMenu* menuBar = [[NSMenu alloc] init];
    NSMenuItem* applicationItem = [[NSMenuItem alloc] init];
    [menuBar addItem:applicationItem];
    NSMenu* applicationMenu = [[NSMenu alloc] init];
    NSString* quitTitle = @"Quit GUI.Forms Gallery";
    NSMenuItem* quit = [[NSMenuItem alloc] initWithTitle:quitTitle
                                                  action:@selector(terminate:)
                                           keyEquivalent:@"q"];
    [applicationMenu addItem:quit];
    [applicationItem setSubmenu:applicationMenu];
    [NSApp setMainMenu:menuBar];
}

namespace gui_forms::host {

HostCapabilities macos_capabilities() {
    return {HostCapabilities::current_protocol_version,
            "appkit-macos",
            HostCapability::lifecycle |
                HostCapability::scale_notifications |
                HostCapability::occlusion |
                HostCapability::scheduled_wake |
                HostCapability::pointer_input |
                HostCapability::keyboard_input |
                HostCapability::text_composition |
                HostCapability::monitor_geometry |
                HostCapability::pointer_capture |
                HostCapability::cursor |
                HostCapability::clipboard |
                HostCapability::clipboard_images |
                HostCapability::typed_drag_destination |
                HostCapability::dialogs |
                HostCapability::sound_cues};
}

class DispatchAndCollectWake final {
public:
    DispatchAndCollectWake(
        GUIFormsView* view,
        std::function<void()> dispatch_pending) noexcept
        : view_(view), dispatch_pending_(std::move(dispatch_pending)) {}

    void operator()() const
    {
        GUIFormsView* view = view_;
        const std::function<void()> dispatch_pending = dispatch_pending_;
        dispatch_async(dispatch_get_main_queue(), ^{
            if (dispatch_pending) dispatch_pending();
            [view collectDamage];
        });
    }

private:
    GUIFormsView* view_;
    std::function<void()> dispatch_pending_;
};

class RequestNativeClose final {
public:
    explicit RequestNativeClose(NSWindow* window) noexcept : window_(window) {}

    void operator()() const
    {
        NSWindow* window = window_;
        dispatch_async(dispatch_get_main_queue(), ^{
            [window performClose:nil];
        });
    }

private:
    NSWindow* window_;
};

class RequestNativeShow final {
public:
    explicit RequestNativeShow(NSWindow* window) noexcept : window_(window) {}

    void operator()() const
    {
        NSWindow* window = window_;
        dispatch_async(dispatch_get_main_queue(), ^{
            [window makeKeyAndOrderFront:nil];
            [NSApp activateIgnoringOtherApps:YES];
        });
    }

private:
    NSWindow* window_;
};

class RequestNativeTitle final {
public:
    RequestNativeTitle(NSWindow* const window, GUIFormsView* const view) noexcept
        : window_(window), view_(view) {}
    HostServiceStatus operator()(const std::string_view title) const {
        if (![NSThread isMainThread]) return {HostServiceError::wrong_thread};
        try {
            @try {
                NSWindow* const window = window_;
                GUIFormsView* const view = view_;
                if (window == nil || view == nil || ![view acceptsTitleUpdate])
                    return {HostServiceError::after_shutdown};
                if (title.size() > 65536U || title.find('\0') != std::string_view::npos ||
                    !validate_utf8(title).valid()) return {HostServiceError::invalid_argument};
                NSString* const text = native_string(title);
                if (text == nil) return {HostServiceError::backend_failure};
                [window setTitle:text];
                return {};
            } @catch (NSException* failure) {
                static_cast<void>(failure);
                return {HostServiceError::backend_failure};
            }
        } catch (...) { return {HostServiceError::backend_failure}; }
    }
private:
    __weak NSWindow* window_;
    __weak GUIFormsView* view_;
};
class RequestNativeFullScreen final {
public:
    explicit RequestNativeFullScreen(NSWindow* window) noexcept : window_(window) {}
    void operator()() const {
        NSWindow* window = window_;
        dispatch_async(dispatch_get_main_queue(), ^{ [window toggleFullScreen:nil]; });
    }
private:
    NSWindow* window_;
};

class RequestNativeHide final {
public:
    explicit RequestNativeHide(NSWindow* window) noexcept : window_(window) {}

    void operator()() const
    {
        NSWindow* window = window_;
        dispatch_async(dispatch_get_main_queue(), ^{
            [window orderOut:nil];
        });
    }

private:
    NSWindow* window_;
};

class ShowNativeDialog final {
public:
    explicit ShowNativeDialog(GUIFormsView* view) noexcept : view_(view) {}

    HostDialogResult operator()(const HostDialogRequest& request) const
    {
        return [view_ showHostDialog:request];
    }

private:
    GUIFormsView* view_;
};

class ShowNativeTooltip final {
public:
    explicit ShowNativeTooltip(GUIFormsView* view) noexcept : view_(view) {}

    HostServiceStatus operator()(const HostTooltipRequest& request) const
    {
        return [view_ showHostTooltip:request];
    }

private:
    GUIFormsView* view_;
};

class HideNativeTooltip final {
public:
    explicit HideNativeTooltip(GUIFormsView* view) noexcept : view_(view) {}

    void operator()() const { [view_ hideHostTooltip]; }

private:
    GUIFormsView* view_;
};

class ReadNativeClipboard final {
public:
    explicit ReadNativeClipboard(GUIFormsView* view) noexcept : view_(view) {}

    HostClipboardTextResult operator()() const
    {
        return [view_ readHostClipboard];
    }

private:
    GUIFormsView* view_;
};

class WriteNativeClipboard final {
public:
    explicit WriteNativeClipboard(GUIFormsView* view) noexcept : view_(view) {}

    HostServiceStatus operator()(std::string_view text) const
    {
        return [view_ writeHostClipboard:text];
    }

private:
    GUIFormsView* view_;
};

class StableWindowIdEquals final {
public:
    explicit StableWindowIdEquals(std::string_view stable_id) noexcept
        : stable_id_(stable_id) {}

    bool operator()(const MacApplicationWindow& candidate) const noexcept
    {
        return candidate.stable_id == stable_id_;
    }

private:
    std::string_view stable_id_;
};


int run_macos(std::unique_ptr<Window> model, MacHostOptions options) {
    if (!model || !options.initially_visible || options.hide_on_close) {
        return 2;
    }
    std::vector<std::string> windowDragRegionIds;
    if (!options.window_drag_region_id.empty()) {
        windowDragRegionIds.push_back(options.window_drag_region_id);
    }
    windowDragRegionIds.insert(windowDragRegionIds.end(),
                               options.window_drag_region_ids.begin(),
                               options.window_drag_region_ids.end());
    if (!validate_window_chrome_drag_regions(
            *model, windowDragRegionIds).accepted()) {
        return 2;
    }
    @autoreleasepool {
        NSApplication* application = [NSApplication sharedApplication];
        [application setActivationPolicy:NSApplicationActivationPolicyRegular];
        install_application_menu();

        const NSRect frame = NSMakeRect(0.0, 0.0,
                                        options.initial_size.width,
                                        options.initial_size.height);
        const bool transparentFullSizeContent =
            options.titlebar_presentation ==
            MacTitlebarPresentation::transparent_full_size_content;
        NSWindowStyleMask style = NSWindowStyleMaskTitled |
            NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable |
            NSWindowStyleMaskResizable;
        if (transparentFullSizeContent) {
            style |= NSWindowStyleMaskFullSizeContentView;
        }
        NSWindow* nativeWindow = [[NSWindow alloc]
            initWithContentRect:frame
                      styleMask:style
                        backing:NSBackingStoreBuffered
                          defer:NO];
        // ARC owns this local for the complete run loop. AppKit's historical
        // self-release-on-close policy would otherwise release the same window
        // a second time when the autorelease pool drains.
        [nativeWindow setReleasedWhenClosed:NO];
        GUIFormsView* view = [[GUIFormsView alloc]
            initWithModel:std::move(model)
            transparentFullSizeContent:transparentFullSizeContent
            windowDragRegionIds:std::move(windowDragRegionIds)];
        [view installCloseRequestHandler:std::move(options.close_request)];
        GUIFormsWindowDelegate* delegate =
            [[GUIFormsWindowDelegate alloc] initWithView:view
                                 stopsApplicationOnClose:YES
                                             hideOnClose:NO
                                            closedHandler:std::move(options.closed)];
        [nativeWindow setDelegate:delegate];
        [nativeWindow setTitle:[NSString stringWithUTF8String:options.title.c_str()]];
        if (transparentFullSizeContent) {
            [nativeWindow setTitlebarAppearsTransparent:YES];
            [nativeWindow setTitleVisibility:NSWindowTitleHidden];
        }
        [nativeWindow setContentMinSize:NSMakeSize(options.minimum_size.width,
                                            options.minimum_size.height)];
        [nativeWindow setContentView:view];
        [nativeWindow center];
        if ([view initializeHost] == NO) {
            [view prepareForShutdown];
            [nativeWindow setDelegate:nil];
            [nativeWindow setContentView:nil];
            return 5;
        }
        if (options.host_ready) {
            const std::function<void()> dispatchPending = options.dispatch_pending;
            options.host_ready(
                DispatchAndCollectWake(view, dispatchPending),
                RequestNativeClose(nativeWindow),
                ShowNativeDialog(view),
                ShowNativeTooltip(view),
                HideNativeTooltip(view),
                ReadNativeClipboard(view),
                WriteNativeClipboard(view));
        }
        if (options.full_screen_ready) options.full_screen_ready(RequestNativeFullScreen(nativeWindow));
        if (options.title_ready) options.title_ready(RequestNativeTitle(nativeWindow, view));
        if (options.visibility_ready) {
            options.visibility_ready(RequestNativeShow(nativeWindow),
                                     RequestNativeHide(nativeWindow));
        }
        // Match every host: one FIFO initialization turn runs after portable
        // attach/service publication and before first visible presentation.
        [view drainPostedWork];
        if (options.dispatch_pending) options.dispatch_pending();
        [view collectDamage];
        [nativeWindow makeKeyAndOrderFront:nil];
        [application activateIgnoringOtherApps:YES];
        if (options.close_after_launch_for_testing) {
            const std::uint32_t closeAttempts =
                std::max<std::uint32_t>(1, options.close_attempts_for_testing);
            dispatch_async(dispatch_get_main_queue(), ^{
                for (std::uint32_t attempt = 0;
                     attempt < closeAttempts && nativeWindow.isVisible;
                     ++attempt) {
                    [nativeWindow performClose:nil];
                }
            });
        }
        [application run];
        const std::string metrics = [view metricsJSON];
        const std::string host = [view hostJSON];
        if (options.print_metrics_on_close) {
            std::fprintf(stdout, "{\"window\":%s,\"host\":%s}\n",
                         metrics.c_str(), host.c_str());
            std::fflush(stdout);
        }
        if (options.final_snapshot) {
            options.final_snapshot(metrics, host);
        }
        [view prepareForShutdown];
        [nativeWindow setDelegate:nil];
        [nativeWindow setContentView:nil];
    }
    return 0;
}

int run_macos_application(std::vector<MacApplicationWindow> windows) {
    if (windows.empty()) return 2;
    std::unordered_set<std::string> identities;
    std::size_t primary_count{};
    for (const MacApplicationWindow& entry : windows) {
        if (!entry.model || entry.stable_id.empty() ||
            !identities.insert(entry.stable_id).second ||
            (!entry.owner_id.empty() && entry.owner_id == entry.stable_id)) {
            return 2;
        }
        if (entry.owner_id.empty() && !entry.tool_window) {
            if (!entry.options.initially_visible || entry.options.hide_on_close) {
                return 2;
            }
            ++primary_count;
        }
    }
    for (const MacApplicationWindow& entry : windows) {
        if (!entry.owner_id.empty() && !identities.contains(entry.owner_id)) return 2;
        std::vector<std::string> dragRegionIds;
        if (!entry.options.window_drag_region_id.empty()) {
            dragRegionIds.push_back(entry.options.window_drag_region_id);
        }
        dragRegionIds.insert(dragRegionIds.end(),
                             entry.options.window_drag_region_ids.begin(),
                             entry.options.window_drag_region_ids.end());
        if (!validate_window_chrome_drag_regions(
                *entry.model, dragRegionIds).accepted()) {
            return 2;
        }
        std::string_view owner = entry.owner_id;
        for (std::size_t depth = 0; !owner.empty(); ++depth) {
            if (depth >= windows.size()) return 2;
            const std::vector<MacApplicationWindow>::iterator parent =
                std::find_if(
                    windows.begin(), windows.end(), StableWindowIdEquals(owner));
            owner = (*parent).owner_id;
        }
    }
    if (primary_count != 1U) return 2;

    @autoreleasepool {
        NSApplication* application = [NSApplication sharedApplication];
        [application setActivationPolicy:NSApplicationActivationPolicyRegular];
        install_application_menu();

        NSMutableArray<NSWindow*>* nativeWindows =
            [[NSMutableArray alloc] initWithCapacity:windows.size()];
        NSMutableArray<GUIFormsView*>* views =
            [[NSMutableArray alloc] initWithCapacity:windows.size()];
        NSMutableArray<GUIFormsWindowDelegate*>* delegates =
            [[NSMutableArray alloc] initWithCapacity:windows.size()];

        for (MacApplicationWindow& entry : windows) {
            const NSRect frame = NSMakeRect(0.0, 0.0,
                                            entry.options.initial_size.width,
                                            entry.options.initial_size.height);
            const bool transparentFullSizeContent =
                entry.options.titlebar_presentation ==
                MacTitlebarPresentation::transparent_full_size_content;
            NSWindowStyleMask style = NSWindowStyleMaskTitled |
                NSWindowStyleMaskClosable | NSWindowStyleMaskResizable;
            if (!entry.tool_window && entry.options.minimizable) {
                style |= NSWindowStyleMaskMiniaturizable;
            }
            else style |= NSWindowStyleMaskUtilityWindow;
            if (transparentFullSizeContent) {
                style |= NSWindowStyleMaskFullSizeContentView;
            }
            NSWindow* nativeWindow = entry.tool_window
                ? static_cast<NSWindow*>([[NSPanel alloc]
                      initWithContentRect:frame styleMask:style
                                  backing:NSBackingStoreBuffered defer:NO])
                : [[NSWindow alloc]
                      initWithContentRect:frame styleMask:style
                                  backing:NSBackingStoreBuffered defer:NO];
            [nativeWindow setReleasedWhenClosed:NO];
            std::vector<std::string> windowDragRegionIds;
            if (!entry.options.window_drag_region_id.empty()) {
                windowDragRegionIds.push_back(
                    std::move(entry.options.window_drag_region_id));
            }
            for (std::string& regionId : entry.options.window_drag_region_ids) {
                windowDragRegionIds.push_back(std::move(regionId));
            }
            GUIFormsView* view = [[GUIFormsView alloc]
                initWithModel:std::move(entry.model)
                transparentFullSizeContent:transparentFullSizeContent
                windowDragRegionIds:std::move(windowDragRegionIds)];
            [view installCloseRequestHandler:std::move(entry.options.close_request)];
            const BOOL primary = entry.owner_id.empty() && !entry.tool_window;
            GUIFormsWindowDelegate* delegate =
                [[GUIFormsWindowDelegate alloc] initWithView:view
                                     stopsApplicationOnClose:primary
                                                 hideOnClose:entry.options.hide_on_close
                                                closedHandler:std::move(entry.options.closed)];
            [nativeWindow setDelegate:delegate];
            [nativeWindow setTitle:native_string(entry.options.title)];
            if (transparentFullSizeContent) {
                [nativeWindow setTitlebarAppearsTransparent:YES];
                [nativeWindow setTitleVisibility:NSWindowTitleHidden];
            }
            [nativeWindow setContentMinSize:NSMakeSize(entry.options.minimum_size.width,
                                                entry.options.minimum_size.height)];
            [nativeWindow setContentView:view];
            [nativeWindows addObject:nativeWindow];
            [views addObject:view];
            [delegates addObject:delegate];
        }

        for (std::size_t index = 0; index < windows.size(); ++index) {
            MacApplicationWindow& entry = windows[index];
            NSWindow* nativeWindow = nativeWindows[index];
            if (entry.owner_id.empty()) {
                [nativeWindow center];
                continue;
            }
            const std::vector<MacApplicationWindow>::iterator owner =
                std::find_if(
                    windows.begin(), windows.end(),
                    StableWindowIdEquals(entry.owner_id));
            const std::size_t ownerIndex = static_cast<std::size_t>(
                std::distance(windows.begin(), owner));
            NSWindow* ownerWindow = nativeWindows[ownerIndex];
            [ownerWindow addChildWindow:nativeWindow ordered:NSWindowAbove];
            if (!entry.options.initially_visible) {
                // addChildWindow:ordered: may order a previously hidden child
                // onscreen. Preserve the portable initial-visibility contract
                // after establishing native ownership.
                [nativeWindow orderOut:nil];
            }
            NSScreen* screen = ownerWindow.screen ?: NSScreen.mainScreen;
            NSRect frame = nativeWindow.frame;
            const NSRect ownerFrame = ownerWindow.frame;
            const NSRect work = screen.visibleFrame;
            frame.origin.x = std::clamp(NSMaxX(ownerFrame) + 10.0,
                                        NSMinX(work), NSMaxX(work) - frame.size.width);
            frame.origin.y = std::clamp(NSMaxY(ownerFrame) - frame.size.height,
                                        NSMinY(work), NSMaxY(work) - frame.size.height);
            [nativeWindow setFrame:frame display:NO];
        }

        for (std::size_t index = 0; index < windows.size(); ++index) {
            MacApplicationWindow& entry = windows[index];
            NSWindow* nativeWindow = nativeWindows[index];
            GUIFormsView* view = views[index];
            if ([view initializeHost] == NO) {
                for (GUIFormsView* initializedView in views) {
                    [initializedView prepareForShutdown];
                }
                for (NSWindow* createdWindow in nativeWindows) {
                    [createdWindow setDelegate:nil];
                    [createdWindow setContentView:nil];
                }
                return 5;
            }
            if (entry.options.host_ready) {
                const std::function<void()> dispatchPending =
                    entry.options.dispatch_pending;
                entry.options.host_ready(
                    DispatchAndCollectWake(view, dispatchPending),
                    RequestNativeClose(nativeWindow),
                    ShowNativeDialog(view),
                    ShowNativeTooltip(view),
                    HideNativeTooltip(view),
                    ReadNativeClipboard(view),
                    WriteNativeClipboard(view));
            }
            if (entry.options.full_screen_ready) entry.options.full_screen_ready(RequestNativeFullScreen(nativeWindow));
            if (entry.options.title_ready) entry.options.title_ready(RequestNativeTitle(nativeWindow, view));
            if (entry.options.visibility_ready) {
                entry.options.visibility_ready(
                    RequestNativeShow(nativeWindow),
                    RequestNativeHide(nativeWindow));
            }
            [view drainPostedWork];
            if (entry.options.dispatch_pending) entry.options.dispatch_pending();
            [view collectDamage];
            if (entry.options.initially_visible) {
                if (entry.owner_id.empty() && !entry.tool_window) {
                    [nativeWindow makeKeyAndOrderFront:nil];
                } else {
                    [nativeWindow orderFront:nil];
                }
            }
            if (entry.options.close_after_launch_for_testing) {
                dispatch_async(dispatch_get_main_queue(), ^{
                    [nativeWindow performClose:nil];
                });
            }
        }
        [application activateIgnoringOtherApps:YES];
        [application run];

        // Closing the primary terminates the application session. Close any
        // surviving owned/tool windows while their independent host sessions
        // and delegates are still intact, so each root receives one real
        // HostClosedEvent and its callback observes the actual close point.
        for (GUIFormsWindowDelegate* delegate in delegates) {
            [delegate prepareForApplicationShutdown];
        }
        for (NSWindow* nativeWindow in nativeWindows) {
            if (nativeWindow.isVisible || !nativeWindow.isReleasedWhenClosed) {
                [nativeWindow close];
            }
        }

        for (std::size_t index = 0; index < windows.size(); ++index) {
            MacApplicationWindow& entry = windows[index];
            GUIFormsView* view = views[index];
            NSWindow* nativeWindow = nativeWindows[index];
            const std::string metrics = [view metricsJSON];
            const std::string host = [view hostJSON];
            if (entry.options.print_metrics_on_close) {
                std::fprintf(stdout,
                             "{\"stable_id\":\"%s\",\"window\":%s,\"host\":%s}\n",
                             entry.stable_id.c_str(), metrics.c_str(), host.c_str());
            }
            if (entry.options.final_snapshot) {
                entry.options.final_snapshot(metrics, host);
            }
            [view prepareForShutdown];
            [nativeWindow setDelegate:nil];
            [nativeWindow setContentView:nil];
        }
        std::fflush(stdout);
    }
    return 0;
}

} // namespace gui_forms::host
