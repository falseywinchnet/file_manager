#pragma once

#include "gallery_dml.hpp"
#include "gallery_model.hpp"
#include "gui_forms/gui_forms.hpp"

#include <memory>
#include <string>
#include <string_view>

namespace gui_forms::gallery {

class GalleryContext final {
public:
    GalleryModel model;
    Window* window{};
    FrameRequestToken instrument_frames;
    ImageId status_badge;
    std::string drop_status;

    void synchronize(std::string_view cause);
};

class GalleryControl final : public Control {
public:
    GalleryControl(StableId stable_id,
                   const dml::NodeSpec& specification,
                   std::shared_ptr<GalleryContext> context);

    [[nodiscard]] Size measure(Size available) override;
    void arrange(Rect final_bounds) override;
    void on_paint(Painter& painter, Rect local_damage) override;
    [[nodiscard]] bool hit_test_local(Point local_point) const override;
    void on_pointer(PointerEvent& event) override;
    void on_text_input(TextInputEvent& event) override;
    void on_drag(DragEvent& event) override;
    void on_focus_changed(bool focused) override;
    void on_activate() override;

    [[nodiscard]] dml::NodeKind kind() const noexcept;
    [[nodiscard]] bool selected() const;
    [[nodiscard]] double value() const;
    [[nodiscard]] std::string display_text() const;

private:
    const dml::NodeSpec* specification_{};
    std::shared_ptr<GalleryContext> context_;
    bool pressed_{};
    bool focused_{};
    bool drag_hovered_{};
    std::uint64_t paint_sequence_{};

    void arrange_children(Size size);
};

struct GalleryTree final {
    Control::Ptr root;
    std::shared_ptr<GalleryContext> context;
};

[[nodiscard]] GalleryTree build_gallery_tree();

}  // namespace gui_forms::gallery
