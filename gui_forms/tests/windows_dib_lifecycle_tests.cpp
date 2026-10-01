#include <cstdlib>
#include <exception>
#include <iostream>

namespace gui_forms::host {
void run_windows_dib_lifecycle_fixture();
}

int main() {
    try {
        gui_forms::host::run_windows_dib_lifecycle_fixture();
        std::cout << "Hidden owned Window DIB lifecycle fixture passed\n";
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return EXIT_FAILURE;
    }
}
