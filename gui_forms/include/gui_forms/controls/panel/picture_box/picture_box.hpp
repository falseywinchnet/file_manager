#pragma once

#include "gui_forms/controls/panel/panel.hpp"
#include "gui_forms/event.hpp"

#include <cstdint>

namespace gui_forms {

// Mirrors the five System.Windows.Forms PictureBoxSizeMode policies. Image
// storage remains owned by Window's renderer-neutral ImageRegistry; PictureBox
// is only a retained presentation consumer of a generational ImageId.
enum class PictureBoxSizeMode : std::uint8_t {
    normal,
    stretch_image,
    auto_size,
    center_image,
    zoom,
};

class PictureBox : public Panel {
public:
    explicit PictureBox(StableId stable_id);

    [[nodiscard]] ImageId image() const noexcept { return image_; }
    void set_image(ImageId image);
    void clear_image();
    [[nodiscard]] bool has_valid_image() const noexcept;
    [[nodiscard]] Size image_size() const noexcept;
    [[nodiscard]] PictureBoxSizeMode size_mode() const noexcept {
        return size_mode_;
    }
    void set_size_mode(PictureBoxSizeMode mode);
    [[nodiscard]] double image_opacity() const noexcept { return image_opacity_; }
    void set_image_opacity(double opacity);
    [[nodiscard]] Rect image_bounds() const noexcept;
    [[nodiscard]] Event<ImageId>& image_changed() noexcept { return image_changed_; }

    [[nodiscard]] Size measure(Size available) override;
    void on_paint(Painter& painter, Rect local_damage) override;
    [[nodiscard]] bool hit_test_local(Point local_point) const override;
    [[nodiscard]] SemanticDescriptor semantic_descriptor() const override;

private:
    [[nodiscard]] Rect content_bounds() const noexcept;

    ImageId image_{};
    PictureBoxSizeMode size_mode_{PictureBoxSizeMode::normal};
    double image_opacity_{1.0};
    Event<ImageId> image_changed_;
};

} // namespace gui_forms
