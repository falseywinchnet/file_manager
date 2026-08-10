#include "gui_forms/inspection/inspection_types.hpp"

namespace gui_forms {

PropertyRowSpec::PropertyRowSpec(
    std::string authored_stable_id, std::string authored_name,
    std::string authored_value, std::string authored_description,
    PropertyEditorKind authored_editor,
    std::vector<std::string> authored_choices,
    std::string authored_validation, bool authored_enabled,
    bool authored_required, std::string authored_parent_id,
    std::uint8_t authored_depth, bool authored_expandable,
    bool authored_expanded, bool authored_resettable,
    bool authored_reset_enabled)
    : stable_id(std::move(authored_stable_id)),
      name(std::move(authored_name)), value(std::move(authored_value)),
      description(std::move(authored_description)), editor(authored_editor),
      choices(std::move(authored_choices)),
      validation_message(std::move(authored_validation)),
      parent_id(std::move(authored_parent_id)), depth(authored_depth),
      enabled(authored_enabled), required(authored_required),
      expandable(authored_expandable), expanded(authored_expanded),
      resettable(authored_resettable), reset_enabled(authored_reset_enabled) {}

} // namespace gui_forms
