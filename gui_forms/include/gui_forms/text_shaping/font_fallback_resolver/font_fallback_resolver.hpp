#pragma once

#include "gui_forms/text_shaping/types/text_shaping_types.hpp"

#include <optional>

namespace gui_forms {

class FontFallbackResolver {
public:
    virtual ~FontFallbackResolver() = default;
    [[nodiscard]] virtual std::optional<FontFallbackMatch> resolve(
        const FontFallbackRequest& request) = 0;
};

} // namespace gui_forms
