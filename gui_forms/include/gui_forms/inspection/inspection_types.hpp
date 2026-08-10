#pragma once

#include "gui_forms/input_controls.hpp"
#include "gui_forms/inspection/property_row_spec/property_row_spec.hpp"
#include "gui_forms/inspection/types/property_editor_kind/property_editor_kind.hpp"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace gui_forms {

struct PropertyEditorRequest final {
    std::string stable_id;
    std::string property_path;
    PropertyDescriptor descriptor;
    BindingValue value;
    bool writable{};
    bool top_level{};
};

struct PropertyEditorInputError final {
    std::string attempted_value;
    std::string message;
};

struct PropertyGroupSpec final {
    std::string stable_id;
    std::string title;
    std::vector<PropertyRowSpec> rows;
    bool expanded{true};
};

struct PropertyValueChange final {
    std::string row_id;
    std::string previous_value;
    std::string current_value;
    bool committed{};
};

struct PropertyGroupChange final {
    std::string group_id;
    bool expanded{};
};

struct PropertyRowExpansionChange final {
    std::string row_id;
    bool expanded{};
};

struct PropertyResetRequest final { std::string row_id; };

enum class PropertySort : std::uint8_t { categorized, alphabetical };

struct PropertyGridValueChange final {
    std::string property_name;
    BindingValue previous_value;
    BindingValue current_value;
    PropertyValueOrigin origin{PropertyValueOrigin::computed};
    bool reset{};
};

struct PropertyGridEditError final {
    std::string property_name;
    std::string attempted_value;
    std::string message;
};

} // namespace gui_forms
