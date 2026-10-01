#pragma once

#include "file_manager/document_picker.hpp"

namespace file_manager {
// Only read-selection profiles admit a trusted file alias. Save/Export and
// folder selection retain their separate leaf policies.
[[nodiscard]] inline bool picker_reads_files(const DocumentPickerProfile profile) noexcept {
    const bool reads = profile == DocumentPickerProfile::open_file ||
        profile == DocumentPickerProfile::open_files ||
        profile == DocumentPickerProfile::import_files;
    return reads;
}
} // namespace file_manager
