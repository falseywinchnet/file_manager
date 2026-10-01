#include "unicode_grapheme.hpp"
#include "gui_forms/text.hpp"

#include <algorithm>
#include <array>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <utility>

namespace {
using gui_forms::detail::GraphemeBoundaryBuffer;
using gui_forms::detail::GraphemeStorageRequirement;
using gui_forms::detail::GraphemeStorageStatus;
using gui_forms::detail::bounded_grapheme_boundaries;
using gui_forms::detail::extended_grapheme_boundaries;
using gui_forms::detail::grapheme_storage_requirement;

void require(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}

void compare_existing(std::string_view text) {
    const gui_forms::Utf8ValidationResult validation = gui_forms::validate_utf8(text);
    require(validation.valid(), "valid comparison input");
    const std::vector<std::size_t> expected = extended_grapheme_boundaries(text, validation.scalar_count);
    const GraphemeStorageRequirement requirement = grapheme_storage_requirement(text);
    require(requirement.status == GraphemeStorageStatus::success, "valid storage calculation");
    require(requirement.peak_bytes == requirement.scalar_bytes + requirement.boundary_bytes,
            "all simultaneous arrays counted");
    GraphemeBoundaryBuffer bounded{};
    const GraphemeStorageStatus accepted = bounded_grapheme_boundaries(text, requirement.peak_bytes, bounded);
    require(accepted == GraphemeStorageStatus::success, "exact workspace limit accepted");
    const std::span<const std::size_t> actual = bounded.boundaries();
    require(actual.size() == expected.size(), "grapheme count preserved");
    require(std::equal(actual.begin(), actual.end(), expected.begin()), "grapheme boundaries preserved");
    require(bounded.capacity_bytes() == requirement.boundary_bytes, "exact retained array capacity");
    GraphemeBoundaryBuffer refused{};
    const GraphemeStorageStatus insufficient = bounded_grapheme_boundaries(text, requirement.peak_bytes - 1U, refused);
    require(insufficient == GraphemeStorageStatus::budget_exceeded, "one byte below peak refused");
    require(refused.boundaries().empty(), "refusal did not allocate output");
}

void test_equivalence() {
    const std::array<std::string_view, 9> cases{
        "", "abc", "\r\n", "a\r\nb\nc", "a\xcc\x81",
        "\xd7\x90\xd7\x91", "\xf0\x9f\x87\xba\xf0\x9f\x87\xb8",
        "\xf0\x9f\x91\xa9\xe2\x80\x8d\xf0\x9f\x91\xa9",
        "\xe0\xa4\x95\xe0\xa5\x8d\xe0\xa4\xb7"};
    for (std::size_t index = 0; index < cases.size(); ++index) compare_existing(cases[index]);
    std::string marks("a");
    marks.reserve(16'383);
    for (std::size_t index = 0; index < 8191; ++index) marks.append("\xcc\x81");
    compare_existing(marks);
}

void test_preservation_and_moves() {
    GraphemeBoundaryBuffer output{};
    const GraphemeStorageRequirement first = grapheme_storage_requirement("abc");
    const GraphemeStorageStatus initial = bounded_grapheme_boundaries("abc", first.peak_bytes, output);
    require(initial == GraphemeStorageStatus::success, "initial boundary output");
    const std::size_t retained = output.capacity_bytes();
    const std::span<const std::size_t> old = output.boundaries();
    const std::size_t* old_address = old.data();
    const GraphemeStorageRequirement replacement = grapheme_storage_requirement("xy");
    const GraphemeStorageStatus overlap_refused = bounded_grapheme_boundaries("xy",
        retained + replacement.peak_bytes - 1U, output);
    require(overlap_refused == GraphemeStorageStatus::budget_exceeded, "old output counted in replacement peak");
    require(output.boundaries().data() == old_address && output.boundaries().size() == 4,
            "failed replacement preserved old owner");
    const std::array<std::string_view, 4> malformed{
        "\x80", "\xf0\x9f\x98", "\xc0\xaf", "\xed\xa0\x80"};
    for (std::size_t index = 0; index < malformed.size(); ++index) {
        const GraphemeStorageStatus invalid = bounded_grapheme_boundaries(malformed[index],
            std::numeric_limits<std::size_t>::max(), output);
        require(invalid == GraphemeStorageStatus::invalid_utf8, "malformed input refused");
        require(output.boundaries().data() == old_address, "invalid input preserved output");
    }
    const GraphemeStorageStatus replaced = bounded_grapheme_boundaries("xy",
        retained + replacement.peak_bytes, output);
    require(replaced == GraphemeStorageStatus::success, "exact old plus new budget accepted");
    GraphemeBoundaryBuffer moved(std::move(output));
    require(output.boundaries().empty() && output.capacity_bytes() == 0, "move constructor emptied old owner");
    require(moved.boundaries().size() == 3 && moved.boundaries()[2] == 2, "move retained boundaries");
    output = std::move(moved);
    require(moved.boundaries().empty() && moved.capacity_bytes() == 0, "move assignment emptied old owner");
    require(output.boundaries().size() == 3, "move assignment retained boundaries");
}
} // namespace

int main() {
    try {
        test_equivalence();
        test_preservation_and_moves();
        std::cout << "Bounded grapheme allocation and equivalence fixtures passed\n";
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return EXIT_FAILURE;
    }
}
