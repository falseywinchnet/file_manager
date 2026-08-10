#include "../support/drawing_support.hpp"

void ImageAttributes::set_color_matrix(std::span<const double, 25> matrix) {
    require_alive();
    for (const double entry : matrix) require_finite(entry, "color matrix entry");
    std::copy(matrix.begin(), matrix.end(), value_.color_matrix.begin());
    value_.has_color_matrix = true;
}

void ImageAttributes::reset_color_matrix() {
    require_alive();
    value_.has_color_matrix = false;
    value_.color_matrix = {};
}

void ImageAttributes::set_remap_table(
    std::span<const ImageAttributesSnapshot::ColorRemap> table) {
    require_alive();
    if (table.size() > 4096U) {
        throw std::length_error("image color remap table limit exceeded");
    }
    std::vector<ImageAttributesSnapshot::ColorRemap> copy;
    copy.reserve(table.size());
    for (const auto& entry : table) {
        if (entry.old_color.is_empty() || entry.new_color.is_empty()) {
            throw std::invalid_argument("image color remaps require concrete colors");
        }
        const auto duplicate = std::find_if(copy.begin(), copy.end(),
            [&](const auto& existing) {
                return existing.old_color.argb() == entry.old_color.argb();
            });
        if (duplicate != copy.end()) {
            throw std::invalid_argument("image color remap source colors must be unique");
        }
        copy.push_back(entry);
    }
    value_.remap_table = std::move(copy);
}

void ImageAttributes::reset_remap_table() {
    require_alive();
    value_.remap_table.clear();
}

std::unique_ptr<ImageAttributes> ImageAttributes::clone() const {
    require_alive();
    auto result = std::make_unique<ImageAttributes>();
    result->value_ = value_;
    return result;
}

ImageAttributesSnapshot ImageAttributes::snapshot() const {
    require_alive();
    return value_;
}


} // namespace gui_drawing

