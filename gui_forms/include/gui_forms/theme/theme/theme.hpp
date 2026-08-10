#pragma once

#include "gui_forms/theme/types/theme_types.hpp"

#include <memory>
#include <string_view>
#include <utility>

namespace gui_forms {

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
