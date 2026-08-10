#import <AppKit/AppKit.h>
#import <CoreGraphics/CoreGraphics.h>
#import <CoreVideo/CoreVideo.h>
#import <UniformTypeIdentifiers/UniformTypeIdentifiers.h>

#include "macos_host.hpp"
#include "../../../core/damage/device_damage/device_damage.hpp"
#include "../../../render/skia/raster/skia_raster.hpp"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <exception>
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

std::string utf8_string(id value) {
    NSString* string = nil;
    if ([value isKindOfClass:[NSAttributedString class]]) {
        string = [(NSAttributedString*)value string];
    } else if ([value isKindOfClass:[NSString class]]) {
        string = (NSString*)value;
    }
    if (string == nil) return {};
    const char* bytes = string.UTF8String;
    return bytes == nullptr ? std::string{} : std::string(bytes);
}

NSURL* native_directory_url(const std::string& path) {
    NSString* value = native_string(path);
    return value.length == 0 ? nil : [NSURL fileURLWithPath:value isDirectory:YES];
}

NSArray<UTType*>* native_allowed_types(
    const std::vector<gui_forms::HostFileDialogFilter>& filters) {
    NSMutableArray<UTType*>* types = [[NSMutableArray alloc] init];
    for (const gui_forms::HostFileDialogFilter& filter : filters) {
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
                    const std::pair<NSString*, HostDialogChoice>& button_choice =
                        buttons[index];
                    NSString* const title = button_choice.first;
                    const HostDialogChoice choice = button_choice.second;
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

namespace gui_forms::host {

std::unique_ptr<HostServices> make_macos_host_services(MacHostServiceOptions options) {
    return std::make_unique<AppKitHostServices>(options);
}

} // namespace gui_forms::host
