#include "gui_forms/components/error_provider/error_provider.hpp"

#include "../error_layer/error_layer.hpp"
#include "../error_glyph/error_glyph.hpp"
#include "../guidance_utilities.hpp"
#include "gui_forms/binding.hpp"
#include "gui_forms/components/tool_tip/tool_tip.hpp"
#include "gui_forms/control.hpp"
#include "gui_forms/text.hpp"
#include "gui_forms/window.hpp"

#include <algorithm>
#include <atomic>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <unordered_set>
#include <utility>

namespace gui_forms {

using namespace guidance_detail;
struct ErrorProvider::Entry final {
    std::weak_ptr<Control> target;
    std::string error;
    ErrorIconAlignment alignment{ErrorIconAlignment::middle_right};
    double padding{};
    std::shared_ptr<ErrorLayer> layer;
    std::shared_ptr<ErrorGlyph> glyph;
    std::unique_ptr<PopupToken> popup;
    SubscriptionToken bounds_subscription;
};

ErrorProvider::ErrorProvider(Window& window)
    : window_lifetime_(window.lifetime_), tool_tip_(std::make_unique<ToolTip>(window)),
      provider_id_(next_error_provider_id.fetch_add(1U)) {
    window.verify_access("ErrorProvider construction");
    tool_tip_->set_initial_delay(std::chrono::milliseconds(250));
    tool_tip_->set_show_on_focus(false);
    tool_tip_->set_show_always(true);
    availability_subscription_ = window.control_availability_changed().subscribe(
        *this, [this](const ControlAvailabilityChange&) {
            refresh_all_visuals(false);
        });
    presentation_subscription_ = window.presentation_changed().subscribe(
        *this, [this](const PresentationSettings&) {
            refresh_all_visuals(false);
        });
    root_bounds_subscription_ = window.root()->arranged_bounds_changed().subscribe(
        *this, [this](Rect) {
            if (Window* owner = bound_window()) {
                for (auto& [id, entry] : entries_) {
                    static_cast<void>(id);
                    if (entry->layer) {
                        entry->layer->set_requested_bounds(
                            {0.0, 0.0, owner->client_size().width,
                             owner->client_size().height});
                    }
                    position_visual(*entry);
                }
            }
        });
}

ErrorProvider::~ErrorProvider() {
    if (is_alive()) {
        try {
            dispose();
        } catch (...) {
            tool_tip_.reset();
            entries_.clear();
        }
    }
}

Window* ErrorProvider::bound_window() const noexcept {
    const auto lifetime = window_lifetime_.lock();
    return lifetime ? lifetime->window : nullptr;
}

void ErrorProvider::require_access(std::string_view operation) const {
    if (!is_alive()) {
        throw std::logic_error("GUI.Forms cannot use a disposed ErrorProvider");
    }
    Window* owner = bound_window();
    if (!owner) {
        throw std::logic_error("GUI.Forms cannot use an ErrorProvider after Window shutdown");
    }
    owner->verify_access(operation);
}

bool ErrorProvider::can_extend(const std::shared_ptr<Control>& target) const {
    Window* owner = bound_window();
    if (!owner || !target || !target->is_alive()) return false;
    owner->verify_access("ErrorProvider target query");
    return target->attached_window() == owner &&
           owner->find(target->stable_id().value()).get() == target.get();
}

ErrorProvider::Entry* ErrorProvider::find_entry(const Control& target) {
    const auto found = entries_.find(target.runtime_id().value);
    return found == entries_.end() ? nullptr : found->second.get();
}

const ErrorProvider::Entry* ErrorProvider::find_entry(const Control& target) const {
    const auto found = entries_.find(target.runtime_id().value);
    return found == entries_.end() ? nullptr : found->second.get();
}

ErrorProvider::Entry& ErrorProvider::require_entry(
    const std::shared_ptr<Control>& target) {
    if (!can_extend(target)) {
        throw std::invalid_argument(
            "GUI.Forms ErrorProvider target must be live and attached to its Window");
    }
    if (Entry* existing = find_entry(*target)) return *existing;
    auto entry = std::make_unique<Entry>();
    entry->target = target;
    const std::weak_ptr<Control> weak_target = target;
    entry->bounds_subscription = target->arranged_bounds_changed().subscribe(
        *this, [this, weak_target](Rect) {
            if (const auto current = weak_target.lock()) {
                if (Entry* mapped = find_entry(*current)) position_visual(*mapped);
            }
        });
    Entry& result = *entry;
    entries_.emplace(target->runtime_id().value, std::move(entry));
    return result;
}

void ErrorProvider::set_error(const std::shared_ptr<Control>& target,
                              std::string error_value) {
    require_access("ErrorProvider error mutation");
    if (!validate_utf8(error_value).valid()) {
        throw std::invalid_argument("GUI.Forms error text must be valid UTF-8");
    }
    Entry& entry = require_entry(target);
    if (entry.error == error_value) return;
    entry.error = std::move(error_value);
    if (entry.error.empty()) {
        target->clear_provider_error(provider_id_);
    } else {
        target->set_provider_error(provider_id_, entry.error);
    }
    refresh_visual(entry, true);
    const ErrorProviderChange change{
        std::string(target->stable_id().value()), entry.error, !entry.error.empty()};
    error_changed_.emit(change);
    erase_if_empty(target->runtime_id().value);
}

std::string ErrorProvider::error(const Control& target) const {
    if (Window* owner = bound_window()) owner->verify_access("ErrorProvider error query");
    const Entry* entry = find_entry(target);
    return entry ? entry->error : std::string{};
}

void ErrorProvider::clear() {
    require_access("ErrorProvider clear");
    for (auto& [id, entry] : entries_) {
        static_cast<void>(id);
        if (const auto target = entry->target.lock(); target && target->is_alive()) {
            target->clear_provider_error(provider_id_);
        }
        close_visual(*entry);
    }
    entries_.clear();
}

bool ErrorProvider::has_errors() const noexcept {
    return std::any_of(entries_.begin(), entries_.end(),
                       [this](const auto& pair) {
        const Entry& entry = *pair.second;
        const auto target = entry.target.lock();
        return !entry.error.empty() && target && target->is_alive() &&
               target->attached_window() == bound_window();
    });
}

void ErrorProvider::set_icon_alignment(const std::shared_ptr<Control>& target,
                                       ErrorIconAlignment alignment) {
    require_access("ErrorProvider icon-alignment mutation");
    if (!valid_alignment(alignment)) {
        throw std::invalid_argument("GUI.Forms error icon alignment is invalid");
    }
    Entry& entry = require_entry(target);
    if (entry.alignment == alignment) return;
    entry.alignment = alignment;
    position_visual(entry);
}

ErrorIconAlignment ErrorProvider::icon_alignment(const Control& target) const {
    if (Window* owner = bound_window()) {
        owner->verify_access("ErrorProvider icon-alignment query");
    }
    const Entry* entry = find_entry(target);
    return entry ? entry->alignment : ErrorIconAlignment::middle_right;
}

void ErrorProvider::set_icon_padding(const std::shared_ptr<Control>& target,
                                     double padding) {
    require_access("ErrorProvider icon-padding mutation");
    if (!std::isfinite(padding) || padding < -1024.0 || padding > 1024.0) {
        throw std::invalid_argument(
            "GUI.Forms error icon padding must be finite and bounded");
    }
    Entry& entry = require_entry(target);
    if (entry.padding == padding) return;
    entry.padding = padding;
    position_visual(entry);
}

double ErrorProvider::icon_padding(const Control& target) const {
    if (Window* owner = bound_window()) {
        owner->verify_access("ErrorProvider icon-padding query");
    }
    const Entry* entry = find_entry(target);
    return entry ? entry->padding : 0.0;
}

void ErrorProvider::set_icon_size(double size) {
    require_access("ErrorProvider icon-size mutation");
    if (!std::isfinite(size) || size < 8.0 || size > 64.0) {
        throw std::invalid_argument(
            "ErrorProvider icon size must be finite and between 8 and 64");
    }
    if (icon_size_ == size) return;
    icon_size_ = size;
    refresh_all_visuals(false);
}

void ErrorProvider::set_blink_rate(std::chrono::milliseconds rate) {
    require_access("ErrorProvider blink-rate mutation");
    if (rate < std::chrono::milliseconds(50) ||
        rate > std::chrono::minutes(1)) {
        throw std::invalid_argument(
            "GUI.Forms ErrorProvider blink rate must be between 50 ms and 1 minute");
    }
    if (blink_rate_ == rate) return;
    blink_rate_ = rate;
    refresh_all_visuals(false);
}

void ErrorProvider::set_blink_style(ErrorBlinkStyle style) {
    require_access("ErrorProvider blink-style mutation");
    if (!valid_blink_style(style)) {
        throw std::invalid_argument("GUI.Forms ErrorProvider blink style is invalid");
    }
    if (blink_style_ == style) return;
    blink_style_ = style;
    refresh_all_visuals(true);
}

void ErrorProvider::set_right_to_left(bool value) {
    require_access("ErrorProvider direction mutation");
    if (right_to_left_ == value) return;
    right_to_left_ = value;
    refresh_all_visuals(false);
    right_to_left_changed_.emit(value);
}

void ErrorProvider::set_icon(std::optional<ImageId> icon_value) {
    require_access("ErrorProvider icon mutation");
    Window* owner = bound_window();
    if (icon_value && (icon_value->value == 0U || !owner ||
                       !owner->image_resources().find(*icon_value))) {
        throw std::invalid_argument(
            "GUI.Forms ErrorProvider icon must identify a live Window image");
    }
    if (icon_ == icon_value) return;
    icon_ = icon_value;
    refresh_all_visuals(false);
}

std::shared_ptr<Control> ErrorProvider::container_control() const noexcept {
    Window* owner = bound_window();
    return owner ? owner->root() : std::shared_ptr<Control>{};
}

void ErrorProvider::set_data_source(std::shared_ptr<BindingSource> source) {
    require_access("ErrorProvider data-source mutation");
    if (data_source_.lock() == source) {
        update_binding();
        return;
    }
    if (source && (!source->is_alive() || source->bound_window() != bound_window())) {
        throw std::invalid_argument(
            "GUI.Forms ErrorProvider data source must be live and owned by its Window");
    }
    source_list_subscription_.disconnect();
    source_current_subscription_.disconnect();
    source_completion_subscription_.disconnect();
    source_disposed_subscription_.disconnect();
    clear_bound_errors();
    binding_errors_.clear();
    data_source_ = source;
    if (source) {
        source_list_subscription_ = source->list_changed().subscribe(
            *this, [this](const BindingListChange&) { update_binding(); });
        source_current_subscription_ = source->current_changed().subscribe(
            *this, [this] {
                binding_errors_.clear();
                update_binding();
            });
        source_completion_subscription_ = source->binding_complete().subscribe(
            *this, [this](BindingCompleteEvent& event) {
                binding_completed(event);
            });
        source_disposed_subscription_ = source->disposed_event().subscribe(
            *this, [this] {
                try {
                    source_list_subscription_.disconnect();
                    source_current_subscription_.disconnect();
                    source_completion_subscription_.disconnect();
                    source_disposed_subscription_.disconnect();
                    clear_bound_errors();
                    binding_errors_.clear();
                    data_source_.reset();
                    data_source_changed_.emit();
                } catch (...) {
                }
            });
    }
    data_source_changed_.emit();
    update_binding();
}

void ErrorProvider::set_data_member(std::string member) {
    require_access("ErrorProvider data-member mutation");
    if (!validate_utf8(member).valid()) {
        throw std::invalid_argument(
            "GUI.Forms ErrorProvider data member must be valid UTF-8");
    }
    if (!member.empty()) member = canonical_binding_name(member);
    if (data_member_ == member) return;
    data_member_ = std::move(member);
    binding_errors_.clear();
    data_member_changed_.emit(data_member_);
    update_binding();
}

void ErrorProvider::bind_to_data_and_errors(
    std::shared_ptr<BindingSource> source, std::string member) {
    set_data_member(std::move(member));
    set_data_source(std::move(source));
}

void ErrorProvider::clear_bound_errors() noexcept {
    auto targets = std::move(bound_targets_);
    bound_targets_.clear();
    for (auto& [id, weak] : targets) {
        static_cast<void>(id);
        if (const auto target = weak.lock(); target && target->is_alive()) {
            try { set_error(target, {}); } catch (...) {}
        }
    }
}

void ErrorProvider::binding_completed(BindingCompleteEvent& event) {
    if (!event.binding || event.binding->source() != data_source_.lock()) return;
    Control* raw = event.binding->target();
    if (!raw) return;
    const auto target = raw->weak_from_this().lock();
    if (event.state == BindingCompleteState::success && !event.cancel) {
        binding_errors_.erase(event.binding);
    } else if (target) {
        std::string error = event.error_text;
        if (error.empty()) error = "The value was not accepted.";
        binding_errors_[event.binding] = {target, std::move(error)};
    }
    update_binding();
}

void ErrorProvider::update_binding() {
    require_access("ErrorProvider binding refresh");
    const auto source = data_source_.lock();
    if (!source || !source->is_alive()) {
        clear_bound_errors();
        binding_errors_.clear();
        return;
    }

    struct Aggregate final {
        std::shared_ptr<Control> target;
        std::vector<std::string> errors;
    };
    std::unordered_map<std::uint64_t, Aggregate> aggregates;
    const auto append = [](std::vector<std::string>& values, std::string value) {
        if (!value.empty() &&
            std::find(values.begin(), values.end(), value) == values.end()) {
            values.push_back(std::move(value));
        }
    };
    const auto bindings = source->bindings();
    std::unordered_set<const Binding*> active_bindings;
    for (const auto& binding : bindings) {
        if (binding) active_bindings.insert(binding.get());
        Control* raw = binding ? binding->target() : nullptr;
        const auto target = raw ? raw->weak_from_this().lock() : nullptr;
        if (!target || !can_extend(target)) continue;
        Aggregate& aggregate = aggregates[target->runtime_id().value];
        aggregate.target = target;
        append(aggregate.errors, source->current_error({}));
        std::string field = binding->data_member();
        if (!data_member_.empty()) field = data_member_ + "." + field;
        std::string record_error = source->current_error(field);
        if (record_error.empty() && !data_member_.empty()) {
            record_error = source->current_error(binding->data_member());
        }
        append(aggregate.errors, std::move(record_error));
        if (const auto found = binding_errors_.find(binding.get());
            found != binding_errors_.end()) {
            append(aggregate.errors, found->second.second);
        }
    }

    auto previous = std::move(bound_targets_);
    bound_targets_.clear();
    for (auto& [id, aggregate] : aggregates) {
        std::string text;
        for (const std::string& item : aggregate.errors) {
            if (!text.empty()) text += "\n";
            text += item;
        }
        set_error(aggregate.target, std::move(text));
        bound_targets_[id] = aggregate.target;
        previous.erase(id);
    }
    for (auto& [id, weak] : previous) {
        static_cast<void>(id);
        if (const auto target = weak.lock(); target && target->is_alive()) {
            set_error(target, {});
        }
    }
    std::erase_if(binding_errors_, [&active_bindings](const auto& item) {
        const auto target = item.second.first.lock();
        return !target || !target->is_alive() ||
               !active_bindings.contains(item.first);
    });
}

void ErrorProvider::set_tag(std::any tag_value) {
    require_access("ErrorProvider tag mutation");
    tag_ = std::move(tag_value);
}

Rect ErrorProvider::icon_bounds(const Entry& entry) const noexcept {
    const auto target = entry.target.lock();
    Window* owner = bound_window();
    if (!target || !owner) return {};
    const Rect anchor = target->absolute_bounds();
    ErrorIconAlignment alignment = entry.alignment;
    if (right_to_left_) {
        switch (alignment) {
        case ErrorIconAlignment::top_left: alignment = ErrorIconAlignment::top_right; break;
        case ErrorIconAlignment::top_right: alignment = ErrorIconAlignment::top_left; break;
        case ErrorIconAlignment::middle_left: alignment = ErrorIconAlignment::middle_right; break;
        case ErrorIconAlignment::middle_right: alignment = ErrorIconAlignment::middle_left; break;
        case ErrorIconAlignment::bottom_left: alignment = ErrorIconAlignment::bottom_right; break;
        case ErrorIconAlignment::bottom_right: alignment = ErrorIconAlignment::bottom_left; break;
        }
    }
    const bool left = alignment == ErrorIconAlignment::top_left ||
                      alignment == ErrorIconAlignment::middle_left ||
                      alignment == ErrorIconAlignment::bottom_left;
    double x = left ? anchor.x - icon_size_ - entry.padding
                    : anchor.right() + entry.padding;
    double y = anchor.y;
    if (alignment == ErrorIconAlignment::middle_left ||
        alignment == ErrorIconAlignment::middle_right) {
        y = anchor.y + (anchor.height - icon_size_) * 0.5;
    } else if (alignment == ErrorIconAlignment::bottom_left ||
               alignment == ErrorIconAlignment::bottom_right) {
        y = anchor.bottom() - icon_size_;
    }
    const Size client = owner->client_size();
    x = std::clamp(x, 0.0, std::max(0.0, client.width - icon_size_));
    y = std::clamp(y, 0.0, std::max(0.0, client.height - icon_size_));
    return {x, y, icon_size_, icon_size_};
}

void ErrorProvider::position_visual(Entry& entry) {
    if (entry.glyph && entry.popup && entry.popup->connected()) {
        entry.glyph->set_requested_bounds(icon_bounds(entry));
    }
}

void ErrorProvider::close_visual(Entry& entry) noexcept {
    try {
        if (tool_tip_ && entry.glyph) {
            static_cast<void>(tool_tip_->remove_tool_tip(*entry.glyph));
        }
    } catch (...) {
    }
    if (entry.popup) entry.popup->disconnect();
    entry.popup.reset();
    entry.glyph.reset();
    entry.layer.reset();
}

void ErrorProvider::refresh_visual(Entry& entry, bool error_changed) {
    Window* owner = bound_window();
    const auto target = entry.target.lock();
    const bool available = owner && target && target->is_alive() &&
        target->attached_window() == owner && target->effectively_visible();
    if (entry.error.empty() || !available) {
        close_visual(entry);
        return;
    }

    if (entry.popup && !entry.popup->connected()) close_visual(entry);
    if (!entry.popup) {
        const std::string prefix = "error-provider." +
            std::to_string(provider_id_) + "." +
            std::to_string(target->runtime_id().value);
        entry.layer = make_control<ErrorLayer>(StableId(prefix + ".layer"));
        entry.layer->set_requested_bounds(
            {0.0, 0.0, owner->client_size().width, owner->client_size().height});
        entry.glyph = make_control<ErrorGlyph>(StableId(prefix + ".glyph"));
        entry.glyph->set_requested_bounds(icon_bounds(entry));
        entry.glyph->set_error(entry.error);
        entry.glyph->set_icon(icon_);
        entry.glyph->configure_blink(blink_style_, blink_rate_, true);
        entry.layer->add_child(entry.glyph);
        PopupToken popup = owner->open_popup(
            target, entry.layer, PopupOptions{.require_enabled_owner = false});
        entry.popup = std::make_unique<PopupToken>(std::move(popup));
        tool_tip_->set_tool_tip(entry.glyph, entry.error);
        return;
    }

    entry.glyph->set_error(entry.error);
    entry.glyph->set_icon(icon_);
    entry.glyph->configure_blink(blink_style_, blink_rate_, error_changed);
    tool_tip_->set_tool_tip(entry.glyph, entry.error);
    position_visual(entry);
}

void ErrorProvider::refresh_all_visuals(bool restart_blink) {
    for (auto& [id, entry] : entries_) {
        static_cast<void>(id);
        refresh_visual(*entry, restart_blink);
        if (entry->glyph) entry->glyph->refresh_motion_policy();
    }
}

void ErrorProvider::erase_if_empty(std::uint64_t runtime_id) {
    const auto found = entries_.find(runtime_id);
    if (found == entries_.end()) return;
    const Entry& entry = *found->second;
    if (entry.error.empty() && entry.alignment == ErrorIconAlignment::middle_right &&
        entry.padding == 0.0) {
        entries_.erase(found);
    }
}

ErrorProviderSnapshot ErrorProvider::snapshot() const {
    if (Window* owner = bound_window()) owner->verify_access("ErrorProvider snapshot");
    ErrorProviderSnapshot result;
    result.icons.reserve(entries_.size());
    for (const auto& [id, entry] : entries_) {
        static_cast<void>(id);
        const auto target = entry->target.lock();
        const bool available = target && target->is_alive() &&
            target->attached_window() == bound_window() &&
            target->effectively_visible();
        const bool presented = entry->popup && entry->popup->connected() &&
            entry->glyph;
        if (!entry->error.empty() && target && target->is_alive()) {
            ++result.live_errors;
        }
        if (presented) ++result.presented_icons;
        result.icons.push_back({
            target ? std::string(target->stable_id().value()) : std::string{},
            entry->error, icon_bounds(*entry), available, presented,
            presented && entry->glyph->blink_active(),
            !presented || entry->glyph->phase_visible()});
    }
    std::sort(result.icons.begin(), result.icons.end(),
              [](const ErrorIconSnapshot& left, const ErrorIconSnapshot& right) {
                  return left.target_stable_id < right.target_stable_id;
              });
    return result;
}

void ErrorProvider::verify_dispose_thread() {
    if (Window* owner = bound_window()) owner->verify_access("ErrorProvider disposal");
}

void ErrorProvider::on_dispose() noexcept {
    try {
        source_list_subscription_.disconnect();
        source_current_subscription_.disconnect();
        source_completion_subscription_.disconnect();
        source_disposed_subscription_.disconnect();
        data_source_.reset();
        binding_errors_.clear();
        bound_targets_.clear();
        for (auto& [id, entry] : entries_) {
            static_cast<void>(id);
            if (const auto target = entry->target.lock(); target && target->is_alive()) {
                target->clear_provider_error(provider_id_);
            }
            close_visual(*entry);
        }
        entries_.clear();
        if (tool_tip_ && tool_tip_->is_alive()) tool_tip_->dispose();
    } catch (...) {
    }
    availability_subscription_.disconnect();
    presentation_subscription_.disconnect();
    root_bounds_subscription_.disconnect();
    tool_tip_.reset();
    tag_.reset();
    window_lifetime_.reset();
}

} // namespace gui_forms
