#include "object_order.hpp"

#include <array>
#include <iostream>
#include <stdexcept>

namespace {
struct SortCase final {
    const char* id{};
    const char* name{};
    bool directory{};
    file_manager::EntryKind kind{};
    std::uintmax_t size{};
    std::int64_t modified{};
};

template<file_manager::detail::ObjectSort Mode>
void check_order(const file_manager::detail::ObjectEntries& entries,
                 std::vector<gui_forms::ObjectViewItem> items,
                 const std::array<std::string_view, 6>& expected) {
    std::stable_sort(items.begin(), items.end(),
        file_manager::detail::ObjectOrder<Mode>{entries});
    for (std::size_t index = 0; index < expected.size(); ++index) {
        if (items[index].stable_id != expected[index]) {
            throw std::runtime_error("object order differs at index " + std::to_string(index));
        }
    }
}
}

int main() {
    try {
        using file_manager::EntryKind;
        using file_manager::detail::ObjectSort;
        const std::array<SortCase, 6> cases{{
            {"a", "Alpha", false, EntryKind::document, 20, 20},
            {"b", "alpha", false, EntryKind::image, 10, 10},
            {"c", "beta", false, EntryKind::document, 10, 30},
            {"d", "Alph", false, EntryKind::archive, 30, 10},
            {"u", "é.txt", false, EntryKind::other, 5, 40},
            {"f", "Zfolder", true, EntryKind::folder, 999, 0},
        }};
        file_manager::detail::ObjectEntries entries{};
        std::vector<gui_forms::ObjectViewItem> items{};
        items.reserve(cases.size());
        for (const SortCase& example : cases) {
            file_manager::DirectoryEntry entry{};
            entry.stable_id = example.id;
            entry.name = example.name;
            entry.directory = example.directory;
            entry.kind = example.kind;
            entry.identity.size = example.size;
            entry.identity.modified_nanoseconds = example.modified;
            entries.emplace(example.id, entry);
            gui_forms::ObjectViewItem item{};
            item.stable_id = example.id;
            item.name = example.name;
            items.push_back(std::move(item));
        }
        check_order<ObjectSort::name>(entries, items, {"f", "d", "a", "b", "c", "u"});
        check_order<ObjectSort::kind>(entries, items, {"f", "a", "c", "b", "d", "u"});
        check_order<ObjectSort::size>(entries, items, {"f", "u", "b", "c", "a", "d"});
        check_order<ObjectSort::modified>(entries, items, {"f", "u", "c", "a", "d", "b"});
        // Missing observations preserve the original visible-name fallback.
        const file_manager::detail::ObjectEntries absent{};
        check_order<ObjectSort::name>(absent, items, {"d", "a", "b", "c", "f", "u"});
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
