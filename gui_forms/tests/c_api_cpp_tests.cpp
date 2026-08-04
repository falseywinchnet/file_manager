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
        gui_forms::abi0::Control first(api, "abi.cpp");
        require(first.stable_id() == "abi.cpp" && first.visible(),
                "RAII control did not expose initial retained properties");
        gui_forms::abi0::Control copy = first;
        first.reset();
        require(copy.visible(), "RAII copy did not retain the generational handle");
        copy.set_visible(false);
        require(!copy.visible(), "RAII property mutation failed");
        gui_forms::abi0::Control moved = std::move(copy);
        moved.dispose();
        require(moved.get().slot == 0U, "RAII dispose did not clear local identity");
        std::cout << "gui_forms_c_api_cpp_tests: all tests passed\n";
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        std::cerr << "gui_forms_c_api_cpp_tests: " << error.what() << '\n';
        return EXIT_FAILURE;
    }
}
