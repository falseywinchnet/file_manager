#import <AppKit/AppKit.h>
#include "gui_forms/platform/macos_host.hpp"
#include "gui_forms/gui_forms.hpp"
#include <iostream>
#include <functional>
#include <string>
#include <chrono>
#include <memory>

// Exercise the host's native callback ordering without adding testing APIs to
// the public C++ surface. AppKit can request a snapshot before unocclusion.
@interface NSView (ExposureProbe)
- (void)notifyOcclusion:(BOOL)occluded;
@end

namespace {
class ExposureRoot final : public gui_forms::Control {
public:
    explicit ExposureRoot(gui_forms::StableId id) : Control(std::move(id)) {}
    void on_paint(gui_forms::Painter& painter, gui_forms::Rect) override {
        ++paints;
        painter.fill_rect(client_rectangle(), gui_forms::Color::rgba(30, 95, 180));
        painter.fill_rect({10, 10, 30, 30}, gui_forms::Color::rgba(220, 50, 60));
    }
    int paints{};
};
struct State {
    std::shared_ptr<ExposureRoot> root;
    std::function<void()> close;
    std::string failure;
    bool completed{};
    std::chrono::steady_clock::time_point initial_deadline{};
    unsigned initial_checks{0};
};
void exercise(const std::shared_ptr<State>& state);
void check_initial_frame(void* const context) {
    // The queued callback owns its state, but never retains a native view borrow.
    const std::unique_ptr<std::shared_ptr<State>> retained(
        static_cast<std::shared_ptr<State>*>(context));
    const std::shared_ptr<State>& state = *retained;
    try {
        exercise(state);
    } catch (const std::exception& error) {
        (*state).failure = error.what();
        (*state).close();
    }
}
void queue_initial_check(const std::shared_ptr<State>& state) {
    std::unique_ptr<std::shared_ptr<State>> retained =
        std::make_unique<std::shared_ptr<State>>(state);
    const dispatch_time_t deadline = dispatch_time(DISPATCH_TIME_NOW, 50 * NSEC_PER_MSEC);
    void* const context = retained.release();
    dispatch_after_f(deadline, dispatch_get_main_queue(), context, check_initial_frame);
}
bool has_retained_pixels(NSView* view) {
    static NSColor* reference = nil;
    NSBitmapImageRep* bitmap = [view bitmapImageRepForCachingDisplayInRect:view.bounds];
    if (bitmap == nil) return false;
    [view cacheDisplayInRect:view.bounds toBitmapImageRep:bitmap];
    NSColor* sample = [[bitmap colorAtX:bitmap.pixelsWide / 2 y:bitmap.pixelsHigh / 2]
        colorUsingColorSpace:NSColorSpace.sRGBColorSpace];
    if (sample == nil || sample.alphaComponent < 0.98) return false;
    if (reference == nil) {
        if (sample.blueComponent < 0.6 || sample.redComponent > 0.3) return false;
        reference = sample;
    }
    return std::abs(sample.redComponent - reference.redComponent) < 0.01 &&
        std::abs(sample.greenComponent - reference.greenComponent) < 0.01 &&
        std::abs(sample.blueComponent - reference.blueComponent) < 0.01;
}
void exercise(const std::shared_ptr<State>& state) {
    NSWindow* window = nil;
    for (NSWindow* candidate in NSApp.windows) {
        if ([candidate.title isEqualToString:@"GUI.Forms exposure regression"]) {
            window = candidate;
            break;
        }
    }
    ++(*state).initial_checks;
    // host_ready publishes services before showing the window. A fixed 100 ms
    // delay does not establish first-paint readiness on a shared native runner.
    // Wait for actual application painting; do not force it via the snapshot.
    const bool ready_for_snapshot = window != nil && window.isVisible &&
        (*(*state).root).paints > 0;
    if (!ready_for_snapshot && std::chrono::steady_clock::now() < (*state).initial_deadline) {
        queue_initial_check(state);
        return;
    }
    if (!ready_for_snapshot || !has_retained_pixels(window.contentView)) {
        (*state).failure = "initial native frame was not painted";
        std::cerr << "Initial readiness checks=" << (*state).initial_checks
                  << " application_paints=" << (*(*state).root).paints << '\n';
        (*state).close();
        return;
    }
    NSView* view = window.contentView;
    [view notifyOcclusion:YES];
    int paints_before = (*(*state).root).paints;
    if (!has_retained_pixels(view)) {
        (*state).failure = "native exposure while occlusion is pending lost the retained pixels";
    }
    if ((*(*state).root).paints != paints_before) {
        (*state).failure = "occluded exposure reran application painting instead of presenting the cache";
    }
    [view notifyOcclusion:NO];
    [window miniaturize:nil];
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 150 * NSEC_PER_MSEC), dispatch_get_main_queue(), ^{
        [window deminiaturize:nil];
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 250 * NSEC_PER_MSEC), dispatch_get_main_queue(), ^{
            if (!has_retained_pixels(view)) {
                (*state).failure = "minimize/restore lost the frame without mouse input";
            }
            [window orderOut:nil];
            dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 50 * NSEC_PER_MSEC), dispatch_get_main_queue(), ^{
                [window makeKeyAndOrderFront:nil];
                dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 100 * NSEC_PER_MSEC), dispatch_get_main_queue(), ^{
                    if (!has_retained_pixels(view)) {
                        (*state).failure = "hide/show lost the frame without mouse input";
                    }
                    (*state).completed = true;
                    (*state).close();
                });
            });
        });
    });
}
void ready(const std::shared_ptr<State>& state, std::function<void()>, std::function<void()> close,
           std::function<gui_forms::HostDialogResult(const gui_forms::HostDialogRequest&)>,
           std::function<gui_forms::HostServiceStatus(const gui_forms::HostTooltipRequest&)>,
           std::function<void()>, std::function<gui_forms::HostClipboardTextResult()>,
           std::function<gui_forms::HostServiceStatus(std::string_view)>) {
    (*state).close = std::move(close);
    (*state).initial_deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
    queue_initial_check(state);
}
}
int main() {
    std::shared_ptr<State> state = std::make_shared<State>();
    (*state).root = gui_forms::make_control<ExposureRoot>(gui_forms::StableId("exposure-root"));
    gui_forms::host::MacHostOptions options;
    options.title = "GUI.Forms exposure regression";
    options.initial_size = {320, 200};
    options.print_metrics_on_close = false;
    options.host_ready = std::bind(ready, state, std::placeholders::_1, std::placeholders::_2,
        std::placeholders::_3, std::placeholders::_4, std::placeholders::_5,
        std::placeholders::_6, std::placeholders::_7);
    int result = gui_forms::host::run_macos(std::make_unique<gui_forms::Window>((*state).root,
        options.initial_size), std::move(options));
    if (result || !(*state).completed || !(*state).failure.empty()) {
        std::cerr << (*state).failure << '\n';
        return 1;
    }
    std::cout << "Cached native exposure, minimize/restore and hide/show preserve pixels without input\n";
}
