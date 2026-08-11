#include "file_manager/internal_drag.hpp"

#include <cstdlib>
#include <iostream>
#include <string_view>

namespace {

void require(const bool condition, const std::string_view message) {
    if (!condition) {
        std::cerr << "internal drag test failed: " << message << '\n';
        std::exit(1);
    }
}

} // namespace

int main() {
    file_manager::InternalDragController drag;

    require(!drag.observe({file_manager::DragPointerPhase::down, 10, 10,
                           "source", false}),
            "press must not publish a drop");
    require(!drag.observe({file_manager::DragPointerPhase::up, 12, 12,
                           "folder", false}),
            "a click-sized displacement must not become a drag");

    (void)drag.observe({file_manager::DragPointerPhase::down, 10, 10,
                        "source", false});
    (void)drag.observe({file_manager::DragPointerPhase::move, 18, 10,
                        "folder", false});
    require(drag.active(), "threshold-crossing move must activate drag");
    const auto move = drag.observe({file_manager::DragPointerPhase::up, 18, 10,
                                    "folder", false});
    require(move && move->source_id == "source" &&
                move->destination_id == "folder" && !move->copy,
            "ordinary drop must publish a move intent");

    (void)drag.observe({file_manager::DragPointerPhase::down, 3, 4,
                        "source", false});
    const auto copy = drag.observe({file_manager::DragPointerPhase::up, 20, 4,
                                    "folder", true});
    require(copy && copy->copy,
            "a displaced release without an intermediate move must preserve copy modifier");

    (void)drag.observe({file_manager::DragPointerPhase::down, 0, 0,
                        "source", false});
    require(!drag.observe({file_manager::DragPointerPhase::up, 20, 0,
                           "source", false}),
            "source cannot accept itself");

    (void)drag.observe({file_manager::DragPointerPhase::down, 0, 0,
                        "source", false});
    (void)drag.observe({file_manager::DragPointerPhase::cancel, 20, 0,
                        "folder", false});
    require(!drag.active() && drag.source_id().empty(),
            "cancellation must discard gesture state");

    std::cout << "internal drag tests passed\n";
    return 0;
}
