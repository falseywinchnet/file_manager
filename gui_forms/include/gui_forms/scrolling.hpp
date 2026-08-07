#pragma once

#include "gui_forms/control.hpp"

#include <cstdint>
#include <optional>
#include <string_view>

namespace gui_forms {

// Exact System.Windows.Forms values. EndScroll is retained even though the
// WinForms ScrollableControl itself suppresses that notification; standalone
// scroll bars and future host adapters use the complete vocabulary.
enum class ScrollEventType : std::uint8_t {
    small_decrement = 0,
    small_increment = 1,
    large_decrement = 2,
    large_increment = 3,
    thumb_position = 4,
    thumb_track = 5,
    first = 6,
    last = 7,
    end_scroll = 8,
};

enum class ScrollOrientation : std::uint8_t {
    horizontal = 0,
    vertical = 1,
};

struct ScrollEvent final {
    ScrollEventType type{ScrollEventType::thumb_position};
    double old_value{};
    double new_value{};
    ScrollOrientation orientation{ScrollOrientation::horizontal};
};

struct ScrollAxisSnapshot final {
    bool enabled{true};
    bool visible{};
    double minimum{};
    double maximum{100.0};
    double large_change{10.0};
    double small_change{1.0};
    double value{};
};

struct ScrollSnapshot final {
    bool auto_scroll{};
    Point position{};
    Size margin{};
    Size minimum_content_size{};
    Rect display_rectangle{};
    Rect viewport_rectangle{};
    ScrollAxisSnapshot horizontal;
    ScrollAxisSnapshot vertical;
};

class ScrollableControl;

class ScrollProperties final {
public:
    [[nodiscard]] bool enabled() const noexcept { return enabled_; }
    void set_enabled(bool enabled);
    [[nodiscard]] bool visible() const noexcept { return visible_; }
    void set_visible(bool visible);
    [[nodiscard]] double minimum() const noexcept { return minimum_; }
    void set_minimum(double minimum);
    [[nodiscard]] double maximum() const noexcept { return maximum_; }
    void set_maximum(double maximum);
    [[nodiscard]] double large_change() const noexcept;
    void set_large_change(double value);
    [[nodiscard]] double small_change() const noexcept;
    void set_small_change(double value);
    [[nodiscard]] double value() const noexcept { return value_; }
    void set_value(double value);
    [[nodiscard]] ScrollOrientation orientation() const noexcept {
        return orientation_;
    }
    [[nodiscard]] ScrollAxisSnapshot snapshot() const noexcept;

private:
    friend class ScrollableControl;
    ScrollProperties(ScrollableControl& owner, ScrollOrientation orientation)
        : owner_(&owner), orientation_(orientation) {}
    void set_automatic(double viewport_extent, double content_extent,
                       double value) noexcept;
    [[nodiscard]] double maximum_position() const noexcept;

    ScrollableControl* owner_{};
    ScrollOrientation orientation_{ScrollOrientation::horizontal};
    double minimum_{};
    double maximum_{100.0};
    double large_change_{10.0};
    double small_change_{1.0};
    double value_{};
    bool enabled_{true};
    bool visible_{};
    bool large_change_authored_{};
    bool small_change_authored_{};
};

// General retained scrolling substrate shared by Panel and ContainerControl.
// Child requested bounds remain logical content coordinates. Arrangement
// projects them through one positive scroll offset; paint and hit traversal are
// clipped to the viewport, so scroll chrome cannot be obscured by descendants.
class ScrollableControl : public Control {
public:
    static constexpr std::uint32_t scroll_state_auto_scrolling = 1U;
    static constexpr std::uint32_t scroll_state_hscroll_visible = 2U;
    static constexpr std::uint32_t scroll_state_vscroll_visible = 4U;
    static constexpr std::uint32_t scroll_state_user_has_scrolled = 8U;
    static constexpr std::uint32_t scroll_state_full_drag = 16U;

    explicit ScrollableControl(StableId stable_id);

    [[nodiscard]] bool auto_scroll() const noexcept { return auto_scroll_; }
    virtual void set_auto_scroll(bool enabled);
    [[nodiscard]] Size auto_scroll_margin() const noexcept {
        return auto_scroll_margin_;
    }
    void set_auto_scroll_margin(Size margin);
    void set_auto_scroll_margin(double x, double y);
    [[nodiscard]] Size auto_scroll_min_size() const noexcept {
        return auto_scroll_min_size_;
    }
    void set_auto_scroll_min_size(Size size);
    [[nodiscard]] Point auto_scroll_position() const noexcept {
        return {-scroll_position_.x, -scroll_position_.y};
    }
    void set_auto_scroll_position(Point position);
    [[nodiscard]] Point scroll_position() const noexcept {
        return scroll_position_;
    }
    [[nodiscard]] Rect display_rectangle() const noexcept override;
    [[nodiscard]] Rect viewport_rectangle() const noexcept {
        return effective_viewport_rectangle();
    }
    [[nodiscard]] const ScrollProperties& horizontal_scroll() const noexcept {
        return horizontal_scroll_;
    }
    [[nodiscard]] ScrollProperties& horizontal_scroll() noexcept {
        return horizontal_scroll_;
    }
    [[nodiscard]] const ScrollProperties& vertical_scroll() const noexcept {
        return vertical_scroll_;
    }
    [[nodiscard]] ScrollProperties& vertical_scroll() noexcept {
        return vertical_scroll_;
    }
    [[nodiscard]] bool hscroll() const noexcept {
        return horizontal_scroll_.visible_;
    }
    [[nodiscard]] bool vscroll() const noexcept {
        return vertical_scroll_.visible_;
    }
    void set_hscroll(bool visible);
    void set_vscroll(bool visible);
    [[nodiscard]] bool get_scroll_state(std::uint32_t bit) const noexcept;
    void set_scroll_state(std::uint32_t bit, bool value);
    [[nodiscard]] Event<ScrollEvent&>& scroll() noexcept { return scroll_event_; }
    [[nodiscard]] ScrollSnapshot scroll_snapshot() const noexcept;

    bool scroll_to(Point position,
                   ScrollEventType type = ScrollEventType::thumb_position,
                   bool notify = false);
    bool scroll_by(Point delta,
                   ScrollEventType horizontal_type =
                       ScrollEventType::small_increment,
                   ScrollEventType vertical_type =
                       ScrollEventType::small_increment,
                   bool notify = false);
    void scroll_control_into_view(const Control::Ptr& control);

    void arrange(Rect final_bounds) override;
    void on_pointer(PointerEvent& event) override;
    void on_pointer_bubble(PointerEvent& event) override;
    [[nodiscard]] std::vector<SemanticNode> semantic_virtual_children() const override;
    bool on_semantic_child_action(std::string_view stable_id,
                                  SemanticAction action,
                                  std::string_view value) override;

protected:
    virtual void adjust_scrollbars(bool display_scrollbars);
    [[nodiscard]] virtual Point scroll_to_control(const Control& control) const;
    void set_display_rect_location(Point location);
    void on_paint_overlay(Painter& painter, Rect local_damage) override;
    [[nodiscard]] Rect child_viewport_rectangle() const noexcept override {
        return effective_viewport_rectangle();
    }

private:
    friend class ScrollProperties;
    enum class ScrollPart : std::uint8_t {
        none,
        horizontal_first,
        horizontal_track_before,
        horizontal_thumb,
        horizontal_track_after,
        horizontal_last,
        vertical_first,
        vertical_track_before,
        vertical_thumb,
        vertical_track_after,
        vertical_last,
    };

    struct AxisGeometry final {
        Rect bar{};
        Rect first_button{};
        Rect last_button{};
        Rect track{};
        Rect thumb{};
    };

    void recompute_scroll_layout(Size client_size);
    [[nodiscard]] Rect effective_viewport_rectangle() const noexcept;
    [[nodiscard]] Size content_extent();
    [[nodiscard]] double maximum_offset(ScrollOrientation orientation) const noexcept;
    [[nodiscard]] AxisGeometry axis_geometry(ScrollOrientation orientation) const noexcept;
    [[nodiscard]] ScrollPart part_at(Point local) const noexcept;
    bool handle_wheel(PointerEvent& event);
    bool handle_scroll_pointer(PointerEvent& event);
    bool apply_axis_value(ScrollOrientation orientation, double value,
                          ScrollEventType type, bool notify);
    void notify_scroll(ScrollOrientation orientation, ScrollEventType type,
                       double old_value, double new_value);
    void paint_axis(Painter& painter, ScrollOrientation orientation,
                    const AxisGeometry& geometry) const;
    void axis_properties_changed(ScrollOrientation orientation,
                                 bool position_changed);
    static void validate_size(Size size, const char* message);
    static void validate_axis_value(double value, const char* message);

    ScrollProperties horizontal_scroll_;
    ScrollProperties vertical_scroll_;
    Event<ScrollEvent&> scroll_event_;
    Size auto_scroll_margin_{};
    Size auto_scroll_min_size_{};
    Size content_extent_{};
    Point scroll_position_{};
    Rect viewport_rectangle_{};
    std::uint32_t scroll_state_{scroll_state_full_drag};
    std::optional<ScrollOrientation> dragging_orientation_;
    std::uint64_t dragging_pointer_id_{};
    Point dragging_origin_{};
    double dragging_start_value_{};
    double scrollbar_thickness_{16.0};
    double scrollbar_button_extent_{16.0};
    double minimum_thumb_extent_{18.0};
    bool auto_scroll_{};
    bool arranging_scroll_{};
};

} // namespace gui_forms
