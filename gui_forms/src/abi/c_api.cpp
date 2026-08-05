#include "gui_forms/c_api.h"

#include "gui_forms/basic_controls.hpp"
#include "gui_forms/control.hpp"
#include "gui_forms/range_controls.hpp"
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

// ABI-facing field controls are deliberately small retained visuals. They keep
// the platform host renderer-neutral while the corresponding managed facade
// owns editing, selection, item, and binding behavior.
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
        invalidate(gui_forms::Dirty::paint | gui_forms::Dirty::semantics);
    }

    void on_paint(gui_forms::Painter& painter, Rect damage) override {
        Panel::on_paint(painter, damage);
        const Rect bounds = local_bounds();
        const auto style = style_;
        const double button_width = kind_ == FieldControlKind::numeric_up_down
            ? std::min(18.0, std::max(0.0, bounds.width)) : 0.0;
        if (!text_.empty()) {
            painter.draw_text_utf8({5.0, std::max(14.0, bounds.height * 0.5 + 4.0)},
                                   text_,
                                   {gui_forms::FontRole::content, 12.0, 400, false},
                                   enabled() ? style.text : style.disabled_text);
        }
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
    }

    [[nodiscard]] gui_forms::Event<const RasterPointerSample&>&
    pointer_input() noexcept {
        return pointer_input_;
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

private:
    std::string text_;
    FieldControlKind kind_;
    gui_forms::BasicControlStyle style_;
    gui_forms::Event<const RasterPointerSample&> pointer_input_;
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
        : Control(std::move(stable_id)), input_transparent_(input_transparent) {}

    [[nodiscard]] bool hit_test_local(gui_forms::Point point) const override {
        return !input_transparent_ && Control::hit_test_local(point);
    }

    [[nodiscard]] gui_forms::Event<const RasterPointerSample&>& pointer_input() noexcept {
        return pointer_input_;
    }

    bool set_png(std::span<const std::byte> encoded) {
        require_mutable();
        if (encoded.empty()) {
            if (window() != nullptr && image_.value != 0) {
                static_cast<void>(window()->remove_image(image_));
            }
            image_ = {};
            encoded_.clear();
            invalidate(gui_forms::Dirty::paint | gui_forms::Dirty::semantics);
            return true;
        }
        const auto validation = gui_forms::validate_png(encoded);
        if (!validation) {
            return false;
        }
        encoded_.assign(encoded.begin(), encoded.end());
        if (window() != nullptr) {
            const auto loaded = image_.value == 0
                ? window()->load_png(encoded_)
                : window()->replace_png(image_, encoded_);
            if (!loaded) {
                return false;
            }
            image_ = loaded.image;
        }
        invalidate(gui_forms::Dirty::paint | gui_forms::Dirty::semantics);
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

protected:
    void on_attached_to_window() override {
        Control::on_attached_to_window();
        if (!encoded_.empty() && image_.value == 0) {
            const auto loaded = window()->load_png(encoded_);
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
    gui_forms::ImageId image_{};
    gui_forms::Event<const RasterPointerSample&> pointer_input_;
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
    std::function<void()> host_wake;
    std::function<void()> host_close;
    std::uint64_t callback_faults{};
    std::uint64_t dispatches{};
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
            options.host_ready = [this, record](std::function<void()> wake,
                                                std::function<void()> close) {
                publish_host(record, std::move(wake), std::move(close));
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
            options.host_ready = [this, record](std::function<void()> wake,
                                                std::function<void()> close) {
                publish_host(record, std::move(wake), std::move(close));
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
        pump_pending(record);
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
             event_kind != GF_EVENT_FORM_CLOSED) ||
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
        if (!raster && !field) {
            return fail(GF_ERROR_WRONG_HANDLE_KIND,
                        "pointer subscriptions require a field or custom raster control");
        }
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
        record->native_subscription = raster
            ? raster->pointer_input().subscribe(connect)
            : field->pointer_input().subscribe(connect);
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
            root->dispatch_queue.push_back({callback, context});
            wake = root->host_wake;
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
        for (;;) {
            std::deque<DispatchRecord> pending;
            {
                std::scoped_lock lock(mutex_);
                pending.swap(root->dispatch_queue);
            }
            if (pending.empty()) return;
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
    }

    void cancel_pending(const std::shared_ptr<ControlRecord>& root) noexcept {
        std::deque<DispatchRecord> pending;
        {
            std::scoped_lock lock(mutex_);
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
                      std::function<void()> request_close) {
        bool should_wake{};
        bool should_close{};
        std::function<void()> published_wake;
        std::function<void()> published_close;
        {
            std::scoped_lock lock(mutex_);
            root->host_wake = std::move(wake);
            root->host_close = std::move(request_close);
            should_wake = !root->dispatch_queue.empty();
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
        root->host_running = false;
    }

    std::string managed_trace(const std::shared_ptr<ControlRecord>& root) {
        std::scoped_lock lock(mutex_);
        return "{\"callback_faults\":" + std::to_string(root->callback_faults) +
               ",\"dispatches\":" + std::to_string(root->dispatches) +
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
            std::dynamic_pointer_cast<FieldControl>(root)) {
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
        requested_version != GF_ABI_VERSION_0_7) {
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
    };
    const std::size_t copy_size = std::min<std::size_t>(caller_size, sizeof(implementation));
    std::memcpy(table, &implementation, copy_size);
    return GF_OK;
}
