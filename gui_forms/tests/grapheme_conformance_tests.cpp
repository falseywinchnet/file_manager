#include "gui_forms/text.hpp"

#include "unicode_grapheme_break_cases.hpp"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

using namespace gui_forms;
using namespace gui_forms::unicode_test_data;

void append_utf8(std::string &result, char32_t value) {
  if (value <= 0x7fU) {
    result.push_back(static_cast<char>(value));
  } else if (value <= 0x7ffU) {
    result.push_back(static_cast<char>(0xc0U | (value >> 6U)));
    result.push_back(static_cast<char>(0x80U | (value & 0x3fU)));
  } else if (value <= 0xffffU) {
    result.push_back(static_cast<char>(0xe0U | (value >> 12U)));
    result.push_back(static_cast<char>(0x80U | ((value >> 6U) & 0x3fU)));
    result.push_back(static_cast<char>(0x80U | (value & 0x3fU)));
  } else {
    result.push_back(static_cast<char>(0xf0U | (value >> 18U)));
    result.push_back(static_cast<char>(0x80U | ((value >> 12U) & 0x3fU)));
    result.push_back(static_cast<char>(0x80U | ((value >> 6U) & 0x3fU)));
    result.push_back(static_cast<char>(0x80U | (value & 0x3fU)));
  }
}

[[noreturn]] void fail(const GraphemeBreakCase &test_case,
                       const std::string &reason) {
  std::ostringstream message;
  message << "GraphemeBreakTest-17.0.0.txt line " << test_case.source_line
          << ": " << reason;
  throw std::runtime_error(message.str());
}

void run_case(const GraphemeBreakCase &test_case) {
  std::string text;
  std::vector<std::size_t> scalar_bytes;
  scalar_bytes.push_back(0);
  for (std::size_t index = 0; index < test_case.codepoint_count; ++index) {
    append_utf8(text, codepoints[test_case.codepoint_offset + index]);
    scalar_bytes.push_back(text.size());
  }

  std::vector<std::size_t> expected;
  for (std::size_t index = 0; index < test_case.boundary_count; ++index) {
    const std::size_t scalar = boundaries[test_case.boundary_offset + index];
    if (scalar >= scalar_bytes.size()) {
      fail(test_case, "generated oracle boundary exceeds its scalar sequence");
    }
    expected.push_back(scalar_bytes[scalar]);
  }

  const TextStore store(text);
  if (store.grapheme_count() != GraphemeIndex(expected.size() - 1U)) {
    fail(test_case, "cluster count differs from the official oracle");
  }
  for (std::size_t index = 0; index < expected.size(); ++index) {
    const Utf8Offset actual = store.utf8_offset(GraphemeIndex(index));
    if (actual != Utf8Offset(expected[index])) {
      fail(test_case, "cluster byte boundary differs from the official oracle");
    }
    if (store.grapheme_index(actual) != GraphemeIndex(index) ||
        !store.is_grapheme_boundary(actual)) {
      fail(test_case, "cluster position does not round-trip");
    }
  }
  for (std::size_t scalar = 0; scalar < scalar_bytes.size(); ++scalar) {
    const bool expected_boundary =
        std::find(expected.begin(), expected.end(), scalar_bytes[scalar]) !=
        expected.end();
    if (store.is_grapheme_boundary(Utf8Offset(scalar_bytes[scalar])) !=
        expected_boundary) {
      fail(test_case, "scalar boundary membership differs from the oracle");
    }
  }
}

} // namespace

int main() {
  try {
    if (grapheme_unicode_version() != "17.0.0") {
      throw std::runtime_error("unexpected Unicode grapheme data version");
    }
    for (const GraphemeBreakCase &test_case : cases) {
      run_case(test_case);
    }
    std::cout << "grapheme_conformance_tests: PASS (" << cases.size()
              << " official cases)\n";
    return EXIT_SUCCESS;
  } catch (const std::exception &error) {
    std::cerr << "grapheme_conformance_tests: FAIL: " << error.what() << '\n';
    return EXIT_FAILURE;
  }
}
