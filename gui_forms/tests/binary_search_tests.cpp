#include "gui_forms/detail/algorithm/binary_search.hpp"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <span>
#include <stdexcept>
#include <vector>

namespace {

void require(bool condition, const char *message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

struct KeyedValue final {
    int key{};
    int payload{};
};

struct KeyedValueLess final {
    [[nodiscard]] constexpr bool operator()(const KeyedValue& left,
                                            int right) const noexcept {
        return left.key < right;
    }

    [[nodiscard]] constexpr bool operator()(int left,
                                            const KeyedValue& right) const noexcept {
        return left < right.key;
    }
};

constexpr std::array<int, 6U> constexpr_values{1, 3, 3, 3, 7, 9};
constexpr std::span<const int, constexpr_values.size()> constexpr_span(
    constexpr_values);

static_assert(gui_forms::detail::lower_bound_index(constexpr_span, 3) == 1U);
static_assert(gui_forms::detail::upper_bound_index(constexpr_span, 3) == 4U);
static_assert(gui_forms::detail::binary_search_contains(constexpr_span, 7));
static_assert(!gui_forms::detail::binary_search_contains(constexpr_span, 8));

void test_empty_and_singleton_ranges() {
    const std::array<int, 0U> empty{};
    const std::span<const int, 0U> empty_span(empty);
    require(gui_forms::detail::lower_bound_index(empty_span, 4) == 0U,
            "lower bound of an empty range must be zero");
    require(gui_forms::detail::upper_bound_index(empty_span, 4) == 0U,
            "upper bound of an empty range must be zero");
    require(!gui_forms::detail::binary_search_contains(empty_span, 4),
            "an empty range must not contain a value");

    const std::array<int, 1U> singleton{4};
    const std::span<const int, 1U> singleton_span(singleton);
    require(gui_forms::detail::lower_bound_index(singleton_span, 3) == 0U &&
                gui_forms::detail::lower_bound_index(singleton_span, 4) == 0U &&
                gui_forms::detail::lower_bound_index(singleton_span, 5) == 1U,
            "singleton lower-bound positions must preserve edge behavior");
    require(gui_forms::detail::upper_bound_index(singleton_span, 3) == 0U &&
                gui_forms::detail::upper_bound_index(singleton_span, 4) == 1U &&
                gui_forms::detail::upper_bound_index(singleton_span, 5) == 1U,
            "singleton upper-bound positions must preserve edge behavior");
}

void test_duplicate_and_heterogeneous_keys() {
    const std::array<KeyedValue, 5U> values{{
        {2, 20}, {4, 40}, {4, 41}, {4, 42}, {8, 80}}};
    const std::span<const KeyedValue, values.size()> span(values);
    const KeyedValueLess less{};
    require(gui_forms::detail::lower_bound_index(span, 4, less) == 1U,
            "heterogeneous lower bound must select the first equal key");
    require(gui_forms::detail::upper_bound_index(span, 4, less) == 4U,
            "heterogeneous upper bound must select the position after equal keys");
    require(gui_forms::detail::binary_search_contains(span, 4, less) &&
                !gui_forms::detail::binary_search_contains(span, 5, less),
            "heterogeneous membership must compare keys in both directions");
}

void test_standard_algorithm_equivalence() {
    std::vector<int> values;
    values.reserve(2048U);
    std::uint32_t state = 0x9e3779b9U;
    for (std::size_t index = 0U; index < 2048U; ++index) {
        state = state * 1664525U + 1013904223U;
        values.push_back(static_cast<int>(state % 257U) - 128);
    }
    std::sort(values.begin(), values.end());
    const std::span<const int> span(values);

    for (int query = -160; query <= 160; ++query) {
        const std::vector<int>::const_iterator standard_lower =
            std::lower_bound(values.cbegin(), values.cend(), query);
        const std::vector<int>::const_iterator standard_upper =
            std::upper_bound(values.cbegin(), values.cend(), query);
        const std::size_t expected_lower = static_cast<std::size_t>(
            std::distance(values.cbegin(), standard_lower));
        const std::size_t expected_upper = static_cast<std::size_t>(
            std::distance(values.cbegin(), standard_upper));
        require(gui_forms::detail::lower_bound_index(span, query) ==
                    expected_lower,
                "house lower bound must match the standard-library control");
        require(gui_forms::detail::upper_bound_index(span, query) ==
                    expected_upper,
                "house upper bound must match the standard-library control");
        require(gui_forms::detail::binary_search_contains(span, query) ==
                    std::binary_search(values.cbegin(), values.cend(), query),
                "house membership must match the standard-library control");
    }
}

} // namespace

int main() {
    try {
        test_empty_and_singleton_ranges();
        test_duplicate_and_heterogeneous_keys();
        test_standard_algorithm_equivalence();
        std::cout << "binary search tests passed\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "binary search tests failed: " << error.what() << '\n';
        return 1;
    }
}
