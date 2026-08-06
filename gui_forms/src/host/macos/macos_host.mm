#import <AppKit/AppKit.h>
#import <CoreGraphics/CoreGraphics.h>
#import <UniformTypeIdentifiers/UniformTypeIdentifiers.h>

#include "macos_host.hpp"
#include "../../core/device_damage.hpp"
#include "../../render/skia/skia_raster.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <utility>

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
using gui_forms::Modifier;
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
            const auto* begin = static_cast<const std::uint8_t*>(data.bytes);
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
                                     NSString* extension = @"ttf") {
    NSURL* url = [[NSBundle mainBundle] URLForResource:resource_name
                                        withExtension:extension
                                         subdirectory:@"fonts"];
    if (url == nil) {
        return false;
    }
    NSData* data = [NSData dataWithContentsOfURL:url
                                        options:NSDataReadingMappedIfSafe
                                          error:nil];
    if (data == nil || data.length == 0) {
        return false;
    }
    const auto* bytes = static_cast<const std::byte*>(data.bytes);
    return raster.register_typeface(role, weight, italic,
                                    std::span<const std::byte>(bytes, data.length));
}

static bool register_bundle_fallback_typeface(SkiaRaster& raster,
                                              NSString* resource_name,
                                              NSString* extension,
                                              std::uint16_t weight = 400,
                                              bool italic = false) {
    NSURL* url = [[NSBundle mainBundle] URLForResource:resource_name
                                        withExtension:extension
                                         subdirectory:@"fonts"];
    if (url == nil) return false;
    NSData* data = [NSData dataWithContentsOfURL:url
                                        options:NSDataReadingMappedIfSafe
                                          error:nil];
    if (data == nil || data.length == 0) return false;
    const auto* bytes = static_cast<const std::byte*>(data.bytes);
    return raster.register_fallback_typeface(
        weight, italic, std::span<const std::byte>(bytes, data.length));
}

namespace {

NSCursor* native_cursor(CursorKind cursor) {
    switch (cursor) {
    case CursorKind::arrow: return [NSCursor arrowCursor];
    case CursorKind::text: return [NSCursor IBeamCursor];
    case CursorKind::hand: return [NSCursor pointingHandCursor];
    case CursorKind::crosshair: return [NSCursor crosshairCursor];
    case CursorKind::resize_horizontal: return [NSCursor resizeLeftRightCursor];
    case CursorKind::resize_vertical: return [NSCursor resizeUpDownCursor];
    case CursorKind::wait: return [NSCursor arrowCursor];
    case CursorKind::forbidden: return [NSCursor operationNotAllowedCursor];
    }
    return [NSCursor arrowCursor];
}

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
    case SemanticRole::generic: return NSAccessibilityGroupRole;
    }
    return NSAccessibilityGroupRole;
}

bool semantic_has_action(const SemanticNode& node, SemanticAction action) {
    return std::find(node.actions.begin(), node.actions.end(), action) !=
           node.actions.end();
}

NSURL* native_directory_url(const std::string& path) {
    NSString* value = native_string(path);
    return value.length == 0 ? nil : [NSURL fileURLWithPath:value isDirectory:YES];
}

NSArray<UTType*>* native_allowed_types(
    const std::vector<gui_forms::HostFileDialogFilter>& filters) {
    NSMutableArray<UTType*>* types = [[NSMutableArray alloc] init];
    for (const auto& filter : filters) {
        for (const std::string& extension : filter.extensions) {
            std::string normalized = extension;
            while (!normalized.empty() && normalized.front() == '.') {
                normalized.erase(normalized.begin());
            }
            NSString* value = native_string(normalized);
            UTType* type = value.length == 0
                ? nil : [UTType typeWithFilenameExtension:value];
            if (type != nil && ![types containsObject:type]) {
                [types addObject:type];
            }
        }
    }
    return types;
}

HostPathDialogResult native_path_result(NSModalResponse response,
                                        NSArray<NSURL*>* urls) {
    HostPathDialogResult result;
    if (response != NSModalResponseOK) {
        return result;
    }
    result.outcome = HostDialogOutcome::accepted;
    for (NSURL* url in urls) {
        const std::string path = utf8_string(url.path);
        if (!path.empty()) {
            result.paths.push_back(path);
        }
    }
    return result;
}

void schedule_test_panel_cancel(NSSavePanel* panel, bool enabled) {
    if (!enabled) {
        return;
    }
    dispatch_async(dispatch_get_main_queue(), ^{
        [panel cancel:nil];
    });
}

void schedule_test_alert_cancel(NSAlert* alert, bool enabled) {
    if (!enabled) {
        return;
    }
    dispatch_async(dispatch_get_main_queue(), ^{
        [alert.window orderOut:nil];
        [NSApp abortModal];
    });
}

class AppKitHostServices final : public HostServices {
public:
    explicit AppKitHostServices(const gui_forms::host::MacHostServiceOptions& options)
        : HostServices(gui_forms::host::macos_capabilities()),
          cancel_dialogs_for_testing_(options.cancel_dialogs_for_testing) {
        if (options.clipboard_name.empty()) {
            pasteboard_ = [NSPasteboard generalPasteboard];
        } else {
            NSString* name = [[NSString alloc]
                initWithBytes:options.clipboard_name.data()
                       length:options.clipboard_name.size()
                     encoding:NSUTF8StringEncoding];
            pasteboard_ = name == nil ? nil : [NSPasteboard pasteboardWithName:name];
        }
    }
    ~AppKitHostServices() override {
        shutdown();
    }

protected:
    HostMonitorResult query_monitors_impl() override {
        HostMonitorResult result;
        NSArray<NSScreen*>* screens = [NSScreen screens];
        NSScreen* primary = screens.firstObject;
        double desktop_top = 0.0;
        for (NSScreen* screen in screens) {
            desktop_top = std::max(desktop_top, NSMaxY(screen.frame));
        }
        std::size_t index = 0;
        for (NSScreen* screen in screens) {
            const NSRect frame = screen.frame;
            const NSRect visible = screen.visibleFrame;
            NSNumber* screen_number = screen.deviceDescription[@"NSScreenNumber"];
            std::string identifier = "appkit.screen." + std::to_string(index);
            if (screen_number != nil) {
                identifier = "appkit.display." +
                    std::to_string(screen_number.unsignedIntValue);
            }
            result.monitors.push_back({
                std::move(identifier),
                {frame.origin.x, desktop_top - NSMaxY(frame),
                 frame.size.width, frame.size.height},
                {visible.origin.x, desktop_top - NSMaxY(visible),
                 visible.size.width, visible.size.height},
                screen.backingScaleFactor,
                screen == primary});
            ++index;
        }
        if (result.monitors.empty()) {
            result.status.error = HostServiceError::backend_failure;
        }
        return result;
    }

    HostServiceStatus set_cursor_impl(CursorKind cursor) override {
        [native_cursor(cursor) set];
        return {};
    }

    HostServiceStatus set_pointer_capture_impl(bool, std::uint64_t) override {
        // AppKit owns mouse-drag delivery from mouseDown through mouseUp. This
        // acknowledgment keeps that native sequence synchronized with the
        // retained capture owner; it does not install a global event tap.
        return {};
    }

    HostClipboardTextResult read_clipboard_text_impl() override {
        HostClipboardTextResult result;
        if (pasteboard_ == nil) {
            result.status.error = HostServiceError::backend_failure;
            return result;
        }
        result.generation = static_cast<std::uint64_t>(pasteboard_.changeCount);
        NSString* value = [pasteboard_ stringForType:NSPasteboardTypeString];
        if (value == nil) {
            return result;
        }
        NSData* encoded = [value dataUsingEncoding:NSUTF8StringEncoding];
        if (encoded == nil) {
            result.status.error = HostServiceError::backend_failure;
            return result;
        }
        if (encoded.length != 0) {
            result.text_utf8.assign(static_cast<const char*>(encoded.bytes), encoded.length);
        }
        result.has_text = true;
        return result;
    }

    HostServiceStatus write_clipboard_text_impl(std::string_view text_utf8) override {
        if (pasteboard_ == nil) {
            return {HostServiceError::backend_failure};
        }
        NSString* value = [[NSString alloc] initWithBytes:text_utf8.data()
                                                   length:text_utf8.size()
                                                 encoding:NSUTF8StringEncoding];
        if (value == nil) {
            return {HostServiceError::invalid_utf8};
        }
        [pasteboard_ clearContents];
        return [pasteboard_ setString:value forType:NSPasteboardTypeString]
                   ? HostServiceStatus{}
                   : HostServiceStatus{HostServiceError::backend_failure};
    }

    HostDialogResult show_dialog_impl(const HostDialogRequest& request) override {
        return std::visit([this, &request](const auto& payload) -> HostDialogResult {
            using Payload = std::decay_t<decltype(payload)>;
            if constexpr (std::is_same_v<Payload, HostMessageDialogRequest>) {
                NSAlert* alert = [[NSAlert alloc] init];
                alert.messageText = native_string(payload.title);
                alert.informativeText = native_string(payload.message);
                switch (payload.icon) {
                case HostMessageIcon::warning:
                case HostMessageIcon::question:
                    alert.alertStyle = NSAlertStyleWarning;
                    break;
                case HostMessageIcon::error:
                    alert.alertStyle = NSAlertStyleCritical;
                    break;
                case HostMessageIcon::none:
                case HostMessageIcon::information:
                    alert.alertStyle = NSAlertStyleInformational;
                    break;
                }
                std::vector<std::pair<NSString*, HostDialogChoice>> buttons;
                switch (payload.buttons) {
                case HostMessageButtons::ok:
                    buttons = {{@"OK", HostDialogChoice::ok}};
                    break;
                case HostMessageButtons::ok_cancel:
                    buttons = {{@"OK", HostDialogChoice::ok},
                               {@"Cancel", HostDialogChoice::cancel}};
                    break;
                case HostMessageButtons::yes_no:
                    buttons = {{@"Yes", HostDialogChoice::yes},
                               {@"No", HostDialogChoice::no}};
                    break;
                case HostMessageButtons::yes_no_cancel:
                    buttons = {{@"Yes", HostDialogChoice::yes},
                               {@"No", HostDialogChoice::no},
                               {@"Cancel", HostDialogChoice::cancel}};
                    break;
                case HostMessageButtons::retry_cancel:
                    buttons = {{@"Retry", HostDialogChoice::retry},
                               {@"Cancel", HostDialogChoice::cancel}};
                    break;
                }
                NSButton* defaultButton = nil;
                NSModalResponse cancelResponse = 0;
                for (std::size_t index = 0U; index < buttons.size(); ++index) {
                    const auto& [title, choice] = buttons[index];
                    NSButton* button = [alert addButtonWithTitle:title];
                    // Keep cancellation and default activation independent:
                    // Escape must always reach an admitted cancel choice, while
                    // the window's default cell preserves Return even when
                    // Cancel itself is the declared default.
                    button.keyEquivalent = choice == HostDialogChoice::cancel
                        ? @"\e" : @"";
                    if (choice == payload.default_choice) defaultButton = button;
                    if (choice == HostDialogChoice::cancel) {
                        cancelResponse = NSAlertFirstButtonReturn +
                            static_cast<NSModalResponse>(index);
                    }
                }
                if (defaultButton != nil) {
                    alert.window.defaultButtonCell = defaultButton.cell;
                }
                schedule_test_alert_cancel(alert, cancel_dialogs_for_testing_);
                id escapeMonitor = nil;
                if (cancelResponse != 0) {
                    escapeMonitor = [NSEvent addLocalMonitorForEventsMatchingMask:
                        NSEventMaskKeyDown handler:^NSEvent* (NSEvent* event) {
                            if (event.keyCode == 53U) {
                                [NSApp stopModalWithCode:cancelResponse];
                                return nil;
                            }
                            return event;
                        }];
                }
                const NSModalResponse response = [alert runModal];
                if (escapeMonitor != nil) [NSEvent removeMonitor:escapeMonitor];
                HostMessageDialogResult value;
                const NSInteger index = response - NSAlertFirstButtonReturn;
                if (index >= 0 && static_cast<std::size_t>(index) < buttons.size()) {
                    value.choice = buttons[static_cast<std::size_t>(index)].second;
                    value.outcome = value.choice == HostDialogChoice::cancel
                        ? HostDialogOutcome::cancelled
                        : HostDialogOutcome::accepted;
                }
                return {{}, request.request_id, value};
            } else if constexpr (std::is_same_v<Payload, HostOpenFileDialogRequest>) {
                NSOpenPanel* panel = [NSOpenPanel openPanel];
                panel.title = native_string(payload.title);
                panel.directoryURL = native_directory_url(payload.initial_directory);
                panel.nameFieldStringValue = native_string(payload.suggested_name);
                panel.canChooseFiles = YES;
                panel.canChooseDirectories = NO;
                panel.allowsMultipleSelection = payload.allow_multiple;
                NSArray<UTType*>* types = native_allowed_types(payload.filters);
                if (types.count != 0) {
                    panel.allowedContentTypes = types;
                }
                schedule_test_panel_cancel(panel, cancel_dialogs_for_testing_);
                return {{}, request.request_id,
                        native_path_result([panel runModal], panel.URLs)};
            } else if constexpr (std::is_same_v<Payload, HostSaveFileDialogRequest>) {
                NSSavePanel* panel = [NSSavePanel savePanel];
                panel.title = native_string(payload.title);
                panel.directoryURL = native_directory_url(payload.initial_directory);
                panel.nameFieldStringValue = native_string(payload.suggested_name);
                NSArray<UTType*>* types = native_allowed_types(payload.filters);
                if (types.count == 0 && !payload.default_extension.empty()) {
                    gui_forms::HostFileDialogFilter fallback;
                    fallback.extensions.push_back(payload.default_extension);
                    types = native_allowed_types({fallback});
                }
                if (types.count != 0) {
                    panel.allowedContentTypes = types;
                }
                schedule_test_panel_cancel(panel, cancel_dialogs_for_testing_);
                const NSModalResponse response = [panel runModal];
                NSArray<NSURL*>* urls = panel.URL == nil ? @[] : @[panel.URL];
                return {{}, request.request_id, native_path_result(response, urls)};
            } else if constexpr (std::is_same_v<Payload, HostFolderDialogRequest>) {
                NSOpenPanel* panel = [NSOpenPanel openPanel];
                panel.title = native_string(payload.title);
                panel.directoryURL = native_directory_url(payload.initial_directory);
                panel.canChooseFiles = NO;
                panel.canChooseDirectories = YES;
                panel.allowsMultipleSelection = NO;
                schedule_test_panel_cancel(panel, cancel_dialogs_for_testing_);
                return {{}, request.request_id,
                        native_path_result([panel runModal], panel.URLs)};
            } else if constexpr (std::is_same_v<Payload, HostColorDialogRequest>) {
                NSAlert* alert = [[NSAlert alloc] init];
                alert.messageText = native_string(payload.title);
                NSColorWell* well = [[NSColorWell alloc]
                    initWithFrame:NSMakeRect(0.0, 0.0, 180.0, 28.0)];
                const CGFloat red = static_cast<CGFloat>((payload.initial_rgba >> 24U) & 0xFFU) / 255.0;
                const CGFloat green = static_cast<CGFloat>((payload.initial_rgba >> 16U) & 0xFFU) / 255.0;
                const CGFloat blue = static_cast<CGFloat>((payload.initial_rgba >> 8U) & 0xFFU) / 255.0;
                const CGFloat alpha = payload.allow_alpha
                    ? static_cast<CGFloat>(payload.initial_rgba & 0xFFU) / 255.0 : 1.0;
                well.color = [NSColor colorWithSRGBRed:red green:green blue:blue alpha:alpha];
                alert.accessoryView = well;
                [alert addButtonWithTitle:@"OK"];
                [alert addButtonWithTitle:@"Cancel"];
                schedule_test_alert_cancel(alert, cancel_dialogs_for_testing_);
                const NSModalResponse response = [alert runModal];
                HostColorDialogResult value;
                if (response == NSAlertFirstButtonReturn) {
                    NSColor* color = [well.color colorUsingColorSpace:[NSColorSpace sRGBColorSpace]];
                    const auto byte = [](CGFloat component) {
                        return static_cast<std::uint32_t>(
                            std::lround(std::clamp(component, 0.0, 1.0) * 255.0));
                    };
                    value.outcome = HostDialogOutcome::accepted;
                    value.rgba = (byte(color.redComponent) << 24U) |
                                 (byte(color.greenComponent) << 16U) |
                                 (byte(color.blueComponent) << 8U) |
                                 byte(payload.allow_alpha ? color.alphaComponent : 1.0);
                }
                return {{}, request.request_id, value};
            }
            return {{HostServiceError::unsupported}, request.request_id,
                    HostMessageDialogResult{}};
        }, request.payload);
    }

    HostServiceStatus play_sound_cue_impl(
        const HostSoundCueRequest& request) override {
        NSString* name = @"Ping";
        switch (request.cue) {
        case HostSoundCue::notification: name = @"Ping"; break;
        case HostSoundCue::success: name = @"Glass"; break;
        case HostSoundCue::warning: name = @"Funk"; break;
        case HostSoundCue::error: name = @"Basso"; break;
        case HostSoundCue::operation_complete: name = @"Pop"; break;
        }
        NSSound* sound = [NSSound soundNamed:name];
        if (sound == nil) {
            NSBeep();
            return {};
        }
        sound.volume = static_cast<float>(request.gain);
        return [sound play] ? HostServiceStatus{}
                            : HostServiceStatus{HostServiceError::backend_failure};
    }

    void shutdown_impl() noexcept override {
        pasteboard_ = nil;
    }

private:
    __strong NSPasteboard* pasteboard_{};
    bool cancel_dialogs_for_testing_{};
};

} // namespace

@class GUIFormsAccessibilityElement;

@interface GUIFormsView : NSView <NSTextInputClient, NSDraggingDestination> {
    std::unique_ptr<Window> _model;
    std::unique_ptr<HostSession> _hostSession;
    std::unique_ptr<HostServices> _hostServices;
    gui_forms::SubscriptionToken _closeRequestSubscription;
    std::uint64_t _nextHostSequence;
    SkiaRaster _raster;
    DamageRegion _pendingDamage;
    NSMutableAttributedString* _markedText;
    NSRange _selectedRange;
    NSTrackingArea* _trackingArea;
    dispatch_source_t _wakeSource;
    NSPanel* _tooltipPanel;
    NSTimer* _tooltipTimer;
    std::uint64_t _lastSemanticGeneration;
    std::uint64_t _accessibilityCacheGeneration;
    NSArray* _semanticAccessibilityChildren;
    NSMutableDictionary<NSString*, GUIFormsAccessibilityElement*>*
        _semanticAccessibilityElements;
}
- (instancetype)initWithModel:(std::unique_ptr<Window>)model;
- (void)installCloseRequestHandler:(std::function<void(HostCloseRequest&)>)handler;
- (HostDispatchResult)dispatchHostPayload:(HostEventPayload)payload
                         timestampNanoseconds:(std::uint64_t)timestamp;
- (BOOL)requestClose;
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
- (void)prepareForShutdown;
- (HostDialogResult)showHostDialog:(const HostDialogRequest&)request;
- (HostClipboardTextResult)readHostClipboard;
- (HostServiceStatus)writeHostClipboard:(std::string_view)text;
- (HostServiceStatus)showHostTooltip:(const HostTooltipRequest&)request;
- (void)hideHostTooltip;
- (DragEvent)dragEventFor:(id<NSDraggingInfo>)sender action:(DragAction)action;
- (std::string)metricsJSON;
- (std::string)hostJSON;
- (NSRect)accessibilityFrameForSemanticBounds:(GFRect)bounds;
- (BOOL)performSemanticAction:(SemanticAction)action
                     stableId:(NSString*)stableId
                        value:(NSString*)value;
@end

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
    if (_owner == nil || !semantic_has_action(_node, SemanticAction::expand)) return NO;
    return [_owner performSemanticAction:SemanticAction::expand
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
        return semantic_has_action(_node, SemanticAction::expand);
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

@implementation GUIFormsView

- (instancetype)initWithModel:(std::unique_ptr<Window>)model {
    self = [super initWithFrame:NSMakeRect(0.0, 0.0, 980.0, 680.0)];
    if (self != nil) {
        _model = std::move(model);
        _hostServices = gui_forms::host::make_macos_host_services();
        _hostSession = std::make_unique<HostSession>(
            *_model, gui_forms::host::macos_capabilities(), _hostServices.get());
        _nextHostSequence = 1;
        _lastSemanticGeneration = 0;
        _accessibilityCacheGeneration = 0;
        _semanticAccessibilityChildren = nil;
        _semanticAccessibilityElements = [[NSMutableDictionary alloc] init];
        _markedText = [[NSMutableAttributedString alloc] init];
        _selectedRange = NSMakeRange(NSNotFound, 0);
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
        _model->set_dispatch_wake_handler([weakSelf] {
            dispatch_async(dispatch_get_main_queue(), ^{
                GUIFormsView* strongSelf = weakSelf;
                if (strongSelf != nil) [strongSelf drainPostedWork];
            });
        });
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
                                     gui_forms::FontRole::monospace, 700) &&
            register_bundle_typeface(_raster, @"Cousine-Italic",
                                     gui_forms::FontRole::monospace, 400, true) &&
            register_bundle_typeface(_raster, @"Cousine-BoldItalic",
                                     gui_forms::FontRole::monospace, 700, true) &&
            register_bundle_fallback_typeface(
                _raster, @"NotoSansCJKjp-Regular", @"otf") &&
            register_bundle_fallback_typeface(
                _raster, @"NotoEmoji-Regular", @"ttf");
        _model->metrics().set_renderer(
            fonts_ready
                ? "Skia CPU m152 · HarfBuzz 14.2.1 · FreeType 2.14.2 · bundled fonts + CJK/emoji fallback"
                : "Skia CPU m152 · incomplete bundled font pack",
            true);
        static_cast<void>([self dispatchHostPayload:
            HostAttachEvent{GFSize{980.0, 680.0}, 1.0}
                               timestampNanoseconds:host_now_nanoseconds()]);
        [[NSNotificationCenter defaultCenter]
            addObserver:self
               selector:@selector(screenParametersChanged:)
                   name:NSApplicationDidChangeScreenParametersNotification
                 object:nil];
        [self notifyDisplaysChanged];
        [self collectDamage];
    }
    return self;
}

- (void)installCloseRequestHandler:(std::function<void(HostCloseRequest&)>)handler {
    _closeRequestSubscription.disconnect();
    if (_hostSession != nullptr && handler) {
        _closeRequestSubscription = _hostSession->closing().subscribe(std::move(handler));
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
    return _hostSession->dispatch(std::move(event));
}

- (BOOL)requestClose {
    const HostDispatchResult result = [self dispatchHostPayload:
        HostCloseRequest{HostCloseReason::user, false}
                                           timestampNanoseconds:host_now_nanoseconds()];
    return result.accepted() && result.close_allowed;
}

- (void)notifyClosed {
    static_cast<void>([self dispatchHostPayload:HostClosedEvent{HostCloseReason::user}
                               timestampNanoseconds:host_now_nanoseconds()]);
}

- (void)notifyActivation:(BOOL)active {
    static_cast<void>([self dispatchHostPayload:HostActivationEvent{active == YES}
                               timestampNanoseconds:host_now_nanoseconds()]);
    [self collectDamage];
}

- (void)notifyOcclusion:(BOOL)occluded {
    static_cast<void>([self dispatchHostPayload:HostOcclusionEvent{occluded == YES}
                               timestampNanoseconds:host_now_nanoseconds()]);
    if (occluded == YES) {
        [self armWakeTimer];
    } else {
        [self collectDamage];
    }
}

- (void)notifyScaleChanged {
    const double scale = self.window == nil ? 1.0 : self.window.backingScaleFactor;
    static_cast<void>([self dispatchHostPayload:HostScaleEvent{scale}
                               timestampNanoseconds:host_now_nanoseconds()]);
    [self collectDamage];
}

- (void)notifyDisplaysChanged {
    if (_hostServices == nullptr) {
        return;
    }
    HostMonitorResult monitors = _hostServices->query_monitors();
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
    const gui_forms::SemanticSnapshot snapshot = _model->semantic_snapshot();
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
    const bool handled = _model->perform_semantic_action(identifier, action, actionValue);
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
}

- (void)viewWillMoveToWindow:(NSWindow*)newWindow {
    if (newWindow == nil && _wakeSource != nil) {
        dispatch_source_set_timer(_wakeSource, DISPATCH_TIME_FOREVER,
                                  DISPATCH_TIME_FOREVER, 0);
    }
    [super viewWillMoveToWindow:newWindow];
}

- (void)dealloc {
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    if (_wakeSource != nil) {
        dispatch_source_cancel(_wakeSource);
        _wakeSource = nil;
    }
    [self hideHostTooltip];
    if (_hostSession != nullptr) {
        _hostSession->shutdown();
        _hostSession.reset();
    }
    if (_hostServices != nullptr) {
        _hostServices->shutdown();
        _hostServices.reset();
    }
    _model.reset();
}

- (HostDialogResult)showHostDialog:(const HostDialogRequest&)request {
    if (_hostServices == nullptr) {
        return {{HostServiceError::after_shutdown}, request.request_id,
                HostPathDialogResult{}};
    }
    [self hideHostTooltip];
    return _hostServices->show_dialog(request);
}

- (HostClipboardTextResult)readHostClipboard {
    if (_hostServices == nullptr) {
        return {{HostServiceError::after_shutdown}, {}, 0, false};
    }
    return _hostServices->read_clipboard_text();
}

- (HostServiceStatus)writeHostClipboard:(std::string_view)text {
    if (_hostServices == nullptr) return {HostServiceError::after_shutdown};
    return _hostServices->write_clipboard_text(text);
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
        NSTrackingMouseMoved | NSTrackingActiveInKeyWindow | NSTrackingInVisibleRect;
    _trackingArea = [[NSTrackingArea alloc] initWithRect:NSZeroRect
                                                 options:options
                                                   owner:self
                                                userInfo:nil];
    [self addTrackingArea:_trackingArea];
    [super updateTrackingAreas];
}

- (void)collectDamage {
    if (!_model) {
        return;
    }
    static_cast<void>(_model->poll_frame_schedule(std::chrono::steady_clock::now()));
    const std::uint64_t semanticGeneration = _model->semantic_generation();
    if (semanticGeneration != _lastSemanticGeneration) {
        _lastSemanticGeneration = semanticGeneration;
        _semanticAccessibilityChildren = nil;
        _accessibilityCacheGeneration = 0;
        NSAccessibilityPostNotification(self,
                                        NSAccessibilityLayoutChangedNotification);
    }
    DamageRegion damage = _model->take_damage();
    const double scale = self.window == nil ? 1.0 : self.window.backingScaleFactor;
    for (GFRect rect : damage.rectangles()) {
        rect = gui_forms::detail::align_damage_outward(rect, scale);
        _pendingDamage.add(rect);
        [self setNeedsDisplayInRect:NSMakeRect(rect.x, rect.y, rect.width, rect.height)];
    }
    [self armWakeTimer];
}

- (void)drainPostedWork {
    if (!_model) return;
    static_cast<void>(_model->drain_posted_work());
    [self collectDamage];
}

- (void)armWakeTimer {
    if (!_model || _wakeSource == nil) {
        return;
    }
    const auto wake = _model->next_wake();
    if (!wake) {
        dispatch_source_set_timer(_wakeSource, DISPATCH_TIME_FOREVER,
                                  DISPATCH_TIME_FOREVER, 0);
        return;
    }
    const auto now = std::chrono::steady_clock::now();
    const auto delay = std::max(std::chrono::milliseconds(1),
                                std::chrono::duration_cast<std::chrono::milliseconds>(
                                    *wake - now));
    const auto nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(
        delay).count();
    dispatch_source_set_timer(
        _wakeSource,
        dispatch_time(DISPATCH_TIME_NOW, static_cast<std::int64_t>(nanoseconds)),
        DISPATCH_TIME_FOREVER,
        static_cast<std::uint64_t>(std::chrono::microseconds(250).count() * 1000));
}

- (void)scheduledWake {
    [self collectDamage];
}

- (void)prepareForShutdown {
    if (_wakeSource != nil) {
        dispatch_source_set_timer(_wakeSource, DISPATCH_TIME_FOREVER,
                                  DISPATCH_TIME_FOREVER, 0);
    }
    if (_model) _model->shutdown_dispatcher();
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
    if (!_model) {
        return;
    }
    const auto started = std::chrono::steady_clock::now();
    const double scale = self.window == nil ? 1.0 : self.window.backingScaleFactor;
    const GFSize logicalSize{self.bounds.size.width, self.bounds.size.height};
    if (_raster.resize(logicalSize, scale)) {
        _pendingDamage.add(GFRect{0.0, 0.0, logicalSize.width, logicalSize.height});
    }
    if (_pendingDamage.empty()) {
        _pendingDamage.add(GFRect{dirtyRect.origin.x, dirtyRect.origin.y,
                                dirtyRect.size.width, dirtyRect.size.height});
    }

    static_cast<void>(_raster.synchronize_images(_model->image_resources()));
    _raster.begin_frame(_pendingDamage);
    _model->paint(_raster, _pendingDamage.bounds());
    _raster.end_frame();

    const void* pixels = _raster.pixels();
    if (pixels != nullptr) {
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
        if (image != nullptr) {
            CGContextRef context = NSGraphicsContext.currentContext.CGContext;
            CGContextSaveGState(context);
            CGContextSetBlendMode(context, kCGBlendModeCopy);
            CGContextTranslateCTM(context, 0.0, logicalSize.height);
            CGContextScaleCTM(context, 1.0, -1.0);
            CGContextDrawImage(context,
                               CGRectMake(0.0, 0.0, logicalSize.width, logicalSize.height),
                               image);
            CGContextRestoreGState(context);
            CGImageRelease(image);
        }
        CGColorSpaceRelease(colorSpace);
        CGDataProviderRelease(provider);
    }
    _pendingDamage.clear();
    const auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::steady_clock::now() - started);
    _model->metrics().record_present(static_cast<std::uint64_t>(elapsed.count()));
    // Do not collect the next model invalidation from inside AppKit's paint
    // transaction. setNeedsDisplayInRect: may leave the view dirty without
    // enqueueing a second draw when invoked synchronously by drawRect:, which
    // lets animation callbacks advance while the last raster remains frozen.
    // The retained deadline source owns the next poll; arm it after presenting
    // and collect damage from its main-queue callback.
    [self armWakeTimer];
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
    const GFPoint position = input.position;
    static_cast<void>([self dispatchHostPayload:std::move(input)
                               timestampNanoseconds:host_event_nanoseconds(event)]);
    if (_hostServices != nullptr) {
        const auto target = _model->hit_test(position);
        static_cast<void>(_hostServices->set_cursor(
            target ? target->effective_cursor() : CursorKind::arrow));
    }
    [self collectDamage];
}

- (void)mouseExited:(NSEvent*)event {
    if (_hostServices != nullptr) {
        static_cast<void>(_hostServices->set_cursor(CursorKind::arrow));
    }
}

- (void)mouseDown:(NSEvent*)event {
    [self.window makeFirstResponder:self];
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
    if (const auto focused = _model->focused_control()) {
        const GFRect bounds = focused->absolute_bounds();
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
    return _model ? _model->metrics_snapshot().to_json() : std::string("{}");
}

- (std::string)hostJSON {
    const std::string session = _hostSession
        ? _hostSession->snapshot().to_json() : std::string("{}");
    const std::string services = _hostServices
        ? _hostServices->snapshot().to_json() : std::string("{}");
    return "{\"session\":" + session + ",\"services\":" + services + "}";
}

@end

@interface GUIFormsWindowDelegate : NSObject <NSWindowDelegate> {
    __weak GUIFormsView* _view;
}
- (instancetype)initWithView:(GUIFormsView*)view;
@end

@implementation GUIFormsWindowDelegate
- (instancetype)initWithView:(GUIFormsView*)view {
    self = [super init];
    if (self != nil) {
        _view = view;
    }
    return self;
}
- (BOOL)windowShouldClose:(NSWindow*)sender {
    return _view == nil || [_view requestClose];
}
- (void)windowWillClose:(NSNotification*)notification {
    [_view notifyClosed];
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
                HostCapability::typed_drag_destination |
                HostCapability::dialogs |
                HostCapability::sound_cues};
}

std::unique_ptr<HostServices> make_macos_host_services(MacHostServiceOptions options) {
    return std::make_unique<AppKitHostServices>(options);
}

int run_macos(std::unique_ptr<Window> model, MacHostOptions options) {
    if (!model) {
        return 2;
    }
    @autoreleasepool {
        NSApplication* application = [NSApplication sharedApplication];
        [application setActivationPolicy:NSApplicationActivationPolicyRegular];
        install_application_menu();

        const NSRect frame = NSMakeRect(0.0, 0.0,
                                        options.initial_size.width,
                                        options.initial_size.height);
        const NSWindowStyleMask style = NSWindowStyleMaskTitled |
            NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable |
            NSWindowStyleMaskResizable;
        NSWindow* nativeWindow = [[NSWindow alloc]
            initWithContentRect:frame
                      styleMask:style
                        backing:NSBackingStoreBuffered
                          defer:NO];
        // ARC owns this local for the complete run loop. AppKit's historical
        // self-release-on-close policy would otherwise release the same window
        // a second time when the autorelease pool drains.
        [nativeWindow setReleasedWhenClosed:NO];
        GUIFormsView* view = [[GUIFormsView alloc] initWithModel:std::move(model)];
        [view installCloseRequestHandler:std::move(options.close_request)];
        GUIFormsWindowDelegate* delegate =
            [[GUIFormsWindowDelegate alloc] initWithView:view];
        [nativeWindow setDelegate:delegate];
        [nativeWindow setTitle:[NSString stringWithUTF8String:options.title.c_str()]];
        [nativeWindow setMinSize:NSMakeSize(options.minimum_size.width,
                                            options.minimum_size.height)];
        [nativeWindow setContentView:view];
        [nativeWindow center];
        [nativeWindow makeKeyAndOrderFront:nil];
        [application activateIgnoringOtherApps:YES];
        if (options.host_ready) {
            const std::function<void()> dispatchPending = options.dispatch_pending;
            options.host_ready(
                [view, dispatchPending] {
                    dispatch_async(dispatch_get_main_queue(), ^{
                        if (dispatchPending) dispatchPending();
                        [view collectDamage];
                    });
                },
                [nativeWindow] {
                    dispatch_async(dispatch_get_main_queue(), ^{
                        [nativeWindow performClose:nil];
                    });
                },
                [view](const HostDialogRequest& request) {
                    return [view showHostDialog:request];
                },
                [view](const HostTooltipRequest& request) {
                    return [view showHostTooltip:request];
                },
                [view] {
                    [view hideHostTooltip];
                },
                [view] {
                    return [view readHostClipboard];
                },
                [view](std::string_view text) {
                    return [view writeHostClipboard:text];
                });
        }
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
        if (options.closed) options.closed();

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

} // namespace gui_forms::host
