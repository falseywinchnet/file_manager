#pragma once

#include "gui_forms/types.hpp"

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace gui_forms {

enum class SemanticRole : std::uint8_t {
    generic,
    group,
    static_text,
    button,
    check_box,
    radio_button,
    link,
    text_box,
    list,
    list_item,
    check_list_item,
    combo_box,
    slider,
    scroll_bar,
    progress_bar,
    image,
    tab_group,
    tab,
    split_pane,
    numeric_field,
    tool_tip,
    date_picker,
    calendar,
    date_cell,
    property_grid,
    property_group,
    property_row,
    menu_bar,
    menu_bar_item,
    menu,
    menu_item,
    separator,
};

enum class SemanticState : std::uint32_t {
    none = 0,
    enabled = 1U << 0U,
    visible = 1U << 1U,
    focusable = 1U << 2U,
    focused = 1U << 3U,
    selected = 1U << 4U,
    checked = 1U << 5U,
    mixed = 1U << 6U,
    read_only = 1U << 7U,
    expanded = 1U << 8U,
    busy = 1U << 9U,
    protected_content = 1U << 10U,
};

[[nodiscard]] constexpr SemanticState operator|(SemanticState left,
                                                 SemanticState right) noexcept {
    return static_cast<SemanticState>(static_cast<std::uint32_t>(left) |
                                      static_cast<std::uint32_t>(right));
}
constexpr SemanticState& operator|=(SemanticState& left,
                                    SemanticState right) noexcept {
    left = left | right;
    return left;
}
[[nodiscard]] constexpr bool has_semantic_state(SemanticState states,
                                                 SemanticState requested) noexcept {
    return (static_cast<std::uint32_t>(states) &
            static_cast<std::uint32_t>(requested)) != 0U;
}

enum class SemanticAction : std::uint8_t {
    focus,
    press,
    select,
    increment,
    decrement,
    expand,
    collapse,
    show_menu,
    set_value,
};

struct SemanticDescriptor final {
    SemanticRole role{SemanticRole::generic};
    std::string name;
    std::string value;
    std::string description;
    std::optional<double> numeric_value;
    std::optional<double> minimum_value;
    std::optional<double> maximum_value;
    SemanticState states{SemanticState::none};
    std::vector<SemanticAction> actions;
    bool exposed{};
    bool include_descendants{true};
};

struct SemanticNode final {
    std::uint64_t runtime_id{};
    std::string stable_id;
    SemanticRole role{SemanticRole::generic};
    std::string name;
    std::string value;
    std::string description;
    std::optional<double> numeric_value;
    std::optional<double> minimum_value;
    std::optional<double> maximum_value;
    Rect bounds{};
    SemanticState states{SemanticState::none};
    std::vector<SemanticAction> actions;
    std::vector<SemanticNode> children;
};

struct SemanticSnapshot final {
    std::uint64_t generation{};
    std::size_t node_count{};
    std::vector<SemanticNode> roots;

    [[nodiscard]] std::string to_json() const;
};

[[nodiscard]] const char* semantic_role_name(SemanticRole role) noexcept;
[[nodiscard]] const char* semantic_action_name(SemanticAction action) noexcept;

} // namespace gui_forms
