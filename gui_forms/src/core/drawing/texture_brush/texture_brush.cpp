#include "../support/drawing_support.hpp"

TextureBrush::TextureBrush(const Bitmap& image, WrapMode wrap_mode) {
    require_enum(wrap_mode, 4U, "texture wrap mode");
    value_.kind = BrushKind::texture;
    value_.wrap_mode = wrap_mode;
    value_.image = image.snapshot();
}

TextureBrush::TextureBrush(const ImageReference& image, WrapMode wrap_mode) {
    require_enum(wrap_mode, 4U, "texture wrap mode");
    value_.kind = BrushKind::texture;
    value_.wrap_mode = wrap_mode;
    value_.image = image.snapshot();
}

TextureBrush::TextureBrush(BrushSnapshot value) : value_(std::move(value)) {}

void TextureBrush::set_wrap_mode(WrapMode mode) {
    require_alive();
    require_enum(mode, 4U, "texture wrap mode");
    value_.wrap_mode = mode;
}

void TextureBrush::set_transform(Matrix transform) {
    require_alive();
    if (!transform.finite()) {
        throw std::invalid_argument("texture transform must be finite");
    }
    value_.transform = transform;
}

void TextureBrush::reset_transform() {
    require_alive();
    value_.transform = Matrix{};
}

void TextureBrush::translate_transform(double x, double y) {
    require_alive();
    value_.transform = value_.transform.followed_by(Matrix::translation(x, y));
}

void TextureBrush::scale_transform(double x, double y) {
    require_alive();
    require_finite(x, "texture scale x");
    require_finite(y, "texture scale y");
    if (x == 0.0 || y == 0.0) {
        throw std::invalid_argument("texture scale must be nonzero");
    }
    value_.transform = value_.transform.followed_by(
        Matrix{x, 0.0, 0.0, y, 0.0, 0.0});
}

void TextureBrush::rotate_transform(double degrees) {
    require_alive();
    value_.transform = value_.transform.followed_by(
        Matrix::rotation_at(degrees, {0.0, 0.0}));
}

std::unique_ptr<TextureBrush> TextureBrush::clone() const {
    require_alive();
    return std::unique_ptr<TextureBrush>(new TextureBrush(value_));
}

BrushSnapshot TextureBrush::snapshot() const {
    require_alive();
    return value_;
}


} // namespace gui_drawing

