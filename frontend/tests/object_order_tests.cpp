#include "object_order.hpp"
#include "details_projection.hpp"

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
                 const std::array<std::string_view, 6>& expected,
                 const bool descending = Mode == file_manager::detail::ObjectSort::modified) {
    std::stable_sort(items.begin(), items.end(),
        file_manager::detail::ObjectOrder<Mode>{entries, descending});
    for (std::size_t index = 0; index < expected.size(); ++index) {
        if (items[index].stable_id != expected[index]) {
            throw std::runtime_error("object order differs at index " + std::to_string(index));
        }
    }
}

void check_factual_projection() {
    using gui_forms::ObjectCellAvailability;
    file_manager::DirectoryEntry entry{};
    entry.name = "zero.txt";
    entry.identity.inode = 1U;
    entry.identity.type = std::filesystem::file_type::regular;
    entry.metadata.logical_size = 0U;
    entry.metadata.modified = file_manager::ObservedFileTime{-1, 999999999U};
    entry.modified_text = "Unavailable";
    std::vector<gui_forms::ObjectDetailsCell> cells =
        file_manager::detail::details_cells(entry, entry.name, "Document");
    if (cells.size() != 4U || cells[2U].availability != ObjectCellAvailability::available ||
        cells[2U].text != file_manager::format_bytes(0U) ||
        cells[3U].availability != ObjectCellAvailability::unavailable) {
        throw std::runtime_error("zero bytes and unformattable time need distinct factual states");
    }
    entry.metadata.logical_size.reset();
    entry.identity.size = 999U;
    cells = file_manager::detail::details_cells(entry, entry.name, "Document");
    if (cells[2U].availability != ObjectCellAvailability::unavailable || cells[2U].text != "Unavailable") {
        throw std::runtime_error("identity fingerprint fields must not fill absent display facts");
    }
    entry.identity.type = std::filesystem::file_type::directory;
    cells = file_manager::detail::details_cells(entry, entry.name, "Folder");
    if (cells[2U].availability != ObjectCellAvailability::not_applicable) {
        throw std::runtime_error("directory identity size must not masquerade as folder content size");
    }
    entry.identity = {};
    cells = file_manager::detail::details_cells(entry, entry.name, "Folder");
    if (cells[1U].availability != ObjectCellAvailability::unavailable ||
        cells[2U].availability != ObjectCellAvailability::unavailable) {
        throw std::runtime_error("unobserved entries must preserve name without fabricated type/size");
    }
}

void check_signed_and_missing_order() {
    using file_manager::detail::ObjectSort;
    file_manager::detail::ObjectEntries entries{};
    file_manager::DirectoryEntry before{};
    before.name = "before";
    before.identity.inode = 1U;
    before.kind = file_manager::EntryKind::document;
    before.metadata.modified = file_manager::ObservedFileTime{-1, 999999999U};
    before.identity.modified_nanoseconds = UINT64_MAX;
    file_manager::DirectoryEntry epoch{};
    epoch.name = "epoch";
    epoch.identity.inode = 2U;
    epoch.kind = file_manager::EntryKind::image;
    epoch.metadata.modified = file_manager::ObservedFileTime{0, 0U};
    file_manager::DirectoryEntry absent{};
    absent.name = "absent";
    entries.emplace("before", before);
    entries.emplace("epoch", epoch);
    entries.emplace("absent", absent);
    gui_forms::ObjectViewItem first{};
    first.stable_id = "before";
    gui_forms::ObjectViewItem second{};
    second.stable_id = "epoch";
    gui_forms::ObjectViewItem third{};
    third.stable_id = "absent";
    const file_manager::detail::ObjectOrder<ObjectSort::modified> ascending{entries, false};
    const file_manager::detail::ObjectOrder<ObjectSort::modified> descending{entries, true};
    if (!ascending(first, second) || ascending(second, first) ||
        !descending(second, first) || descending(first, second) ||
        !ascending(first, third) || !descending(first, third) ||
        ascending(third, first) || descending(third, first)) {
        throw std::runtime_error("signed timestamps must sort chronologically with absent values last");
    }
    const file_manager::detail::ObjectOrder<ObjectSort::kind> type_ascending{entries, false};
    const file_manager::detail::ObjectOrder<ObjectSort::kind> type_descending{entries, true};
    if (!type_ascending(first, third) || !type_descending(first, third) ||
        type_ascending(third, first) || type_descending(third, first)) {
        throw std::runtime_error("unavailable Type must remain last in both directions");
    }
}
}

int main() {
    try {
        check_factual_projection();
        check_signed_and_missing_order();
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
            entry.identity.inode = static_cast<std::uint64_t>(items.size()) + 1U;
            entry.identity.type = example.directory ? std::filesystem::file_type::directory
                                                    : std::filesystem::file_type::regular;
            entry.identity.size = example.size;
            entry.identity.modified_nanoseconds = example.modified;
            if (!example.directory) entry.metadata.logical_size = example.size;
            entry.metadata.modified = file_manager::ObservedFileTime{example.modified, 0U};
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
        check_order<ObjectSort::size>(entries, items, {"f", "d", "a", "b", "c", "u"}, true);
        check_order<ObjectSort::modified>(entries, items, {"f", "d", "b", "a", "c", "u"}, false);
        check_order<ObjectSort::name>(entries, items, {"f", "u", "c", "a", "b", "d"}, true);
        // Missing observations preserve the original visible-name fallback.
        const file_manager::detail::ObjectEntries absent{};
        check_order<ObjectSort::name>(absent, items, {"d", "a", "b", "c", "f", "u"});
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
