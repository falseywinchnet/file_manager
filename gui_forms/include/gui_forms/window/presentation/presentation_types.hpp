#pragma once

#include "gui_forms/types.hpp"

#include <compare>
#include <cstdint>
#include <memory>

namespace gui_forms {

class LiveSurface;

// Presentation preferences are logical UI policy and remain independent from
// the native/device pixel scale.
struct PresentationSettings final {
    double text_scale{1.0};
    bool high_contrast{};
    bool reduced_motion{};
    bool sound_enabled{true};
    friend constexpr bool operator==(const PresentationSettings&,
                                     const PresentationSettings&) = default;
};

// Immutable host placement for the newest generation of one live surface.
struct LiveSurfacePresentation final {
    RuntimeId control{};
    std::shared_ptr<LiveSurface> surface;
    Rect destination{};
    Rect clip{};
};

enum class PaintLeaseState : std::uint8_t {
    clean,
    dirty_queued,
    dirty = dirty_queued,
    rendering,
    ready,
    occluded_dirty,
    retired,
    rendering_dirty,
};

struct PaintLeaseSnapshot final {
    PaintLeaseState state{PaintLeaseState::dirty_queued};
    std::uint64_t content_revision{1U};
    std::uint64_t rendered_revision{};
    std::uint64_t presented_revision{};
    std::uint64_t surface_epoch{1U};
    std::uint64_t leases_started{};
    std::uint64_t leases_completed{};
    std::uint64_t leases_abandoned{};
    std::uint64_t reentrant_requests_deferred{};
    std::uint64_t render_wakes_queued{};
    std::uint64_t render_wakes_coalesced{};
    std::uint64_t presentation_receipts_accepted{};
    std::uint64_t presentation_receipts_rejected{};
    bool render_wake_queued{};
    bool dirty_after_render{};
};

// Proof that one coherent retained transaction finished replaying into a
// host-owned backing surface at a still-current surface epoch.
struct PaintReceipt final {
    std::uint64_t rendered_revision{};
    std::uint64_t surface_epoch{};

    [[nodiscard]] explicit constexpr operator bool() const noexcept {
        return rendered_revision != 0U && surface_epoch != 0U;
    }
    friend constexpr auto operator<=>(const PaintReceipt&,
                                      const PaintReceipt&) = default;
};

} // namespace gui_forms
