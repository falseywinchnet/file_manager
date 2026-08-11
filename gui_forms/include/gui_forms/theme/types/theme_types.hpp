#pragma once

#include "gui_forms/surface_material.hpp"

#include <array>
#include <chrono>
#include <cstdint>
#include <string>

namespace gui_forms {

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
    friend constexpr bool operator==(const BasicControlStyle& left,
                                     const BasicControlStyle& right) noexcept {
        return left.face == right.face && left.face_light == right.face_light &&
               left.paper == right.paper && left.highlight == right.highlight &&
               left.border == right.border &&
               left.dark_border == right.dark_border && left.text == right.text &&
               left.disabled_text == right.disabled_text &&
               left.accent == right.accent &&
               left.accent_light == right.accent_light &&
               left.link == right.link &&
               left.visited_link == right.visited_link;
    }
};

enum class ControlVisualRole : std::uint8_t {
    window, panel, card, button, accent_button, command_button, choice,
    editor, menu_item, selection, progress, count,
};
enum class ControlSurfaceState : std::uint8_t {
    normal, hot, pressed, pending, invalid, disabled, deactivated, count,
};
enum class ControlVisualStatus : std::uint8_t { normal, pending, invalid };

struct ControlVisualContext final {
    ControlSurfaceState surface{ControlSurfaceState::normal};
    bool selected{};
    bool focused{};
    bool defaulted{};
    bool high_contrast{};
    friend constexpr bool operator==(const ControlVisualContext& left,
                                     const ControlVisualContext& right) noexcept {
        return left.surface == right.surface &&
               left.selected == right.selected && left.focused == right.focused &&
               left.defaulted == right.defaulted &&
               left.high_contrast == right.high_contrast;
    }
};
struct ControlVisualRecipe final {
    SurfaceMaterial material;
    Color text{Color::rgba(27, 39, 51)};
    Color muted_text{Color::rgba(132, 143, 153)};
    Color glyph{Color::rgba(27, 39, 51)};
    Color focus_ring{Color::rgba(38, 114, 185)};
    Color default_ring{Color::rgba(25, 82, 139)};
    double focus_width{1.0};
    double focus_offset{};
    bool authored_focus_outline{};
    double default_width{2.0};
    Point visual_offset{};
    Point pressed_content_offset{1.0, 1.0};
    friend bool operator==(const ControlVisualRecipe& left,
                           const ControlVisualRecipe& right) noexcept(noexcept(
        left.material == right.material && left.text == right.text &&
        left.muted_text == right.muted_text && left.glyph == right.glyph &&
        left.focus_ring == right.focus_ring &&
        left.default_ring == right.default_ring &&
        left.focus_width == right.focus_width &&
        left.focus_offset == right.focus_offset &&
        left.authored_focus_outline == right.authored_focus_outline &&
        left.default_width == right.default_width &&
        left.visual_offset == right.visual_offset &&
        left.pressed_content_offset == right.pressed_content_offset)) {
        return left.material == right.material && left.text == right.text &&
               left.muted_text == right.muted_text &&
               left.glyph == right.glyph &&
               left.focus_ring == right.focus_ring &&
               left.default_ring == right.default_ring &&
               left.focus_width == right.focus_width &&
               left.focus_offset == right.focus_offset &&
               left.authored_focus_outline == right.authored_focus_outline &&
               left.default_width == right.default_width &&
               left.visual_offset == right.visual_offset &&
               left.pressed_content_offset == right.pressed_content_offset;
    }
};

inline constexpr std::size_t control_visual_role_count =
    static_cast<std::size_t>(ControlVisualRole::count);
inline constexpr std::size_t control_surface_state_count =
    static_cast<std::size_t>(ControlSurfaceState::count);

struct ControlStateRecipes final {
    std::array<ControlVisualRecipe, control_surface_state_count> values;
    [[nodiscard]] static ControlStateRecipes from_parts(
        const ControlVisualRecipe* values, std::size_t value_count);
    [[nodiscard]] const ControlVisualRecipe& resolve(
        ControlSurfaceState state) const noexcept {
        return values[static_cast<std::size_t>(state)];
    }
    friend bool operator==(const ControlStateRecipes& left,
                           const ControlStateRecipes& right) noexcept(noexcept(
        left.values == right.values)) {
        return left.values == right.values;
    }
};

[[nodiscard]] bool valid_control_visual_recipe(
    const ControlVisualRecipe& value) noexcept;
[[nodiscard]] bool valid_control_state_recipes(
    const ControlStateRecipes& value) noexcept;

struct ControlRoleRecipes final {
    std::array<ControlVisualRecipe, control_surface_state_count> ordinary;
    std::array<ControlVisualRecipe, control_surface_state_count> selected;
    std::array<ControlVisualRecipe, control_surface_state_count> high_contrast;
    std::array<ControlVisualRecipe, control_surface_state_count>
        high_contrast_selected;
    friend bool operator==(const ControlRoleRecipes& left,
                           const ControlRoleRecipes& right) noexcept(noexcept(
        left.ordinary == right.ordinary && left.selected == right.selected &&
        left.high_contrast == right.high_contrast &&
        left.high_contrast_selected == right.high_contrast_selected)) {
        return left.ordinary == right.ordinary &&
               left.selected == right.selected &&
               left.high_contrast == right.high_contrast &&
               left.high_contrast_selected == right.high_contrast_selected;
    }
};

struct ThemeSpacingTokens final {
    double micro{2.0};
    double xsmall{4.0};
    double small{6.0};
    double medium{8.0};
    double large{12.0};
    double xlarge{16.0};
    double section{24.0};
    friend constexpr bool operator==(const ThemeSpacingTokens& left,
                                     const ThemeSpacingTokens& right) noexcept {
        return left.micro == right.micro && left.xsmall == right.xsmall &&
               left.small == right.small && left.medium == right.medium &&
               left.large == right.large && left.xlarge == right.xlarge &&
               left.section == right.section;
    }
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
    friend constexpr bool operator==(const ThemeGeometryTokens& left,
                                     const ThemeGeometryTokens& right) noexcept {
        return left.compact_control_height == right.compact_control_height &&
               left.control_height == right.control_height &&
               left.large_control_height == right.large_control_height &&
               left.minimum_touch_target == right.minimum_touch_target &&
               left.splitter_width == right.splitter_width &&
               left.splitter_hit_width == right.splitter_hit_width &&
               left.navigation_extent == right.navigation_extent &&
               left.navigation_minimum == right.navigation_minimum &&
               left.content_minimum == right.content_minimum &&
               left.compact_breakpoint == right.compact_breakpoint;
    }
};
struct ThemeTypographyTokens final {
    FontSpec control{FontRole::control, 12.0, 400, false, 0.24};
    FontSpec field{FontRole::content, 12.0, 400, false};
    FontSpec caption{FontRole::content, 10.0, 400, false};
    FontSpec heading{FontRole::control, 14.0, 650, false, 0.18};
    FontSpec title{FontRole::control, 20.0, 700, false, 0.12};
    FontSpec monospace{FontRole::monospace, 12.0, 400, false};
    friend constexpr bool operator==(const ThemeTypographyTokens& left,
                                     const ThemeTypographyTokens& right) noexcept {
        return left.control == right.control && left.field == right.field &&
               left.caption == right.caption && left.heading == right.heading &&
               left.title == right.title &&
               left.monospace == right.monospace;
    }
};
struct ThemeMotionTokens final {
    std::chrono::milliseconds quick{100};
    std::chrono::milliseconds standard{180};
    std::chrono::milliseconds emphasized{280};
    std::chrono::milliseconds busy_cycle{1400};
    friend constexpr bool operator==(const ThemeMotionTokens& left,
                                     const ThemeMotionTokens& right) noexcept {
        return left.quick == right.quick &&
               left.standard == right.standard &&
               left.emphasized == right.emphasized &&
               left.busy_cycle == right.busy_cycle;
    }
};
struct ThemeStructureTokens final {
    ThemeSpacingTokens spacing;
    ThemeGeometryTokens geometry;
    ThemeTypographyTokens typography;
    ThemeMotionTokens motion;
    friend constexpr bool operator==(const ThemeStructureTokens& left,
                                     const ThemeStructureTokens& right) noexcept {
        return left.spacing == right.spacing &&
               left.geometry == right.geometry &&
               left.typography == right.typography &&
               left.motion == right.motion;
    }
};
struct ThemeDefinition final {
    std::string id{"windows-professional"};
    BasicControlStyle compatibility;
    std::array<ControlRoleRecipes, control_visual_role_count> roles;
    ThemeStructureTokens structure;
    friend bool operator==(const ThemeDefinition& left,
                           const ThemeDefinition& right) noexcept(noexcept(
        left.id == right.id && left.compatibility == right.compatibility &&
        left.roles == right.roles && left.structure == right.structure)) {
        return left.id == right.id &&
               left.compatibility == right.compatibility &&
               left.roles == right.roles && left.structure == right.structure;
    }
};

} // namespace gui_forms
