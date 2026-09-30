#pragma once

#include "file_manager/filesystem_model.hpp"
#include "gui_forms/gui_forms.hpp"
#include <algorithm>
#include <cctype>
#include <unordered_map>

namespace file_manager::detail {

// Byte folding preserves the existing locale-based string comparison. Equal
// folded names remain equivalent, allowing stable_sort to preserve input order.
inline bool folded_name_less(const std::string& left, const std::string& right) {
    const std::size_t shared = std::min(left.size(), right.size());
    for (std::size_t index = 0; index < shared; ++index) {
        const unsigned char a = static_cast<unsigned char>(left[index]);
        const unsigned char b = static_cast<unsigned char>(right[index]);
        const int folded_a = std::tolower(a);
        const int folded_b = std::tolower(b);
        if (folded_a != folded_b) {
            const bool less = folded_a < folded_b;
            return less;
        }
    }
    const bool shorter = left.size() < right.size();
    return shorter;
}

enum class ObjectSort { name, kind, size, modified };
using ObjectEntries = std::unordered_map<std::string, DirectoryEntry>;

// Borrows the immutable entry map only for synchronous stable_sort. The mode
// is selected before sorting and comparisons allocate no lowercase strings.
template<ObjectSort Mode>
struct ObjectOrder final {
    const ObjectEntries& entries;
    bool operator()(const gui_forms::ObjectViewItem& left,
                    const gui_forms::ObjectViewItem& right) const {
        const ObjectEntries::const_iterator left_entry = entries.find(left.stable_id);
        const ObjectEntries::const_iterator right_entry = entries.find(right.stable_id);
        if (left_entry == entries.end() || right_entry == entries.end()) {
            const bool less = folded_name_less(left.name, right.name);
            return less;
        }
        const DirectoryEntry& a = (*left_entry).second;
        const DirectoryEntry& b = (*right_entry).second;
        if (a.directory != b.directory) return a.directory;
        if constexpr (Mode == ObjectSort::kind) {
            if (a.kind != b.kind) {
                const bool less = a.kind < b.kind;
                return less;
            }
        } else if constexpr (Mode == ObjectSort::size) {
            if (a.identity.size != b.identity.size) {
                const bool less = a.identity.size < b.identity.size;
                return less;
            }
        } else if constexpr (Mode == ObjectSort::modified) {
            if (a.identity.modified_nanoseconds != b.identity.modified_nanoseconds) {
                const bool newer = a.identity.modified_nanoseconds > b.identity.modified_nanoseconds;
                return newer;
            }
        }
        const bool less = folded_name_less(a.name, b.name);
        return less;
    }
};

} // namespace file_manager::detail
