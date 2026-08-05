#include "gui_forms/c_api.hpp"

#include <cstdlib>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <utility>

namespace {

void require(bool condition, const char* message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

} // namespace

int main() {
    try {
        gui_forms::abi0::Api api;
        gui_forms::abi0::Control first(api, "abi.cpp", GF_CONTROL_BUTTON);
        require(first.stable_id() == "abi.cpp" && first.visible(),
                "RAII control did not expose initial retained properties");
        gui_forms::abi0::Control copy = first;
        first.reset();
        require(copy.visible(), "RAII copy did not retain the generational handle");
        copy.set_visible(false);
        require(!copy.visible(), "RAII property mutation failed");
        copy.set_name("startButton");
        copy.set_text("Start radio");
        copy.set_enabled(false);
        require(copy.name() == "startButton" && copy.text() == "Start radio" &&
                    !copy.enabled(),
                "RAII ABI 0.2 property round trip failed");
        gui_forms::abi0::Control moved = std::move(copy);
        moved.dispose();
        require(moved.get().slot == 0U, "RAII dispose did not clear local identity");
        gui_forms::abi0::Control window(api, "abi.cpp.window", GF_CONTROL_FORM);
        window.run_window(GF_WINDOW_RUN_FORCE_HEADLESS);
        require(window.last_host_trace().find("event=attach") != std::string::npos,
                "RAII ABI 0.3 headless window trace failed");
        require(window.callback_fault_count() == 0,
                "RAII ABI 0.4 callback fault count changed");
        window.dispose();
        std::cout << "gui_forms_c_api_cpp_tests: all tests passed\n";
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        std::cerr << "gui_forms_c_api_cpp_tests: " << error.what() << '\n';
        return EXIT_FAILURE;
    }
}
