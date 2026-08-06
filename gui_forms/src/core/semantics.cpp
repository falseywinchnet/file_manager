#include "gui_forms/semantics.hpp"

#include <sstream>

namespace gui_forms {
namespace {

void append_json_string(std::ostringstream& stream, std::string_view value) {
    stream << '"';
    for (const unsigned char character : value) {
        switch (character) {
        case '"': stream << "\\\""; break;
        case '\\': stream << "\\\\"; break;
        case '\n': stream << "\\n"; break;
        case '\r': stream << "\\r"; break;
        case '\t': stream << "\\t"; break;
        default:
            if (character < 0x20U) stream << "?";
            else stream << static_cast<char>(character);
            break;
        }
    }
    stream << '"';
}

void append_node(std::ostringstream& stream, const SemanticNode& node) {
    stream << "{\"runtime_id\":" << node.runtime_id << ",\"stable_id\":";
    append_json_string(stream, node.stable_id);
    stream << ",\"role\":";
    append_json_string(stream, semantic_role_name(node.role));
    stream << ",\"name\":";
    append_json_string(stream, node.name);
    stream << ",\"value\":";
    append_json_string(stream, node.value);
    stream << ",\"description\":";
    append_json_string(stream, node.description);
    stream << ",\"numeric_value\":";
    if (node.numeric_value) stream << *node.numeric_value;
    else stream << "null";
    stream << ",\"minimum_value\":";
    if (node.minimum_value) stream << *node.minimum_value;
    else stream << "null";
    stream << ",\"maximum_value\":";
    if (node.maximum_value) stream << *node.maximum_value;
    else stream << "null";
    stream << ",\"bounds\":[" << node.bounds.x << ',' << node.bounds.y << ','
           << node.bounds.width << ',' << node.bounds.height << "],\"states\":"
           << static_cast<std::uint32_t>(node.states) << ",\"actions\":[";
    for (std::size_t index = 0; index < node.actions.size(); ++index) {
        if (index != 0U) stream << ',';
        append_json_string(stream, semantic_action_name(node.actions[index]));
    }
    stream << "],\"children\":[";
    for (std::size_t index = 0; index < node.children.size(); ++index) {
        if (index != 0U) stream << ',';
        append_node(stream, node.children[index]);
    }
    stream << "]}";
}

} // namespace

const char* semantic_role_name(SemanticRole role) noexcept {
    switch (role) {
    case SemanticRole::generic: return "generic";
    case SemanticRole::group: return "group";
    case SemanticRole::static_text: return "static_text";
    case SemanticRole::button: return "button";
    case SemanticRole::check_box: return "check_box";
    case SemanticRole::radio_button: return "radio_button";
    case SemanticRole::link: return "link";
    case SemanticRole::text_box: return "text_box";
    case SemanticRole::list: return "list";
    case SemanticRole::list_item: return "list_item";
    case SemanticRole::check_list_item: return "check_list_item";
    case SemanticRole::combo_box: return "combo_box";
    case SemanticRole::slider: return "slider";
    case SemanticRole::scroll_bar: return "scroll_bar";
    case SemanticRole::progress_bar: return "progress_bar";
    case SemanticRole::image: return "image";
    case SemanticRole::tab_group: return "tab_group";
    case SemanticRole::tab: return "tab";
    case SemanticRole::split_pane: return "split_pane";
    case SemanticRole::numeric_field: return "numeric_field";
    case SemanticRole::tool_tip: return "tool_tip";
    case SemanticRole::date_picker: return "date_picker";
    case SemanticRole::calendar: return "calendar";
    case SemanticRole::date_cell: return "date_cell";
    case SemanticRole::property_grid: return "property_grid";
    case SemanticRole::property_group: return "property_group";
    case SemanticRole::property_row: return "property_row";
    case SemanticRole::menu_bar: return "menu_bar";
    case SemanticRole::menu_bar_item: return "menu_bar_item";
    case SemanticRole::menu: return "menu";
    case SemanticRole::menu_item: return "menu_item";
    case SemanticRole::separator: return "separator";
    }
    return "generic";
}

const char* semantic_action_name(SemanticAction action) noexcept {
    switch (action) {
    case SemanticAction::focus: return "focus";
    case SemanticAction::press: return "press";
    case SemanticAction::select: return "select";
    case SemanticAction::increment: return "increment";
    case SemanticAction::decrement: return "decrement";
    case SemanticAction::expand: return "expand";
    case SemanticAction::collapse: return "collapse";
    case SemanticAction::show_menu: return "show_menu";
    case SemanticAction::set_value: return "set_value";
    }
    return "unknown";
}

std::string SemanticSnapshot::to_json() const {
    std::ostringstream stream;
    stream << "{\"generation\":" << generation << ",\"node_count\":"
           << node_count << ",\"roots\":[";
    for (std::size_t index = 0; index < roots.size(); ++index) {
        if (index != 0U) stream << ',';
        append_node(stream, roots[index]);
    }
    stream << "]}";
    return stream.str();
}

} // namespace gui_forms
