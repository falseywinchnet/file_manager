#pragma once

#include "gui_forms/controls/scrollable_control/container_control/container_control.hpp"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace gui_forms {

enum class ResponsiveTrackOrientation : std::uint8_t {
    horizontal,
    vertical,
};

enum class ResponsiveTrackSizeMode : std::uint8_t {
    fixed,
    content,
    remaining,
};

enum class ResponsiveTrackCollapseReason : std::uint8_t {
    none,
    authored_hidden,
    insufficient_extent,
};

// A zero maximum is unbounded, matching Control::maximum_size. Collapse is
// never inferred from content identity: an authored priority explicitly opts
// the track into automatic collapse. Smaller priority values collapse first.
struct ResponsiveTrackSpec final {
    ResponsiveTrackSizeMode size_mode{ResponsiveTrackSizeMode::content};
    double minimum{};
    double preferred{};
    double maximum{};
    double remaining_weight{1.0};
    bool hidden{};
    std::optional<std::uint16_t> collapse_priority;

    friend bool operator==(const ResponsiveTrackSpec& left,
                           const ResponsiveTrackSpec& right) = default;
};

struct ResponsiveTrackResult final {
    double measured_content{};
    double minimum{};
    double desired{};
    double allocated{};
    double logical_start{};
    double logical_end{};
    std::int64_t device_start{};
    std::int64_t device_end{};
    double collapse_threshold{};
    ResponsiveTrackCollapseReason collapse_reason{
        ResponsiveTrackCollapseReason::none};
    bool collapse_protected{};

    [[nodiscard]] bool collapsed() const noexcept {
        return collapse_reason != ResponsiveTrackCollapseReason::none;
    }
    friend bool operator==(const ResponsiveTrackResult& left,
                           const ResponsiveTrackResult& right) = default;
};

struct ResponsiveTrackResolution final {
    double available_extent{};
    double gap{};
    double device_scale{1.0};
    double minimum_extent{};
    double desired_extent{};
    double allocated_extent{};
    bool overflow{};
    std::vector<std::size_t> collapse_order;
    std::vector<ResponsiveTrackResult> tracks;

    friend bool operator==(const ResponsiveTrackResolution& left,
                           const ResponsiveTrackResolution& right) = default;
};

// Renderer-free one-dimensional solver. Content measurements and protected
// tracks are explicit inputs. Logical allocation is independent of display
// scale; only cumulative output boundaries are snapped to device pixels.
[[nodiscard]] ResponsiveTrackResolution resolve_responsive_tracks(
    std::span<const ResponsiveTrackSpec> specs,
    std::span<const double> measured_content,
    double available_extent,
    double gap,
    double device_scale,
    std::span<const std::size_t> collapse_protected_tracks = {});

struct ResponsiveLayoutSnapshot final {
    ResponsiveTrackOrientation orientation{
        ResponsiveTrackOrientation::horizontal};
    ResponsiveTrackResolution resolution;
    LayoutTransactionState transaction;
    std::uint64_t committed_resolution_revision{};
};

// Nest horizontal and vertical instances to form a retained grid. Each direct
// child occupies one authored track; multiple children may intentionally share
// a track. The panel never changes Control::visible(). Automatic collapse is a
// separate effective-layout state and therefore leaves author visibility
// intact while removing the subtree from paint, hit testing, focus, and
// semantics.
class ResponsiveTrackPanel final : public ContainerControl {
public:
    explicit ResponsiveTrackPanel(StableId stable_id);

    [[nodiscard]] ResponsiveTrackOrientation orientation() const noexcept {
        return orientation_;
    }
    void set_orientation(ResponsiveTrackOrientation orientation);
    [[nodiscard]] double track_gap() const noexcept { return track_gap_; }
    void set_track_gap(double gap);

    [[nodiscard]] std::span<const ResponsiveTrackSpec> track_specs() const noexcept {
        return track_specs_;
    }
    void set_track_specs(std::vector<ResponsiveTrackSpec> specs);
    void set_track_spec(std::size_t track, ResponsiveTrackSpec spec);

    void set_child_track(const Control& child, std::size_t track);
    [[nodiscard]] std::optional<std::size_t> child_track(
        const Control& child) const;

    // A revealed track is protected from automatic collapse until the author
    // clears the request. This is the explicit focus-reveal seam; protection
    // may honestly produce overflow when no authored alternative can collapse.
    void set_track_revealed(std::size_t track, bool revealed);
    [[nodiscard]] bool track_revealed(std::size_t track) const noexcept;

    // If a currently focused track truly cannot fit after other authored
    // priorities collapse, this exact retained target receives focus before
    // the track becomes effectively unavailable. Without a live eligible
    // target the focused track remains protected and overflow is reported.
    void set_track_focus_fallback(std::size_t track, Control& target);
    void clear_track_focus_fallback(std::size_t track);

    [[nodiscard]] ResponsiveLayoutSnapshot layout_snapshot() const;

    [[nodiscard]] Size measure(Size available) override;
    void arrange(Rect final_bounds) override;
    [[nodiscard]] SemanticDescriptor semantic_descriptor() const override;

private:
    using ChildTrackMap = std::unordered_map<std::uint64_t, std::size_t>;
    using FocusFallbackMap =
        std::unordered_map<std::size_t, std::weak_ptr<Control>>;

    [[nodiscard]] ResponsiveTrackResolution layout_children(
        Size available, bool assign);
    void reconcile_metadata();
    static void validate_specs(std::span<const ResponsiveTrackSpec> specs);

    ResponsiveTrackOrientation orientation_{
        ResponsiveTrackOrientation::horizontal};
    double track_gap_{};
    std::vector<ResponsiveTrackSpec> track_specs_{ResponsiveTrackSpec{}};
    ChildTrackMap child_tracks_;
    FocusFallbackMap focus_fallbacks_;
    std::unordered_set<std::size_t> revealed_tracks_;
    ResponsiveTrackResolution committed_resolution_;
    std::uint64_t committed_resolution_revision_{};
};

} // namespace gui_forms
