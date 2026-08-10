#include "gui_forms/components/tool_tip/tool_tip.hpp"

#include "tool_tip_layer/tool_tip_layer.hpp"
#include "tool_tip_bubble/tool_tip_bubble.hpp"
#include "tool_tip_utilities.hpp"
#include "gui_forms/text.hpp"
#include "gui_forms/timer.hpp"
#include "gui_forms/window.hpp"

#include <algorithm>
#include <atomic>
#include <cmath>
#include <stdexcept>
#include <utility>

namespace gui_forms {

using tool_tip_detail::tool_tip_size;

namespace {
std::atomic<std::uint64_t> next_provider_id{1U};
}
struct ToolTip::Entry final {
    std::weak_ptr<Control> target;
    std::string text;
    SubscriptionToken pointer;
    SubscriptionToken focus;
    SubscriptionToken bounds;
};

struct ToolTip::TargetPointerObserver final {
    ToolTip* tool_tip{};
    std::weak_ptr<Control> target;

    void operator()(const PointerEvent& event) const {
        const std::shared_ptr<Control> retained = target.lock();
        if (retained) (*tool_tip).target_pointer(retained, event);
    }
};

struct ToolTip::TargetFocusObserver final {
    ToolTip* tool_tip{};
    std::weak_ptr<Control> target;

    void operator()(bool focused) const {
        const std::shared_ptr<Control> retained = target.lock();
        if (retained) (*tool_tip).target_focus(retained, focused);
    }
};

struct ToolTip::TargetBoundsObserver final {
    ToolTip* tool_tip{};
    std::weak_ptr<Control> target;

    void operator()(Rect) const {
        const std::shared_ptr<Control> retained = target.lock();
        if (retained) (*tool_tip).target_moved(retained);
    }
};

struct ToolTip::PopupRevocationObserver final {
    ToolTip* tool_tip{};
    std::weak_ptr<detail::WindowLifetime> window_lifetime;

    void operator()() const {
        const std::shared_ptr<detail::WindowLifetime> lifetime =
            window_lifetime.lock();
        if (lifetime && (*lifetime).window != nullptr) {
            (*tool_tip).popup_revoked();
        }
    }
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
    timer_subscription_ = (*timer_).tick().subscribe(
        *this, Delegate<>::bind<ToolTip, &ToolTip::timer_tick>(*this));
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
    const std::shared_ptr<gui_forms::detail::WindowLifetime> lifetime = window_lifetime_.lock();
    return lifetime ? (*lifetime).window : nullptr;
}

void ToolTip::require_access(std::string_view operation) const {
    if (!is_alive()) {
        throw std::logic_error("GUI.Forms cannot mutate a disposed ToolTip");
    }
    Window* owner = bound_window();
    if (!owner) {
        throw std::logic_error("GUI.Forms cannot use a ToolTip after Window shutdown");
    }
    (*owner).verify_access(operation);
}

ToolTip::Entry* ToolTip::find_entry(const Control& target) {
    const EntryMap::iterator found = entries_.find(target.runtime_id().value);
    return found == entries_.end() ? nullptr : (*found).second.get();
}

const ToolTip::Entry* ToolTip::find_entry(const Control& target) const {
    const EntryMap::const_iterator found =
        entries_.find(target.runtime_id().value);
    return found == entries_.end() ? nullptr : (*found).second.get();
}

void ToolTip::set_tool_tip(const std::shared_ptr<Control>& target,
                           std::string text) {
    require_access("ToolTip mapping mutation");
    Window* owner = bound_window();
    if (!target || !(*target).is_alive() || !(*target).attached() ||
        (*owner).find((*target).stable_id().value()).get() != target.get()) {
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
        (*existing).text = std::move(text);
        if (visible_target_.lock() == target) {
            show_now(target, keyboard_initiated_, auto_pop_delay_);
        }
        return;
    }

    std::unique_ptr<gui_forms::ToolTip::Entry> entry = std::make_unique<Entry>();
    (*entry).target = target;
    (*entry).text = std::move(text);
    const std::weak_ptr<Control> weak_target = target;
    (*entry).pointer = (*target).pointer_observed().subscribe(
        *this, TargetPointerObserver{this, weak_target});
    (*entry).focus = (*target).focus_observed().subscribe(
        *this, TargetFocusObserver{this, weak_target});
    (*entry).bounds = (*target).arranged_bounds_changed().subscribe(
        *this, TargetBoundsObserver{this, weak_target});
    entries_.emplace((*target).runtime_id().value, std::move(entry));
}

std::string ToolTip::tool_tip(const Control& target) const {
    const Entry* entry = find_entry(target);
    return entry ? (*entry).text : std::string{};
}

bool ToolTip::remove_tool_tip(const Control& target) {
    require_access("ToolTip mapping removal");
    const EntryMap::iterator found = entries_.find(target.runtime_id().value);
    if (found == entries_.end()) return false;
    if (pending_target_.lock().get() == &target) {
        pending_target_.reset();
        pending_action_ = PendingAction::none;
        if ((*timer_).enabled()) (*timer_).stop();
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

void ToolTip::set_maximum_width(double width) {
    require_access("ToolTip maximum width change");
    if (!std::isfinite(width) || width < 80.0 || width > 2048.0) {
        throw std::invalid_argument(
            "ToolTip maximum width must be finite and between 80 and 2048");
    }
    if (maximum_width_ == width) return;
    maximum_width_ = width;
    if (visible()) {
        if (const std::shared_ptr<gui_forms::Control> target = visible_target_.lock()) {
            show_now(target, keyboard_initiated_, auto_pop_delay_);
        }
    }
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
    if (!entry || !(*target).effectively_visible() ||
        (!show_always_ && !(*target).effectively_enabled())) return;
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
    if ((*timer_).enabled()) (*timer_).stop();
    pending_action_ = action;
    if (delay.count() == 0) {
        timer_tick();
        return;
    }
    (*timer_).set_interval(delay);
    (*timer_).start();
}

void ToolTip::timer_tick() {
    if ((*timer_).enabled()) (*timer_).stop();
    const PendingAction action = std::exchange(pending_action_, PendingAction::none);
    if (action == PendingAction::show) {
        const std::shared_ptr<gui_forms::Control> target = pending_target_.lock();
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
    if (!owner || !entry || !(*target).effectively_visible() ||
        (!show_always_ && !(*target).effectively_enabled())) return;

    if ((*timer_).enabled()) (*timer_).stop();
    pending_action_ = PendingAction::none;
    pending_target_.reset();

    if (visible()) close_overlay(true);
    const std::string prefix = "tooltip." + std::to_string(provider_id_) + "." +
                               std::to_string((*target).runtime_id().value);
    std::shared_ptr<gui_forms::ToolTipLayer> layer = make_control<ToolTipLayer>(StableId(prefix + ".layer"));
    (*layer).set_requested_bounds(
        {0.0, 0.0, (*owner).client_size().width, (*owner).client_size().height});
    std::shared_ptr<gui_forms::ToolTipBubble> bubble = make_control<ToolTipBubble>(StableId(prefix + ".bubble"), (*entry).text);
    const Size size = tool_tip_size((*entry).text, maximum_width_);
    (*bubble).set_content_size(size);
    (*bubble).set_requested_bounds({0.0, 0.0, size.width, size.height});
    (*layer).add_child(bubble);

    PopupToken token = (*owner).open_popup(
        target, layer, PopupOptions{.require_enabled_owner = !show_always_});
    overlay_layer_ = layer;
    overlay_bubble_ = bubble;
    visible_target_ = target;
    keyboard_initiated_ = keyboard_initiated;
    popup_ = std::make_unique<PopupHolder>(std::move(token));
    const std::weak_ptr<detail::WindowLifetime> weak_window = window_lifetime_;
    if (Event<>* closed = (*popup_).token.closed_event()) {
        popup_subscription_ = (*closed).subscribe(
            *this, PopupRevocationObserver{this, weak_window});
    }
    position_overlay();
    ToolTipEvent change{std::string((*target).stable_id().value()), (*entry).text,
                        true, keyboard_initiated};
    visibility_changed_.emit(change);
    if (duration && (*duration).count() > 0) arm(*duration, PendingAction::auto_hide);
}

void ToolTip::position_overlay() {
    Window* owner = bound_window();
    const std::shared_ptr<gui_forms::Control> target = visible_target_.lock();
    const std::shared_ptr<gui_forms::ToolTipBubble> bubble = std::dynamic_pointer_cast<ToolTipBubble>(overlay_bubble_);
    if (!owner || !target || !bubble) return;
    const Rect anchor = (*target).absolute_bounds();
    const Rect current = (*bubble).requested_bounds();
    const Size client = (*owner).client_size();
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
    (*bubble).set_requested_bounds({x, y, current.width, current.height});
}

void ToolTip::hide() {
    require_access("ToolTip hide");
    pending_target_.reset();
    pending_action_ = PendingAction::none;
    if ((*timer_).enabled()) (*timer_).stop();
    close_overlay(true);
}

bool ToolTip::visible() const noexcept {
    return popup_ && (*popup_).token.connected();
}

std::shared_ptr<Control> ToolTip::active_control() const noexcept {
    return visible_target_.lock();
}

void ToolTip::close_overlay(bool emit_change) {
    const std::shared_ptr<gui_forms::Control> target = visible_target_.lock();
    std::string text;
    if (target) {
        if (const Entry* entry = find_entry(*target)) text = (*entry).text;
    }
    const bool was_visible = visible();
    popup_subscription_.disconnect();
    std::unique_ptr<PopupHolder> popup = std::move(popup_);
    // PopupAttachment deliberately keeps weak control references so the
    // Window cannot create an ownership cycle. Keep our retained overlay tree
    // alive until disconnect lets Window detach it and remove every stable ID.
    // Releasing these first leaves an expired attachment and a poisoned ID
    // registry that faults the next tooltip for the same target.
    if (popup) (*popup).token.disconnect();
    overlay_layer_.reset();
    overlay_bubble_.reset();
    visible_target_.reset();
    if (was_visible) last_hidden_ = FrameClock::now();
    if (emit_change && was_visible && target) {
        ToolTipEvent change{std::string((*target).stable_id().value()), std::move(text),
                            false, keyboard_initiated_};
        visibility_changed_.emit(change);
    }
}

void ToolTip::popup_revoked() {
    const std::shared_ptr<gui_forms::Control> target = visible_target_.lock();
    std::string text;
    if (target) {
        if (const Entry* entry = find_entry(*target)) text = (*entry).text;
    }
    popup_subscription_.disconnect();
    popup_.reset();
    overlay_layer_.reset();
    overlay_bubble_.reset();
    visible_target_.reset();
    last_hidden_ = FrameClock::now();
    if (target) {
        ToolTipEvent change{std::string((*target).stable_id().value()), std::move(text),
                            false, keyboard_initiated_};
        visibility_changed_.emit(change);
    }
}

void ToolTip::verify_dispose_thread() {
    if (Window* owner = bound_window()) (*owner).verify_access("ToolTip disposal");
}

void ToolTip::on_dispose() noexcept {
    try {
        pending_target_.reset();
        pending_action_ = PendingAction::none;
        if (timer_ && (*timer_).is_alive()) (*timer_).dispose();
        close_overlay(false);
        entries_.clear();
    } catch (...) {
    }
    timer_subscription_.disconnect();
    timer_.reset();
    window_lifetime_.reset();
}


} // namespace gui_forms
