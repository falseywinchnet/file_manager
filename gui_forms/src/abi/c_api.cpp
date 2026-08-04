#include "gui_forms/c_api.h"

#include "gui_forms/control.hpp"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <limits>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>
#include <vector>

namespace {

using gui_forms::ComponentState;
using gui_forms::Control;
using gui_forms::Rect;
using gui_forms::StableId;

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
    void* context{};
    std::thread::id ui_thread;
    bool connected{true};
};

struct ControlRecord final {
    std::shared_ptr<Control> control;
    std::thread::id ui_thread;
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
    gf_result create(gf_string_view stable_id, gf_handle* output) {
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
        record->control = std::make_shared<Control>(StableId(std::move(id)));
        record->ui_thread = std::this_thread::get_id();
        std::scoped_lock lock(mutex_);
        *output = allocate_locked(SlotKind::control, record, {});
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

private:
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
                    slot->subscription && slot->subscription->connected) {
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
    return translate([&] { return registry().create(id, control); });
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

} // namespace

extern "C" GF_C_API_EXPORT gf_result gf_get_api_v0(std::uint32_t requested_version,
                                                    gf_api_v0* table) {
    if (table == nullptr || table->struct_size < sizeof(std::uint32_t) * 2U) {
        return fail(GF_ERROR_INVALID_ARGUMENT,
                    "gf_get_api_v0 requires a size-prefixed output table");
    }
    if (requested_version != GF_ABI_VERSION_0_1) {
        return fail(GF_ERROR_UNSUPPORTED_VERSION,
                    "requested GUI.Forms experimental ABI version is unsupported");
    }
    const std::uint32_t caller_size = table->struct_size;
    const gf_api_v0 implementation{
        sizeof(gf_api_v0),
        GF_ABI_VERSION_0_1,
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
    };
    const std::size_t copy_size = std::min<std::size_t>(caller_size, sizeof(implementation));
    std::memcpy(table, &implementation, copy_size);
    return GF_OK;
}
