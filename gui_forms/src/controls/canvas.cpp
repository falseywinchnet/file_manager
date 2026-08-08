#include "gui_forms/canvas.hpp"

#include "gui_forms/window.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <vector>

namespace gui_forms {
namespace {

[[nodiscard]] bool valid_origin(gui_drawing::PointF origin) noexcept {
    constexpr double limit = 1'000'000'000.0;
    return std::isfinite(origin.x) && std::isfinite(origin.y) &&
        std::abs(origin.x) <= limit && std::abs(origin.y) <= limit;
}

[[nodiscard]] std::vector<std::byte> rgba_to_bgra(
    std::span<const std::byte> source, std::size_t source_row_bytes,
    std::uint32_t width, std::uint32_t height) {
    std::vector<std::byte> result(
        static_cast<std::size_t>(width) * height * 4U);
    const std::size_t tight_row = static_cast<std::size_t>(width) * 4U;
    for (std::uint32_t row = 0; row < height; ++row) {
        const std::byte* input = source.data() +
            static_cast<std::size_t>(row) * source_row_bytes;
        std::byte* output = result.data() + static_cast<std::size_t>(row) * tight_row;
        for (std::uint32_t column = 0; column < width; ++column) {
            const std::size_t pixel = static_cast<std::size_t>(column) * 4U;
            output[pixel] = input[pixel + 2U];
            output[pixel + 1U] = input[pixel + 1U];
            output[pixel + 2U] = input[pixel];
            output[pixel + 3U] = input[pixel + 3U];
        }
    }
    return result;
}

} // namespace

RasterCanvas::RasterCanvas(StableId stable_id) : Control(std::move(stable_id)) {
    set_focusable(true);
    set_tab_stop(true);
    set_cursor(CursorKind::crosshair);
}

void RasterCanvas::set_bitmap(std::shared_ptr<gui_drawing::Bitmap> bitmap) {
    require_mutable();
    if (bitmap_ == bitmap) return;
    if (Window* owner = window(); image_.value != 0U && owner != nullptr) {
        static_cast<void>(owner->remove_image(image_));
    }
    bitmap_ = std::move(bitmap);
    image_ = {};
    presented_generation_ = 0U;
    last_resource_error_ = ImageResourceError::none;
    if (bitmap_ && window()) static_cast<void>(publish_full_bitmap());
    invalidate(Dirty::measure | Dirty::paint | Dirty::semantics);
}

void RasterCanvas::clear_bitmap() { set_bitmap({}); }

void RasterCanvas::set_zoom(double zoom) {
    set_view(zoom, view_origin_);
}

void RasterCanvas::set_view_origin(gui_drawing::PointF origin) {
    set_view(zoom_, origin);
}

void RasterCanvas::set_view(double zoom, gui_drawing::PointF origin) {
    require_mutable();
    if (!std::isfinite(zoom) || zoom < 1.0 / 64.0 || zoom > 256.0) {
        throw std::invalid_argument("canvas zoom must be within [1/64, 256]");
    }
    if (!valid_origin(origin)) {
        throw std::invalid_argument("canvas view origin must be finite and bounded");
    }
    if (zoom_ == zoom && view_origin_ == origin) return;
    zoom_ = zoom;
    view_origin_ = origin;
    invalidate(Dirty::paint | Dirty::semantics);
}

void RasterCanvas::set_sampling(ImageSampling sampling) {
    require_mutable();
    if (sampling != ImageSampling::nearest && sampling != ImageSampling::linear) {
        throw std::invalid_argument("canvas image sampling is outside the enum");
    }
    if (sampling_ == sampling) return;
    sampling_ = sampling;
    invalidate(Dirty::paint);
}

void RasterCanvas::set_transparency_grid(bool visible) {
    require_mutable();
    if (transparency_grid_ == visible) return;
    transparency_grid_ = visible;
    invalidate(Dirty::paint);
}

void RasterCanvas::set_canvas_background(Color color) {
    require_mutable();
    if (background_ == color) return;
    background_ = color;
    invalidate(Dirty::paint);
}

void RasterCanvas::set_transparency_colors(Color first, Color second) {
    require_mutable();
    if (transparency_first_ == first && transparency_second_ == second) return;
    transparency_first_ = first;
    transparency_second_ = second;
    invalidate(Dirty::paint);
}

bool RasterCanvas::publish_full_bitmap() {
    Window* owner = window();
    if (!bitmap_ || owner == nullptr) return false;
    const gui_drawing::ImageSnapshot snapshot = bitmap_->snapshot();
    ImageLoadResult loaded;
    if (snapshot.pixel_format == gui_drawing::PixelFormat::bgra32_premultiplied) {
        loaded = owner->load_bgra32_premultiplied(
            snapshot.width, snapshot.height, snapshot.row_bytes(),
            snapshot.pixels());
    } else {
        const auto converted = rgba_to_bgra(
            snapshot.pixels(), snapshot.row_bytes(), snapshot.width,
            snapshot.height);
        loaded = owner->load_bgra32_premultiplied(
            snapshot.width, snapshot.height,
            static_cast<std::uint64_t>(snapshot.width) * 4U, converted);
    }
    last_resource_error_ = loaded.error;
    if (!loaded) return false;
    image_ = loaded.image;
    presented_generation_ = snapshot.generation;
    return true;
}

bool RasterCanvas::synchronize_bitmap() {
    require_mutable();
    if (!bitmap_) return true;
    if (!window()) return false;
    if (image_.value == 0U) {
        const bool published = publish_full_bitmap();
        if (published) invalidate(Dirty::paint | Dirty::semantics);
        return published;
    }

    const gui_drawing::BitmapDamageSnapshot changes =
        bitmap_->changes_since(presented_generation_);
    if (changes.empty()) return true;
    const gui_drawing::ImageSnapshot snapshot = bitmap_->snapshot();
    for (const gui_drawing::RectI rect : changes.rectangles) {
        const std::size_t source_offset =
            static_cast<std::size_t>(rect.y) * snapshot.row_bytes() +
            static_cast<std::size_t>(rect.x) * 4U;
        const std::size_t required =
            static_cast<std::size_t>(rect.height - 1) * snapshot.row_bytes() +
            static_cast<std::size_t>(rect.width) * 4U;
        std::span<const std::byte> patch(
            snapshot.pixels().data() + source_offset, required);
        std::vector<std::byte> converted;
        std::uint64_t source_row_bytes = snapshot.row_bytes();
        if (snapshot.pixel_format == gui_drawing::PixelFormat::rgba32_premultiplied) {
            converted = rgba_to_bgra(
                patch, snapshot.row_bytes(),
                static_cast<std::uint32_t>(rect.width),
                static_cast<std::uint32_t>(rect.height));
            patch = converted;
            source_row_bytes = static_cast<std::uint64_t>(rect.width) * 4U;
        }
        const ImageLoadResult updated = window()->patch_bgra32_premultiplied(
            image_, static_cast<std::uint32_t>(rect.x),
            static_cast<std::uint32_t>(rect.y),
            static_cast<std::uint32_t>(rect.width),
            static_cast<std::uint32_t>(rect.height), source_row_bytes, patch,
            *this, damage_to_client(rect));
        last_resource_error_ = updated.error;
        if (!updated) {
            if (updated.error == ImageResourceError::stale_image_id) {
                image_ = {};
                presented_generation_ = 0U;
                const bool recovered = publish_full_bitmap();
                if (recovered) invalidate(Dirty::paint | Dirty::semantics);
                return recovered;
            }
            return false;
        }
        image_ = updated.image;
    }
    presented_generation_ = snapshot.generation;
    return true;
}

Rect RasterCanvas::bitmap_to_client(gui_drawing::RectI pixels) const noexcept {
    return {(static_cast<double>(pixels.x) - view_origin_.x) * zoom_,
            (static_cast<double>(pixels.y) - view_origin_.y) * zoom_,
            static_cast<double>(pixels.width) * zoom_,
            static_cast<double>(pixels.height) * zoom_};
}

gui_drawing::PointF RasterCanvas::client_to_bitmap(Point client) const noexcept {
    return {client.x / zoom_ + view_origin_.x,
            client.y / zoom_ + view_origin_.y};
}

gui_drawing::RectF RasterCanvas::visible_bitmap_bounds() const {
    if (!bitmap_) return {};
    const Rect client = client_rectangle();
    const double left = std::clamp(view_origin_.x, 0.0,
                                   static_cast<double>(bitmap_->width()));
    const double top = std::clamp(view_origin_.y, 0.0,
                                  static_cast<double>(bitmap_->height()));
    const double right = std::clamp(view_origin_.x + client.width / zoom_,
                                    0.0, static_cast<double>(bitmap_->width()));
    const double bottom = std::clamp(view_origin_.y + client.height / zoom_,
                                     0.0, static_cast<double>(bitmap_->height()));
    return {left, top, std::max(0.0, right - left),
            std::max(0.0, bottom - top)};
}

Rect RasterCanvas::damage_to_client(gui_drawing::RectI pixels) const noexcept {
    if (sampling_ == ImageSampling::linear) {
        --pixels.x;
        --pixels.y;
        pixels.width += 2;
        pixels.height += 2;
    }
    return Rect::intersection(bitmap_to_client(pixels), client_rectangle());
}

void RasterCanvas::paint_transparency_grid(Painter& painter, Rect bounds) const {
    painter.fill_rect(bounds, transparency_first_);
    const double area = std::max(0.0, bounds.width * bounds.height);
    const double tile = std::max(8.0, std::ceil(std::sqrt(area / 2048.0)));
    const std::int64_t columns = static_cast<std::int64_t>(
        std::ceil(bounds.width / tile));
    const std::int64_t rows = static_cast<std::int64_t>(
        std::ceil(bounds.height / tile));
    for (std::int64_t row = 0; row < rows; ++row) {
        for (std::int64_t column = row & 1; column < columns; column += 2) {
            painter.fill_rect(
                {bounds.x + static_cast<double>(column) * tile,
                 bounds.y + static_cast<double>(row) * tile,
                 std::min(tile, bounds.right() -
                                   (bounds.x + static_cast<double>(column) * tile)),
                 std::min(tile, bounds.bottom() -
                                   (bounds.y + static_cast<double>(row) * tile))},
                transparency_second_);
        }
    }
}

void RasterCanvas::on_paint(Painter& painter, Rect) {
    const Rect bounds = client_rectangle();
    painter.fill_rect(bounds, background_);
    if (!bitmap_ || image_.value == 0U) return;
    const gui_drawing::RectF source = visible_bitmap_bounds();
    if (source.empty()) return;
    const Rect destination{
        (source.x - view_origin_.x) * zoom_,
        (source.y - view_origin_.y) * zoom_,
        source.width * zoom_, source.height * zoom_};
    if (transparency_grid_) paint_transparency_grid(painter, destination);
    painter.draw_image_region_sampled(
        image_, {source.x, source.y, source.width, source.height},
        destination, sampling_, 1.0);
}

SemanticDescriptor RasterCanvas::semantic_descriptor() const {
    SemanticDescriptor descriptor;
    descriptor.role = SemanticRole::image;
    descriptor.name = accessible_name();
    descriptor.description = accessible_description();
    if (bitmap_) {
        descriptor.value = std::to_string(bitmap_->width()) + " x " +
            std::to_string(bitmap_->height()) + " at " +
            std::to_string(zoom_ * 100.0) + "%";
    }
    descriptor.exposed = bitmap_ != nullptr || !descriptor.name.empty() ||
                         !descriptor.description.empty();
    return descriptor;
}

void RasterCanvas::on_attached_to_window() {
    if (bitmap_ && !publish_full_bitmap()) {
        throw std::runtime_error(
            "RasterCanvas could not publish its bitmap resource");
    }
}

void RasterCanvas::on_detaching_from_window(Window& former_window) noexcept {
    if (image_.value != 0U) {
        try {
            static_cast<void>(former_window.remove_image(image_));
        } catch (...) {
        }
    }
    image_ = {};
    presented_generation_ = 0U;
}

void RasterCanvas::on_detached_from_window() noexcept {
    image_ = {};
    presented_generation_ = 0U;
}

void RasterCanvas::on_dispose() noexcept {
    bitmap_.reset();
    image_ = {};
    presented_generation_ = 0U;
    Control::on_dispose();
}

} // namespace gui_forms
