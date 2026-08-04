#pragma once

#include <cstdint>
#include <string>
#include <string_view>

namespace gui_forms::gallery {

enum class StyleMode {
    classic_relief,
    quiet_relief,
};

struct GalleryState final {
    std::string text {"Edit this text"};
    std::string selected_category {"gallery.category.basics"};
    std::string selected_collection_row {"gallery.collection.alpha"};
    bool precise_updates {true};
    bool diagnostics_visible {false};
    StyleMode style_mode {StyleMode::classic_relief};
    double slider_value {42.0};
    double progress_value {42.0};
    double instrument_value {0.42};
    std::uint64_t revision {0};
    std::uint64_t activations {0};
};

class GalleryModel final {
public:
    GalleryModel() = default;

    [[nodiscard]] const GalleryState& state() const noexcept;

    bool activate(std::string_view stable_id);
    bool replace_text(std::string text);
    bool set_slider_value(double value);
    bool select_category(std::string_view stable_id);
    bool select_collection_row(std::string_view stable_id);
    void reset();

private:
    GalleryState state_ {};

    void note_change() noexcept;
};

[[nodiscard]] std::string format_percent(double value);

}  // namespace gui_forms::gallery
