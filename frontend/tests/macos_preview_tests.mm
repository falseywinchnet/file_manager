#import <AppKit/AppKit.h>

#include "application.hpp"
#include "gui_forms/gui_forms.hpp"
#include "gui_forms/platform/macos_host.hpp"

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <unistd.h>

namespace file_manager {
class ApplicationInteractionProbe final {
public:
    static void show_details(Application& application) {
        application.set_view_mode(gui_forms::ObjectViewMode::details);
    }
};
}

namespace {

// Owned generated data only. A failed directory acquisition never authorizes
// cleanup of a pre-existing path. The fixture outlives the application worker.
class PreviewFixture final {
public:
    PreviewFixture() {
        const std::chrono::steady_clock::duration stamp =
            std::chrono::steady_clock::now().time_since_epoch();
        const std::filesystem::path temporary_parent =
            std::filesystem::canonical(std::filesystem::temp_directory_path());
        root_ = temporary_parent /
            ("file-manager-native-preview-" + std::to_string(getpid()) + "-" +
             std::to_string(stamp.count()));
        if (!std::filesystem::create_directory(root_)) {
            throw std::runtime_error("cannot acquire native preview fixture directory");
        }
        acquired_ = true;
    }
    ~PreviewFixture() {
        if (acquired_) {
            std::error_code ignored{};
            std::filesystem::remove_all(root_, ignored);
        }
    }
    PreviewFixture(const PreviewFixture&) = delete;
    PreviewFixture& operator=(const PreviewFixture&) = delete;
    const std::filesystem::path& root() const noexcept { return root_; }

    void populate() const {
        // Generated 2x2 opaque PNG, RGB=(12,226,198). No external image decoder
        // or proprietary specimen is needed to know the expected painted color.
        constexpr std::array<unsigned char, 74> png{
            0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00,
            0x00, 0x0d, 0x49, 0x48, 0x44, 0x52, 0x00, 0x00, 0x00, 0x02,
            0x00, 0x00, 0x00, 0x02, 0x08, 0x06, 0x00, 0x00, 0x00, 0x72,
            0xb6, 0x0d, 0x24, 0x00, 0x00, 0x00, 0x11, 0x49, 0x44, 0x41,
            0x54, 0x78, 0x9c, 0x63, 0xe0, 0x79, 0x74, 0xec, 0x3f, 0x08,
            0x33, 0xc0, 0x18, 0x00, 0x5b, 0xc4, 0x0a, 0xcd, 0x1a, 0x6d,
            0x3d, 0xbc, 0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4e, 0x44,
            0xae, 0x42, 0x60, 0x82};
        std::ofstream image(root_ / "native-preview.png", std::ios::binary);
        image.write(reinterpret_cast<const char*>(png.data()),
                    static_cast<std::streamsize>(png.size()));
        image.close();
        std::ofstream text(root_ / "native-preview.txt", std::ios::binary);
        text << "NATIVE PREVIEW CHECK\nReadable text in the actual Mac window.\n";
        text << "\n\n\n\n\n\n\n\n";
        text << std::string(file_manager::maximum_text_preview_bytes, 'x');
        text.close();
        std::ofstream unsupported(root_ / "native-preview.bin", std::ios::binary);
        unsupported << "Explicit unsupported format explanation";
        unsupported.close();
        if (!image || !text || !unsupported) {
            throw std::runtime_error("cannot write native preview fixtures");
        }
    }
private:
    std::filesystem::path root_{};
    bool acquired_{};
};

enum class PreviewStage { listing, text, image, unsupported, details, finished };

struct PreviewState final {
    std::shared_ptr<file_manager::Application> application{};
    // Borrowed from run_macos until its loop returns; every continuation checks
    // closed before access. The main owner stops the worker before fixture loss.
    gui_forms::Window* model{};
    std::function<void()> close{};
    std::chrono::steady_clock::time_point deadline{};
    PreviewStage stage{PreviewStage::listing};
    std::string failure{};
    bool closed{};

    ~PreviewState() { if (application) (*application).stop(); }
    void drain() { (*application).drain_ui(); }
    void did_close() { closed = true; (*application).stop(); }
};

NSWindow* preview_window() {
    NSWindow* result = nil;
    for (NSWindow* candidate in NSApp.windows) {
        if ([candidate.title isEqualToString:@"File Manager native preview test"]) {
            result = candidate;
            break;
        }
    }
    return result;
}

void save_failure_snapshot(gui_forms::Window& model) {
    NSWindow* const native = preview_window();
    NSView* const view = native.contentView;
    if (view == nil) return;
    NSBitmapImageRep* const bitmap = [view bitmapImageRepForCachingDisplayInRect:view.bounds];
    if (bitmap == nil) return;
    [view cacheDisplayInRect:view.bounds toBitmapImageRep:bitmap];
    NSData* const encoded = [bitmap representationUsingType:NSBitmapImageFileTypePNG properties:@{}];
    const BOOL saved = [encoded writeToFile:@"native-preview-failure.png" atomically:YES];
    std::cerr << "Native preview failure snapshot saved=" << (saved == YES) << '\n';
    const gui_forms::VisualInspectionSnapshot inspection = model.visual_inspection_snapshot();
    std::ofstream diagnostics("native-preview-failure.json", std::ios::binary);
    diagnostics << inspection.to_json();
    diagnostics.close();
    std::cerr << "Retained preview diagnostics saved=" << diagnostics.good() << '\n';
}

bool select_file(PreviewState& state, const std::string_view name) {
    const std::shared_ptr<gui_forms::ObjectView> objects =
        std::dynamic_pointer_cast<gui_forms::ObjectView>(
            (*state.model).find("fm.objects.current-folder"));
    if (!objects) return false;
    for (const gui_forms::ObjectViewItem& item : (*objects).items()) {
        if (item.name == name) {
            const std::string identity = item.stable_id;
            const bool selected = (*state.model).perform_semantic_action(
                identity, gui_forms::SemanticAction::select);
            return selected;
        }
    }
    return false;
}

// AppKit snapshot evidence includes clipping, the native Skia decoder and
// backing-scale presentation. It can force display; it is not paint-latency or
// ordinary pointer-delivery evidence. Sample only the selected preview bounds.
enum class PixelMatch { light_text, image, dark_text, caption_text };

std::size_t matching_pixels(NSView* const view, const gui_forms::Rect bounds,
                            const PixelMatch target) {
    if (view == nil || bounds.empty() || view.bounds.size.width <= 0.0 ||
        view.bounds.size.height <= 0.0) return 0U;
    NSBitmapImageRep* const bitmap = [view bitmapImageRepForCachingDisplayInRect:view.bounds];
    if (bitmap == nil) return 0U;
    [view cacheDisplayInRect:view.bounds toBitmapImageRep:bitmap];
    const double scale_x = static_cast<double>(bitmap.pixelsWide) / view.bounds.size.width;
    const double scale_y = static_cast<double>(bitmap.pixelsHigh) / view.bounds.size.height;
    const double top = view.isFlipped ? bounds.y
        : view.bounds.size.height - bounds.y - bounds.height;
    const NSInteger left = static_cast<NSInteger>(std::clamp(
        std::ceil(bounds.x * scale_x), 0.0, static_cast<double>(bitmap.pixelsWide)));
    const NSInteger right = static_cast<NSInteger>(std::clamp(
        std::floor((bounds.x + bounds.width) * scale_x), 0.0, static_cast<double>(bitmap.pixelsWide)));
    const NSInteger first = static_cast<NSInteger>(std::clamp(
        std::ceil(top * scale_y), 0.0, static_cast<double>(bitmap.pixelsHigh)));
    const NSInteger last = static_cast<NSInteger>(std::clamp(
        std::floor((top + bounds.height) * scale_y), 0.0, static_cast<double>(bitmap.pixelsHigh)));
    std::size_t matches{};
    for (NSInteger y = first; y < last; ++y) {
        for (NSInteger x = left; x < right; ++x) {
            NSColor* const color = [[bitmap colorAtX:x y:y] colorUsingColorSpace:NSColorSpace.sRGBColorSpace];
            if (color == nil || color.alphaComponent < 0.95) continue;
            bool matched{};
            if (target == PixelMatch::image) {
                matched = color.redComponent < 0.15 && color.greenComponent > 0.75 &&
                    color.blueComponent > 0.65 && color.blueComponent < 0.9;
            } else if (target == PixelMatch::dark_text) {
                matched = color.redComponent < 0.2 && color.greenComponent < 0.25 && color.blueComponent < 0.3;
            } else if (target == PixelMatch::caption_text) {
                matched = color.redComponent < 0.6 && color.greenComponent < 0.65 && color.blueComponent < 0.7;
            } else {
                matched = color.redComponent > 0.65 && color.greenComponent > 0.7 && color.blueComponent > 0.75;
            }
            if (matched) ++matches;
        }
    }
    return matches;
}

void run_step(void* context);

void queue_step(const std::shared_ptr<PreviewState>& state) {
    std::unique_ptr<std::shared_ptr<PreviewState>> retained =
        std::make_unique<std::shared_ptr<PreviewState>>(state);
    const dispatch_time_t next = dispatch_time(DISPATCH_TIME_NOW, 50 * NSEC_PER_MSEC);
    void* const context = retained.release();
    dispatch_after_f(next, dispatch_get_main_queue(), context, run_step);
}

void exercise(PreviewState& state) {
    NSWindow* const native = preview_window();
    if (native == nil || !native.isVisible) return;
    if (state.stage == PreviewStage::details) {
        const std::shared_ptr<gui_forms::ObjectView> objects =
            std::dynamic_pointer_cast<gui_forms::ObjectView>((*state.model).find("fm.objects.current-folder"));
        if (!objects || !(*objects).effectively_visible()) return;
        const gui_forms::Rect bounds = (*objects).absolute_bounds();
        const double scale = (*objects).effective_text_scale();
        const gui_forms::Rect name_header{bounds.x + 10.0, bounds.y + 3.0,
            50.0 * scale, (*objects).details_row_height() * scale - 2.0};
        const std::size_t pixels = matching_pixels(native.contentView, name_header, PixelMatch::dark_text);
        if (pixels < 15U) return;
        const std::string selected((*objects).selected_id());
        (*state.model).request_focus(objects);
        const bool focused = (*state.model).dispatch_key({gui_forms::KeyAction::down, gui_forms::PhysicalKey::f6});
        const bool sorted = (*state.model).dispatch_key({gui_forms::KeyAction::down, gui_forms::PhysicalKey::enter});
        double columns_width = 0.0;
        for (const gui_forms::ObjectDetailsColumn& column : (*objects).details_columns()) {
            columns_width += column.width;
        }
        if (columns_width * scale > bounds.width - 7.9 || (*objects).top_row() != 0U ||
            (*objects).items().size() != 3U) {
            throw std::runtime_error("native Details must fit every default header and retain all three fixture rows after sorting");
        }
        if (!focused || !sorted || (*objects).details_sort().column.value != "name" ||
            (*objects).details_sort().direction != gui_forms::ObjectSortDirection::descending ||
            (*objects).selected_id() != selected) {
            throw std::runtime_error("native Details header route failed to sort while preserving selection");
        }
        NSView* const view = native.contentView;
        NSBitmapImageRep* const bitmap = [view bitmapImageRepForCachingDisplayInRect:view.bounds];
        if (bitmap == nil) throw std::runtime_error("native Details snapshot unavailable");
        [view cacheDisplayInRect:view.bounds toBitmapImageRep:bitmap];
        NSData* const encoded = [bitmap representationUsingType:NSBitmapImageFileTypePNG properties:@{}];
        const BOOL saved = [encoded writeToFile:@"native-details.png" atomically:YES];
        if (saved != YES) throw std::runtime_error("cannot save native Details evidence");
        std::cout << "macOS Details: name_header_pixels=" << pixels
                  << " columns=" << (*objects).details_columns().size()
                  << " synthetic_header_sort=passed selection=preserved\n";
        state.stage = PreviewStage::finished;
        state.close();
        return;
    }
    if (state.stage == PreviewStage::listing) {
        if (select_file(state, "native-preview.txt")) {
            // AppKit constrains the window to the runner's desktop (1024-wide
            // in the first recorded run). Exercise the ordinary reveal action
            // instead of assuming the requested wide-window inspector survives.
            const bool revealed = (*state.model).perform_semantic_action(
                "file-manager-app.shell.commands.arrange-group.actions.settings",
                gui_forms::SemanticAction::press);
            if (!revealed) throw std::runtime_error("Properties command could not reveal preview pane");
            const gui_forms::Control::Ptr surface = (*state.model).find(
                "file-manager-app.shell.workspace.selection.inspector.facts.preview.surface");
            if (!surface) throw std::runtime_error("preview surface is absent");
            if (!(*surface).visible()) {
                const bool expanded = (*state.model).perform_semantic_action(
                    "file-manager-app.shell.workspace.selection.inspector.facts.preview.toggle",
                    gui_forms::SemanticAction::press);
                if (!expanded) throw std::runtime_error("Show preview command could not expand preview");
            }
            state.stage = PreviewStage::text;
        }
        return;
    }
    const std::shared_ptr<gui_forms::Label> text =
        std::dynamic_pointer_cast<gui_forms::Label>((*state.model).find("fm.inspector.preview.text"));
    const std::shared_ptr<gui_forms::PictureBox> picture =
        std::dynamic_pointer_cast<gui_forms::PictureBox>((*state.model).find("fm.inspector.preview.image"));
    if (!text || !picture) throw std::runtime_error("preview controls disappeared");
    if (state.stage == PreviewStage::image) {
        if (!(*picture).effectively_visible() || !(*picture).has_valid_image()) return;
        const gui_forms::Rect bounds = (*picture).rectangle_to_window((*picture).image_bounds());
        const std::size_t pixels = matching_pixels(native.contentView, bounds, PixelMatch::image);
        if (pixels < 100U) return;
        std::cout << "macOS PNG preview: matching_pixels=" << pixels << '\n';
        if (!select_file(state, "native-preview.bin")) throw std::runtime_error("cannot select unsupported fixture");
        state.stage = PreviewStage::unsupported;
        return;
    }
    const bool unsupported = state.stage == PreviewStage::unsupported;
    const bool content_ready = unsupported
        ? (*text).text().find("Preview is not available") != std::string::npos
        : (*text).text().starts_with("NATIVE PREVIEW CHECK");
    if (!content_ready || !(*text).effectively_visible() || (*picture).visible()) return;
    const std::size_t pixels = matching_pixels(native.contentView, (*text).absolute_bounds(), PixelMatch::light_text);
    if (pixels < 25U) return;
    if (!unsupported) {
        const std::shared_ptr<gui_forms::Label> coverage = std::dynamic_pointer_cast<gui_forms::Label>(
            (*state.model).find("file-manager-app.shell.workspace.selection.inspector.facts.preview.kind"));
        if (!coverage || !(*coverage).effectively_visible() ||
            (*coverage).text() != "Text excerpt · 64 KiB limit" ||
            (*coverage).absolute_bounds().y < (*text).absolute_bounds().bottom()) {
            throw std::runtime_error("native text truncation notice must remain outside the clipped body");
        }
        const std::size_t caption_pixels = matching_pixels(
            native.contentView, (*coverage).absolute_bounds(), PixelMatch::caption_text);
        if (caption_pixels < 25U) return;
        NSView* const view = native.contentView;
        NSBitmapImageRep* const bitmap = [view bitmapImageRepForCachingDisplayInRect:view.bounds];
        if (bitmap == nil) throw std::runtime_error("native text snapshot unavailable");
        [view cacheDisplayInRect:view.bounds toBitmapImageRep:bitmap];
        NSData* const encoded = [bitmap representationUsingType:NSBitmapImageFileTypePNG properties:@{}];
        const BOOL saved = [encoded writeToFile:@"native-text-preview.png" atomically:YES];
        if (saved != YES) throw std::runtime_error("cannot save native text-preview evidence");
        std::cout << "macOS TXT coverage: caption_pixels=" << caption_pixels << " outside_body=passed\n";
    }
    std::cout << "macOS " << (unsupported ? "unsupported explanation" : "TXT preview")
              << ": readable_pixels=" << pixels << '\n';
    if (unsupported) {
        file_manager::ApplicationInteractionProbe::show_details(*state.application);
        state.stage = PreviewStage::details;
    } else {
        if (!select_file(state, "native-preview.png")) throw std::runtime_error("cannot select PNG fixture");
        state.stage = PreviewStage::image;
    }
}

void run_step(void* const context) {
    const std::unique_ptr<std::shared_ptr<PreviewState>> retained(
        static_cast<std::shared_ptr<PreviewState>*>(context));
    const std::shared_ptr<PreviewState>& owner = *retained;
    PreviewState& state = *owner;
    if (state.closed) return;
    @autoreleasepool {
        try {
            exercise(state);
            if (state.stage == PreviewStage::finished || state.closed) return;
            if (std::chrono::steady_clock::now() >= state.deadline) {
                throw std::runtime_error("native preview timed out at stage " +
                                         std::to_string(static_cast<int>(state.stage)));
            }
            queue_step(owner);
        } catch (const std::exception& error) {
            state.failure = error.what();
            save_failure_snapshot(*state.model);
            state.close();
        }
    }
}

void host_ready(const std::shared_ptr<PreviewState>& state,
    std::function<void()> wake, std::function<void()> close,
    std::function<gui_forms::HostDialogResult(const gui_forms::HostDialogRequest&)>,
    std::function<gui_forms::HostServiceStatus(const gui_forms::HostTooltipRequest&)>,
    std::function<void()>, std::function<gui_forms::HostClipboardTextResult()>,
    std::function<gui_forms::HostServiceStatus(std::string_view)>) {
    (*state).close = close;
    (*state).deadline = std::chrono::steady_clock::now() + std::chrono::seconds(20);
    (*(*state).application).bind_host(std::move(wake), std::move(close));
    queue_step(state);
}

} // namespace

int main() {
    @autoreleasepool {
        try {
            PreviewFixture fixture{};
            fixture.populate();
            const std::shared_ptr<PreviewState> state = std::make_shared<PreviewState>();
            (*state).application = std::make_shared<file_manager::Application>(
                fixture.root(), std::nullopt, false, std::string{});
            std::unique_ptr<gui_forms::Window> model = (*(*state).application).make_window();
            (*state).model = model.get();
            gui_forms::host::MacHostOptions options{};
            options.title = "File Manager native preview test";
            options.initial_size = {1340.0, 850.0};
            options.minimum_size = {150.0, 150.0};
            options.titlebar_presentation = gui_forms::host::MacTitlebarPresentation::transparent_full_size_content;
            options.print_metrics_on_close = true;
            options.host_ready = std::bind_front(host_ready, state);
            options.dispatch_pending = std::bind_front(&PreviewState::drain, state);
            options.closed = std::bind_front(&PreviewState::did_close, state);
            const int result = gui_forms::host::run_macos(std::move(model), std::move(options));
            (*state).model = nullptr;
            (*state).closed = true;
            (*(*state).application).stop();
            if (result != 0 || (*state).stage != PreviewStage::finished || !(*state).failure.empty()) {
                std::cerr << "File Manager native preview failed: " << (*state).failure << '\n';
                return 1;
            }
            return 0;
        } catch (const std::exception& error) {
            std::cerr << error.what() << '\n';
            return 1;
        }
    }
}
