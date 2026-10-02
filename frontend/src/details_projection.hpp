#pragma once

#include "file_manager/filesystem_model.hpp"
#include "gui_forms/gui_forms.hpp"

namespace file_manager::detail {

// Provisional application defaults; the interviews require factual headers but
// do not settle their initial order. Widths are logical units and remain owned
// by ObjectView after user resizing within the current session.
inline std::vector<gui_forms::ObjectDetailsColumn> default_details_columns() {
    using gui_forms::ObjectColumnAlignment;
    std::vector<gui_forms::ObjectDetailsColumn> columns{
        {.id = {"name"}, .label = "Name", .width = 260.0, .minimum_width = 120.0},
        {.id = {"kind"}, .label = "Type", .width = 120.0, .minimum_width = 90.0},
        {.id = {"size"}, .label = "Size", .width = 104.0, .minimum_width = 72.0,
         .alignment = ObjectColumnAlignment::right},
        {.id = {"modified"}, .label = "Date modified", .width = 172.0, .minimum_width = 132.0},
    };
    return columns;
}

inline std::vector<gui_forms::ObjectDetailsCell> details_cells(
    const DirectoryEntry& entry, const std::string& display_name,
    const std::string& type_text) {
    using gui_forms::ObjectCellAvailability;
    ObjectCellAvailability type_state = ObjectCellAvailability::unavailable;
    ObjectCellAvailability size_state = ObjectCellAvailability::unavailable;
    ObjectCellAvailability modified_state = ObjectCellAvailability::unavailable;
    std::string size_text{"Unavailable"};
    std::string modified_text{"Unavailable"};
    std::string observed_type{"Unavailable"};
    if (entry.identity.available()) {
        type_state = ObjectCellAvailability::available;
        observed_type = type_text;
        if (entry.identity.type == std::filesystem::file_type::regular) {
            if (entry.metadata.logical_size) {
                size_text = format_bytes(*entry.metadata.logical_size);
                size_state = ObjectCellAvailability::available;
            }
        } else if (entry.identity.type != std::filesystem::file_type::none &&
                   entry.identity.type != std::filesystem::file_type::unknown) {
            size_text = "—";
            size_state = ObjectCellAvailability::not_applicable;
        }
        if (entry.metadata.modified && !entry.modified_text.empty() &&
            entry.modified_text != "Unavailable") {
            modified_text = entry.modified_text;
            modified_state = ObjectCellAvailability::available;
        }
    }
    std::vector<gui_forms::ObjectDetailsCell> cells{};
    cells.reserve(4U);
    cells.push_back({{"name"}, display_name, ObjectCellAvailability::available});
    cells.push_back({{"kind"}, std::move(observed_type), type_state});
    cells.push_back({{"size"}, std::move(size_text), size_state});
    cells.push_back({{"modified"}, std::move(modified_text), modified_state});
    return cells;
}

} // namespace file_manager::detail
