#include "house_art.hpp"

#include "gui_forms/gui_forms.hpp"

#include <iostream>
#include <memory>
#include <stdexcept>

namespace {

void require(const bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}

} // namespace

int main() {
    try {
        auto root = std::make_shared<gui_forms::Panel>(
            gui_forms::StableId("house-art.test.root"));
        gui_forms::Window window(root, {120.0, 80.0});
        const auto images = file_manager::house_art::make_image_list(window, 22.0);
        require(images->count() == file_manager::house_art::icon_keys.size(),
                "every House icon key must produce exactly one ImageList entry");
        for (const auto& [icon, key] : file_manager::house_art::icon_keys) {
            static_cast<void>(icon);
            require(!key.empty() && images->contains_key(key),
                    "House icon keys must be nonempty and resolvable");
            const auto first = images->resolve(key, gui_forms::ImageVisualState::normal,
                                               1.0);
            const auto dense = images->resolve(key, gui_forms::ImageVisualState::normal,
                                               2.0);
            require(first && dense && first.resolved_scale == 1.0 &&
                        dense.resolved_scale == 2.0,
                    "House icons must provide deterministic 1x and 2x rasters");
        }
        require(window.image_resource_snapshot().resource_count ==
                    file_manager::house_art::icon_keys.size() * 2U,
                "House art must retain exactly its bounded density resources");
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
