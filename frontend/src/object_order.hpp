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
    bool descending{Mode == ObjectSort::modified};
    bool operator()(const gui_forms::ObjectViewItem& left,
                    const gui_forms::ObjectViewItem& right) const {
        const ObjectEntries::const_iterator left_entry = entries.find(left.stable_id);
        const ObjectEntries::const_iterator right_entry = entries.find(right.stable_id);
        if (left_entry == entries.end() || right_entry == entries.end()) {
            if (left_entry != entries.end()) return true;
            if (right_entry != entries.end()) return false;
            const bool less = descending ? folded_name_less(right.name, left.name)
                                         : folded_name_less(left.name, right.name);
            return less;
        }
        const DirectoryEntry& a = (*left_entry).second;
        const DirectoryEntry& b = (*right_entry).second;
        if (a.directory != b.directory) return a.directory;
        if constexpr (Mode == ObjectSort::kind) {
            if (a.identity.available() != b.identity.available()) return a.identity.available();
            if (a.identity.available() && a.kind != b.kind) {
                const bool less = descending ? b.kind < a.kind : a.kind < b.kind;
                return less;
            }
        } else if constexpr (Mode == ObjectSort::size) {
            if (a.metadata.logical_size.has_value() != b.metadata.logical_size.has_value()) {
                return a.metadata.logical_size.has_value();
            }
            if (a.metadata.logical_size && *a.metadata.logical_size != *b.metadata.logical_size) {
                const bool less = descending ? *b.metadata.logical_size < *a.metadata.logical_size
                                             : *a.metadata.logical_size < *b.metadata.logical_size;
                return less;
            }
        } else if constexpr (Mode == ObjectSort::modified) {
            if (a.metadata.modified.has_value() != b.metadata.modified.has_value()) {
                return a.metadata.modified.has_value();
            }
            if (a.metadata.modified) {
                const ObservedFileTime& first = *a.metadata.modified;
                const ObservedFileTime& second = *b.metadata.modified;
                if (first.unix_seconds != second.unix_seconds) {
                    const bool less = descending ? second.unix_seconds < first.unix_seconds
                                                 : first.unix_seconds < second.unix_seconds;
                    return less;
                }
                if (first.nanoseconds != second.nanoseconds) {
                    const bool less = descending ? second.nanoseconds < first.nanoseconds
                                                 : first.nanoseconds < second.nanoseconds;
                    return less;
                }
            }
        }
        if constexpr (Mode == ObjectSort::name) {
            if (descending) {
                const bool less = folded_name_less(b.name, a.name);
                return less;
            }
        }
        const bool less = folded_name_less(a.name, b.name);
        return less;
    }
};

} // namespace file_manager::detail
