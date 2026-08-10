#pragma once

#include "gui_forms/input_controls.hpp"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace gui_forms {

enum class PropertyEditorKind : std::uint8_t {
    read_only,
    text,
    choice,
    boolean,
    custom,
};

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

struct PropertyRowSpec final {
    PropertyRowSpec() = default;
    PropertyRowSpec(std::string authored_stable_id,
                    std::string authored_name,
                    std::string authored_value,
                    std::string authored_description = {},
                    PropertyEditorKind authored_editor =
                        PropertyEditorKind::read_only,
                    std::vector<std::string> authored_choices = {},
                    std::string authored_validation = {},
                    bool authored_enabled = true,
                    bool authored_required = false,
                    std::string authored_parent_id = {},
                    std::uint8_t authored_depth = 0,
                    bool authored_expandable = false,
                    bool authored_expanded = false,
                    bool authored_resettable = false,
                    bool authored_reset_enabled = false);

    std::string stable_id;
    std::string name;
    std::string value;
    std::string description;
    PropertyEditorKind editor{PropertyEditorKind::read_only};
    std::vector<std::string> choices;
    std::string validation_message;
    std::string parent_id;
    std::uint8_t depth{};
    bool enabled{true};
    bool required{};
    bool expandable{};
    bool expanded{};
    bool resettable{};
    bool reset_enabled{};
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
