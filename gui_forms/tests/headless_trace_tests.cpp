#include "support/headless_trace.hpp"

#include <cstdlib>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {

void require(bool condition, const char* message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

} // namespace

int main() {
    try {
        const std::string first = gui_forms::tests::canonical_lifecycle_trace();
        const std::string second = gui_forms::tests::canonical_lifecycle_trace();
        require(first == second, "canonical lifecycle traces must be byte-identical");
        require(first.find("reparent id=trace.target parent=trace.right") != std::string::npos,
                "trace must contain visual reparenting");
        require(first.find("dispose id=trace.target state=disposed") != std::string::npos,
                "trace must contain deterministic disposal");
        require(first.find("idle frame=0 wake=0 layout=0 paint=0 present=0 callback=0") !=
                    std::string::npos,
                "quiescent trace must prove zero idle work");
        std::cout << first;
        std::cout << "gui_forms_headless_trace_tests: byte-identical replay passed\n";
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        std::cerr << "gui_forms_headless_trace_tests: " << error.what() << '\n';
        return EXIT_FAILURE;
    }
}
