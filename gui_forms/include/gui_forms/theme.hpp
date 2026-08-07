#pragma once

#include "gui_forms/surface_material.hpp"

#include <array>
#include <chrono>
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <utility>

namespace gui_forms {

// Compatibility projection used by existing controls with explicit local
// styling. New theme-aware controls resolve a richer material recipe first.
struct BasicControlStyle final {
    Color face{Color::rgba(229, 234, 239)};
    Color face_light{Color::rgba(247, 249, 251)};
    Color paper{Color::rgba(255, 255, 255)};
    Color highlight{Color::rgba(255, 255, 255)};
    Color border{Color::rgba(148, 162, 175)};
    Color dark_border{Color::rgba(76, 94, 111)};
    Color text{Color::rgba(27, 39, 51)};
    Color disabled_text{Color::rgba(132, 143, 153)};
    Color accent{Color::rgba(38, 114, 185)};
    Color accent_light{Color::rgba(216, 235, 249)};
    Color link{Color::rgba(25, 82, 139)};
    Color visited_link{Color::rgba(93, 65, 145)};

    friend constexpr bool operator==(const BasicControlStyle&,
                                     const BasicControlStyle&) = default;
};

enum class ControlVisualRole : std::uint8_t {
    window,
    panel,
    card,
    button,
    accent_button,
    command_button,
    choice,
    editor,
    menu_item,
    selection,
    progress,
    count,
};

enum class ControlSurfaceState : std::uint8_t {
    normal,
    hot,
    pressed,
    pending,
    invalid,
    disabled,
    deactivated,
    count,
};

enum class ControlVisualStatus : std::uint8_t {
    normal,
    pending,
    invalid,
};

struct ControlVisualContext final {
    ControlSurfaceState surface{ControlSurfaceState::normal};
    bool selected{};
    bool focused{};
    bool defaulted{};
    bool high_contrast{};
    friend constexpr bool operator==(const ControlVisualContext&,
                                     const ControlVisualContext&) = default;
};

struct ControlVisualRecipe final {
    SurfaceMaterial material;
    Color text{Color::rgba(27, 39, 51)};
    Color muted_text{Color::rgba(132, 143, 153)};
    Color glyph{Color::rgba(27, 39, 51)};
    Color focus_ring{Color::rgba(38, 114, 185)};
    Color default_ring{Color::rgba(25, 82, 139)};
    double focus_width{1.0};
    double default_width{2.0};
    Point pressed_content_offset{1.0, 1.0};

    friend bool operator==(const ControlVisualRecipe&,
                           const ControlVisualRecipe&) = default;
};

inline constexpr std::size_t control_visual_role_count =
    static_cast<std::size_t>(ControlVisualRole::count);
inline constexpr std::size_t control_surface_state_count =
    static_cast<std::size_t>(ControlSurfaceState::count);

struct ControlRoleRecipes final {
    std::array<ControlVisualRecipe, control_surface_state_count> ordinary;
    std::array<ControlVisualRecipe, control_surface_state_count> selected;
    std::array<ControlVisualRecipe, control_surface_state_count> high_contrast;
    std::array<ControlVisualRecipe, control_surface_state_count>
        high_contrast_selected;

    friend bool operator==(const ControlRoleRecipes&,
                           const ControlRoleRecipes&) = default;
};

// Immutable structural scales accompany visual recipes. They are expressed in
// logical units and remain renderer/platform neutral; applications may derive
// component layouts from them without copying house constants into each view.
struct ThemeSpacingTokens final {
    double micro{2.0};
    double xsmall{4.0};
    double small{6.0};
    double medium{8.0};
    double large{12.0};
    double xlarge{16.0};
    double section{24.0};

    friend constexpr bool operator==(const ThemeSpacingTokens&,
                                     const ThemeSpacingTokens&) = default;
};

struct ThemeGeometryTokens final {
    double compact_control_height{24.0};
    double control_height{30.0};
    double large_control_height{36.0};
    double minimum_touch_target{40.0};
    double splitter_width{3.0};
    double splitter_hit_width{9.0};
    double navigation_extent{260.0};
    double navigation_minimum{120.0};
    double content_minimum{240.0};
    double compact_breakpoint{720.0};

    friend constexpr bool operator==(const ThemeGeometryTokens&,
                                     const ThemeGeometryTokens&) = default;
};

struct ThemeTypographyTokens final {
    FontSpec control{FontRole::control, 12.0, 400, false, 0.24};
    FontSpec field{FontRole::content, 12.0, 400, false};
    FontSpec caption{FontRole::content, 10.0, 400, false};
    FontSpec heading{FontRole::control, 14.0, 650, false, 0.18};
    FontSpec title{FontRole::control, 20.0, 700, false, 0.12};
    FontSpec monospace{FontRole::monospace, 12.0, 400, false};

    friend constexpr bool operator==(const ThemeTypographyTokens&,
                                     const ThemeTypographyTokens&) = default;
};

struct ThemeMotionTokens final {
    std::chrono::milliseconds quick{100};
    std::chrono::milliseconds standard{180};
    std::chrono::milliseconds emphasized{280};
    std::chrono::milliseconds busy_cycle{1400};

    friend constexpr bool operator==(const ThemeMotionTokens&,
                                     const ThemeMotionTokens&) = default;
};

struct ThemeStructureTokens final {
    ThemeSpacingTokens spacing;
    ThemeGeometryTokens geometry;
    ThemeTypographyTokens typography;
    ThemeMotionTokens motion;

    friend constexpr bool operator==(const ThemeStructureTokens&,
                                     const ThemeStructureTokens&) = default;
};

struct ThemeDefinition final {
    std::string id{"windows-professional"};
    BasicControlStyle compatibility;
    std::array<ControlRoleRecipes, control_visual_role_count> roles;
    ThemeStructureTokens structure;

    friend bool operator==(const ThemeDefinition&,
                           const ThemeDefinition&) = default;
};

class Theme final {
public:
    [[nodiscard]] static std::shared_ptr<const Theme> create(
        ThemeDefinition definition);

    [[nodiscard]] std::string_view id() const noexcept { return definition_.id; }
    [[nodiscard]] const BasicControlStyle& basic_style() const noexcept {
        return definition_.compatibility;
    }
    [[nodiscard]] const ThemeStructureTokens& structure() const noexcept {
        return definition_.structure;
    }
    [[nodiscard]] const ControlVisualRecipe& resolve(
        ControlVisualRole role, ControlVisualContext context) const noexcept;
    [[nodiscard]] const ThemeDefinition& definition() const noexcept {
        return definition_;
    }

private:
    explicit Theme(ThemeDefinition definition)
        : definition_(std::move(definition)) {}
    ThemeDefinition definition_;
};

[[nodiscard]] ThemeDefinition windows_professional_theme_definition();
[[nodiscard]] std::shared_ptr<const Theme> default_theme();

} // namespace gui_forms
