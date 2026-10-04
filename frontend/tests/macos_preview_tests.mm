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
        parent_ = temporary_parent;
        root_ = temporary_parent /
            ("file-manager-native-preview-" + std::to_string(getpid()) + "-" +
             std::to_string(stamp.count()));
        if (!std::filesystem::create_directory(root_)) {
            throw std::runtime_error("cannot acquire native preview fixture directory");
        }
        identity_ = file_manager::observe_identity(root_);
        if (!identity_.available()) {
            throw std::runtime_error("native fixture identity unavailable; directory retained");
        }
        acquired_ = true;
    }
    ~PreviewFixture() {
        try {
            if (acquired_ && root_.is_absolute() && root_.parent_path() == parent_ &&
                file_manager::observe_identity(root_) == identity_) {
                std::error_code ignored{};
                const std::filesystem::path current = std::filesystem::canonical(root_, ignored);
                if (!ignored && current == root_) std::filesystem::remove_all(root_, ignored);
            }
        } catch (const std::exception&) {
            // Cleanup cannot replace a test failure with termination. Retain the
            // generated directory if even validation cannot acquire its storage.
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
    std::filesystem::path parent_{};
    file_manager::ObjectIdentity identity_{};
    bool acquired_{};
};

enum class PreviewStage {
    listing, text, image, unsupported, details, selection,
    folder_created, folder_undone, file_renamed, rename_undone, finished,
};

struct PreviewState final {
    std::shared_ptr<file_manager::Application> application{};
    // Borrowed from run_macos until its loop returns; every continuation checks
    // closed before access. The main owner stops the worker before fixture loss.
    gui_forms::Window* model{};
    std::function<void()> close{};
    std::chrono::steady_clock::time_point deadline{};
    PreviewStage stage{PreviewStage::listing};
    std::string failure{};
    std::filesystem::path fixture_root{};
    file_manager::ObjectIdentity renamed_identity{};
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

void require_filled_preview(gui_forms::Window& model, const gui_forms::Control& content) {
    const gui_forms::Control::Ptr surface = model.find(
        "file-manager-app.shell.workspace.selection.inspector.facts.preview.surface");
    if (!surface) throw std::runtime_error("native preview surface disappeared");
    const gui_forms::Rect body = content.absolute_bounds();
    const gui_forms::Rect available = (*surface).absolute_bounds();
    if (!available.contains(body) || body.width < available.width - 20.0 ||
        body.height < available.height - 20.0 || body.x < available.x + 6.0 ||
        body.y < available.y + 6.0) {
        throw std::runtime_error("native preview body does not fill its padded surface");
    }
    std::cout << "macOS preview extent: body=" << body.width << 'x' << body.height
              << " surface=" << available.width << 'x' << available.height << '\n';
}

// Borrow the live AppKit view and filename only during this synchronous capture.
void save_preview_snapshot(NSView* const view, NSString* const filename) {
    NSBitmapImageRep* const bitmap = [view bitmapImageRepForCachingDisplayInRect:view.bounds];
    if (bitmap == nil) throw std::runtime_error("native preview snapshot unavailable");
    [view cacheDisplayInRect:view.bounds toBitmapImageRep:bitmap];
    NSData* const encoded = [bitmap representationUsingType:NSBitmapImageFileTypePNG properties:@{}];
    const BOOL saved = [encoded writeToFile:filename atomically:YES];
    if (saved != YES) throw std::runtime_error("cannot save native preview evidence");
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

void press_menu_command(PreviewState& state, const std::string_view menu,
                        const std::string_view row) {
    const bool opened = (*state.model).perform_semantic_action(menu, gui_forms::SemanticAction::expand);
    if (!opened) throw std::runtime_error("native local-action menu did not open");
    const bool pressed = (*state.model).perform_semantic_action(row, gui_forms::SemanticAction::press);
    if (!pressed) throw std::runtime_error("native local-action command was unavailable");
}

bool contains_file(const gui_forms::ObjectView& objects, const std::string_view name) {
    for (const gui_forms::ObjectViewItem& item : objects.items()) {
        if (item.name == name) return true;
    }
    return false;
}

void require_inspector_name(gui_forms::Window& model, const std::string_view expected) {
    const std::shared_ptr<gui_forms::PropertyList> properties =
        std::dynamic_pointer_cast<gui_forms::PropertyList>(model.find("fm.selection.properties"));
    const std::shared_ptr<gui_forms::Label> preview_name = std::dynamic_pointer_cast<gui_forms::Label>(
        model.find("file-manager-app.shell.workspace.selection.inspector.facts.preview.name"));
    if (!properties || !preview_name) throw std::runtime_error("native inspector controls disappeared");
    if ((*properties).value("fm.property.name") != expected || (*preview_name).text() != expected) {
        throw std::runtime_error("native inspector retained a stale name after Rename or Undo");
    }
}

void require_protected_commands_unavailable(gui_forms::Window& model) {
    const std::shared_ptr<gui_forms::MenuStrip> menu =
        std::dynamic_pointer_cast<gui_forms::MenuStrip>(model.find("fm.application.menu"));
    if (!menu) throw std::runtime_error("native application menu disappeared");
    std::size_t checked = 0U;
    for (const gui_forms::MenuStripItemSpec& category : (*menu).items()) {
        if (category.stable_id != "fm.menu.edit") continue;
        for (const gui_forms::MenuItemSpec& item : category.items) {
            if (item.stable_id != "edit.copy" && item.stable_id != "edit.move" &&
                item.stable_id != "edit.paste" && item.stable_id != "edit.delete") continue;
            if (!item.command || (*item.command).state().enabled) {
                throw std::runtime_error("ordinary native actions enabled an unfinished protected command");
            }
            ++checked;
        }
    }
    if (checked != 4U) throw std::runtime_error("native transfer/deletion policy coverage is incomplete");
}

// Synthetic menu/key delivery through the actual native window model. Filesystem
// observations and visible rows must both converge; no private mutation method is
// invoked. This does not prove physical input or the packaged main() entry point.
void exercise_local_actions(PreviewState& state, NSWindow* const native) {
    const std::shared_ptr<gui_forms::ObjectView> objects =
        std::dynamic_pointer_cast<gui_forms::ObjectView>((*state.model).find("fm.objects.current-folder"));
    const std::shared_ptr<gui_forms::TextBox> rename =
        std::dynamic_pointer_cast<gui_forms::TextBox>((*state.model).find("fm.operations.rename"));
    if (!objects || !rename) throw std::runtime_error("native action controls disappeared");
    const std::filesystem::path folder = state.fixture_root / "New folder";
    const std::filesystem::path original = state.fixture_root / "native-preview.txt";
    const std::filesystem::path renamed = state.fixture_root / "renamed-preview.txt";
    if (state.stage == PreviewStage::folder_created) {
        if (!contains_file(*objects, "New folder") || !(*rename).effectively_visible()) return;
        const file_manager::ObjectIdentity created = file_manager::observe_identity(folder);
        if (!created.available() || created.type != std::filesystem::file_type::directory ||
            (*rename).text() != "New folder") {
            throw std::runtime_error("native New Folder did not create and begin naming the directory");
        }
        const bool cancelled = (*state.model).dispatch_key(
            {gui_forms::KeyAction::down, gui_forms::PhysicalKey::escape});
        if (!cancelled || (*rename).visible()) throw std::runtime_error("native initial naming did not cancel");
        press_menu_command(state, "fm.menu.edit", "fm.application.menu.menu.popup.row.edit.undo");
        state.stage = PreviewStage::folder_undone;
        return;
    }
    if (state.stage == PreviewStage::folder_undone) {
        if (contains_file(*objects, "New folder")) return;
        if (file_manager::observe_identity(folder).available()) {
            throw std::runtime_error("native New Folder Undo left the generated folder behind");
        }
        if (!select_file(state, "native-preview.txt")) return;
        state.renamed_identity = file_manager::observe_identity(original);
        if (!state.renamed_identity.available()) throw std::runtime_error("native rename source unavailable");
        const bool focused = (*state.model).request_focus(objects);
        const bool began = (*state.model).dispatch_key(
            {gui_forms::KeyAction::down, gui_forms::PhysicalKey::f2});
        if (!focused || !began || !(*rename).effectively_visible() ||
            (*rename).selected_text() != "native-preview") {
            std::cerr << "Native Rename state: object_focus=" << focused
                      << " key_handled=" << began
                      << " editor_visible=" << (*rename).effectively_visible()
                      << " selected_text=" << (*rename).selected_text() << '\n';
            throw std::runtime_error("native F2 did not select the filename basename");
        }
        const bool typed = (*state.model).dispatch_text({"renamed-preview"});
        if (!typed || (*rename).text() != "renamed-preview.txt") {
            throw std::runtime_error("native basename edit did not preserve the extension");
        }
        const bool committed = (*state.model).dispatch_key(
            {gui_forms::KeyAction::down, gui_forms::PhysicalKey::enter});
        if (!committed) throw std::runtime_error("native Rename Enter was not handled");
        state.stage = PreviewStage::file_renamed;
        return;
    }
    if (state.stage == PreviewStage::file_renamed) {
        if (!contains_file(*objects, "renamed-preview.txt") || (*rename).visible()) return;
        require_inspector_name(*state.model, "renamed-preview.txt");
        const std::shared_ptr<gui_forms::Label> text = std::dynamic_pointer_cast<gui_forms::Label>(
            (*state.model).find("fm.inspector.preview.text"));
        if (!text || !(*text).effectively_visible() || !(*text).text().starts_with("NATIVE PREVIEW CHECK")) return;
        const file_manager::ObjectIdentity observed = file_manager::observe_identity(renamed);
        if (observed != state.renamed_identity || file_manager::observe_identity(original).available()) {
            throw std::runtime_error("native Rename did not preserve the selected filesystem object");
        }
        require_protected_commands_unavailable(*state.model);
        NSView* const view = native.contentView;
        NSBitmapImageRep* const bitmap = [view bitmapImageRepForCachingDisplayInRect:view.bounds];
        if (bitmap == nil) throw std::runtime_error("native local-action snapshot unavailable");
        [view cacheDisplayInRect:view.bounds toBitmapImageRep:bitmap];
        NSData* const encoded = [bitmap representationUsingType:NSBitmapImageFileTypePNG properties:@{}];
        const BOOL saved = [encoded writeToFile:@"native-local-actions.png" atomically:YES];
        if (saved != YES) throw std::runtime_error("cannot save native local-action evidence");
        press_menu_command(state, "fm.menu.edit", "fm.application.menu.menu.popup.row.edit.undo");
        state.stage = PreviewStage::rename_undone;
        return;
    }
    if (!contains_file(*objects, "native-preview.txt") || contains_file(*objects, "renamed-preview.txt")) return;
    require_inspector_name(*state.model, "native-preview.txt");
    if (file_manager::observe_identity(original) != state.renamed_identity ||
        file_manager::observe_identity(renamed).available()) {
        throw std::runtime_error("native Rename Undo did not restore the original object and name");
    }
    std::cout << "macOS ordinary local actions: menu_create=passed create_undo=passed "
                 "F2_basename_rename=passed rename_undo=passed inspector_names=passed protected_commands=unavailable "
                 "quarantine=absent delivery=synthetic_native_window\n";
    state.stage = PreviewStage::finished;
    state.close();
}

void exercise(PreviewState& state) {
    NSWindow* const native = preview_window();
    if (native == nil || !native.isVisible) return;
    if (state.stage == PreviewStage::folder_created || state.stage == PreviewStage::folder_undone ||
        state.stage == PreviewStage::file_renamed || state.stage == PreviewStage::rename_undone) {
        exercise_local_actions(state, native);
        return;
    }
    if (state.stage == PreviewStage::selection) {
        const std::shared_ptr<gui_forms::PropertyList> properties =
            std::dynamic_pointer_cast<gui_forms::PropertyList>((*state.model).find("fm.selection.properties"));
        const std::shared_ptr<gui_forms::Label> caption = std::dynamic_pointer_cast<gui_forms::Label>(
            (*state.model).find("file-manager-app.shell.workspace.selection.inspector.facts.preview.kind"));
        if (!properties || !caption) throw std::runtime_error("native selection inspector disappeared");
        const std::uintmax_t bytes = std::filesystem::file_size(state.fixture_root / "native-preview.txt") +
            std::filesystem::file_size(state.fixture_root / "native-preview.png") +
            std::filesystem::file_size(state.fixture_root / "native-preview.bin");
        const std::string expected = file_manager::format_bytes(bytes) + " in files";
        if ((*properties).value("fm.property.name") != "3 selected objects" ||
            (*properties).value("fm.property.kind") != "Multiple kinds" ||
            (*properties).value("fm.property.size") != expected || (*caption).text() != "3 files") {
            throw std::runtime_error("native selection facts differ from the generated files");
        }
        (*state.model).perform_layout();
        save_preview_snapshot(native.contentView, @"native-selection-facts.png");
        std::cout << "macOS selection facts: three_files=passed logical_total=passed mixed_type=passed\n";
        const bool expanded = (*state.model).perform_semantic_action(
            "file-manager-app.shell.workspace.selection.inspector.facts.preview.toggle",
            gui_forms::SemanticAction::press);
        if (!expanded) throw std::runtime_error("native preview could not restore after fact inspection");
        press_menu_command(state, "fm.menu.file", "fm.application.menu.menu.popup.row.file.new-folder");
        state.stage = PreviewStage::folder_created;
        return;
    }
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
        require_protected_commands_unavailable(*state.model);
        std::vector<std::string> selected_ids{};
        selected_ids.reserve((*objects).items().size());
        for (const gui_forms::ObjectViewItem& item : (*objects).items()) {
            selected_ids.push_back(item.stable_id);
        }
        (*objects).set_selected_ids(std::move(selected_ids));
        const bool collapsed = (*state.model).perform_semantic_action(
            "file-manager-app.shell.workspace.selection.inspector.facts.preview.toggle",
            gui_forms::SemanticAction::press);
        if (!collapsed) throw std::runtime_error("native selection preview could not collapse for fact inspection");
        state.stage = PreviewStage::selection;
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
        require_filled_preview(*state.model, *picture);
        save_preview_snapshot(native.contentView, @"native-image-preview.png");
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
    require_filled_preview(*state.model, *text);
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
        save_preview_snapshot(native.contentView, @"native-text-preview.png");
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
                fixture.root(), std::nullopt, file_manager::OperationPolicy::ordinary_local, std::string{});
            (*state).fixture_root = fixture.root();
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
