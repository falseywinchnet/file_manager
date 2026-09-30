#include "file_manager/internal_drag.hpp"

#include <cstdlib>
#include <stdexcept>
#include <iostream>
#include <string_view>

namespace {

void require(const bool condition, const std::string_view message) {
    if (!condition) {
        std::cerr << "internal drag test failed: " << message << '\n';
        throw std::runtime_error(std::string(message));
    }
}

} // namespace

int main() {
    try {
        file_manager::InternalDragController drag{};

        const std::optional<file_manager::InternalDropIntent> press = drag.observe({file_manager::DragPointerPhase::down, 10, 10,
                               "source", false});
        require(!press,
                "press must not publish a drop");
        const std::optional<file_manager::InternalDropIntent> click = drag.observe({file_manager::DragPointerPhase::up, 12, 12,
                               "folder", false});
        require(!click,
                "a click-sized displacement must not become a drag");

        static_cast<void>(drag.observe({file_manager::DragPointerPhase::down, 10, 10,
                            "source", false}));
        static_cast<void>(drag.observe({file_manager::DragPointerPhase::move, 18, 10,
                            "folder", false}));
        require(drag.active(), "threshold-crossing move must activate drag");
        const std::optional<file_manager::InternalDropIntent> move = drag.observe({file_manager::DragPointerPhase::up, 18, 10,
                                        "folder", false});
        require(move && (*move).source_id == "source" &&
                    (*move).destination_id == "folder" && !(*move).copy,
                "ordinary drop must publish a move intent");

        static_cast<void>(drag.observe({file_manager::DragPointerPhase::down, 3, 4,
                            "source", false}));
        const std::optional<file_manager::InternalDropIntent> copy = drag.observe({file_manager::DragPointerPhase::up, 20, 4,
                                        "folder", true});
        require(copy && (*copy).copy,
                "a displaced release without an intermediate move must preserve copy modifier");

        static_cast<void>(drag.observe({file_manager::DragPointerPhase::down, 0, 0,
                            "source", false}));
        const std::optional<file_manager::InternalDropIntent> self_drop = drag.observe({file_manager::DragPointerPhase::up, 20, 0,
                               "source", false});
        require(!self_drop,
                "source cannot accept itself");

        static_cast<void>(drag.observe({file_manager::DragPointerPhase::down, 0, 0,
                            "source", false}));
        static_cast<void>(drag.observe({file_manager::DragPointerPhase::cancel, 20, 0,
                            "folder", false}));
        require(!drag.active() && drag.source_id().empty(),
                "cancellation must discard gesture state");

        std::cout << "internal drag tests passed\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
