#include "gallery_model.hpp"

#include "gallery_dml.hpp"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <utility>

namespace gui_forms::gallery {
namespace {

[[nodiscard]] bool is_category(std::string_view id) noexcept
{
    return id == "gallery.category.basics" || id == "gallery.category.values" ||
           id == "gallery.category.collections" || id == "gallery.category.instrument";
}

[[nodiscard]] bool is_collection_row(std::string_view id) noexcept
{
    return id == "gallery.collection.alpha" || id == "gallery.collection.beta" ||
           id == "gallery.collection.gamma" || id == "gallery.collection.delta";
}

}  // namespace

const GalleryState& GalleryModel::state() const noexcept
{
    return state_;
}

bool GalleryModel::activate(std::string_view stable_id)
{
    const dml::NodeSpec* const node = dml::find(stable_id);
    if (node == nullptr || (node->flags & dml::enabled) == 0U) {
        return false;
    }

    bool changed = false;
    if (stable_id == "gallery.command.reset") {
        reset();
        ++state_.activations;
        return true;
    }
    if (stable_id == "gallery.command.diagnostics") {
        state_.diagnostics_visible = !state_.diagnostics_visible;
        changed = true;
    } else if (stable_id == "gallery.default-button") {
        ++state_.activations;
        note_change();
        return true;
    } else if (stable_id == "gallery.checkbox") {
        state_.precise_updates = !state_.precise_updates;
        changed = true;
    } else if (stable_id == "gallery.radio.classic") {
        changed = state_.style_mode != StyleMode::classic_relief;
        state_.style_mode = StyleMode::classic_relief;
    } else if (stable_id == "gallery.radio.quiet") {
        changed = state_.style_mode != StyleMode::quiet_relief;
        state_.style_mode = StyleMode::quiet_relief;
    } else if (is_category(stable_id)) {
        changed = select_category(stable_id);
    } else if (is_collection_row(stable_id)) {
        changed = select_collection_row(stable_id);
    } else {
        return false;
    }

    ++state_.activations;
    if (changed && !is_category(stable_id) && !is_collection_row(stable_id)) {
        note_change();
    }
    return true;
}

bool GalleryModel::replace_text(std::string text)
{
    if (state_.text == text) {
        return false;
    }
    state_.text = std::move(text);
    note_change();
    return true;
}

bool GalleryModel::set_slider_value(double value)
{
    if (!std::isfinite(value)) {
        return false;
    }

    const double bounded = std::clamp(value, 0.0, 100.0);
    if (bounded == state_.slider_value) {
        return false;
    }

    state_.slider_value = bounded;
    state_.progress_value = bounded;
    state_.instrument_value = bounded / 100.0;
    note_change();
    return true;
}

bool GalleryModel::select_category(std::string_view stable_id)
{
    if (!is_category(stable_id) || dml::find(stable_id) == nullptr ||
        state_.selected_category == stable_id) {
        return false;
    }
    state_.selected_category.assign(stable_id);
    note_change();
    return true;
}

bool GalleryModel::select_collection_row(std::string_view stable_id)
{
    if (!is_collection_row(stable_id) || dml::find(stable_id) == nullptr ||
        state_.selected_collection_row == stable_id) {
        return false;
    }
    state_.selected_collection_row.assign(stable_id);
    note_change();
    return true;
}

void GalleryModel::reset()
{
    const std::uint64_t next_revision = state_.revision + 1U;
    const std::uint64_t activation_count = state_.activations;
    state_ = GalleryState {};
    state_.revision = next_revision;
    state_.activations = activation_count;
}

void GalleryModel::note_change() noexcept
{
    ++state_.revision;
}

std::string format_percent(double value)
{
    std::ostringstream output;
    output << std::fixed << std::setprecision(0) << std::clamp(value, 0.0, 100.0) << '%';
    return output.str();
}

}  // namespace gui_forms::gallery
