#include "gui_forms/host.hpp"

#include "gui_forms/window.hpp"

#include <algorithm>
#include <cmath>
#include <array>
#include <sstream>
#include <type_traits>
#include <unordered_set>

namespace gui_forms {
namespace {
bool valid_size(Size size) noexcept {
    return std::isfinite(size.width) && std::isfinite(size.height) &&
           size.width > 0.0 && size.height > 0.0;
}

bool valid_scale(double scale) noexcept {
    return std::isfinite(scale) && scale > 0.0;
}

bool valid_utf8(std::string_view text) noexcept {
    const auto* bytes = reinterpret_cast<const unsigned char*>(text.data());
    std::size_t index = 0;
    while (index < text.size()) {
        const unsigned char lead = bytes[index];
        if (lead <= 0x7FU) {
            ++index;
            continue;
        }
        std::size_t length = 0;
        std::uint32_t codepoint = 0;
        if (lead >= 0xC2U && lead <= 0xDFU) {
            length = 2;
            codepoint = lead & 0x1FU;
        } else if (lead >= 0xE0U && lead <= 0xEFU) {
            length = 3;
            codepoint = lead & 0x0FU;
        } else if (lead >= 0xF0U && lead <= 0xF4U) {
            length = 4;
            codepoint = lead & 0x07U;
        } else {
            return false;
        }
        if (index + length > text.size()) {
            return false;
        }
        for (std::size_t offset = 1; offset < length; ++offset) {
            const unsigned char continuation = bytes[index + offset];
            if ((continuation & 0xC0U) != 0x80U) {
                return false;
            }
            codepoint = (codepoint << 6U) | (continuation & 0x3FU);
        }
        if ((length == 3 && codepoint < 0x800U) ||
            (length == 4 && codepoint < 0x10000U) ||
            (codepoint >= 0xD800U && codepoint <= 0xDFFFU) ||
            codepoint > 0x10FFFFU) {
            return false;
        }
        index += length;
    }
    return true;
}

bool valid_monitor(const HostMonitor& monitor) noexcept {
    const auto valid_rect = [](Rect rect) {
        return std::isfinite(rect.x) && std::isfinite(rect.y) &&
               valid_size({rect.width, rect.height});
    };
    return !monitor.id.empty() && valid_rect(monitor.frame) &&
           valid_rect(monitor.work_area) && valid_scale(monitor.scale);
}

bool valid_monitor_set(const std::vector<HostMonitor>& monitors) {
    if (monitors.empty() || monitors.size() > 64U) {
        return false;
    }
    std::unordered_set<std::string> identifiers;
    std::size_t primary_count = 0;
    for (const HostMonitor& monitor : monitors) {
        if (!valid_monitor(monitor) || !identifiers.insert(monitor.id).second) {
            return false;
        }
        primary_count += monitor.primary ? 1U : 0U;
    }
    return primary_count == 1U;
}

bool valid_drag_event(const DragEvent& event) noexcept {
    constexpr std::uint8_t known_effects =
        static_cast<std::uint8_t>(DragEffect::copy) |
        static_cast<std::uint8_t>(DragEffect::move) |
        static_cast<std::uint8_t>(DragEffect::link);
    const auto effects = static_cast<std::uint8_t>(event.allowed_effects);
    if (event.session_id == 0 || !std::isfinite(event.position.x) ||
        !std::isfinite(event.position.y) || (effects & ~known_effects) != 0 ||
        event.accepted_effect != DragEffect::none ||
        event.items.size() > DragLimits::maximum_items) {
        return false;
    }
    if (event.action != DragAction::leave &&
        (event.items.empty() || event.allowed_effects == DragEffect::none)) {
        return false;
    }
    std::size_t total = 0;
    const auto add_size = [&total](std::size_t value) {
        if (value > DragLimits::maximum_total_bytes - total) {
            return false;
        }
        total += value;
        return true;
    };
    for (const DragDataItem& item : event.items) {
        const bool valid = std::visit([&add_size](const auto& data) {
            using Data = std::decay_t<decltype(data)>;
            if constexpr (std::is_same_v<Data, DragTextData>) {
                return data.text_utf8.size() <= DragLimits::maximum_text_bytes &&
                       data.text_utf8.find('\0') == std::string::npos &&
                       valid_utf8(data.text_utf8) && add_size(data.text_utf8.size());
            } else if constexpr (std::is_same_v<Data, DragFileListData>) {
                if (data.paths_utf8.empty() ||
                    data.paths_utf8.size() > DragLimits::maximum_paths) {
                    return false;
                }
                for (const std::string& path : data.paths_utf8) {
                    if (path.empty() || path.size() > DragLimits::maximum_path_bytes ||
                        path.find('\0') != std::string::npos || !valid_utf8(path) ||
                        !add_size(path.size())) {
                        return false;
                    }
                }
                return true;
            } else if constexpr (std::is_same_v<Data, DragBinaryData>) {
                return !data.media_type.empty() &&
                       data.media_type.size() <= DragLimits::maximum_media_type_bytes &&
                       data.media_type.find('\0') == std::string::npos &&
                       valid_utf8(data.media_type) && add_size(data.bytes.size());
            }
            return false;
        }, item);
        if (!valid) {
            return false;
        }
    }
    return true;
}
} // namespace
HostSession::HostSession(Window& window,
                         HostCapabilities capabilities,
                         HostServices* services)
    : window_(&window), services_(services), ui_thread_(std::this_thread::get_id()) {
    snapshot_.capabilities = std::move(capabilities);
    if (services_ != nullptr) {
        if (window.host_services_ != nullptr && window.host_services_ != services_) {
            throw std::logic_error("GUI.Forms Window already has an active host service seam");
        }
        window.host_services_ = services_;
        capture_observation_ = window.pointer_capture_changed().subscribe(
            [this](const PointerCaptureChange& change) {
                if (!snapshot_.shutdown && services_ != nullptr) {
                    static_cast<void>(services_->set_pointer_capture(
                        change.captured, change.pointer_id));
                }
            });
        modal_observation_ = services_->modal_changed().subscribe(
            [this](const HostModalTransition& transition) {
                if (snapshot_.shutdown) {
                    return;
                }
                snapshot_.modal_depth = transition.depth;
                ++snapshot_.modal_transitions;
                if (transition.entering && window_ != nullptr) {
                    window_->release_pointer();
                }
            });
    }
}

HostSession::~HostSession() {
    shutdown();
}

HostDispatchResult HostSession::dispatch(HostEvent event) {
    HostDispatchResult result;
    if (std::this_thread::get_id() != ui_thread_) {
        result.error = HostDispatchError::wrong_thread;
        return result;
    } else if (event.sequence == 0) {
        result.error = HostDispatchError::sequence_zero;
    } else if (event.sequence <= snapshot_.last_sequence) {
        result.error = HostDispatchError::non_monotonic_sequence;
    } else if (snapshot_.shutdown) {
        result.error = HostDispatchError::after_shutdown;
    }
    if (result.accepted()) {
        const bool attach = std::holds_alternative<HostAttachEvent>(event.payload);
        const bool shutdown = std::holds_alternative<HostShutdownEvent>(event.payload);
        const bool closed = std::holds_alternative<HostClosedEvent>(event.payload);
        const bool passive_terminal = std::visit([](const auto& payload) {
            using Payload = std::decay_t<decltype(payload)>;
            if constexpr (std::is_same_v<Payload, HostActivationEvent>) {
                return !payload.active;
            } else if constexpr (std::is_same_v<Payload, HostOcclusionEvent>) {
                return payload.occluded;
            }
            return false;
        }, event.payload);
        switch (snapshot_.phase) {
        case HostLifecyclePhase::constructed:
            if (!attach && !shutdown) result.error = HostDispatchError::invalid_lifecycle;
            break;
        case HostLifecyclePhase::attached:
            if (attach) result.error = HostDispatchError::invalid_lifecycle;
            break;
        case HostLifecyclePhase::close_authorized:
            if (!closed && !shutdown && !passive_terminal) {
                result.error = HostDispatchError::invalid_lifecycle;
            }
            break;
        case HostLifecyclePhase::closed:
            if (!shutdown) result.error = HostDispatchError::invalid_lifecycle;
            break;
        case HostLifecyclePhase::shutdown:
            result.error = HostDispatchError::after_shutdown;
            break;
        }
    }
    if (!result.accepted()) {
        ++snapshot_.events_rejected;
        observed_.emit(event, result);
        return result;
    }

    const bool invalid_geometry = std::visit(
        [](const auto& payload) {
            using Payload = std::decay_t<decltype(payload)>;
            if constexpr (std::is_same_v<Payload, HostAttachEvent>) {
                return !valid_size(payload.client_size) || !valid_scale(payload.scale);
            } else if constexpr (std::is_same_v<Payload, HostResizeEvent>) {
                return !valid_size(payload.client_size);
            } else if constexpr (std::is_same_v<Payload, HostScaleEvent>) {
                return !valid_scale(payload.scale);
            } else if constexpr (std::is_same_v<Payload, HostDisplayEvent>) {
                return !valid_monitor_set(payload.monitors);
            }
            return false;
        }, event.payload);
    if (invalid_geometry) {
        result.error = HostDispatchError::invalid_geometry;
        ++snapshot_.events_rejected;
        observed_.emit(event, result);
        return result;
    }
    if (const auto* drag = std::get_if<DragEvent>(&event.payload);
        drag != nullptr && !valid_drag_event(*drag)) {
        result.error = HostDispatchError::invalid_payload;
        ++snapshot_.events_rejected;
        observed_.emit(event, result);
        return result;
    }

    snapshot_.last_sequence = event.sequence;
    ++snapshot_.events_accepted;
    const bool modal_input = snapshot_.modal_depth != 0 &&
        (std::holds_alternative<PointerEvent>(event.payload) ||
         std::holds_alternative<KeyEvent>(event.payload) ||
         std::holds_alternative<TextInputEvent>(event.payload) ||
         std::holds_alternative<DragEvent>(event.payload));
    if (modal_input) {
        ++snapshot_.modal_input_suppressions;
        observed_.emit(event, result);
        return result;
    }
    try {
        std::visit(
            [this, &result, &event](auto& payload) {
            using Payload = std::decay_t<decltype(payload)>;
            if constexpr (std::is_same_v<Payload, HostAttachEvent>) {
                UpdateScope update = window_->begin_update();
                window_->resize(payload.client_size);
                window_->set_scale(payload.scale);
                update.close();
                snapshot_.attached = true;
                snapshot_.phase = HostLifecyclePhase::attached;
            } else if constexpr (std::is_same_v<Payload, HostResizeEvent>) {
                window_->resize(payload.client_size);
            } else if constexpr (std::is_same_v<Payload, HostScaleEvent>) {
                window_->set_scale(payload.scale);
            } else if constexpr (std::is_same_v<Payload, HostActivationEvent>) {
                snapshot_.active = payload.active;
            } else if constexpr (std::is_same_v<Payload, HostOcclusionEvent>) {
                snapshot_.occluded = payload.occluded;
                window_->set_occluded(
                    payload.occluded,
                    FrameTime{std::chrono::nanoseconds(event.timestamp_nanoseconds)});
            } else if constexpr (std::is_same_v<Payload, HostDisplayEvent>) {
                ++snapshot_.display_changes;
                snapshot_.monitor_count = payload.monitors.size();
            } else if constexpr (std::is_same_v<Payload, DragEvent>) {
                ++snapshot_.drag_events;
                snapshot_.drag_drops += payload.action == DragAction::drop ? 1U : 0U;
                const DragDispatchResult drag = window_->dispatch_drag(std::move(payload));
                result.handled = drag.handled;
                result.drag_effect = drag.accepted_effect;
                result.input_deferred = drag.deferred;
                result.input_capacity_rejected = drag.capacity_rejected;
            } else if constexpr (std::is_same_v<Payload, PointerEvent>) {
                const DeferredInputSnapshot before =
                    window_->deferred_input_snapshot();
                result.handled = window_->dispatch_pointer(std::move(payload));
                const DeferredInputSnapshot after =
                    window_->deferred_input_snapshot();
                result.input_deferred = after.deferred > before.deferred;
                result.input_capacity_rejected =
                    after.rejected_capacity > before.rejected_capacity;
            } else if constexpr (std::is_same_v<Payload, KeyEvent>) {
                const DeferredInputSnapshot before =
                    window_->deferred_input_snapshot();
                result.handled = window_->dispatch_key(std::move(payload));
                const DeferredInputSnapshot after =
                    window_->deferred_input_snapshot();
                result.input_deferred = after.deferred > before.deferred;
                result.input_capacity_rejected =
                    after.rejected_capacity > before.rejected_capacity;
            } else if constexpr (std::is_same_v<Payload, TextInputEvent>) {
                const DeferredInputSnapshot before =
                    window_->deferred_input_snapshot();
                result.handled = window_->dispatch_text(std::move(payload));
                const DeferredInputSnapshot after =
                    window_->deferred_input_snapshot();
                result.input_deferred = after.deferred > before.deferred;
                result.input_capacity_rejected =
                    after.rejected_capacity > before.rejected_capacity;
            } else if constexpr (std::is_same_v<Payload, HostCloseRequest>) {
                ++snapshot_.close_requests;
                closing_.emit(payload);
                result.close_allowed = !payload.cancel;
                snapshot_.close_cancellations += payload.cancel ? 1U : 0U;
                if (!payload.cancel) {
                    snapshot_.phase = HostLifecyclePhase::close_authorized;
                }
            } else if constexpr (std::is_same_v<Payload, HostClosedEvent>) {
                snapshot_.phase = HostLifecyclePhase::closed;
                snapshot_.attached = false;
                snapshot_.closed = true;
                snapshot_.active = false;
                snapshot_.occluded = false;
                window_->shutdown_dispatcher();
                window_->cancel_frame_requests();
                window_->release_pointer();
                window_->cancel_drag();
            } else if constexpr (std::is_same_v<Payload, HostShutdownEvent>) {
                shutdown();
            }
            }, event.payload);
        observed_.emit(event, result);
    } catch (...) {
        // Event callbacks are ordinary C++ and deliberately propagate through
        // Event::emit. The portable host seam is the foreign-ABI boundary: no
        // application exception may unwind into AppKit, Win32, X11, or
        // Wayland. Retain the consumed sequence, report the fault, and revoke
        // transient input ownership that a partially delivered event may have
        // acquired.
        result.handled = false;
        result.input_deferred = false;
        result.input_capacity_rejected = false;
        result.close_allowed = false;
        result.drag_effect = DragEffect::none;
        result.error = HostDispatchError::callback_fault;
        ++snapshot_.callback_faults;
        if (window_ != nullptr) {
            try {
                window_->release_pointer();
                window_->cancel_drag();
            } catch (...) {
                // Cleanup is best effort at an exception boundary. The
                // original callback fault remains the observable result.
            }
        }
    }
    return result;
}

void HostSession::shutdown() noexcept {
    if (snapshot_.shutdown || std::this_thread::get_id() != ui_thread_) {
        return;
    }
    snapshot_.active = false;
    snapshot_.attached = false;
    snapshot_.occluded = false;
    if (window_ != nullptr) {
        window_->shutdown_dispatcher();
        window_->cancel_frame_requests();
        window_->release_pointer();
        window_->cancel_drag();
        if (window_->host_services_ == services_) window_->host_services_ = nullptr;
    }
    snapshot_.shutdown = true;
    snapshot_.phase = HostLifecyclePhase::shutdown;
}

HostSessionSnapshot HostSession::snapshot() const {
    return snapshot_;
}

} // namespace gui_forms
