#include "gui_forms/c_api.h"

#include "gui_forms/basic_controls.hpp"
#include "gui_forms/control.hpp"
#include "gui_forms/range_controls.hpp"
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
#include <cmath>
#include <cstring>
#include <deque>
#include <functional>
#include <limits>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <thread>
#include <unordered_map>
#include <utility>
#include <vector>

namespace {

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
        style_.text = foreground;
        style_.disabled_text = foreground;
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

[[nodiscard]] gui_forms::Color color_from_argb(std::uint32_t argb) noexcept {
    return gui_forms::Color::rgba(
        static_cast<std::uint8_t>((argb >> 16U) & 0xffU),
        static_cast<std::uint8_t>((argb >> 8U) & 0xffU),
        static_cast<std::uint8_t>(argb & 0xffU),
        static_cast<std::uint8_t>((argb >> 24U) & 0xffU));
}

class RasterControl final : public Control {
public:
    explicit RasterControl(StableId stable_id, bool input_transparent = false)
        : Control(std::move(stable_id)), input_transparent_(input_transparent) {
        set_focusable(!input_transparent_);
    }

    [[nodiscard]] bool hit_test_local(gui_forms::Point point) const override {
        return !input_transparent_ && Control::hit_test_local(point);
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
        std::vector<std::byte> replacement;
        try {
            replacement.assign(pixels.begin(), pixels.end());
        } catch (const std::bad_alloc&) {
            return false;
        }
        bool replacement_invalidated = false;
        if (window() != nullptr) {
            const bool replacing = image_.value != 0;
            const auto loaded = replacing
                ? window()->replace_bgra32_premultiplied(
                    image_, width, height, row_bytes, replacement, *this)
                : window()->load_bgra32_premultiplied(
                    width, height, row_bytes, replacement);
            if (!loaded) {
                return false;
            }
            image_ = loaded.image;
            replacement_invalidated = replacing;
        }
        encoded_ = std::move(replacement);
        encoding_ = ImageResourceEncoding::bgra32_premultiplied;
        pixel_width_ = width;
        pixel_height_ = height;
        pixel_row_bytes_ = row_bytes;
        if (!replacement_invalidated) {
            invalidate(gui_forms::Dirty::paint | gui_forms::Dirty::semantics);
        }
        return true;
    }

    void on_paint(gui_forms::Painter& painter, Rect) override {
        if (image_.value == 0) {
            return;
        }
        const Rect bounds = committed_arranged_bounds();
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
        Control::on_attached_to_window();
        if (!encoded_.empty() && image_.value == 0) {
            const auto loaded = encoding_ == ImageResourceEncoding::png
                ? window()->load_png(encoded_)
                : window()->load_bgra32_premultiplied(
                    pixel_width_, pixel_height_, pixel_row_bytes_, encoded_);
            if (loaded) {
                image_ = loaded.image;
            }
        }
    }

    void on_detached_from_window() noexcept override {
        if (window() != nullptr && image_.value != 0) {
            static_cast<void>(window()->remove_image(image_));
        }
        image_ = {};
        Control::on_detached_from_window();
    }

private:
    bool input_transparent_{};
    std::vector<std::byte> encoded_;
    ImageResourceEncoding encoding_{ImageResourceEncoding::png};
    std::uint32_t pixel_width_{};
    std::uint32_t pixel_height_{};
    std::uint64_t pixel_row_bytes_{};
    gui_forms::ImageId image_{};
    gui_forms::Event<const RasterPointerSample&> pointer_input_;
    gui_forms::Event<const RasterKeySample&> key_input_;
};

thread_local gf_result last_error_code = GF_OK;
thread_local std::string last_error_message;

gf_result fail(gf_result result, std::string message) noexcept {
    last_error_code = result;
    try {
        last_error_message = std::move(message);
    } catch (...) {
        last_error_message.clear();
    }
    return result;
}

template <typename Operation>
gf_result translate(Operation&& operation) noexcept {
    try {
        return operation();
    } catch (const std::invalid_argument& error) {
        return fail(GF_ERROR_INVALID_ARGUMENT, error.what());
    } catch (const std::logic_error& error) {
        return fail(GF_ERROR_INVALID_ARGUMENT, error.what());
    } catch (const std::exception& error) {
        return fail(GF_ERROR_INTERNAL, error.what());
    } catch (...) {
        return fail(GF_ERROR_INTERNAL, "GUI.Forms ABI caught a non-standard exception");
    }
}

enum class SlotKind {
    empty,
    control,
    subscription,
};

struct ControlRecord;

struct SubscriptionRecord final {
    gf_event_callback callback{};
    gf_event_callback_v2 callback_v2{};
    gf_pointer_callback pointer_callback{};
    gf_key_callback key_callback{};
    gf_key_callback key_preview_callback{};
    gf_text_callback text_callback{};
    void* context{};
    std::thread::id ui_thread;
    gui_forms::SubscriptionToken native_subscription;
    std::uint32_t event_kind{};
    bool connected{true};
};

struct DispatchRecord final {
    gf_dispatch_callback callback{};
    void* context{};
};

struct ControlRecord final {
    std::shared_ptr<Control> control;
    std::thread::id ui_thread;
    std::uint32_t kind{GF_CONTROL_GENERIC};
    std::string name;
    std::string text;
    std::string host_trace;
    std::deque<DispatchRecord> dispatch_queue;
    std::uint32_t dispatch_depth{};
    std::uint32_t managed_callback_depth{};
    bool dispatch_wake_pending{};
    std::function<void()> host_wake;
    std::function<void()> host_close;
    std::function<gui_forms::HostDialogResult(
        const gui_forms::HostDialogRequest&)> host_dialog;
    std::function<gui_forms::HostServiceStatus(
        const gui_forms::HostTooltipRequest&)> host_tooltip_show;
    std::function<void()> host_tooltip_hide;
    std::function<gui_forms::HostClipboardTextResult()> host_clipboard_read;
    std::function<gui_forms::HostServiceStatus(std::string_view)> host_clipboard_write;
    std::string last_dialog_path;
    std::uint64_t callback_faults{};
    std::uint64_t dispatches{};
    std::uint64_t dispatch_turns{};
    bool close_requested{};
    bool host_running{};
    std::uint64_t external_references{1};
    std::vector<gf_event_token> subscriptions;
};

struct Slot final {
    std::uint32_t generation{1};
    SlotKind kind{SlotKind::empty};
    std::shared_ptr<ControlRecord> control;
    std::shared_ptr<SubscriptionRecord> subscription;
};

class Registry final {
public:
    gf_result create(std::uint32_t kind, gf_string_view stable_id, gf_handle* output) {
        if (output == nullptr || stable_id.data == nullptr || stable_id.size == 0U ||
            stable_id.size > static_cast<std::uint64_t>(std::numeric_limits<std::size_t>::max())) {
            return fail(GF_ERROR_INVALID_ARGUMENT,
                        "control_create requires a non-empty bounded stable ID and output");
        }
        std::string id(stable_id.data, static_cast<std::size_t>(stable_id.size));
        if (id.find('\0') != std::string::npos) {
            return fail(GF_ERROR_INVALID_ARGUMENT, "stable ID may not contain NUL bytes");
        }
        auto record = std::make_shared<ControlRecord>();
        StableId native_id(std::move(id));
        switch (kind) {
        case GF_CONTROL_FORM:
            record->control = std::make_shared<FormControl>(std::move(native_id));
            break;
        case GF_CONTROL_USER_CONTROL:
            record->control = std::make_shared<gui_forms::Panel>(std::move(native_id));
            break;
        case GF_CONTROL_PANEL: {
            auto panel = std::make_shared<gui_forms::Panel>(std::move(native_id));
            panel->set_background(gui_forms::BasicControlStyle{}.paper);
            panel->set_border_style(gui_forms::BorderStyle::line);
            record->control = std::move(panel);
            break;
        }
        case GF_CONTROL_BUTTON:
            record->control = std::make_shared<gui_forms::Button>(std::move(native_id));
            break;
        case GF_CONTROL_CHECK_BOX:
            record->control = std::make_shared<gui_forms::CheckBox>(std::move(native_id));
            break;
        case GF_CONTROL_LABEL:
            record->control = std::make_shared<gui_forms::Label>(std::move(native_id));
            break;
        case GF_CONTROL_COMBO_BOX:
            record->control = std::make_shared<FieldControl>(
                std::move(native_id), FieldControlKind::combo_box);
            break;
        case GF_CONTROL_LIST_BOX:
            record->control = std::make_shared<FieldControl>(
                std::move(native_id), FieldControlKind::list_box);
            break;
        case GF_CONTROL_TEXT_BOX:
            record->control = std::make_shared<FieldControl>(
                std::move(native_id), FieldControlKind::text_box);
            break;
        case GF_CONTROL_TRACK_BAR:
            record->control = std::make_shared<gui_forms::TrackBar>(std::move(native_id));
            break;
        case GF_CONTROL_RADIO_BUTTON:
            record->control = std::make_shared<gui_forms::RadioButton>(std::move(native_id));
            break;
        case GF_CONTROL_GROUP_BOX:
            record->control = std::make_shared<gui_forms::GroupBox>(std::move(native_id));
            break;
        case GF_CONTROL_PROGRESS_BAR:
            record->control = std::make_shared<gui_forms::ProgressBar>(std::move(native_id));
            break;
        case GF_CONTROL_LINK_LABEL:
            record->control = std::make_shared<gui_forms::LinkLabel>(std::move(native_id));
            break;
        case GF_CONTROL_PICTURE_BOX:
            record->control = std::make_shared<FieldControl>(
                std::move(native_id), FieldControlKind::picture_box);
            break;
        case GF_CONTROL_DATA_GRID_VIEW:
            record->control = std::make_shared<FieldControl>(
                std::move(native_id), FieldControlKind::data_grid);
            break;
        case GF_CONTROL_TOOL_STRIP:
            record->control = std::make_shared<FieldControl>(
                std::move(native_id), FieldControlKind::tool_strip);
            break;
        case GF_CONTROL_NUMERIC_UP_DOWN:
            record->control = std::make_shared<FieldControl>(
                std::move(native_id), FieldControlKind::numeric_up_down);
            break;
        case GF_CONTROL_INPUT_TRANSPARENT:
            record->control =
                std::make_shared<InputTransparentControl>(std::move(native_id));
            break;
        case GF_CONTROL_INPUT_TRANSPARENT_CUSTOM:
            record->control =
                std::make_shared<RasterControl>(std::move(native_id), true);
            break;
        case GF_CONTROL_CUSTOM:
            record->control = std::make_shared<RasterControl>(std::move(native_id));
            break;
        default:
            record->control = std::make_shared<Control>(std::move(native_id));
            break;
        }
        record->ui_thread = std::this_thread::get_id();
        record->kind = kind;
        std::scoped_lock lock(mutex_);
        *output = allocate_locked(SlotKind::control, record, {});
        return GF_OK;
    }

    gf_result set_string(gf_handle handle, gf_string_view input, bool is_text) {
        if ((input.size != 0U && input.data == nullptr) ||
            input.size > static_cast<std::uint64_t>(std::numeric_limits<std::size_t>::max())) {
            return fail(GF_ERROR_INVALID_ARGUMENT, "string input is not a bounded UTF-8 view");
        }
        std::string value;
        if (input.size != 0U) {
            value.assign(input.data, static_cast<std::size_t>(input.size));
        }
        if (value.find('\0') != std::string::npos) {
            return fail(GF_ERROR_INVALID_ARGUMENT, "string input may not contain NUL bytes");
        }
        if (!gui_forms::validate_utf8(value).valid()) {
            return fail(GF_ERROR_INVALID_ARGUMENT,
                        "string input must contain valid UTF-8");
        }
        std::shared_ptr<ControlRecord> record;
        if (const gf_result result = get_control(handle, record); result != GF_OK) {
            return result;
        }
        (is_text ? record->text : record->name) = value;
        if (is_text) {
            if (const auto button =
                    std::dynamic_pointer_cast<gui_forms::ButtonBase>(record->control)) {
                button->set_text(value);
            } else if (const auto label =
                           std::dynamic_pointer_cast<gui_forms::Label>(record->control)) {
                label->set_text(value);
            } else if (const auto group =
                           std::dynamic_pointer_cast<gui_forms::GroupBox>(record->control)) {
                group->set_text(value);
            } else if (const auto field =
                           std::dynamic_pointer_cast<FieldControl>(record->control)) {
                field->set_text(value);
            }
        }
        emit_changed(handle, record);
        return GF_OK;
    }

    gf_result get_string(gf_handle handle, char* buffer, std::uint64_t capacity,
                         std::uint64_t* required_size, bool is_text) {
        if (required_size == nullptr) {
            return fail(GF_ERROR_INVALID_ARGUMENT, "string read requires a size output");
        }
        std::shared_ptr<ControlRecord> record;
        if (const gf_result result = get_control(handle, record); result != GF_OK) {
            return result;
        }
        const std::string& value = is_text ? record->text : record->name;
        *required_size = value.size();
        if (capacity < value.size() || (value.size() != 0U && buffer == nullptr)) {
            return fail(GF_ERROR_BUFFER_TOO_SMALL,
                        "string buffer is smaller than the required UTF-8 byte count");
        }
        if (!value.empty()) {
            std::memcpy(buffer, value.data(), value.size());
        }
        return GF_OK;
    }

    gf_result set_enabled(gf_handle handle, std::uint32_t enabled) {
        if (enabled > 1U) {
            return fail(GF_ERROR_INVALID_ARGUMENT, "enabled must be zero or one");
        }
        std::shared_ptr<ControlRecord> record;
        if (const gf_result result = get_control(handle, record); result != GF_OK) {
            return result;
        }
        record->control->set_enabled(enabled != 0U);
        emit_changed(handle, record);
        return GF_OK;
    }

    gf_result get_enabled(gf_handle handle, std::uint32_t* enabled) {
        if (enabled == nullptr) {
            return fail(GF_ERROR_INVALID_ARGUMENT, "get_enabled requires an output");
        }
        std::shared_ptr<ControlRecord> record;
        if (const gf_result result = get_control(handle, record); result != GF_OK) {
            return result;
        }
        *enabled = record->control->enabled() ? 1U : 0U;
        return GF_OK;
    }

    gf_result set_cursor(gf_handle handle, std::uint32_t cursor_kind) {
        if (cursor_kind > GF_CURSOR_FORBIDDEN) {
            return fail(GF_ERROR_INVALID_ARGUMENT,
                        "cursor kind is outside the ABI 0.19 range");
        }
        std::shared_ptr<ControlRecord> record;
        if (const gf_result result = get_control(handle, record); result != GF_OK) {
            return result;
        }
        if (cursor_kind == GF_CURSOR_INHERIT) {
            record->control->set_cursor(std::nullopt);
        } else {
            record->control->set_cursor(static_cast<gui_forms::CursorKind>(
                cursor_kind - GF_CURSOR_ARROW));
        }
        return GF_OK;
    }

    gf_result get_cursor(gf_handle handle, std::uint32_t* cursor_kind) {
        if (cursor_kind == nullptr) {
            return fail(GF_ERROR_INVALID_ARGUMENT, "get_cursor requires an output");
        }
        std::shared_ptr<ControlRecord> record;
        if (const gf_result result = get_control(handle, record); result != GF_OK) {
            return result;
        }
        const auto value = record->control->cursor();
        *cursor_kind = value.has_value()
            ? static_cast<std::uint32_t>(*value) +
                  static_cast<std::uint32_t>(GF_CURSOR_ARROW)
            : static_cast<std::uint32_t>(GF_CURSOR_INHERIT);
        return GF_OK;
    }

    gf_result set_control_png(gf_handle handle, const std::uint8_t* encoded,
                              std::uint64_t encoded_size) {
        if ((encoded_size != 0U && encoded == nullptr) ||
            encoded_size > 32ULL * 1024ULL * 1024ULL ||
            encoded_size > static_cast<std::uint64_t>(
                std::numeric_limits<std::size_t>::max())) {
            return fail(GF_ERROR_INVALID_ARGUMENT,
                        "set_control_png requires a bounded PNG byte span");
        }
        std::shared_ptr<ControlRecord> record;
        if (const gf_result result = get_control(handle, record); result != GF_OK) {
            return result;
        }
        const auto raster = std::dynamic_pointer_cast<RasterControl>(record->control);
        if (!raster) {
            return fail(GF_ERROR_WRONG_HANDLE_KIND,
                        "set_control_png requires a custom raster control");
        }
        const auto bytes = std::span<const std::byte>(
            reinterpret_cast<const std::byte*>(encoded),
            static_cast<std::size_t>(encoded_size));
        if (!raster->set_png(bytes)) {
            return fail(GF_ERROR_INVALID_ARGUMENT,
                        "set_control_png rejected an invalid or unbounded PNG");
        }
        // Raster replacement is renderer state, not a managed property change.
        // Emitting the generic state callback here can also re-enter a managed
        // UnmanagedCallersOnly thunk when paint was requested by an ABI dispatch.
        return GF_OK;
    }

    gf_result set_control_pixels(gf_handle handle, const std::uint8_t* pixels,
                                 std::uint32_t width, std::uint32_t height,
                                 std::uint64_t row_bytes,
                                 std::uint32_t pixel_format) {
        if (pixel_format != GF_PIXEL_FORMAT_BGRA32_PREMULTIPLIED) {
            return fail(GF_ERROR_INVALID_ARGUMENT,
                        "set_control_pixels requires BGRA32 premultiplied pixels");
        }
        if ((width == 0U || height == 0U) && pixels == nullptr) {
            std::shared_ptr<ControlRecord> record;
            if (const gf_result result = get_control(handle, record); result != GF_OK) {
                return result;
            }
            const auto raster = std::dynamic_pointer_cast<RasterControl>(record->control);
            return raster && raster->set_bgra32_premultiplied(0, 0, 0, {})
                ? GF_OK
                : fail(GF_ERROR_WRONG_HANDLE_KIND,
                       "set_control_pixels requires a custom raster control");
        }
        if (pixels == nullptr || width == 0U || height == 0U ||
            row_bytes < static_cast<std::uint64_t>(width) * 4U ||
            row_bytes > std::numeric_limits<std::size_t>::max() ||
            height > std::numeric_limits<std::size_t>::max() / row_bytes) {
            return fail(GF_ERROR_INVALID_ARGUMENT,
                        "set_control_pixels requires a bounded BGRA32 surface");
        }
        const std::size_t byte_count =
            static_cast<std::size_t>(row_bytes) * height;
        std::shared_ptr<ControlRecord> record;
        if (const gf_result result = get_control(handle, record); result != GF_OK) {
            return result;
        }
        const auto raster = std::dynamic_pointer_cast<RasterControl>(record->control);
        if (!raster) {
            return fail(GF_ERROR_WRONG_HANDLE_KIND,
                        "set_control_pixels requires a custom raster control");
        }
        const auto bytes = std::span<const std::byte>(
            reinterpret_cast<const std::byte*>(pixels), byte_count);
        if (!raster->set_bgra32_premultiplied(width, height, row_bytes, bytes)) {
            return fail(GF_ERROR_INVALID_ARGUMENT,
                        "set_control_pixels rejected an invalid or unbounded surface");
        }
        return GF_OK;
    }

    gf_result set_child_index(gf_handle parent_handle, gf_handle child_handle,
                              std::uint64_t index) {
        if (index > static_cast<std::uint64_t>(
                        std::numeric_limits<std::size_t>::max())) {
            return fail(GF_ERROR_INVALID_ARGUMENT,
                        "set_child_index index exceeds the platform bound");
        }
        std::shared_ptr<ControlRecord> parent;
        std::shared_ptr<ControlRecord> child;
        if (const gf_result result = get_control(parent_handle, parent);
            result != GF_OK) {
            return result;
        }
        if (const gf_result result = get_control(child_handle, child);
            result != GF_OK) {
            return result;
        }
        if (!parent->control->set_child_index(
                child->control->runtime_id(), static_cast<std::size_t>(index))) {
            return fail(GF_ERROR_INVALID_ARGUMENT,
                        "set_child_index child is not owned by parent");
        }
        // Reordering is initiated and already observed by the managed owner.
        return GF_OK;
    }

    gf_result set_control_colors(gf_handle handle, std::uint32_t foreground_argb,
                                 std::uint32_t background_argb) {
        std::shared_ptr<ControlRecord> record;
        if (const gf_result result = get_control(handle, record); result != GF_OK) {
            return result;
        }
        const auto foreground = color_from_argb(foreground_argb);
        const auto background = color_from_argb(background_argb);
        if (const auto field = std::dynamic_pointer_cast<FieldControl>(record->control)) {
            field->set_colors(foreground, background);
        } else if (const auto label =
                       std::dynamic_pointer_cast<gui_forms::Label>(record->control)) {
            label->set_foreground(foreground);
        } else if (const auto button =
                       std::dynamic_pointer_cast<gui_forms::ButtonBase>(record->control)) {
            auto style = button->style();
            style.text = foreground;
            style.disabled_text = foreground;
            style.face = background;
            style.face_light = background;
            button->set_style(style);
        } else if (const auto panel =
                       std::dynamic_pointer_cast<gui_forms::Panel>(record->control)) {
            auto style = panel->style();
            style.text = foreground;
            style.disabled_text = foreground;
            panel->set_style(style);
            panel->set_background(background);
        }
        // Style projection does not mutate a WinForms-observable property. The
        // managed side already owns and has raised the corresponding change.
        return GF_OK;
    }

    gf_result set_field_selection(gf_handle handle, std::uint64_t start,
                                  std::uint64_t length,
                                  std::uint32_t caret_visible) {
        if (caret_visible > 1U) {
            return fail(GF_ERROR_INVALID_ARGUMENT,
                        "caret visibility must be zero or one");
        }
        std::shared_ptr<ControlRecord> record;
        if (const gf_result result = get_control(handle, record); result != GF_OK) {
            return result;
        }
        const auto field = std::dynamic_pointer_cast<FieldControl>(record->control);
        if (!field) {
            return fail(GF_ERROR_WRONG_HANDLE_KIND,
                        "field selection requires a retained field control");
        }
        if (!field->set_selection(start, length, caret_visible != 0U)) {
            return fail(GF_ERROR_INVALID_ARGUMENT,
                        "field selection exceeds the UTF-8 text extent");
        }
        return GF_OK;
    }

    gf_result set_field_edit_state(gf_handle handle, std::uint64_t anchor,
                                   std::uint64_t caret,
                                   std::uint32_t caret_visible) {
        if (caret_visible > 1U) {
            return fail(GF_ERROR_INVALID_ARGUMENT,
                        "caret visibility must be zero or one");
        }
        std::shared_ptr<ControlRecord> record;
        if (const gf_result result = get_control(handle, record); result != GF_OK) {
            return result;
        }
        const auto field = std::dynamic_pointer_cast<FieldControl>(record->control);
        if (!field) {
            return fail(GF_ERROR_WRONG_HANDLE_KIND,
                        "field edit state requires a retained field control");
        }
        if (!field->set_edit_state(anchor, caret, caret_visible != 0U)) {
            return fail(GF_ERROR_INVALID_ARGUMENT,
                        "field anchor and caret must be UTF-8 grapheme boundaries");
        }
        return GF_OK;
    }

    gf_result field_position_from_point(gf_handle handle, double local_x,
                                        std::uint64_t* position) {
        if (position == nullptr || !std::isfinite(local_x)) {
            return fail(GF_ERROR_INVALID_ARGUMENT,
                        "field hit test requires a finite point and output");
        }
        std::shared_ptr<ControlRecord> record;
        if (const gf_result result = get_control(handle, record); result != GF_OK) {
            return result;
        }
        const auto field = std::dynamic_pointer_cast<FieldControl>(record->control);
        if (!field) {
            return fail(GF_ERROR_WRONG_HANDLE_KIND,
                        "field hit test requires a retained field control");
        }
        *position = field->position_at(local_x);
        return GF_OK;
    }

    gf_result write_clipboard_text(gf_handle owner_handle, gf_string_view input) {
        std::string value;
        if (!copy_view(input, value) ||
            value.size() > gui_forms::HostServices::maximum_clipboard_text_bytes ||
            !gui_forms::validate_utf8(value).valid()) {
            return fail(GF_ERROR_INVALID_ARGUMENT,
                        "clipboard text must be bounded valid UTF-8 without NUL");
        }
        std::function<gui_forms::HostServiceStatus(std::string_view)> provider;
        {
            std::scoped_lock lock(mutex_);
            std::shared_ptr<ControlRecord> owner;
            if (const gf_result result = control_locked(owner_handle, owner);
                result != GF_OK) return result;
            if (const gf_result result = require_thread(*owner); result != GF_OK) {
                return result;
            }
            const auto root = root_record_locked(owner);
            if (!root) return fail(GF_ERROR_INVALID_ARGUMENT,
                                   "clipboard owner has no retained root");
            provider = root->host_clipboard_write;
            if (!provider) {
                clipboard_text_ = value;
                ++clipboard_generation_;
                return GF_OK;
            }
        }
        const auto status = provider(value);
        if (!status.accepted()) {
            return fail(GF_ERROR_INTERNAL,
                        std::string("clipboard host write failed: ") +
                        gui_forms::host_service_error_name(status.error));
        }
        return GF_OK;
    }

    gf_result read_clipboard_text(gf_handle owner_handle, char* buffer,
                                  std::uint64_t capacity,
                                  std::uint64_t* required_size,
                                  std::uint32_t* has_text) {
        if (required_size == nullptr || has_text == nullptr) {
            return fail(GF_ERROR_INVALID_ARGUMENT,
                        "clipboard read requires size and presence outputs");
        }
        std::function<gui_forms::HostClipboardTextResult()> provider;
        std::string value;
        bool present{};
        {
            std::scoped_lock lock(mutex_);
            std::shared_ptr<ControlRecord> owner;
            if (const gf_result result = control_locked(owner_handle, owner);
                result != GF_OK) return result;
            if (const gf_result result = require_thread(*owner); result != GF_OK) {
                return result;
            }
            const auto root = root_record_locked(owner);
            if (!root) return fail(GF_ERROR_INVALID_ARGUMENT,
                                   "clipboard owner has no retained root");
            provider = root->host_clipboard_read;
            if (!provider) {
                value = clipboard_text_;
                present = clipboard_generation_ != 0U;
            }
        }
        if (provider) {
            const auto result = provider();
            if (!result.status.accepted()) {
                return fail(GF_ERROR_INTERNAL,
                            std::string("clipboard host read failed: ") +
                            gui_forms::host_service_error_name(result.status.error));
            }
            value = result.text_utf8;
            present = result.has_text;
        }
        *required_size = value.size();
        *has_text = present ? 1U : 0U;
        if (capacity < value.size() || (value.size() != 0U && buffer == nullptr)) {
            return fail(GF_ERROR_BUFFER_TOO_SMALL,
                        "clipboard buffer is smaller than the required UTF-8 size");
        }
        if (!value.empty()) std::memcpy(buffer, value.data(), value.size());
        return GF_OK;
    }

    gf_result field_navigate(gf_handle handle, std::uint64_t position,
                             std::int32_t direction, std::uint64_t* result) {
        if (result == nullptr) {
            return fail(GF_ERROR_INVALID_ARGUMENT,
                        "field navigation requires an output position");
        }
        std::shared_ptr<ControlRecord> record;
        if (const gf_result status = get_control(handle, record); status != GF_OK) {
            return status;
        }
        const auto field = std::dynamic_pointer_cast<FieldControl>(record->control);
        if (!field) {
            return fail(GF_ERROR_WRONG_HANDLE_KIND,
                        "field navigation requires a retained field control");
        }
        if (!field->navigate(position, direction, *result)) {
            return fail(GF_ERROR_INVALID_ARGUMENT,
                        "field navigation requires a grapheme boundary and -1/+1");
        }
        return GF_OK;
    }

    gf_result field_replace(gf_handle handle, std::uint64_t start,
                            std::uint64_t length, gf_string_view input,
                            gf_field_edit_result* result) {
        if (result == nullptr) {
            return fail(GF_ERROR_INVALID_ARGUMENT,
                        "field replacement requires an edit result output");
        }
        std::string replacement;
        if (!copy_view(input, replacement) ||
            !gui_forms::validate_utf8(replacement).valid()) {
            return fail(GF_ERROR_INVALID_ARGUMENT,
                        "field replacement must be bounded valid UTF-8 without NUL");
        }
        std::shared_ptr<ControlRecord> record;
        if (const gf_result status = get_control(handle, record); status != GF_OK) {
            return status;
        }
        const auto field = std::dynamic_pointer_cast<FieldControl>(record->control);
        if (!field) {
            return fail(GF_ERROR_WRONG_HANDLE_KIND,
                        "field replacement requires a retained field control");
        }
        if (!field->replace(start, length, replacement, *result)) {
            return fail(GF_ERROR_INVALID_ARGUMENT,
                        "field replacement range must use UTF-8 grapheme boundaries");
        }
        record->text.assign(field->text());
        return GF_OK;
    }

    gf_result field_history(gf_handle handle, std::int32_t direction,
                            gf_field_edit_result* result) {
        if (result == nullptr) {
            return fail(GF_ERROR_INVALID_ARGUMENT,
                        "field history requires an edit result output");
        }
        std::shared_ptr<ControlRecord> record;
        if (const gf_result status = get_control(handle, record); status != GF_OK) {
            return status;
        }
        const auto field = std::dynamic_pointer_cast<FieldControl>(record->control);
        if (!field) {
            return fail(GF_ERROR_WRONG_HANDLE_KIND,
                        "field history requires a retained field control");
        }
        if (!field->history(direction, *result)) {
            return fail(GF_ERROR_INVALID_ARGUMENT,
                        "field history direction must be -1 (undo) or +1 (redo)");
        }
        record->text.assign(field->text());
        return GF_OK;
    }

    gf_result field_clear_history(gf_handle handle) {
        std::shared_ptr<ControlRecord> record;
        if (const gf_result status = get_control(handle, record); status != GF_OK) {
            return status;
        }
        const auto field = std::dynamic_pointer_cast<FieldControl>(record->control);
        if (!field) {
            return fail(GF_ERROR_WRONG_HANDLE_KIND,
                        "field history reset requires a retained field control");
        }
        field->clear_history();
        return GF_OK;
    }

    gf_result set_check_state(gf_handle handle, std::uint32_t check_state) {
        if (check_state > static_cast<std::uint32_t>(gui_forms::CheckState::indeterminate)) {
            return fail(GF_ERROR_INVALID_ARGUMENT,
                        "check state must be unchecked, checked, or indeterminate");
        }
        std::shared_ptr<ControlRecord> record;
        if (const gf_result result = get_control(handle, record); result != GF_OK) {
            return result;
        }
        if (const auto check_box =
                std::dynamic_pointer_cast<gui_forms::CheckBox>(record->control)) {
            check_box->set_check_state(
                static_cast<gui_forms::CheckState>(check_state));
            return GF_OK;
        }
        if (const auto radio =
                std::dynamic_pointer_cast<gui_forms::RadioButton>(record->control)) {
            if (check_state ==
                static_cast<std::uint32_t>(gui_forms::CheckState::indeterminate)) {
                return fail(GF_ERROR_INVALID_ARGUMENT,
                            "radio buttons do not support indeterminate state");
            }
            radio->set_checked(
                check_state == static_cast<std::uint32_t>(gui_forms::CheckState::checked));
            return GF_OK;
        }
        return fail(GF_ERROR_WRONG_HANDLE_KIND,
                    "check state requires a checkbox or radio button");
    }

    gf_result get_check_state(gf_handle handle, std::uint32_t* check_state) {
        if (check_state == nullptr) {
            return fail(GF_ERROR_INVALID_ARGUMENT,
                        "get_check_state requires an output");
        }
        std::shared_ptr<ControlRecord> record;
        if (const gf_result result = get_control(handle, record); result != GF_OK) {
            return result;
        }
        if (const auto check_box =
                std::dynamic_pointer_cast<gui_forms::CheckBox>(record->control)) {
            *check_state = static_cast<std::uint32_t>(check_box->check_state());
            return GF_OK;
        }
        if (const auto radio =
                std::dynamic_pointer_cast<gui_forms::RadioButton>(record->control)) {
            *check_state = radio->checked()
                ? static_cast<std::uint32_t>(gui_forms::CheckState::checked)
                : static_cast<std::uint32_t>(gui_forms::CheckState::unchecked);
            return GF_OK;
        }
        return fail(GF_ERROR_WRONG_HANDLE_KIND,
                    "check state requires a checkbox or radio button");
    }

    gf_result set_range(gf_handle handle, double minimum, double maximum) {
        std::shared_ptr<ControlRecord> record;
        if (const gf_result result = get_control(handle, record); result != GF_OK) {
            return result;
        }
        const auto range =
            std::dynamic_pointer_cast<gui_forms::RangeControl>(record->control);
        if (!range) {
            return fail(GF_ERROR_WRONG_HANDLE_KIND,
                        "range state requires a track bar or progress bar");
        }
        range->set_range(minimum, maximum);
        return GF_OK;
    }

    gf_result get_range(gf_handle handle, double* minimum, double* maximum) {
        if (minimum == nullptr || maximum == nullptr) {
            return fail(GF_ERROR_INVALID_ARGUMENT,
                        "get_range requires minimum and maximum outputs");
        }
        std::shared_ptr<ControlRecord> record;
        if (const gf_result result = get_control(handle, record); result != GF_OK) {
            return result;
        }
        const auto range =
            std::dynamic_pointer_cast<gui_forms::RangeControl>(record->control);
        if (!range) {
            return fail(GF_ERROR_WRONG_HANDLE_KIND,
                        "range state requires a track bar or progress bar");
        }
        *minimum = range->minimum();
        *maximum = range->maximum();
        return GF_OK;
    }

    gf_result set_range_value(gf_handle handle, double value) {
        std::shared_ptr<ControlRecord> record;
        if (const gf_result result = get_control(handle, record); result != GF_OK) {
            return result;
        }
        const auto range =
            std::dynamic_pointer_cast<gui_forms::RangeControl>(record->control);
        if (!range) {
            return fail(GF_ERROR_WRONG_HANDLE_KIND,
                        "range value requires a track bar or progress bar");
        }
        range->set_value(value);
        return GF_OK;
    }

    gf_result get_range_value(gf_handle handle, double* value) {
        if (value == nullptr) {
            return fail(GF_ERROR_INVALID_ARGUMENT,
                        "get_range_value requires an output");
        }
        std::shared_ptr<ControlRecord> record;
        if (const gf_result result = get_control(handle, record); result != GF_OK) {
            return result;
        }
        const auto range =
            std::dynamic_pointer_cast<gui_forms::RangeControl>(record->control);
        if (!range) {
            return fail(GF_ERROR_WRONG_HANDLE_KIND,
                        "range value requires a track bar or progress bar");
        }
        *value = range->value();
        return GF_OK;
    }

    gf_result set_pointer_capture(gf_handle handle, std::uint32_t captured) {
        if (captured > 1U) {
            return fail(GF_ERROR_INVALID_ARGUMENT,
                        "pointer capture must be zero or one");
        }
        std::shared_ptr<ControlRecord> record;
        if (const gf_result result = get_control(handle, record); result != GF_OK) {
            return result;
        }
        record->control->set_pointer_capture(captured != 0U);
        return GF_OK;
    }

    gf_result get_pointer_capture(gf_handle handle, std::uint32_t* captured) {
        if (captured == nullptr) {
            return fail(GF_ERROR_INVALID_ARGUMENT,
                        "get_pointer_capture requires an output");
        }
        std::shared_ptr<ControlRecord> record;
        if (const gf_result result = get_control(handle, record); result != GF_OK) {
            return result;
        }
        *captured = record->control->has_pointer_capture() ? 1U : 0U;
        return GF_OK;
    }

    gf_result show_path_dialog(gf_handle owner_handle, std::uint32_t kind,
                               gf_string_view title,
                               gf_string_view initial_directory,
                               gf_string_view suggested_name,
                               gf_string_view default_extension,
                               gf_string_view filter,
                               std::uint32_t flags,
                               std::uint32_t* accepted) {
        constexpr std::uint32_t known_flags =
            GF_PATH_DIALOG_ALLOW_MULTIPLE | GF_PATH_DIALOG_CONFIRM_OVERWRITE;
        if (accepted == nullptr ||
            (kind != GF_PATH_DIALOG_OPEN_FILE &&
             kind != GF_PATH_DIALOG_SAVE_FILE &&
             kind != GF_PATH_DIALOG_SELECT_FOLDER) ||
            (flags & ~known_flags) != 0U) {
            return fail(GF_ERROR_INVALID_ARGUMENT,
                        "show_path_dialog received an invalid kind, flag, or output");
        }
        std::string title_value;
        std::string directory_value;
        std::string suggested_value;
        std::string extension_value;
        std::string filter_value;
        if (!copy_view(title, title_value) ||
            !copy_view(initial_directory, directory_value) ||
            !copy_view(suggested_name, suggested_value) ||
            !copy_view(default_extension, extension_value) ||
            !copy_view(filter, filter_value)) {
            return fail(GF_ERROR_INVALID_ARGUMENT,
                        "path-dialog strings must be bounded UTF-8 views without NUL");
        }

        std::shared_ptr<ControlRecord> owner;
        std::shared_ptr<ControlRecord> root;
        std::function<gui_forms::HostDialogResult(
            const gui_forms::HostDialogRequest&)> provider;
        {
            std::scoped_lock lock(mutex_);
            if (const gf_result result = control_locked(owner_handle, owner);
                result != GF_OK) return result;
            if (const gf_result result = require_thread(*owner); result != GF_OK) {
                return result;
            }
            root = root_record_locked(owner);
            if (!root) {
                return fail(GF_ERROR_INVALID_ARGUMENT,
                            "path-dialog owner has no retained root");
            }
            root->last_dialog_path.clear();
            provider = root->host_dialog;
        }

        gui_forms::HostDialogRequest request;
        request.request_id = next_dialog_request_++;
        request.owner_id = std::string(root->control->stable_id().value());
        if (kind == GF_PATH_DIALOG_OPEN_FILE) {
            gui_forms::HostOpenFileDialogRequest payload;
            payload.title = std::move(title_value);
            payload.initial_directory = std::move(directory_value);
            payload.suggested_name = std::move(suggested_value);
            payload.filters = parse_filters(filter_value);
            payload.allow_multiple =
                (flags & GF_PATH_DIALOG_ALLOW_MULTIPLE) != 0U;
            request.payload = std::move(payload);
        } else if (kind == GF_PATH_DIALOG_SAVE_FILE) {
            gui_forms::HostSaveFileDialogRequest payload;
            payload.title = std::move(title_value);
            payload.initial_directory = std::move(directory_value);
            payload.suggested_name = std::move(suggested_value);
            payload.default_extension = std::move(extension_value);
            payload.filters = parse_filters(filter_value);
            payload.confirm_overwrite =
                (flags & GF_PATH_DIALOG_CONFIRM_OVERWRITE) != 0U;
            request.payload = std::move(payload);
        } else {
            request.payload = gui_forms::HostFolderDialogRequest{
                std::move(title_value), std::move(directory_value)};
        }

        *accepted = 0U;
        if (!provider) return GF_OK;
        const gui_forms::HostDialogResult result = provider(request);
        if (!result.status.accepted()) {
            return fail(GF_ERROR_INTERNAL,
                        std::string("path-dialog host failed: ") +
                        gui_forms::host_service_error_name(result.status.error));
        }
        const auto* paths =
            std::get_if<gui_forms::HostPathDialogResult>(&result.payload);
        if (paths == nullptr) {
            return fail(GF_ERROR_INTERNAL,
                        "path-dialog host returned the wrong result payload");
        }
        if (paths->outcome == gui_forms::HostDialogOutcome::accepted &&
            !paths->paths.empty()) {
            std::scoped_lock lock(mutex_);
            root->last_dialog_path = paths->paths.front();
            *accepted = 1U;
        }
        return GF_OK;
    }

    gf_result last_dialog_path(gf_handle owner_handle, char* buffer,
                               std::uint64_t capacity,
                               std::uint64_t* required_size) {
        if (required_size == nullptr) {
            return fail(GF_ERROR_INVALID_ARGUMENT,
                        "last_dialog_path requires a size output");
        }
        std::shared_ptr<ControlRecord> owner;
        std::string value;
        {
            std::scoped_lock lock(mutex_);
            if (const gf_result result = control_locked(owner_handle, owner);
                result != GF_OK) return result;
            if (const gf_result result = require_thread(*owner); result != GF_OK) {
                return result;
            }
            const auto root = root_record_locked(owner);
            if (!root) return fail(GF_ERROR_INVALID_ARGUMENT,
                                   "dialog owner has no retained root");
            value = root->last_dialog_path;
        }
        *required_size = value.size();
        if (capacity < value.size() || (value.size() != 0U && buffer == nullptr)) {
            return fail(GF_ERROR_BUFFER_TOO_SMALL,
                        "dialog path buffer is smaller than the required UTF-8 size");
        }
        if (!value.empty()) std::memcpy(buffer, value.data(), value.size());
        return GF_OK;
    }

    gf_result show_tooltip(gf_handle owner_handle, gf_string_view text,
                           double x, double y,
                           std::uint32_t duration_milliseconds) {
        if (!std::isfinite(x) || !std::isfinite(y)) {
            return fail(GF_ERROR_INVALID_ARGUMENT,
                        "tooltip anchor must be finite");
        }
        std::string text_value;
        if (!copy_view(text, text_value)) {
            return fail(GF_ERROR_INVALID_ARGUMENT,
                        "tooltip text must be a bounded UTF-8 view without NUL");
        }
        std::shared_ptr<ControlRecord> owner;
        std::function<gui_forms::HostServiceStatus(
            const gui_forms::HostTooltipRequest&)> provider;
        gui_forms::Point anchor;
        {
            std::scoped_lock lock(mutex_);
            if (const gf_result result = control_locked(owner_handle, owner);
                result != GF_OK) return result;
            if (const gf_result result = require_thread(*owner); result != GF_OK) {
                return result;
            }
            const auto root = root_record_locked(owner);
            if (!root) return fail(GF_ERROR_INVALID_ARGUMENT,
                                   "tooltip owner has no retained root");
            provider = root->host_tooltip_show;
            const Rect bounds = owner->control->absolute_bounds();
            anchor = {bounds.x + x, bounds.y + y};
        }
        if (!provider) return GF_OK;
        const gui_forms::HostServiceStatus result = provider(
            {std::move(text_value), anchor, duration_milliseconds});
        return result.accepted() ? GF_OK :
            fail(GF_ERROR_INTERNAL,
                 std::string("tooltip host failed: ") +
                 gui_forms::host_service_error_name(result.error));
    }

    gf_result hide_tooltip(gf_handle owner_handle) {
        std::function<void()> hide;
        {
            std::scoped_lock lock(mutex_);
            std::shared_ptr<ControlRecord> owner;
            if (const gf_result result = control_locked(owner_handle, owner);
                result != GF_OK) return result;
            if (const gf_result result = require_thread(*owner); result != GF_OK) {
                return result;
            }
            const auto root = root_record_locked(owner);
            if (!root) return fail(GF_ERROR_INVALID_ARGUMENT,
                                   "tooltip owner has no retained root");
            hide = root->host_tooltip_hide;
        }
        if (hide) hide();
        return GF_OK;
    }

    gf_result run_window(gf_handle handle, std::uint32_t flags) {
        constexpr std::uint32_t known_flags =
            GF_WINDOW_RUN_AUTOMATION_CLOSE | GF_WINDOW_RUN_FORCE_HEADLESS |
            GF_WINDOW_RUN_AUTOMATION_ACTIVATE | GF_WINDOW_RUN_POPUP;
        if ((flags & ~known_flags) != 0U) {
            return fail(GF_ERROR_INVALID_ARGUMENT, "run_window received unknown flags");
        }
        std::shared_ptr<ControlRecord> record;
        if (const gf_result result = get_control(handle, record); result != GF_OK) {
            return result;
        }
        {
            std::scoped_lock lock(mutex_);
            const bool popup = (flags & GF_WINDOW_RUN_POPUP) != 0U;
            const bool valid_root = record->kind == GF_CONTROL_FORM ||
                (popup && record->kind == GF_CONTROL_CUSTOM);
            if (record->host_running || record->control->attached() ||
                record->control->parent() || !valid_root) {
                return fail(GF_ERROR_INVALID_ARGUMENT,
                            "run_window requires an unattached form or custom popup");
            }
            record->host_running = true;
            record->close_requested = false;
            record->callback_faults = 0;
            record->dispatches = 0;
            record->dispatch_turns = 0;
        }
        auto reset = std::unique_ptr<ControlRecord, std::function<void(ControlRecord*)>>(
            record.get(), [this, record](ControlRecord*) { finish_host(record); });

        Rect requested = record->control->requested_bounds();
        Size client_size{requested.width > 0.0 ? requested.width : 960.0,
                         requested.height > 0.0 ? requested.height : 640.0};
        auto model = std::make_unique<Window>(record->control, client_size);
        const bool auto_close = (flags & GF_WINDOW_RUN_AUTOMATION_CLOSE) != 0U;
        const bool force_headless = (flags & GF_WINDOW_RUN_FORCE_HEADLESS) != 0U;
        const bool auto_activate =
            (flags & GF_WINDOW_RUN_AUTOMATION_ACTIVATE) != 0U;
        const bool popup = (flags & GF_WINDOW_RUN_POPUP) != 0U;
        const std::string title = record->text.empty()
            ? std::string("GUI.Forms Managed Surface") : record->text;

        if (!force_headless) {
#if defined(GF_C_API_HAS_WINDOWS_HOST)
            gui_forms::host::WindowsHostOptions options;
            options.title = title;
            options.initial_size = client_size;
            options.minimum_size = popup ? Size{40.0, 20.0} : Size{320.0, 200.0};
            options.popup_window = popup;
            options.quit_thread_on_close = !popup;
            options.initial_position = {requested.x, requested.y};
            options.print_metrics_on_close = false;
            options.automation_enabled = true;
            options.close_after_launch_for_testing = auto_close;
            const auto automation_controls = named_controls_snapshot(record);
            options.automation_resolve = [automation_controls](std::string_view name) {
                const auto found = automation_controls->find(std::string(name));
                return found == automation_controls->end()
                    ? std::shared_ptr<Control>{} : found->second;
            };
            options.close_request = [this, handle](gui_forms::HostCloseRequest& request) {
                request.cancel = emit_v2(handle, GF_EVENT_FORM_CLOSING) ==
                                 GF_EVENT_CALLBACK_CANCEL;
            };
            options.host_ready = [this, record](
                std::function<void()> wake,
                std::function<void()> close,
                std::function<gui_forms::HostDialogResult(
                    const gui_forms::HostDialogRequest&)> dialog,
                std::function<gui_forms::HostServiceStatus(
                    const gui_forms::HostTooltipRequest&)> tooltip_show,
                std::function<void()> tooltip_hide,
                std::function<gui_forms::HostClipboardTextResult()> clipboard_read,
                std::function<gui_forms::HostServiceStatus(
                    std::string_view)> clipboard_write) {
                publish_host(record, std::move(wake), std::move(close),
                             std::move(dialog), std::move(tooltip_show),
                             std::move(tooltip_hide), std::move(clipboard_read),
                             std::move(clipboard_write));
            };
            options.dispatch_pending = [this, record, automation_controls] {
                pump_pending(record);
                refresh_named_controls(record, *automation_controls);
            };
            options.closed = [this, handle] {
                static_cast<void>(emit_v2(handle, GF_EVENT_FORM_CLOSED));
            };
            options.final_snapshot = [this, record](std::string_view metrics,
                                                    std::string_view host) {
                record->host_trace = "{\"window\":" + std::string(metrics) +
                                     ",\"host\":" + std::string(host) +
                                     ",\"managed\":" + managed_trace(record) + "}";
            };
            const int result = gui_forms::host::run_windows(std::move(model),
                                                             std::move(options));
            return result == 0 ? GF_OK :
                fail(GF_ERROR_INTERNAL, "Win32 GUI.Forms host returned a failure");
#elif defined(GF_C_API_HAS_MACOS_HOST)
            gui_forms::host::MacHostOptions options;
            options.title = title;
            options.initial_size = client_size;
            options.minimum_size = {320.0, 200.0};
            options.print_metrics_on_close = false;
            options.close_after_launch_for_testing = auto_close;
            options.close_request = [this, handle](gui_forms::HostCloseRequest& request) {
                request.cancel = emit_v2(handle, GF_EVENT_FORM_CLOSING) ==
                                 GF_EVENT_CALLBACK_CANCEL;
            };
            options.host_ready = [this, record](
                std::function<void()> wake,
                std::function<void()> close,
                std::function<gui_forms::HostDialogResult(
                    const gui_forms::HostDialogRequest&)> dialog,
                std::function<gui_forms::HostServiceStatus(
                    const gui_forms::HostTooltipRequest&)> tooltip_show,
                std::function<void()> tooltip_hide,
                std::function<gui_forms::HostClipboardTextResult()> clipboard_read,
                std::function<gui_forms::HostServiceStatus(
                    std::string_view)> clipboard_write) {
                publish_host(record, std::move(wake), std::move(close),
                             std::move(dialog), std::move(tooltip_show),
                             std::move(tooltip_hide), std::move(clipboard_read),
                             std::move(clipboard_write));
            };
            options.dispatch_pending = [this, record] { pump_pending(record); };
            options.closed = [this, handle] {
                static_cast<void>(emit_v2(handle, GF_EVENT_FORM_CLOSED));
            };
            options.final_snapshot = [this, record](std::string_view metrics,
                                                    std::string_view host) {
                record->host_trace = "{\"window\":" + std::string(metrics) +
                                     ",\"host\":" + std::string(host) +
                                     ",\"managed\":" + managed_trace(record) + "}";
            };
            const int result = gui_forms::host::run_macos(std::move(model),
                                                           std::move(options));
            return result == 0 ? GF_OK :
                fail(GF_ERROR_INTERNAL, "AppKit GUI.Forms host returned a failure");
#endif
        }

        gui_forms::host::HeadlessHost host(*model);
        static_cast<void>(host.dispatch(gui_forms::HostAttachEvent{client_size, 1.0}, 1));
        static_cast<void>(host.dispatch(gui_forms::HostActivationEvent{true}, 2));
        model->flush();
        std::uint64_t timestamp = 3;
        if (auto_activate) {
            std::shared_ptr<Control> target = first_button(record->control);
            if (!target) target = first_pointer_control(record->control);
            if (target) {
                const Rect bounds = target->absolute_bounds();
                const gui_forms::Point center{
                    bounds.x + bounds.width / 2.0,
                    bounds.y + bounds.height / 2.0};
                static_cast<void>(host.dispatch(
                    gui_forms::PointerEvent{gui_forms::PointerAction::down,
                                            gui_forms::PointerButton::primary,
                                            center, {}, gui_forms::Modifier::none, 1},
                    timestamp++));
                static_cast<void>(host.dispatch(
                    gui_forms::PointerEvent{gui_forms::PointerAction::up,
                                            gui_forms::PointerButton::primary,
                                            center, {}, gui_forms::Modifier::none, 1},
                    timestamp++));
            }
        }
        bool dispatch_quiescent = false;
        for (std::size_t turn = 0U;
             turn < gui_forms::maximum_posted_callbacks; ++turn) {
            pump_pending(record);
            std::scoped_lock lock(mutex_);
            dispatch_quiescent = record->dispatch_queue.empty();
            if (dispatch_quiescent) break;
        }
        if (!dispatch_quiescent) {
            return fail(GF_ERROR_INTERNAL,
                        "headless BeginInvoke queue did not reach quiescence");
        }
        gui_forms::HostCloseRequest close{
            gui_forms::HostCloseReason::application, false};
        close.cancel = emit_v2(handle, GF_EVENT_FORM_CLOSING) ==
                       GF_EVENT_CALLBACK_CANCEL;
        const auto close_result = host.dispatch(close, timestamp++);
        if (close_result.accepted() && close_result.close_allowed) {
            static_cast<void>(host.dispatch(
                gui_forms::HostClosedEvent{gui_forms::HostCloseReason::application},
                timestamp++));
            static_cast<void>(emit_v2(handle, GF_EVENT_FORM_CLOSED));
        }
        static_cast<void>(host.dispatch(gui_forms::HostShutdownEvent{}, timestamp));
        record->host_trace = host.trace() + "managed=" + managed_trace(record) + "\n";
        return GF_OK;
    }

    gf_result last_host_trace(gf_handle handle, char* buffer,
                              std::uint64_t capacity,
                              std::uint64_t* required_size) {
        if (required_size == nullptr) {
            return fail(GF_ERROR_INVALID_ARGUMENT,
                        "last_host_trace requires a size output");
        }
        std::shared_ptr<ControlRecord> record;
        if (const gf_result result = get_control(handle, record); result != GF_OK) {
            return result;
        }
        *required_size = record->host_trace.size();
        if (capacity < record->host_trace.size() ||
            (!record->host_trace.empty() && buffer == nullptr)) {
            return fail(GF_ERROR_BUFFER_TOO_SMALL,
                        "host trace buffer is smaller than the required UTF-8 byte count");
        }
        if (!record->host_trace.empty()) {
            std::memcpy(buffer, record->host_trace.data(), record->host_trace.size());
        }
        return GF_OK;
    }

    gf_result retain(gf_handle handle) {
        std::scoped_lock lock(mutex_);
        std::shared_ptr<ControlRecord> record;
        if (const gf_result result = control_locked(handle, record); result != GF_OK) {
            return result;
        }
        if (const gf_result result = require_thread(*record); result != GF_OK) {
            return result;
        }
        if (record->external_references ==
            std::numeric_limits<std::uint64_t>::max()) {
            return fail(GF_ERROR_INTERNAL, "control retain count overflow");
        }
        ++record->external_references;
        return GF_OK;
    }

    gf_result release(gf_handle handle) {
        std::scoped_lock lock(mutex_);
        std::shared_ptr<ControlRecord> record;
        if (const gf_result result = control_locked(handle, record); result != GF_OK) {
            return result;
        }
        if (const gf_result result = require_thread(*record); result != GF_OK) {
            return result;
        }
        if (record->external_references == 0U) {
            return fail(GF_ERROR_STALE_HANDLE, "control handle has no external references");
        }
        --record->external_references;
        if (record->external_references == 0U) {
            invalidate_control_locked(handle, *record);
        }
        return GF_OK;
    }

    gf_result dispose(gf_handle handle) {
        std::shared_ptr<ControlRecord> record;
        std::vector<std::shared_ptr<ControlRecord>> subtree_records;
        {
            std::scoped_lock lock(mutex_);
            if (const gf_result result = control_locked(handle, record); result != GF_OK) {
                return result;
            }
            if (const gf_result result = require_thread(*record); result != GF_OK) {
                return result;
            }
            // Stale every public identity in the owned visual subtree and
            // revoke callback tokens before user-observable teardown can run.
            for (const Slot& candidate : slots_) {
                if (candidate.kind == SlotKind::control && candidate.control &&
                    contains_control(record->control, candidate.control->control)) {
                    subtree_records.push_back(candidate.control);
                }
            }
            for (const auto& subtree_record : subtree_records) {
                const auto found = std::find_if(
                    slots_.begin(), slots_.end(), [&](const Slot& candidate) {
                        return candidate.kind == SlotKind::control &&
                               candidate.control == subtree_record;
                    });
                if (found != slots_.end()) {
                    const gf_handle subtree_handle{
                        static_cast<std::uint32_t>(
                            std::distance(slots_.begin(), found) + 1),
                        found->generation};
                    invalidate_control_locked(subtree_handle, *subtree_record);
                }
            }
        }
        for (const auto& subtree_record : subtree_records) {
            cancel_pending(subtree_record);
        }
        record->control->dispose();
        return GF_OK;
    }

    gf_result component_state(gf_handle handle, std::uint32_t* output) {
        if (output == nullptr) {
            return fail(GF_ERROR_INVALID_ARGUMENT, "component_state requires an output");
        }
        std::shared_ptr<ControlRecord> record;
        if (const gf_result result = get_control(handle, record); result != GF_OK) {
            return result;
        }
        switch (record->control->component_state()) {
        case ComponentState::alive: *output = GF_COMPONENT_ALIVE; break;
        case ComponentState::disposing: *output = GF_COMPONENT_DISPOSING; break;
        case ComponentState::disposed: *output = GF_COMPONENT_DISPOSED; break;
        }
        return GF_OK;
    }

    gf_result stable_id(gf_handle handle,
                        char* buffer,
                        std::uint64_t capacity,
                        std::uint64_t* required_size) {
        if (required_size == nullptr) {
            return fail(GF_ERROR_INVALID_ARGUMENT, "stable_id requires a size output");
        }
        std::shared_ptr<ControlRecord> record;
        if (const gf_result result = get_control(handle, record); result != GF_OK) {
            return result;
        }
        const std::string_view value = record->control->stable_id().value();
        *required_size = value.size();
        if (capacity < value.size() || (value.size() != 0U && buffer == nullptr)) {
            return fail(GF_ERROR_BUFFER_TOO_SMALL,
                        "stable_id buffer is smaller than the required UTF-8 byte count");
        }
        if (!value.empty()) {
            std::memcpy(buffer, value.data(), value.size());
        }
        return GF_OK;
    }

    gf_result set_visible(gf_handle handle, std::uint32_t visible) {
        if (visible > 1U) {
            return fail(GF_ERROR_INVALID_ARGUMENT, "visible must be zero or one");
        }
        std::shared_ptr<ControlRecord> record;
        if (const gf_result result = get_control(handle, record); result != GF_OK) {
            return result;
        }
        record->control->set_visible(visible != 0U);
        emit_changed(handle, record);
        return GF_OK;
    }

    gf_result get_visible(gf_handle handle, std::uint32_t* visible) {
        if (visible == nullptr) {
            return fail(GF_ERROR_INVALID_ARGUMENT, "get_visible requires an output");
        }
        std::shared_ptr<ControlRecord> record;
        if (const gf_result result = get_control(handle, record); result != GF_OK) {
            return result;
        }
        *visible = record->control->visible() ? 1U : 0U;
        return GF_OK;
    }

    gf_result set_bounds(gf_handle handle, gf_rect bounds) {
        if (!std::isfinite(bounds.x) || !std::isfinite(bounds.y) ||
            !std::isfinite(bounds.width) || !std::isfinite(bounds.height) ||
            bounds.width < 0.0 || bounds.height < 0.0) {
            return fail(GF_ERROR_INVALID_ARGUMENT,
                        "bounds must be finite with non-negative dimensions");
        }
        std::shared_ptr<ControlRecord> record;
        if (const gf_result result = get_control(handle, record); result != GF_OK) {
            return result;
        }
        record->control->set_requested_bounds(
            {bounds.x, bounds.y, bounds.width, bounds.height});
        emit_changed(handle, record);
        return GF_OK;
    }

    gf_result get_bounds(gf_handle handle, gf_rect* bounds) {
        if (bounds == nullptr) {
            return fail(GF_ERROR_INVALID_ARGUMENT, "get_bounds requires an output");
        }
        std::shared_ptr<ControlRecord> record;
        if (const gf_result result = get_control(handle, record); result != GF_OK) {
            return result;
        }
        const Rect value = record->control->requested_bounds();
        *bounds = {value.x, value.y, value.width, value.height};
        return GF_OK;
    }

    gf_result get_control_absolute_bounds(gf_handle handle, gf_rect* bounds) {
        if (bounds == nullptr) {
            return fail(GF_ERROR_INVALID_ARGUMENT,
                        "get_control_absolute_bounds requires an output");
        }
        std::shared_ptr<ControlRecord> record;
        if (const gf_result result = get_control(handle, record); result != GF_OK) {
            return result;
        }
        Rect value = record->control->absolute_bounds();
        if (!record->control->attached()) {
            value = record->control->requested_bounds();
            for (auto ancestor = record->control->parent(); ancestor;
                 ancestor = ancestor->parent()) {
                const Rect parent_bounds = ancestor->requested_bounds();
                value.x += parent_bounds.x;
                value.y += parent_bounds.y;
            }
        }
        *bounds = {value.x, value.y, value.width, value.height};
        return GF_OK;
    }

    gf_result add_child(gf_handle parent_handle, gf_handle child_handle) {
        std::shared_ptr<ControlRecord> parent;
        std::shared_ptr<ControlRecord> child;
        if (const gf_result result = get_control(parent_handle, parent); result != GF_OK) {
            return result;
        }
        if (const gf_result result = get_control(child_handle, child); result != GF_OK) {
            return result;
        }
        if (parent->ui_thread != child->ui_thread) {
            return fail(GF_ERROR_WRONG_THREAD,
                        "visual parent and child belong to different UI threads");
        }
        parent->control->add_child(child->control);
        emit_changed(parent_handle, parent);
        return GF_OK;
    }

    gf_result remove_child(gf_handle parent_handle, gf_handle child_handle) {
        std::shared_ptr<ControlRecord> parent;
        std::shared_ptr<ControlRecord> child;
        if (const gf_result result = get_control(parent_handle, parent); result != GF_OK) {
            return result;
        }
        if (const gf_result result = get_control(child_handle, child); result != GF_OK) {
            return result;
        }
        const Control::Ptr removed = parent->control->remove_child(child->control->runtime_id());
        if (!removed) {
            return fail(GF_ERROR_INVALID_ARGUMENT,
                        "remove_child target is not a child of the supplied parent");
        }
        emit_changed(parent_handle, parent);
        return GF_OK;
    }

    gf_result subscribe(gf_handle sender_handle,
                        std::uint32_t event_kind,
                        gf_event_callback callback,
                        void* context,
                        gf_event_token* output) {
        if (event_kind != GF_EVENT_STATE_CHANGED || callback == nullptr || output == nullptr) {
            return fail(GF_ERROR_INVALID_ARGUMENT,
                        "subscribe requires the state-changed event, callback, and output");
        }
        std::scoped_lock lock(mutex_);
        std::shared_ptr<ControlRecord> sender;
        if (const gf_result result = control_locked(sender_handle, sender); result != GF_OK) {
            return result;
        }
        if (const gf_result result = require_thread(*sender); result != GF_OK) {
            return result;
        }
        auto record = std::make_shared<SubscriptionRecord>();
        record->callback = callback;
        record->context = context;
        record->ui_thread = sender->ui_thread;
        *output = allocate_locked(SlotKind::subscription, {}, record);
        sender->subscriptions.push_back(*output);
        return GF_OK;
    }

    gf_result disconnect(gf_event_token token) {
        std::scoped_lock lock(mutex_);
        std::shared_ptr<SubscriptionRecord> record;
        if (const gf_result result = subscription_locked(token, record); result != GF_OK) {
            return result;
        }
        if (record->ui_thread != std::this_thread::get_id()) {
            return fail(GF_ERROR_WRONG_THREAD,
                        "event token operation was attempted from a non-owner thread");
        }
        record->connected = false;
        invalidate_slot_locked(token);
        return GF_OK;
    }

    gf_result subscribe_v2(gf_handle sender_handle,
                           std::uint32_t event_kind,
                           gf_event_callback_v2 callback,
                           void* context,
                           gf_event_token* output) {
        if ((event_kind != GF_EVENT_CLICKED &&
             event_kind != GF_EVENT_FORM_CLOSING &&
             event_kind != GF_EVENT_FORM_CLOSED &&
             event_kind != GF_EVENT_RANGE_VALUE_CHANGED &&
             event_kind != GF_EVENT_RANGE_SCROLL) ||
            callback == nullptr || output == nullptr) {
            return fail(GF_ERROR_INVALID_ARGUMENT,
                        "subscribe_v2 requires a supported typed event, callback, and output");
        }
        std::scoped_lock lock(mutex_);
        std::shared_ptr<ControlRecord> sender;
        if (const gf_result result = control_locked(sender_handle, sender); result != GF_OK) {
            return result;
        }
        if (const gf_result result = require_thread(*sender); result != GF_OK) {
            return result;
        }
        if (event_kind == GF_EVENT_CLICKED &&
            !std::dynamic_pointer_cast<gui_forms::ButtonBase>(sender->control)) {
            return fail(GF_ERROR_WRONG_HANDLE_KIND,
                        "clicked subscriptions require a button control");
        }
        if ((event_kind == GF_EVENT_FORM_CLOSING ||
             event_kind == GF_EVENT_FORM_CLOSED) && sender->kind != GF_CONTROL_FORM) {
            return fail(GF_ERROR_WRONG_HANDLE_KIND,
                        "form lifecycle subscriptions require a form control");
        }
        const auto range =
            std::dynamic_pointer_cast<gui_forms::RangeControl>(sender->control);
        if ((event_kind == GF_EVENT_RANGE_VALUE_CHANGED ||
             event_kind == GF_EVENT_RANGE_SCROLL) && !range) {
            return fail(GF_ERROR_WRONG_HANDLE_KIND,
                        "range subscriptions require a track bar or progress bar");
        }
        auto record = std::make_shared<SubscriptionRecord>();
        record->callback_v2 = callback;
        record->context = context;
        record->ui_thread = sender->ui_thread;
        record->event_kind = event_kind;
        *output = allocate_locked(SlotKind::subscription, {}, record);
        sender->subscriptions.push_back(*output);
        if (event_kind == GF_EVENT_CLICKED) {
            auto button = std::dynamic_pointer_cast<gui_forms::ButtonBase>(sender->control);
            record->native_subscription = button->clicked().subscribe(
                [this, sender_handle](gui_forms::ButtonBase&) {
                    static_cast<void>(emit_v2(sender_handle, GF_EVENT_CLICKED));
                });
        } else if (event_kind == GF_EVENT_RANGE_VALUE_CHANGED) {
            record->native_subscription = range->value_changed().subscribe(
                [this, sender_handle](double) {
                    static_cast<void>(emit_v2(
                        sender_handle, GF_EVENT_RANGE_VALUE_CHANGED));
                });
        } else if (event_kind == GF_EVENT_RANGE_SCROLL) {
            record->native_subscription = range->scroll().subscribe(
                [this, sender_handle](const gui_forms::RangeScrollEvent&) {
                    static_cast<void>(emit_v2(sender_handle, GF_EVENT_RANGE_SCROLL));
                });
        }
        return GF_OK;
    }

    gf_result subscribe_pointer(gf_handle sender_handle,
                                gf_pointer_callback callback, void* context,
                                gf_event_token* output) {
        if (callback == nullptr || output == nullptr) {
            return fail(GF_ERROR_INVALID_ARGUMENT,
                        "subscribe_pointer requires a callback and output");
        }
        std::scoped_lock lock(mutex_);
        std::shared_ptr<ControlRecord> sender;
        if (const gf_result result = control_locked(sender_handle, sender);
            result != GF_OK) {
            return result;
        }
        if (const gf_result result = require_thread(*sender); result != GF_OK) {
            return result;
        }
        const auto raster = std::dynamic_pointer_cast<RasterControl>(sender->control);
        const auto field = std::dynamic_pointer_cast<FieldControl>(sender->control);
        auto record = std::make_shared<SubscriptionRecord>();
        record->pointer_callback = callback;
        record->context = context;
        record->ui_thread = sender->ui_thread;
        *output = allocate_locked(SlotKind::subscription, {}, record);
        sender->subscriptions.push_back(*output);
        const auto connect = [record, sender_handle](const RasterPointerSample& sample) {
                if (!record->connected || record->pointer_callback == nullptr) {
                    return;
                }
                static_cast<void>(record->pointer_callback(
                    sender_handle, sample.event_kind, sample.x, sample.y,
                    sample.wheel_delta, sample.button, record->context));
            };
        if (raster) {
            record->native_subscription = raster->pointer_input().subscribe(connect);
        } else if (field) {
            record->native_subscription = field->pointer_input().subscribe(connect);
        } else {
            const std::weak_ptr<Control> weak_control = sender->control;
            record->native_subscription = sender->control->pointer_observed().subscribe(
                [record, sender_handle, weak_control](const gui_forms::PointerEvent& event) {
                    if (!record->connected || record->pointer_callback == nullptr) return;
                    const auto control = weak_control.lock();
                    if (!control) return;
                    const Rect bounds = control->absolute_bounds();
                    std::uint32_t kind = GF_EVENT_MOUSE_MOVE;
                    switch (event.action) {
                    case gui_forms::PointerAction::down: kind = GF_EVENT_MOUSE_DOWN; break;
                    case gui_forms::PointerAction::up: kind = GF_EVENT_MOUSE_UP; break;
                    case gui_forms::PointerAction::wheel: kind = GF_EVENT_MOUSE_WHEEL; break;
                    case gui_forms::PointerAction::enter: kind = GF_EVENT_MOUSE_ENTER; break;
                    case gui_forms::PointerAction::leave: kind = GF_EVENT_MOUSE_LEAVE; break;
                    case gui_forms::PointerAction::move: break;
                    }
                    static_cast<void>(record->pointer_callback(
                        sender_handle, kind, event.position.x - bounds.x,
                        event.position.y - bounds.y,
                        event.wheel_delta.y,
                        static_cast<std::uint32_t>(event.button), record->context));
                });
        }
        return GF_OK;
    }

    gf_result subscribe_key(gf_handle sender_handle,
                            gf_key_callback callback, void* context,
                            gf_event_token* output) {
        if (callback == nullptr || output == nullptr) {
            return fail(GF_ERROR_INVALID_ARGUMENT,
                        "subscribe_key requires a callback and output");
        }
        std::scoped_lock lock(mutex_);
        std::shared_ptr<ControlRecord> sender;
        if (const gf_result result = control_locked(sender_handle, sender);
            result != GF_OK) return result;
        if (const gf_result result = require_thread(*sender); result != GF_OK) return result;
        const auto field = std::dynamic_pointer_cast<FieldControl>(sender->control);
        const auto raster = std::dynamic_pointer_cast<RasterControl>(sender->control);
        if (!field && !raster) {
            return fail(GF_ERROR_WRONG_HANDLE_KIND,
                        "key subscriptions require an interactive retained control");
        }
        auto record = std::make_shared<SubscriptionRecord>();
        record->key_callback = callback;
        record->context = context;
        record->ui_thread = sender->ui_thread;
        *output = allocate_locked(SlotKind::subscription, {}, record);
        sender->subscriptions.push_back(*output);
        auto& key_input = field ? field->key_input() : raster->key_input();
        record->native_subscription = key_input.subscribe(
            [record, sender_handle](const RasterKeySample& sample) {
                if (!record->connected || record->key_callback == nullptr) return;
                static_cast<void>(record->key_callback(
                    sender_handle, sample.event_kind, sample.physical_key,
                    sample.modifiers, sample.repeat ? 1U : 0U, record->context));
            });
        return GF_OK;
    }

    gf_result subscribe_key_preview(gf_handle sender_handle,
                                    gf_key_callback callback, void* context,
                                    gf_event_token* output) {
        if (callback == nullptr || output == nullptr) {
            return fail(GF_ERROR_INVALID_ARGUMENT,
                        "subscribe_key_preview requires a callback and output");
        }
        std::scoped_lock lock(mutex_);
        std::shared_ptr<ControlRecord> sender;
        if (const gf_result result = control_locked(sender_handle, sender);
            result != GF_OK) return result;
        if (const gf_result result = require_thread(*sender); result != GF_OK) {
            return result;
        }
        const auto form = std::dynamic_pointer_cast<FormControl>(sender->control);
        if (!form || sender->kind != GF_CONTROL_FORM) {
            return fail(GF_ERROR_WRONG_HANDLE_KIND,
                        "key preview subscriptions require a form control");
        }
        auto record = std::make_shared<SubscriptionRecord>();
        record->key_preview_callback = callback;
        record->context = context;
        record->ui_thread = sender->ui_thread;
        *output = allocate_locked(SlotKind::subscription, {}, record);
        sender->subscriptions.push_back(*output);
        record->native_subscription = form->key_preview().subscribe(
            [this, record, sender_handle, sender](RasterKeySample& sample) {
                if (!record->connected ||
                    record->key_preview_callback == nullptr) return;
                const std::uint32_t result = record->key_preview_callback(
                    sender_handle, sample.event_kind, sample.physical_key,
                    sample.modifiers, sample.repeat ? 1U : 0U,
                    record->context);
                if (result == GF_EVENT_CALLBACK_CANCEL) {
                    sample.handled = true;
                } else if (result == GF_EVENT_CALLBACK_FAULTED ||
                           result > GF_EVENT_CALLBACK_FAULTED) {
                    std::scoped_lock callback_lock(mutex_);
                    const auto root = root_record_locked(sender);
                    ++root->callback_faults;
                }
            });
        return GF_OK;
    }

    gf_result subscribe_text(gf_handle sender_handle,
                             gf_text_callback callback, void* context,
                             gf_event_token* output) {
        if (callback == nullptr || output == nullptr) {
            return fail(GF_ERROR_INVALID_ARGUMENT,
                        "subscribe_text requires a callback and output");
        }
        std::scoped_lock lock(mutex_);
        std::shared_ptr<ControlRecord> sender;
        if (const gf_result result = control_locked(sender_handle, sender);
            result != GF_OK) return result;
        if (const gf_result result = require_thread(*sender); result != GF_OK) return result;
        const auto field = std::dynamic_pointer_cast<FieldControl>(sender->control);
        if (!field) {
            return fail(GF_ERROR_WRONG_HANDLE_KIND,
                        "text subscriptions require a retained field control");
        }
        auto record = std::make_shared<SubscriptionRecord>();
        record->text_callback = callback;
        record->context = context;
        record->ui_thread = sender->ui_thread;
        *output = allocate_locked(SlotKind::subscription, {}, record);
        sender->subscriptions.push_back(*output);
        record->native_subscription = field->text_input().subscribe(
            [record, sender_handle](const RasterTextSample& sample) {
                if (!record->connected || record->text_callback == nullptr) return;
                const gf_string_view text{sample.text.data(), sample.text.size()};
                static_cast<void>(record->text_callback(
                    sender_handle, text, sample.composing ? 1U : 0U,
                    sample.replacement_start, sample.replacement_length,
                    record->context));
            });
        return GF_OK;
    }

    gf_result begin_invoke(gf_handle control_handle,
                           gf_dispatch_callback callback,
                           void* context) {
        if (callback == nullptr) {
            return fail(GF_ERROR_INVALID_ARGUMENT,
                        "begin_invoke requires a callback");
        }
        std::function<void()> wake;
        {
            std::scoped_lock lock(mutex_);
            std::shared_ptr<ControlRecord> control;
            if (const gf_result result = control_locked(control_handle, control);
                result != GF_OK) {
                return result;
            }
            const auto root = root_record_locked(control);
            if (!root || root->kind != GF_CONTROL_FORM || root->control->parent()) {
                return fail(GF_ERROR_WRONG_HANDLE_KIND,
                            "begin_invoke requires a control rooted in a top-level form");
            }
            if (root->dispatch_queue.size() >=
                gui_forms::maximum_posted_callbacks) {
                return fail(GF_ERROR_INVALID_ARGUMENT,
                            "begin_invoke reached the posted callback bound");
            }
            root->dispatch_queue.push_back({callback, context});
            if (root->dispatch_depth == 0U &&
                root->managed_callback_depth == 0U &&
                !root->dispatch_wake_pending && root->host_wake) {
                root->dispatch_wake_pending = true;
                wake = root->host_wake;
            }
        }
        if (wake) wake();
        return GF_OK;
    }

    gf_result request_close(gf_handle form_handle) {
        std::function<void()> request;
        {
            std::scoped_lock lock(mutex_);
            std::shared_ptr<ControlRecord> form;
            if (const gf_result result = control_locked(form_handle, form);
                result != GF_OK) {
                return result;
            }
            const bool closable_root = form->kind == GF_CONTROL_FORM ||
                (form->kind == GF_CONTROL_CUSTOM && form->host_running);
            if (!closable_root || form->control->parent()) {
                return fail(GF_ERROR_WRONG_HANDLE_KIND,
                            "request_close requires a running top-level form or popup");
            }
            form->close_requested = true;
            request = form->host_close;
        }
        if (request) request();
        return GF_OK;
    }

    gf_result callback_fault_count(gf_handle control_handle,
                                   std::uint64_t* count) {
        if (count == nullptr) {
            return fail(GF_ERROR_INVALID_ARGUMENT,
                        "callback_fault_count requires an output");
        }
        std::scoped_lock lock(mutex_);
        std::shared_ptr<ControlRecord> control;
        if (const gf_result result = control_locked(control_handle, control);
            result != GF_OK) {
            return result;
        }
        const auto root = root_record_locked(control);
        *count = root ? root->callback_faults : control->callback_faults;
        return GF_OK;
    }

private:
    std::shared_ptr<ControlRecord> root_record_locked(
        const std::shared_ptr<ControlRecord>& record) {
        std::shared_ptr<Control> root = record->control;
        while (const auto parent = root->parent()) root = parent;
        for (const Slot& candidate : slots_) {
            if (candidate.kind == SlotKind::control && candidate.control &&
                candidate.control->control == root) {
                return candidate.control;
            }
        }
        return record;
    }

    std::uint32_t emit_v2(gf_handle sender_handle, std::uint32_t event_kind) {
        std::vector<std::shared_ptr<SubscriptionRecord>> snapshot;
        std::shared_ptr<ControlRecord> root;
        {
            std::scoped_lock lock(mutex_);
            std::shared_ptr<ControlRecord> sender;
            if (control_locked(sender_handle, sender) != GF_OK) {
                return GF_EVENT_CALLBACK_CONTINUE;
            }
            root = root_record_locked(sender);
            snapshot.reserve(sender->subscriptions.size());
            for (const gf_event_token token : sender->subscriptions) {
                Slot* slot = slot_locked(token);
                if (slot != nullptr && slot->kind == SlotKind::subscription &&
                    slot->subscription && slot->subscription->connected &&
                    slot->subscription->event_kind == event_kind &&
                    slot->subscription->callback_v2 != nullptr) {
                    snapshot.push_back(slot->subscription);
                }
            }
        }
        {
            std::scoped_lock lock(mutex_);
            ++root->managed_callback_depth;
        }
        auto callback_guard = std::unique_ptr<ControlRecord, std::function<void(ControlRecord*)>>(
            root.get(), [this, root](ControlRecord*) {
                std::function<void()> wake;
                {
                    std::scoped_lock lock(mutex_);
                    if (root->managed_callback_depth > 0U) {
                        --root->managed_callback_depth;
                    }
                    if (root->managed_callback_depth == 0U &&
                        !root->dispatch_queue.empty() &&
                        !root->dispatch_wake_pending && root->host_wake) {
                        root->dispatch_wake_pending = true;
                        wake = root->host_wake;
                    }
                }
                if (wake) wake();
            });
        std::uint32_t aggregate = GF_EVENT_CALLBACK_CONTINUE;
        for (const auto& subscription : snapshot) {
            if (!subscription->connected) continue;
            const std::uint32_t result = subscription->callback_v2(
                sender_handle, event_kind, subscription->context);
            if (result == GF_EVENT_CALLBACK_CANCEL) {
                aggregate = GF_EVENT_CALLBACK_CANCEL;
            } else if (result == GF_EVENT_CALLBACK_FAULTED ||
                       result > GF_EVENT_CALLBACK_FAULTED) {
                std::scoped_lock lock(mutex_);
                ++root->callback_faults;
            }
        }
        return aggregate;
    }

    void pump_pending(const std::shared_ptr<ControlRecord>& root) {
        {
            std::scoped_lock lock(mutex_);
            root->dispatch_wake_pending = false;
            if (root->dispatch_depth != 0U || root->managed_callback_depth != 0U) {
                return;
            }
            ++root->dispatch_depth;
        }
        auto dispatch_guard = std::unique_ptr<ControlRecord, std::function<void(ControlRecord*)>>(
            root.get(), [this, root](ControlRecord*) {
                std::function<void()> wake;
                {
                    std::scoped_lock lock(mutex_);
                    if (root->dispatch_depth > 0U) --root->dispatch_depth;
                    if (root->dispatch_depth == 0U &&
                        root->managed_callback_depth == 0U &&
                        !root->dispatch_queue.empty() &&
                        !root->dispatch_wake_pending && root->host_wake) {
                        root->dispatch_wake_pending = true;
                        wake = root->host_wake;
                    }
                }
                if (wake) wake();
            });
        std::deque<DispatchRecord> pending;
        {
            std::scoped_lock lock(mutex_);
            pending.swap(root->dispatch_queue);
            if (!pending.empty()) ++root->dispatch_turns;
        }
        for (const DispatchRecord& dispatch : pending) {
            const std::uint32_t result = dispatch.callback(dispatch.context, 0U);
            std::scoped_lock lock(mutex_);
            ++root->dispatches;
            if (result == GF_EVENT_CALLBACK_FAULTED ||
                result > GF_EVENT_CALLBACK_FAULTED) {
                ++root->callback_faults;
            }
        }
    }

    void cancel_pending(const std::shared_ptr<ControlRecord>& root) noexcept {
        std::deque<DispatchRecord> pending;
        {
            std::scoped_lock lock(mutex_);
            root->dispatch_wake_pending = false;
            pending.swap(root->dispatch_queue);
        }
        for (const DispatchRecord& dispatch : pending) {
            try {
                static_cast<void>(dispatch.callback(dispatch.context, 1U));
            } catch (...) {
            }
        }
    }

    void publish_host(const std::shared_ptr<ControlRecord>& root,
                      std::function<void()> wake,
                      std::function<void()> request_close,
                      std::function<gui_forms::HostDialogResult(
                          const gui_forms::HostDialogRequest&)> dialog = {},
                      std::function<gui_forms::HostServiceStatus(
                          const gui_forms::HostTooltipRequest&)> tooltip_show = {},
                      std::function<void()> tooltip_hide = {},
                      std::function<gui_forms::HostClipboardTextResult()>
                          clipboard_read = {},
                      std::function<gui_forms::HostServiceStatus(std::string_view)>
                          clipboard_write = {}) {
        bool should_wake{};
        bool should_close{};
        std::function<void()> published_wake;
        std::function<void()> published_close;
        {
            std::scoped_lock lock(mutex_);
            root->host_wake = std::move(wake);
            root->host_close = std::move(request_close);
            root->host_dialog = std::move(dialog);
            root->host_tooltip_show = std::move(tooltip_show);
            root->host_tooltip_hide = std::move(tooltip_hide);
            root->host_clipboard_read = std::move(clipboard_read);
            root->host_clipboard_write = std::move(clipboard_write);
            should_wake = !root->dispatch_queue.empty() &&
                !root->dispatch_wake_pending;
            if (should_wake) root->dispatch_wake_pending = true;
            should_close = root->close_requested;
            published_wake = root->host_wake;
            published_close = root->host_close;
        }
        if (should_wake && published_wake) published_wake();
        if (should_close && published_close) published_close();
    }

    void finish_host(const std::shared_ptr<ControlRecord>& root) noexcept {
        cancel_pending(root);
        std::scoped_lock lock(mutex_);
        root->host_wake = {};
        root->host_close = {};
        root->host_dialog = {};
        root->host_tooltip_show = {};
        root->host_tooltip_hide = {};
        root->host_clipboard_read = {};
        root->host_clipboard_write = {};
        root->dispatch_wake_pending = false;
        root->host_running = false;
    }

    static bool copy_view(gf_string_view input, std::string& output) {
        if ((input.size != 0U && input.data == nullptr) ||
            input.size > static_cast<std::uint64_t>(
                std::numeric_limits<std::size_t>::max())) return false;
        output.assign(input.size == 0U ? "" : input.data,
                      static_cast<std::size_t>(input.size));
        return output.find('\0') == std::string::npos;
    }

    static std::vector<gui_forms::HostFileDialogFilter>
    parse_filters(std::string_view serialized) {
        std::vector<std::string> parts;
        std::size_t begin = 0;
        while (begin <= serialized.size()) {
            const std::size_t end = serialized.find('|', begin);
            parts.emplace_back(serialized.substr(
                begin, end == std::string_view::npos
                    ? serialized.size() - begin : end - begin));
            if (end == std::string_view::npos) break;
            begin = end + 1U;
        }
        std::vector<gui_forms::HostFileDialogFilter> result;
        for (std::size_t index = 0; index + 1U < parts.size() &&
             result.size() < gui_forms::HostServices::maximum_dialog_filters;
             index += 2U) {
            gui_forms::HostFileDialogFilter item;
            item.label = std::move(parts[index]);
            std::string_view patterns = parts[index + 1U];
            std::size_t pattern_begin = 0;
            while (pattern_begin <= patterns.size() &&
                   item.extensions.size() <
                       gui_forms::HostServices::maximum_dialog_extensions) {
                const std::size_t pattern_end = patterns.find(';', pattern_begin);
                std::string extension(patterns.substr(
                    pattern_begin, pattern_end == std::string_view::npos
                        ? patterns.size() - pattern_begin
                        : pattern_end - pattern_begin));
                while (!extension.empty() &&
                       (extension.front() == '*' || extension.front() == '.')) {
                    extension.erase(extension.begin());
                }
                if (!extension.empty()) item.extensions.push_back(std::move(extension));
                if (pattern_end == std::string_view::npos) break;
                pattern_begin = pattern_end + 1U;
            }
            result.push_back(std::move(item));
        }
        return result;
    }

    std::string managed_trace(const std::shared_ptr<ControlRecord>& root) {
        std::scoped_lock lock(mutex_);
        return "{\"callback_faults\":" + std::to_string(root->callback_faults) +
               ",\"dispatches\":" + std::to_string(root->dispatches) +
               ",\"dispatch_turns\":" + std::to_string(root->dispatch_turns) +
               ",\"close_requested\":" +
               std::string(root->close_requested ? "true" : "false") + "}";
    }

    static std::shared_ptr<gui_forms::ButtonBase> first_button(
        const std::shared_ptr<Control>& root) {
        if (const auto button =
                std::dynamic_pointer_cast<gui_forms::ButtonBase>(root)) {
            return button;
        }
        for (const auto& child : root->children()) {
            if (auto button = first_button(child)) return button;
        }
        return {};
    }

    static std::shared_ptr<Control> first_pointer_control(
        const std::shared_ptr<Control>& root) {
        if (std::dynamic_pointer_cast<RasterControl>(root) ||
            std::dynamic_pointer_cast<FieldControl>(root) ||
            std::dynamic_pointer_cast<gui_forms::RangeControl>(root)) {
            return root;
        }
        for (const auto& child : root->children()) {
            if (auto target = first_pointer_control(child)) return target;
        }
        return {};
    }

    std::shared_ptr<std::unordered_map<std::string, std::shared_ptr<Control>>>
    named_controls_snapshot(const std::shared_ptr<ControlRecord>& root) {
        auto result = std::make_shared<
            std::unordered_map<std::string, std::shared_ptr<Control>>>();
        refresh_named_controls(root, *result);
        return result;
    }

    void refresh_named_controls(
        const std::shared_ptr<ControlRecord>& root,
        std::unordered_map<std::string, std::shared_ptr<Control>>& result) {
        std::scoped_lock lock(mutex_);
        result.clear();
        for (const Slot& candidate : slots_) {
            if (candidate.kind == SlotKind::control && candidate.control &&
                !candidate.control->name.empty() &&
                contains_control(root->control, candidate.control->control)) {
                result.insert_or_assign(candidate.control->name,
                                        candidate.control->control);
            }
        }
    }

    static bool contains_control(const std::shared_ptr<Control>& root,
                                 const std::shared_ptr<Control>& candidate) {
        if (root == candidate) {
            return true;
        }
        for (const auto& child : root->children()) {
            if (contains_control(child, candidate)) {
                return true;
            }
        }
        return false;
    }

    gf_result require_thread(const ControlRecord& record) const {
        if (record.ui_thread != std::this_thread::get_id()) {
            return fail(GF_ERROR_WRONG_THREAD,
                        "control operation was attempted from a non-owner thread");
        }
        return GF_OK;
    }

    gf_result get_control(gf_handle handle, std::shared_ptr<ControlRecord>& output) {
        std::scoped_lock lock(mutex_);
        if (const gf_result result = control_locked(handle, output); result != GF_OK) {
            return result;
        }
        return require_thread(*output);
    }

    gf_result control_locked(gf_handle handle, std::shared_ptr<ControlRecord>& output) {
        Slot* slot = slot_locked(handle);
        if (slot == nullptr) {
            return fail(GF_ERROR_STALE_HANDLE, "control handle is stale");
        }
        if (slot->kind != SlotKind::control || !slot->control) {
            return fail(GF_ERROR_WRONG_HANDLE_KIND, "handle does not identify a control");
        }
        output = slot->control;
        if (!output->control->is_alive()) {
            return fail(GF_ERROR_DISPOSED, "control has been disposed");
        }
        return GF_OK;
    }

    gf_result subscription_locked(gf_event_token token,
                                  std::shared_ptr<SubscriptionRecord>& output) {
        Slot* slot = slot_locked(token);
        if (slot == nullptr) {
            return fail(GF_ERROR_STALE_HANDLE, "event token is stale");
        }
        if (slot->kind != SlotKind::subscription || !slot->subscription) {
            return fail(GF_ERROR_WRONG_HANDLE_KIND,
                        "handle does not identify an event subscription");
        }
        output = slot->subscription;
        return GF_OK;
    }

    Slot* slot_locked(gf_handle handle) {
        if (handle.slot == 0U || handle.slot > slots_.size()) {
            return nullptr;
        }
        Slot& slot = slots_[handle.slot - 1U];
        if (slot.kind == SlotKind::empty || slot.generation != handle.generation) {
            return nullptr;
        }
        return &slot;
    }

    gf_handle allocate_locked(SlotKind kind,
                              std::shared_ptr<ControlRecord> control,
                              std::shared_ptr<SubscriptionRecord> subscription) {
        auto found = std::find_if(slots_.begin(), slots_.end(),
                                  [](const Slot& slot) {
                                      return slot.kind == SlotKind::empty;
                                  });
        if (found == slots_.end()) {
            slots_.push_back({});
            found = std::prev(slots_.end());
        }
        found->kind = kind;
        found->control = std::move(control);
        found->subscription = std::move(subscription);
        return {static_cast<std::uint32_t>(std::distance(slots_.begin(), found) + 1),
                found->generation};
    }

    void invalidate_control_locked(gf_handle handle, ControlRecord& record) {
        for (const gf_event_token token : record.subscriptions) {
            if (Slot* slot = slot_locked(token);
                slot != nullptr && slot->kind == SlotKind::subscription) {
                slot->subscription->connected = false;
                invalidate_slot_locked(token);
            }
        }
        record.subscriptions.clear();
        invalidate_slot_locked(handle);
    }

    void invalidate_slot_locked(gf_handle handle) {
        Slot* slot = slot_locked(handle);
        if (slot == nullptr) {
            return;
        }
        slot->kind = SlotKind::empty;
        slot->control.reset();
        slot->subscription.reset();
        ++slot->generation;
        if (slot->generation == 0U) {
            slot->generation = 1U;
        }
    }

    void emit_changed(gf_handle sender_handle,
                      const std::shared_ptr<ControlRecord>& sender) {
        std::vector<std::shared_ptr<SubscriptionRecord>> snapshot;
        {
            std::scoped_lock lock(mutex_);
            snapshot.reserve(sender->subscriptions.size());
            for (const gf_event_token token : sender->subscriptions) {
                Slot* slot = slot_locked(token);
                if (slot != nullptr && slot->kind == SlotKind::subscription &&
                    slot->subscription && slot->subscription->connected &&
                    slot->subscription->callback != nullptr) {
                    snapshot.push_back(slot->subscription);
                }
            }
        }
        for (const auto& subscription : snapshot) {
            if (!subscription->connected) {
                continue;
            }
            const gf_event_callback callback = subscription->callback;
            void* const context = subscription->context;
            callback(sender_handle, GF_EVENT_STATE_CHANGED, context);
        }
    }

    std::mutex mutex_;
    std::vector<Slot> slots_;
    std::uint64_t next_dialog_request_{1};
    std::string clipboard_text_;
    std::uint64_t clipboard_generation_{};
};

Registry& registry() {
    static Registry instance;
    return instance;
}

gf_result api_last_error(gf_error_view* error) noexcept {
    if (error == nullptr) {
        return fail(GF_ERROR_INVALID_ARGUMENT, "last_error requires an output");
    }
    error->code = last_error_code;
    error->message = {last_error_message.data(), last_error_message.size()};
    return GF_OK;
}

gf_result api_control_create(gf_string_view id, gf_handle* control) noexcept {
    return translate([&] { return registry().create(GF_CONTROL_GENERIC, id, control); });
}
gf_result api_control_create_kind(std::uint32_t kind, gf_string_view id,
                                  gf_handle* control) noexcept {
    return translate([&] { return registry().create(kind, id, control); });
}
gf_result api_retain(gf_handle handle) noexcept {
    return translate([&] { return registry().retain(handle); });
}
gf_result api_release(gf_handle handle) noexcept {
    return translate([&] { return registry().release(handle); });
}
gf_result api_dispose(gf_handle handle) noexcept {
    return translate([&] { return registry().dispose(handle); });
}
gf_result api_component_state(gf_handle handle, std::uint32_t* state) noexcept {
    return translate([&] { return registry().component_state(handle, state); });
}
gf_result api_stable_id(gf_handle handle,
                        char* buffer,
                        std::uint64_t capacity,
                        std::uint64_t* required) noexcept {
    return translate([&] { return registry().stable_id(handle, buffer, capacity, required); });
}
gf_result api_set_visible(gf_handle handle, std::uint32_t visible) noexcept {
    return translate([&] { return registry().set_visible(handle, visible); });
}
gf_result api_get_visible(gf_handle handle, std::uint32_t* visible) noexcept {
    return translate([&] { return registry().get_visible(handle, visible); });
}
gf_result api_set_bounds(gf_handle handle, gf_rect bounds) noexcept {
    return translate([&] { return registry().set_bounds(handle, bounds); });
}
gf_result api_get_bounds(gf_handle handle, gf_rect* bounds) noexcept {
    return translate([&] { return registry().get_bounds(handle, bounds); });
}
gf_result api_get_control_absolute_bounds(gf_handle handle,
                                          gf_rect* bounds) noexcept {
    return translate([&] {
        return registry().get_control_absolute_bounds(handle, bounds);
    });
}
gf_result api_add_child(gf_handle parent, gf_handle child) noexcept {
    return translate([&] { return registry().add_child(parent, child); });
}
gf_result api_remove_child(gf_handle parent, gf_handle child) noexcept {
    return translate([&] { return registry().remove_child(parent, child); });
}
gf_result api_subscribe(gf_handle sender,
                        std::uint32_t event_kind,
                        gf_event_callback callback,
                        void* context,
                        gf_event_token* token) noexcept {
    return translate(
        [&] { return registry().subscribe(sender, event_kind, callback, context, token); });
}
gf_result api_disconnect(gf_event_token token) noexcept {
    return translate([&] { return registry().disconnect(token); });
}
gf_result api_set_name(gf_handle handle, gf_string_view value) noexcept {
    return translate([&] { return registry().set_string(handle, value, false); });
}
gf_result api_get_name(gf_handle handle, char* buffer, std::uint64_t capacity,
                       std::uint64_t* required) noexcept {
    return translate([&] { return registry().get_string(handle, buffer, capacity, required, false); });
}
gf_result api_set_text(gf_handle handle, gf_string_view value) noexcept {
    return translate([&] { return registry().set_string(handle, value, true); });
}
gf_result api_get_text(gf_handle handle, char* buffer, std::uint64_t capacity,
                       std::uint64_t* required) noexcept {
    return translate([&] { return registry().get_string(handle, buffer, capacity, required, true); });
}
gf_result api_set_enabled(gf_handle handle, std::uint32_t enabled) noexcept {
    return translate([&] { return registry().set_enabled(handle, enabled); });
}
gf_result api_get_enabled(gf_handle handle, std::uint32_t* enabled) noexcept {
    return translate([&] { return registry().get_enabled(handle, enabled); });
}
gf_result api_set_cursor(gf_handle handle, std::uint32_t cursor_kind) noexcept {
    return translate([&] { return registry().set_cursor(handle, cursor_kind); });
}
gf_result api_get_cursor(gf_handle handle, std::uint32_t* cursor_kind) noexcept {
    return translate([&] { return registry().get_cursor(handle, cursor_kind); });
}
gf_result api_run_window(gf_handle handle, std::uint32_t flags) noexcept {
    return translate([&] { return registry().run_window(handle, flags); });
}
gf_result api_last_host_trace(gf_handle handle, char* buffer,
                              std::uint64_t capacity,
                              std::uint64_t* required) noexcept {
    return translate([&] {
        return registry().last_host_trace(handle, buffer, capacity, required);
    });
}
gf_result api_subscribe_v2(gf_handle sender, std::uint32_t event_kind,
                           gf_event_callback_v2 callback, void* context,
                           gf_event_token* token) noexcept {
    return translate([&] {
        return registry().subscribe_v2(sender, event_kind, callback, context, token);
    });
}
gf_result api_begin_invoke(gf_handle control, gf_dispatch_callback callback,
                           void* context) noexcept {
    return translate([&] { return registry().begin_invoke(control, callback, context); });
}
gf_result api_request_close(gf_handle form) noexcept {
    return translate([&] { return registry().request_close(form); });
}
gf_result api_callback_fault_count(gf_handle control,
                                   std::uint64_t* count) noexcept {
    return translate([&] { return registry().callback_fault_count(control, count); });
}
gf_result api_set_control_png(gf_handle control, const std::uint8_t* encoded,
                              std::uint64_t encoded_size) noexcept {
    return translate([&] {
        return registry().set_control_png(control, encoded, encoded_size);
    });
}
gf_result api_set_control_pixels(gf_handle control, const std::uint8_t* pixels,
                                 std::uint32_t width, std::uint32_t height,
                                 std::uint64_t row_bytes,
                                 std::uint32_t pixel_format) noexcept {
    return translate([&] {
        return registry().set_control_pixels(control, pixels, width, height,
                                             row_bytes, pixel_format);
    });
}
gf_result api_set_child_index(gf_handle parent, gf_handle child,
                              std::uint64_t index) noexcept {
    return translate([&] { return registry().set_child_index(parent, child, index); });
}
gf_result api_set_control_colors(gf_handle control, std::uint32_t foreground_argb,
                                 std::uint32_t background_argb) noexcept {
    return translate([&] {
        return registry().set_control_colors(control, foreground_argb,
                                             background_argb);
    });
}
gf_result api_subscribe_pointer(gf_handle sender, gf_pointer_callback callback,
                                void* context, gf_event_token* token) noexcept {
    return translate([&] {
        return registry().subscribe_pointer(sender, callback, context, token);
    });
}
gf_result api_set_check_state(gf_handle control, std::uint32_t check_state) noexcept {
    return translate([&] { return registry().set_check_state(control, check_state); });
}
gf_result api_get_check_state(gf_handle control, std::uint32_t* check_state) noexcept {
    return translate([&] { return registry().get_check_state(control, check_state); });
}
gf_result api_subscribe_key(gf_handle sender, gf_key_callback callback,
                            void* context, gf_event_token* token) noexcept {
    return translate([&] { return registry().subscribe_key(sender, callback, context, token); });
}
gf_result api_subscribe_key_preview(gf_handle sender, gf_key_callback callback,
                                    void* context,
                                    gf_event_token* token) noexcept {
    return translate([&] {
        return registry().subscribe_key_preview(sender, callback, context, token);
    });
}
gf_result api_subscribe_text(gf_handle sender, gf_text_callback callback,
                             void* context, gf_event_token* token) noexcept {
    return translate([&] { return registry().subscribe_text(sender, callback, context, token); });
}
gf_result api_set_range(gf_handle control, double minimum, double maximum) noexcept {
    return translate([&] { return registry().set_range(control, minimum, maximum); });
}
gf_result api_get_range(gf_handle control, double* minimum, double* maximum) noexcept {
    return translate([&] { return registry().get_range(control, minimum, maximum); });
}
gf_result api_set_range_value(gf_handle control, double value) noexcept {
    return translate([&] { return registry().set_range_value(control, value); });
}
gf_result api_get_range_value(gf_handle control, double* value) noexcept {
    return translate([&] { return registry().get_range_value(control, value); });
}
gf_result api_set_pointer_capture(gf_handle control, std::uint32_t captured) noexcept {
    return translate([&] { return registry().set_pointer_capture(control, captured); });
}
gf_result api_get_pointer_capture(gf_handle control, std::uint32_t* captured) noexcept {
    return translate([&] { return registry().get_pointer_capture(control, captured); });
}
gf_result api_show_path_dialog(gf_handle owner, std::uint32_t kind,
                               gf_string_view title,
                               gf_string_view initial_directory,
                               gf_string_view suggested_name,
                               gf_string_view default_extension,
                               gf_string_view filter,
                               std::uint32_t flags,
                               std::uint32_t* accepted) noexcept {
    return translate([&] {
        return registry().show_path_dialog(owner, kind, title,
                                           initial_directory, suggested_name,
                                           default_extension, filter, flags,
                                           accepted);
    });
}
gf_result api_last_dialog_path(gf_handle owner, char* buffer,
                               std::uint64_t capacity,
                               std::uint64_t* required) noexcept {
    return translate([&] {
        return registry().last_dialog_path(owner, buffer, capacity, required);
    });
}
gf_result api_show_tooltip(gf_handle owner, gf_string_view text, double x,
                           double y,
                           std::uint32_t duration_milliseconds) noexcept {
    return translate([&] {
        return registry().show_tooltip(owner, text, x, y,
                                       duration_milliseconds);
    });
}
gf_result api_hide_tooltip(gf_handle owner) noexcept {
    return translate([&] { return registry().hide_tooltip(owner); });
}

gf_result api_set_field_selection(gf_handle control,
                                  std::uint64_t selection_start_utf8,
                                  std::uint64_t selection_length_utf8,
                                  std::uint32_t caret_visible) noexcept {
    return translate([&] {
        return registry().set_field_selection(control, selection_start_utf8,
                                              selection_length_utf8,
                                              caret_visible);
    });
}
gf_result api_set_field_edit_state(gf_handle control,
                                   std::uint64_t anchor_utf8,
                                   std::uint64_t caret_utf8,
                                   std::uint32_t caret_visible) noexcept {
    return translate([&] {
        return registry().set_field_edit_state(control, anchor_utf8, caret_utf8,
                                               caret_visible);
    });
}
gf_result api_field_position_from_point(gf_handle control, double local_x,
                                        std::uint64_t* position_utf8) noexcept {
    return translate([&] {
        return registry().field_position_from_point(control, local_x,
                                                    position_utf8);
    });
}
gf_result api_write_clipboard_text(gf_handle owner, gf_string_view text) noexcept {
    return translate([&] { return registry().write_clipboard_text(owner, text); });
}
gf_result api_read_clipboard_text(gf_handle owner, char* buffer,
                                  std::uint64_t capacity,
                                  std::uint64_t* required_size,
                                  std::uint32_t* has_text) noexcept {
    return translate([&] {
        return registry().read_clipboard_text(owner, buffer, capacity,
                                              required_size, has_text);
    });
}
gf_result api_field_navigate(gf_handle control, std::uint64_t position_utf8,
                             std::int32_t direction,
                             std::uint64_t* result_utf8) noexcept {
    return translate([&] {
        return registry().field_navigate(control, position_utf8, direction,
                                         result_utf8);
    });
}
gf_result api_field_replace(gf_handle control, std::uint64_t start_utf8,
                            std::uint64_t length_utf8,
                            gf_string_view replacement,
                            gf_field_edit_result* result) noexcept {
    return translate([&] {
        return registry().field_replace(control, start_utf8, length_utf8,
                                        replacement, result);
    });
}
gf_result api_field_history(gf_handle control, std::int32_t direction,
                            gf_field_edit_result* result) noexcept {
    return translate([&] {
        return registry().field_history(control, direction, result);
    });
}
gf_result api_field_clear_history(gf_handle control) noexcept {
    return translate([&] { return registry().field_clear_history(control); });
}

} // namespace

extern "C" GF_C_API_EXPORT gf_result gf_get_api_v0(std::uint32_t requested_version,
                                                    gf_api_v0* table) {
    if (table == nullptr || table->struct_size < sizeof(std::uint32_t) * 2U) {
        return fail(GF_ERROR_INVALID_ARGUMENT,
                    "gf_get_api_v0 requires a size-prefixed output table");
    }
    if (requested_version != GF_ABI_VERSION_0_1 &&
        requested_version != GF_ABI_VERSION_0_2 &&
        requested_version != GF_ABI_VERSION_0_3 &&
        requested_version != GF_ABI_VERSION_0_4 &&
        requested_version != GF_ABI_VERSION_0_5 &&
        requested_version != GF_ABI_VERSION_0_6 &&
        requested_version != GF_ABI_VERSION_0_7 &&
        requested_version != GF_ABI_VERSION_0_8 &&
        requested_version != GF_ABI_VERSION_0_9 &&
        requested_version != GF_ABI_VERSION_0_10 &&
        requested_version != GF_ABI_VERSION_0_11 &&
        requested_version != GF_ABI_VERSION_0_12 &&
        requested_version != GF_ABI_VERSION_0_13 &&
        requested_version != GF_ABI_VERSION_0_14 &&
        requested_version != GF_ABI_VERSION_0_15 &&
        requested_version != GF_ABI_VERSION_0_16 &&
        requested_version != GF_ABI_VERSION_0_17 &&
        requested_version != GF_ABI_VERSION_0_18 &&
        requested_version != GF_ABI_VERSION_0_19) {
        return fail(GF_ERROR_UNSUPPORTED_VERSION,
                    "requested GUI.Forms experimental ABI version is unsupported");
    }
    const std::uint32_t caller_size = table->struct_size;
    const gf_api_v0 implementation{
        sizeof(gf_api_v0),
        requested_version,
        &api_last_error,
        &api_control_create,
        &api_retain,
        &api_release,
        &api_dispose,
        &api_component_state,
        &api_stable_id,
        &api_set_visible,
        &api_get_visible,
        &api_set_bounds,
        &api_get_bounds,
        &api_add_child,
        &api_remove_child,
        &api_subscribe,
        &api_disconnect,
        &api_control_create_kind,
        &api_set_name,
        &api_get_name,
        &api_set_text,
        &api_get_text,
        &api_set_enabled,
        &api_get_enabled,
        &api_run_window,
        &api_last_host_trace,
        &api_subscribe_v2,
        &api_begin_invoke,
        &api_request_close,
        &api_callback_fault_count,
        &api_set_control_png,
        &api_set_child_index,
        &api_set_control_colors,
        &api_subscribe_pointer,
        &api_set_check_state,
        &api_get_check_state,
        &api_subscribe_key,
        &api_subscribe_text,
        &api_set_range,
        &api_get_range,
        &api_set_range_value,
        &api_get_range_value,
        &api_set_pointer_capture,
        &api_get_pointer_capture,
        &api_show_path_dialog,
        &api_last_dialog_path,
        &api_show_tooltip,
        &api_hide_tooltip,
        &api_set_field_selection,
        &api_set_field_edit_state,
        &api_field_position_from_point,
        &api_write_clipboard_text,
        &api_read_clipboard_text,
        &api_field_navigate,
        &api_field_replace,
        &api_field_history,
        &api_field_clear_history,
        &api_set_control_pixels,
        &api_get_control_absolute_bounds,
        &api_subscribe_key_preview,
        &api_set_cursor,
        &api_get_cursor,
    };
    const std::size_t copy_size = std::min<std::size_t>(caller_size, sizeof(implementation));
    std::memcpy(table, &implementation, copy_size);
    return GF_OK;
}
