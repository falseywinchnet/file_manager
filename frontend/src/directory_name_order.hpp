#pragma once

#include "file_manager/filesystem_model.hpp"

#include <algorithm>
#include <cctype>

namespace file_manager::detail {

// Comparison borrows names for this call and performs no allocation.
struct DirectoryNameOrder final {
    bool operator()(const DirectoryEntry& left, const DirectoryEntry& right) const {
        if (left.directory != right.directory) return left.directory;
        const std::size_t shared_length = std::min(left.name.size(), right.name.size());
        for (std::size_t index = 0; index < shared_length; ++index) {
            const unsigned char left_byte = static_cast<unsigned char>(left.name[index]);
            const unsigned char right_byte = static_cast<unsigned char>(right.name[index]);
            const int left_lower = std::tolower(left_byte);
            const int right_lower = std::tolower(right_byte);
            if (left_lower != right_lower) {
                const bool less = left_lower < right_lower;
                return less;
            }
        }
        if (left.name.size() != right.name.size()) {
            const bool shorter = left.name.size() < right.name.size();
            return shorter;
        }
        const bool less = left.name < right.name;
        return less;
    }
};

} // namespace file_manager::detail
