#include "gui_forms/types/cursor_image/cursor_image.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <utility>

namespace gui_forms {
CursorImages::CursorImages(std::vector<CursorImage> images, double x, double y)
    : images_(std::move(images)), hotspot_x_(x), hotspot_y_(y) {}
CursorImagesPtr CursorImages::create(std::vector<CursorImage> images, double x, double y) {
    if (images.empty() || images.size() > 8 || !std::isfinite(x) ||
        !std::isfinite(y) || x < 0 || y < 0 || x >= 1 || y >= 1) {
        throw std::invalid_argument("Invalid cursor representations or hotspot");
    }
    double prior = 0;
    double logical_width = 0;
    double logical_height = 0;
    for (const CursorImage& image : images) {
        if (image.width < 1 || image.height < 1 || image.width > 256 ||
            image.height > 256 || !std::isfinite(image.scale) ||
            image.scale < 0.5 || image.scale > 8 || image.scale <= prior ||
            image.width / image.scale > 256 || image.height / image.scale > 256 ||
            image.rgba.size() != static_cast<std::size_t>(image.width) * image.height) {
            throw std::invalid_argument("Invalid cursor dimensions, scale or RGBA length");
        }
        if (prior != 0 && (std::abs(image.width / image.scale - logical_width) > 0.001 ||
                           std::abs(image.height / image.scale - logical_height) > 0.001)) {
            throw std::invalid_argument("Cursor variants must have the same logical size");
        }
        logical_width = image.width / image.scale;
        logical_height = image.height / image.scale;
        prior = image.scale;
    }
    return CursorImagesPtr(new CursorImages(std::move(images), x, y));
}
const CursorImage& CursorImages::select(double scale) const noexcept {
    if (!std::isfinite(scale) || scale <= 0) scale = 1;
    for (const CursorImage& image : images_) {
        if (image.scale >= scale) return image;
    }
    return images_.back();
}
CursorImage CursorImages::rasterize(double scale) const {
    if (!std::isfinite(scale) || scale <= 0) scale = 1;
    const double logical_width = images_.front().width / images_.front().scale;
    const double logical_height = images_.front().height / images_.front().scale;
    scale = std::clamp(scale, 0.5, std::min(8.0, 256.0 / std::max(logical_width, logical_height)));
    const CursorImage& source = select(scale);
    const int width = std::clamp(static_cast<int>(std::lround(logical_width * scale)), 1, 256);
    const int height = std::clamp(static_cast<int>(std::lround(logical_height * scale)), 1, 256);
    CursorImage result{width, height, scale, {}};
    result.rgba.reserve(static_cast<std::size_t>(width) * height);
    for (int y = 0; y < height; ++y) {
        const int sy = std::clamp(hotspot_y(source) + static_cast<int>(std::lround(
            (y - hotspot_y(result)) * static_cast<double>(source.height) / height)), 0, source.height - 1);
        for (int x = 0; x < width; ++x) {
            const int sx = std::clamp(hotspot_x(source) + static_cast<int>(std::lround(
                (x - hotspot_x(result)) * static_cast<double>(source.width) / width)), 0, source.width - 1);
            result.rgba.push_back(source.rgba[static_cast<std::size_t>(sy) * source.width + sx]);
        }
    }
    return result;
}
int CursorImages::hotspot_x(const CursorImage& image) const noexcept {
    return std::clamp(static_cast<int>(std::lround(hotspot_x_ * image.width)), 0, image.width - 1);
}
int CursorImages::hotspot_y(const CursorImage& image) const noexcept {
    return std::clamp(static_cast<int>(std::lround(hotspot_y_ * image.height)), 0, image.height - 1);
}
} // namespace gui_forms
