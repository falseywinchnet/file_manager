#include "windows_key_translation.hpp"

#include <array>
#include <cstdlib>
#include <iostream>
#include <stdexcept>

namespace {
using namespace gui_forms;

void require(bool condition, const char* message) {
    if (!condition) { throw std::runtime_error(message); }
}

void test_us_slash_translation() {
    static_assert(PhysicalKey::slash == 0x38U);
    const std::uint32_t usage = host::detail::physical_key_from_virtual_key(VK_OEM_2);
    require(usage == PhysicalKey::slash, "US slash OEM virtual key maps to named HID usage");
    const std::array<WPARAM, 3> unrelated{VK_OEM_1, VK_OEM_3, VK_DIVIDE};
    for (std::size_t index = 0; index < unrelated.size(); ++index) {
        const std::uint32_t other = host::detail::physical_key_from_virtual_key(unrelated[index]);
        require(other == 0, "unadmitted OEM and keypad usages remain unchanged");
    }
}

void test_existing_translation_preserved() {
    for (WPARAM native = 'A'; native <= 'Z'; ++native) {
        const std::uint32_t usage = host::detail::physical_key_from_virtual_key(native);
        const WPARAM relative = native - 'A';
        const std::uint32_t index = static_cast<std::uint32_t>(relative);
        const std::uint32_t expected = index + 0x04U;
        require(usage == expected, "letter mapping preserved");
    }
    for (WPARAM native = '1'; native <= '9'; ++native) {
        const std::uint32_t usage = host::detail::physical_key_from_virtual_key(native);
        const WPARAM relative = native - '1';
        const std::uint32_t index = static_cast<std::uint32_t>(relative);
        const std::uint32_t expected = index + 0x1EU;
        require(usage == expected, "digit mapping preserved");
    }
    struct Mapping final { WPARAM native{}; std::uint32_t expected{}; };
    const std::array<Mapping, 19> existing{{
        {'0', 0x27U}, {VK_RETURN, PhysicalKey::enter}, {VK_ESCAPE, PhysicalKey::escape},
        {VK_BACK, PhysicalKey::backspace}, {VK_TAB, PhysicalKey::tab}, {VK_SPACE, PhysicalKey::space},
        {VK_HOME, PhysicalKey::home}, {VK_PRIOR, PhysicalKey::page_up}, {VK_END, PhysicalKey::end},
        {VK_NEXT, PhysicalKey::page_down}, {VK_DELETE, PhysicalKey::delete_forward},
        {VK_F1, PhysicalKey::f1}, {VK_F2, PhysicalKey::f2}, {VK_F4, PhysicalKey::f4},
        {VK_RIGHT, PhysicalKey::right}, {VK_LEFT, PhysicalKey::left},
        {VK_DOWN, PhysicalKey::down}, {VK_UP, PhysicalKey::up}, {0xffffU, 0}}};
    for (std::size_t index = 0; index < existing.size(); ++index) {
        const Mapping& mapping = existing[index];
        const std::uint32_t usage = host::detail::physical_key_from_virtual_key(mapping.native);
        require(usage == mapping.expected, "existing special key mapping preserved");
    }
}

} // namespace

int main() {
    try {
        test_us_slash_translation();
        test_existing_translation_preserved();
        std::cout << "Windows virtual-key translation fixtures passed; native layout/input delivery not exercised\n";
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return EXIT_FAILURE;
    }
}
