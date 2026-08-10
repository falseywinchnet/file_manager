#pragma once

#include "gui_forms/c_api.h"

#include "gui_forms/basic_controls.hpp"
#include "gui_forms/control.hpp"
#include "gui_forms/inspection_controls.hpp"
#include "gui_forms/live_surface.hpp"
#include "gui_forms/range_controls.hpp"
#include "gui_forms/scrolling.hpp"
#include "gui_forms/text.hpp"
#include "gui_forms/window.hpp"
#include "headless_host.hpp"
#if defined(GF_C_API_HAS_WINDOWS_HOST)
#include "windows_host.hpp"
#endif
#if defined(GF_C_API_HAS_MACOS_HOST)
#include "macos_host.hpp"
#endif

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstring>
#include <deque>
#include <functional>
#include <limits>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <stdexcept>
#include <string>
#include <thread>
#include <unordered_map>
#include <utility>
#include <vector>

namespace gui_forms::abi::detail {

using gui_forms::ComponentState;
using gui_forms::Control;
using gui_forms::ImageResourceEncoding;
using gui_forms::Rect;
using gui_forms::Size;
using gui_forms::StableId;
using gui_forms::Window;
enum class FieldControlKind {
    text_box,
    combo_box,
    list_box,
    picture_box,
    data_grid,
    tool_strip,
    numeric_up_down,
};

struct RasterPointerSample final {
    std::uint32_t event_kind{};
    double x{};
    double y{};
    double wheel_delta{};
    std::uint32_t button{};
};

struct RasterKeySample final {
    std::uint32_t event_kind{};
    std::uint32_t physical_key{};
    std::uint32_t modifiers{};
    bool repeat{};
    bool handled{};
};

struct RasterTextSample final {
    std::string text;
    bool composing{};
    std::int32_t replacement_start{-1};
    std::int32_t replacement_length{};
};

// Some compatibility widgets participate in retained layout and painting order
// but are not interactive surfaces. In particular, DockPanelSuite creates an
// empty auto-hide strip covering its entire client area. Keeping this policy in
// a dedicated ABI kind avoids encoding third-party names in the portable core.
class InputTransparentControl final : public Control {
public:
    explicit InputTransparentControl(StableId stable_id)
        : Control(std::move(stable_id)) {}

    [[nodiscard]] bool hit_test_local(gui_forms::Point) const override {
        return false;
    }
};

// A form is a retained panel plus a preview seam for focus-scope commands.
// Keeping this in the ABI adapter lets the portable Window route keys once,
// before the focused child, without teaching the core about WinForms dialog
// buttons or managed callback types.
class FormControl final : public gui_forms::Panel {
public:
    explicit FormControl(StableId stable_id)
        : Panel(std::move(stable_id)) {}

    [[nodiscard]] gui_forms::Event<RasterKeySample&>& key_preview() noexcept {
        return key_preview_;
    }

    void on_key_preview(gui_forms::KeyEvent& event) override {
        RasterKeySample sample{
            event.action == gui_forms::KeyAction::down
                ? GF_EVENT_KEY_DOWN : GF_EVENT_KEY_UP,
            event.physical_key,
            static_cast<std::uint32_t>(event.modifiers),
            event.repeat,
            false,
        };
        key_preview_.emit(sample);
        event.handled = sample.handled;
    }

private:
    gui_forms::Event<RasterKeySample&> key_preview_;
};

// ABI-facing fields retain the platform-neutral text editor state. The managed
// compatibility facade projects WinForms properties and events, while this
// object owns Unicode mutation, directional selection, history, glyph geometry,
// hit testing, clipping, and the horizontal viewport.
class FieldControl final : public gui_forms::Panel {
public:
    FieldControl(StableId stable_id, FieldControlKind kind)
        : Panel(std::move(stable_id)), kind_(kind) {
        // Panel is a container and therefore defaults to the backplane. ABI
        // fields are leaf controls: keeping them there lets later container
        // siblings erase their border and text even when z-order is correct.
        set_paint_plane(gui_forms::PaintPlane::control);
        const auto style = style_;
        set_background(kind_ == FieldControlKind::tool_strip ? style.face : style.paper);
        set_border_style(kind_ == FieldControlKind::tool_strip
                             ? gui_forms::BorderStyle::none
                             : gui_forms::BorderStyle::sunken);
        if (kind_ == FieldControlKind::text_box ||
            kind_ == FieldControlKind::combo_box ||
            kind_ == FieldControlKind::list_box ||
            kind_ == FieldControlKind::data_grid ||
            kind_ == FieldControlKind::numeric_up_down) {
            set_focusable(true);
        }
        if (kind_ == FieldControlKind::text_box ||
            kind_ == FieldControlKind::combo_box ||
            kind_ == FieldControlKind::numeric_up_down) {
            set_cursor(gui_forms::CursorKind::text);
        }
    }

    void set_colors(gui_forms::Color foreground, gui_forms::Color background) {
        require_mutable();
        const auto disabled_channel = [](std::uint8_t foreground_channel,
                                         std::uint8_t background_channel) {
            return static_cast<std::uint8_t>(
                (static_cast<unsigned>(foreground_channel) * 45U +
                 static_cast<unsigned>(background_channel) * 55U) / 100U);
        };
        style_.text = foreground;
        style_.disabled_text = gui_forms::Color::rgba(
            disabled_channel(foreground.red, background.red),
            disabled_channel(foreground.green, background.green),
            disabled_channel(foreground.blue, background.blue),
            foreground.alpha);
        set_background(background);
        invalidate(gui_forms::Dirty::paint | gui_forms::Dirty::semantics);
    }

    void set_text(std::string text) {
        require_mutable();
        if (text_ == text) {
            return;
        }
        text_ = std::move(text);
        text_store_.set_text(text_);
        anchor_ = std::min(anchor_, static_cast<std::uint64_t>(text_.size()));
        caret_ = std::min(caret_, static_cast<std::uint64_t>(text_.size()));
        if (!text_store_.is_grapheme_boundary(gui_forms::Utf8Offset(anchor_))) {
            anchor_ = text_store_.utf8_offset(
                text_store_.grapheme_index(gui_forms::Utf8Offset(anchor_))).value();
        }
        if (!text_store_.is_grapheme_boundary(gui_forms::Utf8Offset(caret_))) {
            caret_ = text_store_.utf8_offset(
                text_store_.grapheme_index(gui_forms::Utf8Offset(caret_))).value();
        }
        layout_positions_.clear();
        layout_offsets_.clear();
        clear_history();
        invalidate(gui_forms::Dirty::paint | gui_forms::Dirty::semantics);
    }

    [[nodiscard]] bool replace(std::uint64_t start, std::uint64_t length,
                               std::string_view replacement,
                               gf_field_edit_result& result) {
        require_mutable();
        if (start > text_.size() || length > text_.size() - start ||
            !text_store_.is_grapheme_boundary(gui_forms::Utf8Offset(start)) ||
            !text_store_.is_grapheme_boundary(gui_forms::Utf8Offset(start + length)) ||
            !gui_forms::validate_utf8(replacement).valid()) {
            return false;
        }
        const bool text_changes =
            std::string_view(text_).substr(static_cast<std::size_t>(start),
                                           static_cast<std::size_t>(length)) !=
            replacement;
        if (text_changes) {
            push_undo(snapshot());
            clear_redo();
            static_cast<void>(text_store_.replace(
                {gui_forms::Utf8Offset(start),
                 gui_forms::Utf8Offset(start + length)}, replacement));
            text_.assign(text_store_.utf8());
            layout_positions_.clear();
            layout_offsets_.clear();
        }
        const std::uint64_t next = start + replacement.size();
        const bool state_changes = anchor_ != next || caret_ != next;
        anchor_ = next;
        caret_ = next;
        caret_visible_ = true;
        if (text_changes || state_changes) {
            invalidate(gui_forms::Dirty::paint | gui_forms::Dirty::semantics);
        }
        result = edit_result(text_changes);
        return true;
    }

    [[nodiscard]] bool history(std::int32_t direction,
                               gf_field_edit_result& result) {
        require_mutable();
        if (direction != -1 && direction != 1) return false;
        auto& source = direction < 0 ? undo_ : redo_;
        auto& destination = direction < 0 ? redo_ : undo_;
        if (source.empty()) {
            result = edit_result(false);
            return true;
        }
        FieldSnapshot target = std::move(source.back());
        history_bytes_ -= target.text.size();
        source.pop_back();
        push_history(destination, snapshot());
        apply_snapshot(std::move(target));
        result = edit_result(true);
        return true;
    }

    void clear_history() noexcept {
        undo_.clear();
        redo_.clear();
        history_bytes_ = 0;
    }

    [[nodiscard]] std::string_view text() const noexcept { return text_; }

    bool set_selection(std::uint64_t start, std::uint64_t length,
                       bool caret_visible) {
        if (start > text_.size() || length > text_.size() - start) return false;
        return set_edit_state(start, start + length, caret_visible);
    }

    bool set_edit_state(std::uint64_t anchor, std::uint64_t caret,
                        bool caret_visible) {
        require_mutable();
        if (anchor > text_.size() || caret > text_.size() ||
            !text_store_.is_grapheme_boundary(gui_forms::Utf8Offset(anchor)) ||
            !text_store_.is_grapheme_boundary(gui_forms::Utf8Offset(caret))) {
            return false;
        }
        if (anchor_ == anchor && caret_ == caret &&
            caret_visible_ == caret_visible) {
            return true;
        }
        anchor_ = anchor;
        caret_ = caret;
        caret_visible_ = caret_visible;
        invalidate(gui_forms::Dirty::paint | gui_forms::Dirty::semantics);
        return true;
    }

    [[nodiscard]] std::uint64_t position_at(double local_x) const noexcept {
        if (layout_positions_.size() != layout_offsets_.size() ||
            layout_positions_.empty()) {
            const double guessed = std::max(0.0, local_x - text_left_ + horizontal_offset_);
            const std::size_t index = std::min<std::size_t>(
                static_cast<std::size_t>(std::lround(guessed / 7.0)),
                text_store_.grapheme_count().value());
            return text_store_.utf8_offset(gui_forms::GraphemeIndex(index)).value();
        }
        const double content_x = std::max(0.0, local_x - text_left_ + horizontal_offset_);
        const auto right = std::lower_bound(layout_positions_.begin(),
                                            layout_positions_.end(), content_x);
        if (right == layout_positions_.begin()) return layout_offsets_.front();
        if (right == layout_positions_.end()) return layout_offsets_.back();
        const std::size_t right_index = static_cast<std::size_t>(
            std::distance(layout_positions_.begin(), right));
        const double left_distance = content_x - layout_positions_[right_index - 1U];
        const double right_distance = layout_positions_[right_index] - content_x;
        return layout_offsets_[left_distance < right_distance
            ? right_index - 1U : right_index];
    }

    [[nodiscard]] bool navigate(std::uint64_t position, std::int32_t direction,
                                std::uint64_t& result) const noexcept {
        if (position > text_.size() ||
            !text_store_.is_grapheme_boundary(gui_forms::Utf8Offset(position)) ||
            (direction != -1 && direction != 1)) return false;
        const auto source = gui_forms::Utf8Offset(position);
        result = (direction < 0
            ? text_store_.previous_grapheme_boundary(source)
            : text_store_.next_grapheme_boundary(source)).value();
        return true;
    }

    void on_paint(gui_forms::Painter& painter, Rect damage) override {
        Panel::on_paint(painter, damage);
        const Rect bounds = local_bounds();
        const auto style = style_;
        const double button_width = kind_ == FieldControlKind::numeric_up_down
            ? std::min(18.0, std::max(0.0, bounds.width)) : 0.0;
        const double text_right = std::max(5.0, bounds.width - button_width -
                                                (kind_ == FieldControlKind::combo_box ? 18.0 : 3.0));
        const gui_forms::FontSpec font{gui_forms::FontRole::content, 12.0, 400, false};
        layout_positions_.clear();
        layout_offsets_.clear();
        const std::size_t graphemes = text_store_.grapheme_count().value();
        layout_positions_.reserve(graphemes + 1U);
        layout_offsets_.reserve(graphemes + 1U);
        for (std::size_t index = 0; index <= graphemes; ++index) {
            const std::uint64_t offset = text_store_.utf8_offset(
                gui_forms::GraphemeIndex(index)).value();
            layout_offsets_.push_back(offset);
            layout_positions_.push_back(painter.measure_text_utf8(
                std::string_view(text_).substr(0, static_cast<std::size_t>(offset)),
                font).width);
        }
        const auto boundary_x = [&](std::uint64_t offset) {
            const std::size_t index = text_store_.grapheme_index(
                gui_forms::Utf8Offset(offset)).value();
            return layout_positions_[std::min(index, layout_positions_.size() - 1U)];
        };
        const double viewport_width = std::max(0.0, text_right - text_left_);
        const double caret_content_x = boundary_x(caret_);
        if (caret_content_x < horizontal_offset_) {
            horizontal_offset_ = caret_content_x;
        } else if (caret_content_x > horizontal_offset_ + viewport_width) {
            horizontal_offset_ = caret_content_x - viewport_width;
        }
        const double maximum_offset = std::max(
            0.0, layout_positions_.back() - viewport_width);
        horizontal_offset_ = std::clamp(horizontal_offset_, 0.0, maximum_offset);

        const std::uint64_t selection_start = std::min(anchor_, caret_);
        const std::uint64_t selection_end = std::max(anchor_, caret_);
        const double text_origin_x = text_left_ - horizontal_offset_;
        const double selection_x = text_origin_x + boundary_x(selection_start);
        const double selection_end_x = text_origin_x + boundary_x(selection_end);
        const double baseline = std::max(14.0, bounds.height * 0.5 + 4.0);
        painter.save();
        painter.clip_rect({text_left_, 2.0, viewport_width,
                           std::max(0.0, bounds.height - 4.0)});
        if (focused_ && selection_end > selection_start &&
            selection_end_x > selection_x) {
            painter.fill_rect({selection_x, 3.0, selection_end_x - selection_x,
                               std::max(0.0, bounds.height - 6.0)}, style.accent);
        }
        if (!text_.empty()) {
            painter.draw_text_utf8({text_origin_x, baseline}, text_, font,
                                   enabled() ? style.text : style.disabled_text);
            if (focused_ && selection_end > selection_start &&
                selection_end_x > selection_x) {
                painter.save();
                painter.clip_rect({selection_x, 3.0,
                                   selection_end_x - selection_x,
                                   std::max(0.0, bounds.height - 6.0)});
                painter.draw_text_utf8({text_origin_x, baseline}, text_, font,
                                       style.highlight);
                painter.restore();
            }
        }
        if (focused_ && caret_visible_ && anchor_ == caret_) {
            const double caret_x = text_origin_x + caret_content_x;
            painter.draw_line({caret_x, 4.0},
                              {caret_x, std::max(4.0, bounds.height - 4.0)},
                              enabled() ? style.text : style.disabled_text, 1.0);
        }
        painter.restore();
        if (kind_ == FieldControlKind::combo_box && bounds.width >= 18.0) {
            const double x = bounds.width - 14.0;
            const double y = bounds.height * 0.5 - 1.0;
            painter.draw_line({x, y}, {x + 4.0, y + 4.0}, style.dark_border, 1.0);
            painter.draw_line({x + 4.0, y + 4.0}, {x + 8.0, y}, style.dark_border, 1.0);
        } else if (kind_ == FieldControlKind::data_grid && bounds.height >= 24.0) {
            painter.fill_rect({1.0, 1.0, std::max(0.0, bounds.width - 2.0), 21.0},
                              style.face);
            painter.draw_line({1.0, 22.0}, {std::max(1.0, bounds.width - 1.0), 22.0},
                              style.border, 1.0);
        } else if (kind_ == FieldControlKind::numeric_up_down &&
                   button_width > 0.0 && bounds.height >= 12.0) {
            const double left = bounds.width - button_width;
            const double middle = std::floor(bounds.height * 0.5);
            painter.fill_rect({left, 1.0, button_width - 1.0,
                               std::max(0.0, bounds.height - 2.0)}, style.face);
            painter.draw_line({left, 1.0}, {left, bounds.height - 1.0},
                              style.border, 1.0);
            painter.draw_line({left, middle}, {bounds.width - 1.0, middle},
                              style.border, 1.0);
            const double center = left + button_width * 0.5;
            painter.draw_line({center - 3.0, middle - 3.0},
                              {center, middle - 6.0}, style.dark_border, 1.0);
            painter.draw_line({center, middle - 6.0},
                              {center + 3.0, middle - 3.0}, style.dark_border, 1.0);
            painter.draw_line({center - 3.0, middle + 4.0},
                              {center, middle + 7.0}, style.dark_border, 1.0);
            painter.draw_line({center, middle + 7.0},
                              {center + 3.0, middle + 4.0}, style.dark_border, 1.0);
        }
        if (focused_ && bounds.width > 2.0 && bounds.height > 2.0) {
            const auto focus = style.accent;
            painter.draw_line({1.0, 1.0}, {bounds.width - 1.0, 1.0}, focus, 1.0);
            painter.draw_line({1.0, 1.0}, {1.0, bounds.height - 1.0}, focus, 1.0);
            painter.draw_line({1.0, bounds.height - 1.0},
                              {bounds.width - 1.0, bounds.height - 1.0}, focus, 1.0);
            painter.draw_line({bounds.width - 1.0, 1.0},
                              {bounds.width - 1.0, bounds.height - 1.0}, focus, 1.0);
        }
    }

    void on_focus_changed(bool focused) override {
        focused_ = focused;
        invalidate(gui_forms::Dirty::paint | gui_forms::Dirty::semantics);
    }

    [[nodiscard]] gui_forms::Event<const RasterPointerSample&>&
    pointer_input() noexcept {
        return pointer_input_;
    }

    [[nodiscard]] gui_forms::Event<const RasterKeySample&>& key_input() noexcept {
        return key_input_;
    }

    [[nodiscard]] gui_forms::Event<const RasterTextSample&>& text_input() noexcept {
        return text_input_;
    }

    void on_pointer(gui_forms::PointerEvent& event) override {
        const Rect absolute = absolute_bounds();
        std::uint32_t kind = GF_EVENT_MOUSE_MOVE;
        switch (event.action) {
        case gui_forms::PointerAction::down: kind = GF_EVENT_MOUSE_DOWN; break;
        case gui_forms::PointerAction::up: kind = GF_EVENT_MOUSE_UP; break;
        case gui_forms::PointerAction::wheel: kind = GF_EVENT_MOUSE_WHEEL; break;
        case gui_forms::PointerAction::enter: kind = GF_EVENT_MOUSE_ENTER; break;
        case gui_forms::PointerAction::leave: kind = GF_EVENT_MOUSE_LEAVE; break;
        case gui_forms::PointerAction::move: break;
        }
        pointer_input_.emit({kind,
                             event.position.x - absolute.x,
                             event.position.y - absolute.y,
                             event.wheel_delta.y,
                             static_cast<std::uint32_t>(event.button)});
        event.handled = true;
    }

    void on_key(gui_forms::KeyEvent& event) override {
        key_input_.emit({event.action == gui_forms::KeyAction::down
                             ? GF_EVENT_KEY_DOWN : GF_EVENT_KEY_UP,
                         event.physical_key,
                         static_cast<std::uint32_t>(event.modifiers),
                         event.repeat});
        event.handled = true;
    }

    void on_text_input(gui_forms::TextInputEvent& event) override {
        text_input_.emit({event.text_utf8, event.composing,
                          event.replacement_start, event.replacement_length});
        event.handled = true;
    }

private:
    struct FieldSnapshot final {
        std::string text;
        std::uint64_t anchor{};
        std::uint64_t caret{};
    };

    [[nodiscard]] FieldSnapshot snapshot() const {
        return {text_, anchor_, caret_};
    }

    void apply_snapshot(FieldSnapshot snapshot) {
        text_ = std::move(snapshot.text);
        text_store_.set_text(text_);
        anchor_ = snapshot.anchor;
        caret_ = snapshot.caret;
        caret_visible_ = true;
        layout_positions_.clear();
        layout_offsets_.clear();
        invalidate(gui_forms::Dirty::paint | gui_forms::Dirty::semantics);
    }

    void push_history(std::deque<FieldSnapshot>& history,
                      FieldSnapshot snapshot) {
        history_bytes_ += snapshot.text.size();
        history.push_back(std::move(snapshot));
        while (history_bytes_ > maximum_history_bytes_ ||
               undo_.size() + redo_.size() > maximum_history_entries_) {
            if (!undo_.empty()) {
                history_bytes_ -= undo_.front().text.size();
                undo_.pop_front();
            } else if (!redo_.empty()) {
                history_bytes_ -= redo_.front().text.size();
                redo_.pop_front();
            } else {
                break;
            }
        }
    }

    void push_undo(FieldSnapshot snapshot) {
        push_history(undo_, std::move(snapshot));
    }

    void clear_redo() noexcept {
        for (const auto& snapshot : redo_) history_bytes_ -= snapshot.text.size();
        redo_.clear();
    }

    [[nodiscard]] gf_field_edit_result edit_result(bool changed) const noexcept {
        return {anchor_, caret_, text_store_.revision(), changed ? 1U : 0U,
                undo_.empty() ? 0U : 1U, redo_.empty() ? 0U : 1U};
    }

    std::string text_;
    gui_forms::TextStore text_store_;
    FieldControlKind kind_;
    gui_forms::BasicControlStyle style_;
    std::uint64_t anchor_{};
    std::uint64_t caret_{};
    std::vector<double> layout_positions_;
    std::vector<std::uint64_t> layout_offsets_;
    double horizontal_offset_{};
    static constexpr double text_left_{5.0};
    bool caret_visible_{true};
    bool focused_{};
    std::deque<FieldSnapshot> undo_;
    std::deque<FieldSnapshot> redo_;
    std::size_t history_bytes_{};
    static constexpr std::size_t maximum_history_entries_{128U};
    static constexpr std::size_t maximum_history_bytes_{8U * 1024U * 1024U};
    gui_forms::Event<const RasterPointerSample&> pointer_input_;
    gui_forms::Event<const RasterKeySample&> key_input_;
    gui_forms::Event<const RasterTextSample&> text_input_;
};

[[nodiscard]] inline gui_forms::Color color_from_argb(
    std::uint32_t argb) noexcept {
    return gui_forms::Color::rgba(
        static_cast<std::uint8_t>((argb >> 16U) & 0xffU),
        static_cast<std::uint8_t>((argb >> 8U) & 0xffU),
        static_cast<std::uint8_t>(argb & 0xffU),
        static_cast<std::uint8_t>((argb >> 24U) & 0xffU));
}

// A compatibility control may be both owner-painted and scrollable. WinForms
// exposes those as independent capabilities through inheritance and styles;
// treating the ABI kind as an exclusive class choice makes legitimate custom
// ScrollableControl subclasses impossible to construct. Keep the raster surface
// on the scrolling substrate. For ordinary owner-painted Control subclasses the
// additional capability is dormant (AutoScroll and both axes default off).
class RasterControl final : public gui_forms::ScrollableControl {
public:
    explicit RasterControl(StableId stable_id, bool input_transparent = false)
        : ScrollableControl(std::move(stable_id)),
          input_transparent_(input_transparent) {
        set_focusable(!input_transparent_);
    }

    [[nodiscard]] bool hit_test_local(gui_forms::Point point) const override {
        return !input_transparent_ && ScrollableControl::hit_test_local(point);
    }

    [[nodiscard]] gui_forms::Event<const RasterPointerSample&>& pointer_input() noexcept {
        return pointer_input_;
    }

    [[nodiscard]] gui_forms::Event<const RasterKeySample&>& key_input() noexcept {
        return key_input_;
    }

    bool set_png(std::span<const std::byte> encoded) {
        require_mutable();
        if (encoded.empty()) {
            if (window() != nullptr && image_.value != 0) {
                static_cast<void>(window()->remove_image(image_));
            }
            image_ = {};
            encoded_.clear();
            encoding_ = ImageResourceEncoding::png;
            pixel_width_ = 0;
            pixel_height_ = 0;
            pixel_row_bytes_ = 0;
            invalidate(gui_forms::Dirty::paint | gui_forms::Dirty::semantics);
            return true;
        }
        const auto validation = gui_forms::validate_png(encoded);
        if (!validation) {
            return false;
        }
        encoded_.assign(encoded.begin(), encoded.end());
        encoding_ = ImageResourceEncoding::png;
        pixel_width_ = validation.metadata.width;
        pixel_height_ = validation.metadata.height;
        pixel_row_bytes_ = validation.metadata.source_row_bytes;
        bool replacement_invalidated = false;
        if (window() != nullptr) {
            const bool replacing = image_.value != 0;
            const auto loaded = replacing
                ? window()->replace_png(image_, encoded_, *this)
                : window()->load_png(encoded_);
            if (!loaded) {
                return false;
            }
            image_ = loaded.image;
            replacement_invalidated = replacing;
        }
        if (!replacement_invalidated) {
            invalidate(gui_forms::Dirty::paint | gui_forms::Dirty::semantics);
        }
        return true;
    }

    bool set_bgra32_premultiplied(std::uint32_t width, std::uint32_t height,
                                  std::uint64_t row_bytes,
                                  std::span<const std::byte> pixels) {
        require_mutable();
        if (pixels.empty()) {
            return set_png({});
        }
        bool replacement_invalidated = false;
        if (window() != nullptr) {
            const bool replacing = image_.value != 0;
            const bool same_size = replacing && width == pixel_width_ &&
                height == pixel_height_;
            const auto loaded = same_size
                ? window()->update_bgra32_premultiplied(
                    image_, width, height, row_bytes, pixels, *this)
                : replacing
                ? window()->replace_bgra32_premultiplied(
                    image_, width, height, row_bytes, pixels, *this)
                : window()->load_bgra32_premultiplied(
                    width, height, row_bytes, pixels);
            if (!loaded) {
                return false;
            }
            image_ = loaded.image;
            replacement_invalidated = replacing;
            encoded_.clear();
        } else {
            try {
                encoded_.assign(pixels.begin(), pixels.end());
            } catch (const std::bad_alloc&) {
                return false;
            }
        }
        encoding_ = ImageResourceEncoding::bgra32_premultiplied;
        pixel_width_ = width;
        pixel_height_ = height;
        pixel_row_bytes_ = row_bytes;
        if (!replacement_invalidated) {
            invalidate(gui_forms::Dirty::paint | gui_forms::Dirty::semantics);
        }
        return true;
    }

    void set_live_surface(std::shared_ptr<gui_forms::LiveSurface> surface) {
        require_mutable();
        if (live_surface_ == surface) return;
        disconnect_live_surface_wake();
        live_surface_ = std::move(surface);
        connect_live_surface_wake();
        invalidate(gui_forms::Dirty::paint | gui_forms::Dirty::semantics);
    }

    void clear_live_surface(const std::shared_ptr<gui_forms::LiveSurface>& surface) {
        require_mutable();
        if (!live_surface_ || (surface && live_surface_ != surface)) {
            return;
        }
        disconnect_live_surface_wake();
        live_surface_.reset();
        invalidate(gui_forms::Dirty::paint | gui_forms::Dirty::semantics);
    }

    void on_paint(gui_forms::Painter& painter, Rect) override {
        const Rect bounds = committed_arranged_bounds();
        if (live_surface_) {
            const std::uint64_t candidate_generation =
                live_surface_->snapshot().published_generation;
            painter.draw_live_surface(live_surface_,
                                      {0.0, 0.0, bounds.width, bounds.height},
                                      1.0);
            // A live wake stays coalesced until the retained paint consumes a
            // candidate, not merely until its dispatcher callback runs. This
            // prevents a fast producer from filling the UI queue while one
            // frame is still waiting to render. If publication raced the
            // paint, latest-frame semantics request exactly one follow-up.
            const auto wake_state = live_wake_state_;
            if (wake_state && wake_state->connected.load(
                                  std::memory_order_acquire)) {
                wake_state->queued.store(false, std::memory_order_release);
                if (live_surface_->snapshot().published_generation !=
                    candidate_generation) {
                    queue_live_surface_paint(
                        std::static_pointer_cast<RasterControl>(
                            shared_from_this()),
                        wake_state);
                }
            }
            return;
        }
        if (image_.value == 0) {
            return;
        }
        painter.draw_image(image_, {0.0, 0.0, bounds.width, bounds.height}, 1.0);
    }

    void on_pointer(gui_forms::PointerEvent& event) override {
        const Rect absolute = absolute_bounds();
        std::uint32_t kind = GF_EVENT_MOUSE_MOVE;
        switch (event.action) {
        case gui_forms::PointerAction::down: kind = GF_EVENT_MOUSE_DOWN; break;
        case gui_forms::PointerAction::up: kind = GF_EVENT_MOUSE_UP; break;
        case gui_forms::PointerAction::wheel: kind = GF_EVENT_MOUSE_WHEEL; break;
        case gui_forms::PointerAction::enter: kind = GF_EVENT_MOUSE_ENTER; break;
        case gui_forms::PointerAction::leave: kind = GF_EVENT_MOUSE_LEAVE; break;
        case gui_forms::PointerAction::move: break;
        }
        pointer_input_.emit({
            kind,
            event.position.x - absolute.x,
            event.position.y - absolute.y,
            event.wheel_delta.y,
            static_cast<std::uint32_t>(event.button),
        });
    }

    void on_key(gui_forms::KeyEvent& event) override {
        key_input_.emit({event.action == gui_forms::KeyAction::down
                             ? GF_EVENT_KEY_DOWN : GF_EVENT_KEY_UP,
                         event.physical_key,
                         static_cast<std::uint32_t>(event.modifiers),
                         event.repeat});
        event.handled = true;
    }

protected:
    void on_attached_to_window() override {
        ScrollableControl::on_attached_to_window();
        if (!encoded_.empty() && image_.value == 0) {
            const auto loaded = encoding_ == ImageResourceEncoding::png
                ? window()->load_png(encoded_)
                : window()->load_bgra32_premultiplied(
                    pixel_width_, pixel_height_, pixel_row_bytes_, encoded_);
            if (loaded) {
                image_ = loaded.image;
            }
        }
        connect_live_surface_wake();
    }

    void on_detached_from_window() noexcept override {
        disconnect_live_surface_wake();
        if (window() != nullptr && image_.value != 0) {
            if (const auto resource = window()->image_resources().find(image_);
                resource &&
                resource->encoding == ImageResourceEncoding::bgra32_premultiplied) {
                try {
                    encoded_.assign(resource->encoded.begin(), resource->encoded.end());
                } catch (...) {
                    encoded_.clear();
                }
            }
            static_cast<void>(window()->remove_image(image_));
        }
        image_ = {};
        ScrollableControl::on_detached_from_window();
    }

private:
    struct LiveWakeState final {
        std::atomic<bool> connected{true};
        std::atomic<bool> queued{};
    };

    static void queue_live_surface_paint(
        const std::weak_ptr<RasterControl>& weak_target,
        const std::weak_ptr<LiveWakeState>& weak_state) noexcept {
        const auto state = weak_state.lock();
        if (!state || !state->connected.load(std::memory_order_acquire) ||
            state->queued.exchange(true, std::memory_order_acq_rel)) {
            return;
        }
        const auto target = weak_target.lock();
        if (!target) {
            state->queued.store(false, std::memory_order_release);
            return;
        }
        try {
            static_cast<void>(target->begin_invoke(
                [weak_target, weak_state] {
                    const auto queued_state = weak_state.lock();
                    if (!queued_state || !queued_state->connected.load(
                                             std::memory_order_acquire)) {
                        return;
                    }
                    if (const auto queued_target = weak_target.lock()) {
                        if (auto* owner = queued_target->window();
                            owner != nullptr &&
                            owner->queue_live_surface_presentation(
                                queued_target, queued_target->live_surface_)) {
                            // The host consumes the newest immutable generation
                            // as a compositor layer. Rearm publication now;
                            // no retained paint transaction is outstanding.
                            queued_state->queued.store(
                                false, std::memory_order_release);
                            return;
                        }
                        queued_target->invalidate(
                            gui_forms::invalidation::paint_only);
                    }
                    // Do not rearm here. The retained paint transaction owns
                    // release after it samples the newest complete candidate.
                }));
        } catch (...) {
            state->queued.store(false, std::memory_order_release);
        }
    }

    void connect_live_surface_wake() {
        disconnect_live_surface_wake();
        if (!live_surface_ || window() == nullptr) return;
        const auto self = std::static_pointer_cast<RasterControl>(
            shared_from_this());
        if (window()->queue_live_surface_presentation(self, live_surface_)) {
            // Registration is persistent. The terminal host display clock now
            // samples newest generations; producer publication must not post
            // one dispatcher callback per frame.
            live_surface_direct_ = true;
            return;
        }
        auto wake_state = std::make_shared<LiveWakeState>();
        live_wake_state_ = wake_state;
        const std::weak_ptr<RasterControl> weak_target =
            self;
        live_wake_ = live_surface_->connect_presentation_wake(
            [weak_target, weak_state = std::weak_ptr<LiveWakeState>(wake_state)] {
                queue_live_surface_paint(weak_target, weak_state);
            });
    }

    void disconnect_live_surface_wake() noexcept {
        live_surface_direct_ = false;
        if (live_wake_state_) {
            live_wake_state_->connected.store(false, std::memory_order_release);
        }
        live_wake_.disconnect();
        live_wake_state_.reset();
    }

    bool input_transparent_{};
    std::vector<std::byte> encoded_;
    ImageResourceEncoding encoding_{ImageResourceEncoding::png};
    std::uint32_t pixel_width_{};
    std::uint32_t pixel_height_{};
    std::uint64_t pixel_row_bytes_{};
    gui_forms::ImageId image_{};
    std::shared_ptr<gui_forms::LiveSurface> live_surface_;
    bool live_surface_direct_{};
    gui_forms::LiveSurfaceWakeConnection live_wake_;
    std::shared_ptr<LiveWakeState> live_wake_state_;
    gui_forms::Event<const RasterPointerSample&> pointer_input_;
    gui_forms::Event<const RasterKeySample&> key_input_;
};

// A foreign object is represented to the native property engine as an
// ordinary retained Control whose registrations happen to dispatch through a
// bounded C callback table. The proxy is intentionally nonvisual and never
// retains a foreign pointer other than the caller-owned callback context.
class AbiPropertyObjectControl final : public Control {
    struct PropertyState;

public:
    explicit AbiPropertyObjectControl(StableId stable_id)
        : Control(std::move(stable_id)) {
        clear_bindable_properties();
    }

    void define(const gf_property_descriptor_v1& authored,
                const gf_property_callbacks_v1& callbacks) {
        require_mutable();
        constexpr std::size_t minimum_callback_size =
            offsetof(gf_property_callbacks_v1, parse) +
            sizeof(gf_property_parse_callback);
        if (authored.struct_size < sizeof(gf_property_descriptor_v1) ||
            callbacks.struct_size < minimum_callback_size) {
            throw std::invalid_argument(
                "property proxy definitions require complete size-prefixed records");
        }
        auto state = std::make_shared<PropertyState>();
        std::memcpy(&state->callbacks, &callbacks,
                    std::min<std::size_t>(callbacks.struct_size,
                                          sizeof(state->callbacks)));
        state->descriptor = copy_descriptor(authored);
        if (state->descriptor.readable && callbacks.get == nullptr) {
            throw std::invalid_argument(
                "readable foreign properties require a getter callback");
        }
        if (state->descriptor.writable && callbacks.set == nullptr) {
            throw std::invalid_argument(
                "writable foreign properties require a setter callback");
        }
        if (state->descriptor.resettable && callbacks.reset == nullptr) {
            throw std::invalid_argument(
                "resettable foreign properties require a reset callback");
        }
        if (!state->descriptor.editor_name.empty() &&
            state->callbacks.edit == nullptr) {
            throw std::invalid_argument(
                "foreign property editor identities require an edit callback");
        }
        const std::string canonical = gui_forms::canonical_binding_name(
            state->descriptor.name);
        if (properties_.contains(canonical)) {
            throw std::invalid_argument(
                "foreign property names must be unique ignoring case");
        }

        gui_forms::PropertyRegistration registration;
        registration.descriptor = state->descriptor;
        if (registration.descriptor.readable) {
            registration.get = [state] { return state->get(); };
        }
        if (registration.descriptor.writable) {
            registration.set = [state](const gui_forms::BindingValue& value) {
                state->set(value);
                state->changed.emit();
            };
        }
        if (registration.descriptor.change_notifications) {
            registration.connect_changed = [state](
                gui_forms::Component& owner, std::function<void()> changed) {
                return state->changed.subscribe(owner, std::move(changed));
            };
        }
        if (registration.descriptor.resettable) {
            registration.reset = [state] {
                state->reset();
                state->changed.emit();
            };
        }
        if (callbacks.should_serialize != nullptr) {
            registration.should_serialize = [state] {
                return state->should_serialize();
            };
            registration.origin = [state] {
                return state->should_serialize()
                    ? gui_forms::PropertyValueOrigin::local
                    : gui_forms::PropertyValueOrigin::defaulted;
            };
        }
        define_bindable_property(std::move(registration));
        properties_.emplace(canonical, std::move(state));
    }

    void notify_changed(std::string_view name) {
        require_mutable();
        const auto found = properties_.find(
            gui_forms::canonical_binding_name(name));
        if (found == properties_.end()) {
            throw std::invalid_argument(
                "foreign property change names must identify a definition");
        }
        found->second->changed.emit();
    }

    void install_converters(gui_forms::PropertyValueConverterRegistry& target) {
        for (const auto& [canonical, state] : properties_) {
            static_cast<void>(canonical);
            const std::string& name = state->descriptor.converter_name;
            if (name.empty() || target.find(name) != nullptr ||
                state->callbacks.format == nullptr) {
                continue;
            }
            gui_forms::PropertyValueConverter converter;
            if (state->callbacks.format != nullptr) {
                converter.format = [weak = std::weak_ptr<PropertyState>(state)](
                    const gui_forms::BindingValue& value,
                    const gui_forms::PropertyDescriptor&) {
                    const auto current = weak.lock();
                    if (!current) {
                        throw std::logic_error(
                            "foreign property converter outlived its proxy");
                    }
                    return current->format(value);
                };
            }
            converter.parse = [weak = std::weak_ptr<PropertyState>(state)](
                std::string_view text, const gui_forms::BindingValue&,
                const gui_forms::PropertyDescriptor&) {
                const auto current = weak.lock();
                if (!current) {
                    throw std::logic_error(
                        "foreign property converter outlived its proxy");
                }
                return current->parse(text);
            };
            if (!target.register_converter(name, std::move(converter))) {
                throw std::logic_error(
                    "foreign property converter identity collision");
            }
        }
    }

    void install_editors(gui_forms::PropertyEditorRegistry& target,
                         std::set<std::string>& installed) {
        for (const auto& [canonical, state] : properties_) {
            static_cast<void>(canonical);
            const std::string& name = state->descriptor.editor_name;
            if (name.empty() || state->callbacks.edit == nullptr ||
                !installed.insert(name).second) {
                continue;
            }
            const bool registered = target.register_factory(
                name, [weak = std::weak_ptr<PropertyState>(state)](
                          const gui_forms::PropertyEditorRequest& request)
                    -> std::optional<gui_forms::PropertyEditorBinding> {
                    const auto retained = weak.lock();
                    if (!retained) {
                        throw std::logic_error(
                            "foreign property editor outlived its proxy");
                    }
                    auto button = gui_forms::make_control<gui_forms::Button>(
                        StableId(request.stable_id));
                    auto current = std::make_shared<gui_forms::BindingValue>(
                        request.value);
                    auto failures = std::make_shared<
                        gui_forms::Event<const gui_forms::PropertyEditorInputError&>>();
                    const auto update_text = [weak_button =
                            std::weak_ptr<gui_forms::Button>(button), weak](
                            const gui_forms::BindingValue& value) {
                        const auto editor = weak_button.lock();
                        const auto state = weak.lock();
                        if (!editor || !state) return;
                        std::string display = state->format(value);
                        if (display.size() > 96U) {
                            display.resize(93U);
                            display += "...";
                        }
                        editor->set_text(display.empty()
                            ? std::string("Edit \xE2\x80\xA6")
                            : display + "  \xE2\x80\xA6");
                    };
                    update_text(*current);
                    button->set_accessible_name(request.property_path);
                    button->set_accessible_description(
                        request.descriptor.description);
                    button->set_enabled(request.writable);

                    gui_forms::PropertyEditorBinding binding;
                    binding.control = button;
                    binding.synchronize = [current, update_text](
                        const gui_forms::BindingValue& value) {
                        *current = value;
                        update_text(value);
                    };
                    binding.connect_committed =
                        [weak_button = std::weak_ptr<gui_forms::Button>(button),
                         weak, current, failures](
                            gui_forms::Component& owner,
                            std::function<void(gui_forms::BindingValue)> committed) {
                            const auto editor = weak_button.lock();
                            if (!editor) return gui_forms::SubscriptionToken{};
                            return editor->clicked().subscribe(
                                owner, [weak, current, failures,
                                        committed = std::move(committed)](
                                           gui_forms::ButtonBase&) {
                                    const auto state = weak.lock();
                                    if (!state) return;
                                    try {
                                        committed(state->edit(*current));
                                    } catch (const std::exception& error) {
                                        const gui_forms::PropertyEditorInputError failure{
                                            state->format(*current), error.what()};
                                        failures->emit(failure);
                                    }
                                });
                        };
                    binding.connect_failed =
                        [failures](gui_forms::Component& owner,
                                   std::function<void(
                                       const gui_forms::PropertyEditorInputError&)>
                                       failed) {
                            return failures->subscribe(owner, std::move(failed));
                        };
                    return binding;
                });
            if (!registered) {
                throw std::logic_error(
                    "foreign property editor identity collision");
            }
        }
    }

private:
    static constexpr std::uint64_t maximum_callback_text_bytes =
        1024ULL * 1024ULL;

    static std::string copy_text(gf_string_view view,
                                 std::size_t maximum,
                                 std::string_view field) {
        if ((view.size != 0U && view.data == nullptr) ||
            view.size > maximum ||
            view.size > static_cast<std::uint64_t>(
                std::numeric_limits<std::size_t>::max())) {
            throw std::invalid_argument(
                std::string(field) + " is not a bounded string view");
        }
        std::string result;
        if (view.size != 0U) {
            result.assign(view.data, static_cast<std::size_t>(view.size));
        }
        if (result.find('\0') != std::string::npos ||
            !gui_forms::validate_utf8(result).valid()) {
            throw std::invalid_argument(
                std::string(field) + " must contain valid UTF-8 without NUL");
        }
        return result;
    }

    static gui_forms::BindingValueKind native_kind(std::uint32_t kind) {
        switch (kind) {
        case GF_PROPERTY_BOOLEAN:
            return gui_forms::BindingValueKind::boolean;
        case GF_PROPERTY_SIGNED_INTEGER:
            return gui_forms::BindingValueKind::signed_integer;
        case GF_PROPERTY_UNSIGNED_INTEGER:
            return gui_forms::BindingValueKind::unsigned_integer;
        case GF_PROPERTY_NUMBER:
            return gui_forms::BindingValueKind::number;
        case GF_PROPERTY_TEXT:
            return gui_forms::BindingValueKind::text;
        case GF_PROPERTY_COLOR:
            return gui_forms::BindingValueKind::color;
        case GF_PROPERTY_ENUMERATION:
            return gui_forms::BindingValueKind::enumeration;
        default:
            throw std::invalid_argument(
                "foreign property kind is not supported by ABI 0.23");
        }
    }

    static std::uint32_t abi_kind(gui_forms::BindingValueKind kind) {
        switch (kind) {
        case gui_forms::BindingValueKind::null: return GF_PROPERTY_NULL;
        case gui_forms::BindingValueKind::boolean: return GF_PROPERTY_BOOLEAN;
        case gui_forms::BindingValueKind::signed_integer:
            return GF_PROPERTY_SIGNED_INTEGER;
        case gui_forms::BindingValueKind::unsigned_integer:
            return GF_PROPERTY_UNSIGNED_INTEGER;
        case gui_forms::BindingValueKind::number: return GF_PROPERTY_NUMBER;
        case gui_forms::BindingValueKind::text: return GF_PROPERTY_TEXT;
        case gui_forms::BindingValueKind::color: return GF_PROPERTY_COLOR;
        case gui_forms::BindingValueKind::enumeration:
            return GF_PROPERTY_ENUMERATION;
        default:
            throw std::invalid_argument(
                "property value cannot cross the ABI 0.23 scalar channel");
        }
    }

    static gf_property_value to_abi(const gui_forms::BindingValue& value) {
        gf_property_value result{};
        result.kind = abi_kind(gui_forms::binding_value_kind(value));
        if (const auto* item = std::get_if<bool>(&value)) {
            result.boolean_value = *item ? 1U : 0U;
        } else if (const auto* item = std::get_if<std::int64_t>(&value)) {
            result.signed_value = *item;
        } else if (const auto* item = std::get_if<std::uint64_t>(&value)) {
            result.unsigned_value = *item;
        } else if (const auto* item = std::get_if<double>(&value)) {
            result.number_value = *item;
        } else if (const auto* item = std::get_if<std::string>(&value)) {
            result.text_value = {item->data(), item->size()};
        } else if (const auto* item = std::get_if<gui_forms::Color>(&value)) {
            result.color_argb =
                (static_cast<std::uint32_t>(item->alpha) << 24U) |
                (static_cast<std::uint32_t>(item->red) << 16U) |
                (static_cast<std::uint32_t>(item->green) << 8U) |
                static_cast<std::uint32_t>(item->blue);
        } else if (const auto* item =
                       std::get_if<gui_forms::PropertyEnumValue>(&value)) {
            result.signed_value = item->value;
            result.text_value = {item->name.data(), item->name.size()};
        }
        return result;
    }

    static gui_forms::BindingValue from_abi(
        const gf_property_value& value, std::string text,
        const gui_forms::PropertyDescriptor& descriptor) {
        if (value.kind == GF_PROPERTY_NULL) {
            return gui_forms::BindingValue{};
        }
        const auto expected = native_kind(value.kind);
        if (expected != descriptor.kind) {
            throw std::invalid_argument(
                "foreign callback returned a value outside its declared kind");
        }
        switch (value.kind) {
        case GF_PROPERTY_BOOLEAN:
            if (value.boolean_value > 1U) {
                throw std::invalid_argument(
                    "foreign Boolean callback returned a non-Boolean value");
            }
            return gui_forms::BindingValue{value.boolean_value != 0U};
        case GF_PROPERTY_SIGNED_INTEGER:
            return gui_forms::BindingValue{value.signed_value};
        case GF_PROPERTY_UNSIGNED_INTEGER:
            return gui_forms::BindingValue{value.unsigned_value};
        case GF_PROPERTY_NUMBER:
            if (!std::isfinite(value.number_value)) {
                throw std::invalid_argument(
                    "foreign numeric callback returned a non-finite value");
            }
            return gui_forms::BindingValue{value.number_value};
        case GF_PROPERTY_TEXT:
            return gui_forms::BindingValue{std::move(text)};
        case GF_PROPERTY_COLOR:
            return gui_forms::BindingValue{color_from_argb(value.color_argb)};
        case GF_PROPERTY_ENUMERATION: {
            std::string name = std::move(text);
            if (name.empty() && descriptor.enumeration) {
                const auto found = std::find_if(
                    descriptor.enumeration->choices.begin(),
                    descriptor.enumeration->choices.end(),
                    [&](const gui_forms::PropertyEnumChoice& choice) {
                        return choice.value == value.signed_value;
                    });
                if (found != descriptor.enumeration->choices.end()) {
                    name = found->name;
                }
            }
            return gui_forms::BindingValue{gui_forms::PropertyEnumValue{
                descriptor.enumeration ? descriptor.enumeration->type_name
                                       : std::string{},
                std::move(name), value.signed_value}};
        }
        default:
            throw std::invalid_argument(
                "foreign callback returned an unsupported property kind");
        }
    }

    static std::string callback_text(
        const std::function<std::uint32_t(char*, std::uint64_t,
                                          std::uint64_t*)>& invoke,
        std::string_view operation) {
        std::vector<char> buffer(256U);
        for (std::size_t attempt = 0U; attempt < 2U; ++attempt) {
            std::uint64_t required = 0U;
            const std::uint32_t result = invoke(
                buffer.data(), buffer.size(), &required);
            if (required > maximum_callback_text_bytes) {
                throw std::invalid_argument(
                    std::string(operation) + " exceeded the 1 MiB text bound");
            }
            if ((result == GF_ERROR_BUFFER_TOO_SMALL ||
                 required > buffer.size()) && attempt == 0U) {
                buffer.resize(std::max<std::size_t>(
                    1U, static_cast<std::size_t>(required)));
                continue;
            }
            if (result != GF_OK) {
                throw std::runtime_error(
                    std::string(operation) + " callback failed with result " +
                    std::to_string(result));
            }
            if (required > buffer.size()) {
                throw std::invalid_argument(
                    std::string(operation) + " callback reported an unstable size");
            }
            std::string text(buffer.data(), static_cast<std::size_t>(required));
            if (text.find('\0') != std::string::npos ||
                !gui_forms::validate_utf8(text).valid()) {
                throw std::invalid_argument(
                    std::string(operation) + " callback returned invalid UTF-8");
            }
            return text;
        }
        throw std::runtime_error(
            std::string(operation) + " callback did not stabilize");
    }

    static gui_forms::PropertyDescriptor copy_descriptor(
        const gf_property_descriptor_v1& authored) {
        gui_forms::PropertyDescriptor result;
        result.name = copy_text(authored.name, 256U, "property name");
        result.category = copy_text(authored.category, 256U, "property category");
        result.description = copy_text(
            authored.description, 4096U, "property description");
        result.converter_name = copy_text(
            authored.converter_name, 256U, "property converter name");
        result.editor_name = copy_text(
            authored.editor_name, 256U, "property editor name");
        result.kind = native_kind(authored.kind);
        result.readable =
            (authored.flags & GF_PROPERTY_READABLE) != 0U;
        result.writable =
            (authored.flags & GF_PROPERTY_WRITABLE) != 0U;
        result.browsable =
            (authored.flags & GF_PROPERTY_BROWSABLE) != 0U;
        result.nullable =
            (authored.flags & GF_PROPERTY_NULLABLE) != 0U;
        result.standard_values_exclusive =
            (authored.flags & GF_PROPERTY_STANDARD_VALUES_EXCLUSIVE) != 0U;
        result.resettable =
            (authored.flags & GF_PROPERTY_RESETTABLE) != 0U;
        result.change_notifications =
            (authored.flags & GF_PROPERTY_CHANGE_NOTIFICATIONS) != 0U;
        result.invalidation_effects = gui_forms::Dirty::paint |
                                      gui_forms::Dirty::semantics;
        if (result.name.empty() || (!result.readable && !result.writable)) {
            throw std::invalid_argument(
                "foreign property definitions require a name and access mode");
        }
        if (authored.enum_choice_count >
                gui_forms::maximum_property_enum_choices ||
            (authored.enum_choice_count != 0U &&
             authored.enum_choices == nullptr)) {
            throw std::invalid_argument(
                "foreign enum choices are missing or unbounded");
        }
        if (result.kind == gui_forms::BindingValueKind::enumeration) {
            auto enumeration =
                std::make_shared<gui_forms::PropertyEnumDescriptor>();
            enumeration->type_name = copy_text(
                authored.enum_type_name, 256U, "property enum type");
            enumeration->flags =
                (authored.flags & GF_PROPERTY_ENUM_FLAGS) != 0U;
            enumeration->choices.reserve(
                static_cast<std::size_t>(authored.enum_choice_count));
            for (std::uint64_t index = 0U;
                 index < authored.enum_choice_count; ++index) {
                enumeration->choices.push_back({
                    copy_text(authored.enum_choices[index].name,
                              gui_forms::maximum_property_enum_text_bytes,
                              "property enum choice"),
                    authored.enum_choices[index].value});
            }
            result.enumeration = std::move(enumeration);
        } else if (authored.enum_choice_count != 0U ||
                   authored.enum_type_name.size != 0U) {
            throw std::invalid_argument(
                "only enumeration properties may define enum metadata");
        }
        if (authored.standard_value_count >
                gui_forms::maximum_property_standard_values ||
            (authored.standard_value_count != 0U &&
             authored.standard_values == nullptr)) {
            throw std::invalid_argument(
                "foreign standard values are missing or unbounded");
        }
        result.standard_values.reserve(
            static_cast<std::size_t>(authored.standard_value_count));
        for (std::uint64_t index = 0U;
             index < authored.standard_value_count; ++index) {
            const gf_property_value& value = authored.standard_values[index];
            const std::string value_text = copy_text(
                value.text_value, maximum_callback_text_bytes,
                "property standard value");
            result.standard_values.push_back(from_abi(
                value, value_text, result));
        }
        return result;
    }

    struct PropertyState final {
        gui_forms::PropertyDescriptor descriptor;
        gf_property_callbacks_v1 callbacks{};
        gui_forms::Event<> changed;

        [[nodiscard]] gui_forms::BindingValue get() const {
            gf_property_value value{};
            const std::string text = callback_text(
                [&](char* buffer, std::uint64_t capacity,
                    std::uint64_t* required) {
                    return callbacks.get(callbacks.context, &value, buffer,
                                         capacity, required);
                }, "property getter");
            return from_abi(value, text, descriptor);
        }

        void set(const gui_forms::BindingValue& value) const {
            gf_property_value native = to_abi(value);
            const std::uint32_t result = callbacks.set(
                callbacks.context, &native);
            if (result != GF_OK) {
                throw std::runtime_error(
                    "property setter callback failed with result " +
                    std::to_string(result));
            }
        }

        void reset() const {
            const std::uint32_t result = callbacks.reset(callbacks.context);
            if (result != GF_OK) {
                throw std::runtime_error(
                    "property reset callback failed with result " +
                    std::to_string(result));
            }
        }

        [[nodiscard]] bool should_serialize() const {
            std::uint32_t result_value = 0U;
            const std::uint32_t result = callbacks.should_serialize(
                callbacks.context, &result_value);
            if (result != GF_OK || result_value > 1U) {
                throw std::runtime_error(
                    "property serialization callback failed or returned a non-Boolean value");
            }
            return result_value != 0U;
        }

        [[nodiscard]] std::string format(
            const gui_forms::BindingValue& value) const {
            gf_property_value native = to_abi(value);
            return callback_text(
                [&](char* buffer, std::uint64_t capacity,
                    std::uint64_t* required) {
                    return callbacks.format(callbacks.context, &native,
                                            buffer, capacity, required);
                }, "property formatter");
        }

        [[nodiscard]] std::optional<gui_forms::BindingValue> parse(
            std::string_view text) const {
            if (callbacks.parse == nullptr) return {};
            gf_property_value value{};
            const gf_string_view input{text.data(), text.size()};
            try {
                const std::string value_text = callback_text(
                    [&](char* buffer, std::uint64_t capacity,
                        std::uint64_t* required) {
                        return callbacks.parse(callbacks.context, input, &value,
                                               buffer, capacity, required);
                    }, "property parser");
                return from_abi(value, value_text, descriptor);
            } catch (const std::invalid_argument&) {
                return {};
            } catch (const std::runtime_error&) {
                return {};
            }
        }

        [[nodiscard]] gui_forms::BindingValue edit(
            const gui_forms::BindingValue& current) const {
            if (callbacks.edit == nullptr) {
                throw std::logic_error(
                    "foreign property editor callback is unavailable");
            }
            gf_property_value input = to_abi(current);
            gf_property_value output{};
            const std::string value_text = callback_text(
                [&](char* buffer, std::uint64_t capacity,
                    std::uint64_t* required) {
                    return callbacks.edit(callbacks.context, &input, &output,
                                          buffer, capacity, required);
                }, "property editor");
            return from_abi(output, value_text, descriptor);
        }
    };

    std::map<std::string, std::shared_ptr<PropertyState>> properties_;
};

} // namespace gui_forms::abi::detail
