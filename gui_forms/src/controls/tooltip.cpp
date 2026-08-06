#include "gui_forms/tooltip.hpp"

#include "gui_forms/basic_controls.hpp"
#include "gui_forms/text.hpp"
#include "gui_forms/timer.hpp"
#include "gui_forms/window.hpp"

#include <algorithm>
#include <atomic>
#include <cmath>
#include <stdexcept>
#include <utility>

namespace gui_forms {
namespace {

std::atomic<std::uint64_t> next_provider_id{1U};

class ToolTipLayer final : public Control {
public:
    explicit ToolTipLayer(StableId stable_id) : Control(std::move(stable_id)) {
        set_paint_plane(PaintPlane::overlay);
        set_focusable(false);
    }

    [[nodiscard]] bool hit_test_local(Point) const override { return false; }
};

class ToolTipBubble final : public Panel {
public:
    ToolTipBubble(StableId stable_id, std::string text)
        : Panel(std::move(stable_id)), text_(std::move(text)) {
        set_paint_plane(PaintPlane::overlay);
        set_focusable(false);
        set_border_style(BorderStyle::none);
        set_background(Color::rgba(255, 255, 225));
    }

    void initialize_control_tree() {
        label_ = make_control<Label>(
            StableId(std::string(stable_id().value()) + ".text"), text_);
        label_->set_paint_plane(PaintPlane::overlay);
        label_->set_font({FontRole::content, 12.0, 400, false});
        label_->set_foreground(Color::rgba(24, 31, 38));
        label_->set_text_wrapping(TextWrapping::word);
        label_->set_vertical_alignment(VerticalAlignment::center);
        add_child(label_);
    }

    void set_content_size(Size size) {
        label_->set_requested_bounds(
            {7.0, 4.0, std::max(0.0, size.width - 17.0),
             std::max(0.0, size.height - 11.0)});
    }

    void on_paint(Painter& painter, Rect) override {
        const Rect local{0.0, 0.0, committed_arranged_bounds().width,
                         committed_arranged_bounds().height};
        painter.fill_rect({3.0, 3.0, std::max(0.0, local.width - 3.0),
                           std::max(0.0, local.height - 3.0)},
                          Color::rgba(31, 42, 51, 66));
        const Rect body{0.0, 0.0, std::max(0.0, local.width - 3.0),
                        std::max(0.0, local.height - 3.0)};
        painter.fill_rect(body, Color::rgba(255, 255, 225));
        painter.stroke_rect({0.5, 0.5, std::max(0.0, body.width - 1.0),
                             std::max(0.0, body.height - 1.0)},
                            Color::rgba(104, 111, 118), 1.0);
    }

    [[nodiscard]] bool hit_test_local(Point) const override { return false; }

    [[nodiscard]] SemanticDescriptor semantic_descriptor() const override {
        SemanticDescriptor descriptor;
        descriptor.role = SemanticRole::tool_tip;
        descriptor.name = text_;
        descriptor.exposed = true;
        descriptor.include_descendants = false;
        return descriptor;
    }

private:
    std::string text_;
    std::shared_ptr<Label> label_;
};

[[nodiscard]] Size tooltip_size(std::string_view text) noexcept {
    constexpr double advance = 6.8;
    constexpr double maximum_text_width = 336.0;
    constexpr double line_height = 17.0;
    std::size_t longest = 0U;
    std::size_t lines = 1U;
    std::size_t current = 0U;
    for (char value : text) {
        if (value == '\n') {
            longest = std::max(longest, current);
            current = 0U;
            ++lines;
        } else {
            ++current;
        }
    }
    longest = std::max(longest, current);
    const double unwrapped = static_cast<double>(longest) * advance;
    const double text_width = std::clamp(unwrapped, 60.0, maximum_text_width);
    if (unwrapped > maximum_text_width) {
        lines += static_cast<std::size_t>(std::ceil(unwrapped / maximum_text_width)) - 1U;
    }
    return {text_width + 20.0, static_cast<double>(lines) * line_height + 12.0};
}

} // namespace

struct ToolTip::Entry final {
    std::weak_ptr<Control> target;
    std::string text;
    SubscriptionToken pointer;
    SubscriptionToken focus;
    SubscriptionToken bounds;
};

class ToolTip::PopupHolder final {
public:
    explicit PopupHolder(PopupToken value) : token(std::move(value)) {}
    PopupToken token;
};

ToolTip::ToolTip(Window& window)
    : window_lifetime_(window.lifetime_), timer_(std::make_unique<Timer>(window)),
      provider_id_(next_provider_id.fetch_add(1U)) {
    window.verify_access("ToolTip construction");
    timer_subscription_ = timer_->tick().subscribe(*this, [this] { timer_tick(); });
}

ToolTip::~ToolTip() {
    if (is_alive()) {
        try {
            dispose();
        } catch (...) {
            popup_.reset();
            timer_.reset();
        }
    }
}

Window* ToolTip::bound_window() const noexcept {
    const auto lifetime = window_lifetime_.lock();
    return lifetime ? lifetime->window : nullptr;
}

void ToolTip::require_access(std::string_view operation) const {
    if (!is_alive()) {
        throw std::logic_error("GUI.Forms cannot mutate a disposed ToolTip");
    }
    Window* owner = bound_window();
    if (!owner) {
        throw std::logic_error("GUI.Forms cannot use a ToolTip after Window shutdown");
    }
    owner->verify_access(operation);
}

ToolTip::Entry* ToolTip::find_entry(const Control& target) {
    const auto found = entries_.find(target.runtime_id().value);
    return found == entries_.end() ? nullptr : found->second.get();
}

const ToolTip::Entry* ToolTip::find_entry(const Control& target) const {
    const auto found = entries_.find(target.runtime_id().value);
    return found == entries_.end() ? nullptr : found->second.get();
}

void ToolTip::set_tool_tip(const std::shared_ptr<Control>& target,
                           std::string text) {
    require_access("ToolTip mapping mutation");
    Window* owner = bound_window();
    if (!target || !target->is_alive() || !target->attached() ||
        owner->find(target->stable_id().value()).get() != target.get()) {
        throw std::invalid_argument(
            "GUI.Forms ToolTip target must be a live attached control in its Window");
    }
    if (!validate_utf8(text).valid()) {
        throw std::invalid_argument("GUI.Forms ToolTip text must be valid UTF-8");
    }
    if (text.empty()) {
        static_cast<void>(remove_tool_tip(*target));
        return;
    }
    if (Entry* existing = find_entry(*target)) {
        existing->text = std::move(text);
        if (visible_target_.lock() == target) {
            show_now(target, keyboard_initiated_, auto_pop_delay_);
        }
        return;
    }

    auto entry = std::make_unique<Entry>();
    entry->target = target;
    entry->text = std::move(text);
    const std::weak_ptr<Control> weak_target = target;
    entry->pointer = target->pointer_observed().subscribe(
        *this, [this, weak_target](const PointerEvent& event) {
            if (const auto control = weak_target.lock()) target_pointer(control, event);
        });
    entry->focus = target->focus_observed().subscribe(
        *this, [this, weak_target](bool focused) {
            if (const auto control = weak_target.lock()) target_focus(control, focused);
        });
    entry->bounds = target->arranged_bounds_changed().subscribe(
        *this, [this, weak_target](Rect) {
            if (const auto control = weak_target.lock()) target_moved(control);
        });
    entries_.emplace(target->runtime_id().value, std::move(entry));
}

std::string ToolTip::tool_tip(const Control& target) const {
    const Entry* entry = find_entry(target);
    return entry ? entry->text : std::string{};
}

bool ToolTip::remove_tool_tip(const Control& target) {
    require_access("ToolTip mapping removal");
    const auto found = entries_.find(target.runtime_id().value);
    if (found == entries_.end()) return false;
    if (pending_target_.lock().get() == &target) {
        pending_target_.reset();
        pending_action_ = PendingAction::none;
        if (timer_->enabled()) timer_->stop();
    }
    if (visible_target_.lock().get() == &target) close_overlay(true);
    entries_.erase(found);
    return true;
}

void ToolTip::clear() {
    require_access("ToolTip mapping clear");
    hide();
    entries_.clear();
}

void ToolTip::set_initial_delay(std::chrono::milliseconds delay) {
    require_access("ToolTip initial-delay mutation");
    if (delay.count() < 0) throw std::invalid_argument("ToolTip delay may not be negative");
    initial_delay_ = delay;
}

void ToolTip::set_reshow_delay(std::chrono::milliseconds delay) {
    require_access("ToolTip reshow-delay mutation");
    if (delay.count() < 0) throw std::invalid_argument("ToolTip delay may not be negative");
    reshow_delay_ = delay;
}

void ToolTip::set_auto_pop_delay(std::chrono::milliseconds delay) {
    require_access("ToolTip auto-pop mutation");
    if (delay.count() < 0) throw std::invalid_argument("ToolTip delay may not be negative");
    auto_pop_delay_ = delay;
}

void ToolTip::set_show_on_hover(bool value) {
    require_access("ToolTip hover policy mutation");
    show_on_hover_ = value;
}

void ToolTip::set_show_on_focus(bool value) {
    require_access("ToolTip focus policy mutation");
    show_on_focus_ = value;
}

void ToolTip::set_show_always(bool value) {
    require_access("ToolTip enabled-target policy mutation");
    show_always_ = value;
}

void ToolTip::target_pointer(const std::shared_ptr<Control>& target,
                             const PointerEvent& event) {
    if (!show_on_hover_) return;
    if (event.action == PointerAction::enter) {
        schedule_show(target, false, event.position);
    } else if (event.action == PointerAction::move) {
        pointer_position_ = event.position;
    } else if (event.action == PointerAction::leave) {
        if (pending_target_.lock() == target || visible_target_.lock() == target) hide();
    }
}

void ToolTip::target_focus(const std::shared_ptr<Control>& target, bool focused) {
    if (!show_on_focus_) return;
    if (focused) {
        schedule_show(target, true, {});
    } else if (pending_target_.lock() == target || visible_target_.lock() == target) {
        hide();
    }
}

void ToolTip::target_moved(const std::shared_ptr<Control>& target) {
    if (visible_target_.lock() == target) position_overlay();
}

void ToolTip::schedule_show(const std::shared_ptr<Control>& target,
                            bool keyboard_initiated, Point pointer_position) {
    const Entry* entry = find_entry(*target);
    if (!entry || !target->effectively_visible() ||
        (!show_always_ && !target->effectively_enabled())) return;
    pending_target_ = target;
    keyboard_initiated_ = keyboard_initiated;
    anchor_to_target_ = keyboard_initiated;
    pointer_position_ = pointer_position;
    const FrameTime now = FrameClock::now();
    const bool recently_visible = last_hidden_ != FrameTime{} &&
        now - last_hidden_ <= initial_delay_;
    arm(recently_visible ? reshow_delay_ : initial_delay_, PendingAction::show);
}

void ToolTip::arm(std::chrono::milliseconds delay, PendingAction action) {
    if (timer_->enabled()) timer_->stop();
    pending_action_ = action;
    if (delay.count() == 0) {
        timer_tick();
        return;
    }
    timer_->set_interval(delay);
    timer_->start();
}

void ToolTip::timer_tick() {
    if (timer_->enabled()) timer_->stop();
    const PendingAction action = std::exchange(pending_action_, PendingAction::none);
    if (action == PendingAction::show) {
        const auto target = pending_target_.lock();
        pending_target_.reset();
        if (target) show_now(target, keyboard_initiated_, auto_pop_delay_);
    } else if (action == PendingAction::auto_hide) {
        close_overlay(true);
    }
}

void ToolTip::show(const std::shared_ptr<Control>& target) {
    show(target, auto_pop_delay_);
}

void ToolTip::show(const std::shared_ptr<Control>& target,
                   std::chrono::milliseconds duration) {
    require_access("ToolTip explicit show");
    if (duration.count() < 0) {
        throw std::invalid_argument("ToolTip duration may not be negative");
    }
    if (!target || !find_entry(*target)) {
        throw std::invalid_argument("ToolTip explicit show requires a mapped target");
    }
    anchor_to_target_ = true;
    show_now(target, false, duration);
}

void ToolTip::show_now(const std::shared_ptr<Control>& target,
                       bool keyboard_initiated,
                       std::optional<std::chrono::milliseconds> duration) {
    Window* owner = bound_window();
    Entry* entry = target ? find_entry(*target) : nullptr;
    if (!owner || !entry || !target->effectively_visible() ||
        (!show_always_ && !target->effectively_enabled())) return;

    if (timer_->enabled()) timer_->stop();
    pending_action_ = PendingAction::none;
    pending_target_.reset();

    if (visible()) close_overlay(true);
    const std::string prefix = "tooltip." + std::to_string(provider_id_) + "." +
                               std::to_string(target->runtime_id().value);
    auto layer = make_control<ToolTipLayer>(StableId(prefix + ".layer"));
    layer->set_requested_bounds(
        {0.0, 0.0, owner->client_size().width, owner->client_size().height});
    auto bubble = make_control<ToolTipBubble>(StableId(prefix + ".bubble"), entry->text);
    const Size size = tooltip_size(entry->text);
    bubble->set_content_size(size);
    bubble->set_requested_bounds({0.0, 0.0, size.width, size.height});
    layer->add_child(bubble);

    PopupToken token = owner->open_popup(
        target, layer, PopupOptions{.require_enabled_owner = !show_always_});
    overlay_layer_ = layer;
    overlay_bubble_ = bubble;
    visible_target_ = target;
    keyboard_initiated_ = keyboard_initiated;
    popup_ = std::make_unique<PopupHolder>(std::move(token));
    const std::weak_ptr<detail::WindowLifetime> weak_window = window_lifetime_;
    if (Event<>* closed = popup_->token.closed_event()) {
        popup_subscription_ = closed->subscribe(*this, [this, weak_window] {
            if (const auto lifetime = weak_window.lock(); lifetime && lifetime->window) {
                popup_revoked();
            }
        });
    }
    position_overlay();
    ToolTipEvent change{std::string(target->stable_id().value()), entry->text,
                        true, keyboard_initiated};
    visibility_changed_.emit(change);
    if (duration && duration->count() > 0) arm(*duration, PendingAction::auto_hide);
}

void ToolTip::position_overlay() {
    Window* owner = bound_window();
    const auto target = visible_target_.lock();
    const auto bubble = std::dynamic_pointer_cast<ToolTipBubble>(overlay_bubble_);
    if (!owner || !target || !bubble) return;
    const Rect anchor = target->absolute_bounds();
    const Rect current = bubble->requested_bounds();
    const Size client = owner->client_size();
    double x = anchor_to_target_ ? anchor.x + 8.0 : pointer_position_.x + 14.0;
    double y = anchor_to_target_ ? anchor.y + anchor.height + 7.0
                                 : pointer_position_.y + 19.0;
    if (x + current.width > client.width - 4.0) {
        x = std::max(4.0, client.width - current.width - 4.0);
    }
    if (y + current.height > client.height - 4.0) {
        y = anchor_to_target_ ? anchor.y - current.height - 7.0
                              : pointer_position_.y - current.height - 9.0;
    }
    y = std::clamp(y, 4.0, std::max(4.0, client.height - current.height - 4.0));
    x = std::clamp(x, 4.0, std::max(4.0, client.width - current.width - 4.0));
    bubble->set_requested_bounds({x, y, current.width, current.height});
}

void ToolTip::hide() {
    require_access("ToolTip hide");
    pending_target_.reset();
    pending_action_ = PendingAction::none;
    if (timer_->enabled()) timer_->stop();
    close_overlay(true);
}

bool ToolTip::visible() const noexcept {
    return popup_ && popup_->token.connected();
}

std::shared_ptr<Control> ToolTip::active_control() const noexcept {
    return visible_target_.lock();
}

void ToolTip::close_overlay(bool emit_change) {
    const auto target = visible_target_.lock();
    std::string text;
    if (target) {
        if (const Entry* entry = find_entry(*target)) text = entry->text;
    }
    const bool was_visible = visible();
    popup_subscription_.disconnect();
    auto popup = std::move(popup_);
    overlay_layer_.reset();
    overlay_bubble_.reset();
    visible_target_.reset();
    if (popup) popup->token.disconnect();
    if (was_visible) last_hidden_ = FrameClock::now();
    if (emit_change && was_visible && target) {
        ToolTipEvent change{std::string(target->stable_id().value()), std::move(text),
                            false, keyboard_initiated_};
        visibility_changed_.emit(change);
    }
}

void ToolTip::popup_revoked() {
    const auto target = visible_target_.lock();
    std::string text;
    if (target) {
        if (const Entry* entry = find_entry(*target)) text = entry->text;
    }
    popup_subscription_.disconnect();
    popup_.reset();
    overlay_layer_.reset();
    overlay_bubble_.reset();
    visible_target_.reset();
    last_hidden_ = FrameClock::now();
    if (target) {
        ToolTipEvent change{std::string(target->stable_id().value()), std::move(text),
                            false, keyboard_initiated_};
        visibility_changed_.emit(change);
    }
}

void ToolTip::verify_dispose_thread() {
    if (Window* owner = bound_window()) owner->verify_access("ToolTip disposal");
}

void ToolTip::on_dispose() noexcept {
    try {
        pending_target_.reset();
        pending_action_ = PendingAction::none;
        if (timer_ && timer_->is_alive()) timer_->dispose();
        close_overlay(false);
        entries_.clear();
    } catch (...) {
    }
    timer_subscription_.disconnect();
    timer_.reset();
    window_lifetime_.reset();
}

} // namespace gui_forms
