#include "prepared_window_input.hpp"
#include "gui_forms/text.hpp"
#include <algorithm>
#include <new>

namespace gui_forms::detail {
namespace {
bool window_scalar_edge(const std::string_view text, const std::size_t offset) noexcept {
    if (offset > text.size()) return false;
    if (offset == text.size()) return true;
    const unsigned char byte = static_cast<unsigned char>(text[offset]);
    const bool edge = (byte & 0xc0U) != 0x80U;
    return edge;
}
bool certified_pair(const PreparedWindowView& view, std::size_t& cursor,
    const std::uint64_t source, const std::uint32_t display) noexcept {
    while (cursor < view.endpoints.size() && view.endpoints[cursor].source.value < source) ++cursor;
    if (cursor == view.endpoints.size()) return false;
    const PreparedSourceEndpoint& endpoint = view.endpoints[cursor];
    const bool paired = endpoint.source.value == source && endpoint.display.value == display;
    return paired;
}
PreparedTextStatus window_mapping(const PreparedWindowView& view) noexcept {
    const PreparedTextKey& key = view.key.text;
    std::uint64_t source = key.source.begin.value;
    std::uint32_t display = key.display_begin.value;
    for (const DocumentMapSpan& map : view.mappings) {
        if (map.source.begin.value != source || map.begin.value != display ||
            map.source.end.value <= source || map.end.value <= display ||
            map.source.end.value > key.source.end.value || map.end.value > key.display_end.value ||
            !window_scalar_edge(view.display, map.end.value - key.display_begin.value))
            return PreparedTextStatus::invalid_input;
        if (map.kind == DocumentMapKind::identity_utf8) {
            if (map.source.end.value - source != map.end.value - display)
                return PreparedTextStatus::invalid_input;
        } else if (map.kind != DocumentMapKind::atomic_token) return PreparedTextStatus::invalid_input;
        source = map.source.end.value;
        display = map.end.value;
    }
    if (source != key.source.end.value || display != key.display_end.value)
        return PreparedTextStatus::invalid_input;
    if (view.endpoints.empty()) return PreparedTextStatus::context_required;
    if (view.display.empty() && (!view.mappings.empty() || view.endpoints.size() != 1))
        return PreparedTextStatus::invalid_input;
    const PreparedSourceEndpoint& first = view.endpoints.front();
    const PreparedSourceEndpoint& last = view.endpoints.back();
    if (first.source.value != key.source.begin.value || first.display.value != key.display_begin.value ||
        last.source.value != key.source.end.value || last.display.value != key.display_end.value)
        return PreparedTextStatus::context_required;
    std::size_t mapping = 0;
    for (std::size_t index = 0; index < view.endpoints.size(); ++index) {
        const PreparedSourceEndpoint& endpoint = view.endpoints[index];
        if (index != 0 && (endpoint.source.value <= view.endpoints[index - 1].source.value ||
            endpoint.display.value <= view.endpoints[index - 1].display.value))
            return PreparedTextStatus::invalid_input;
        if (endpoint.source.value < key.source.begin.value || endpoint.source.value > key.source.end.value ||
            endpoint.display.value < key.display_begin.value || endpoint.display.value > key.display_end.value ||
            !window_scalar_edge(view.display, endpoint.display.value - key.display_begin.value))
            return PreparedTextStatus::invalid_input;
        if (view.display.empty()) continue;
        while (mapping + 1 < view.mappings.size() && view.mappings[mapping].end.value < endpoint.display.value) ++mapping;
        const DocumentMapSpan& map = view.mappings[mapping];
        if (endpoint.source.value < map.source.begin.value || endpoint.source.value > map.source.end.value ||
            endpoint.display.value < map.begin.value || endpoint.display.value > map.end.value)
            return PreparedTextStatus::invalid_input;
        if (map.kind == DocumentMapKind::identity_utf8) {
            if (endpoint.source.value - map.source.begin.value != endpoint.display.value - map.begin.value)
                return PreparedTextStatus::invalid_input;
        } else if (!((endpoint.source.value == map.source.begin.value && endpoint.display.value == map.begin.value) ||
                     (endpoint.source.value == map.source.end.value && endpoint.display.value == map.end.value)))
            return PreparedTextStatus::invalid_input;
    }
    return PreparedTextStatus::success;
}
}
bool same_prepared_window_key(const PreparedWindowKey& left, const PreparedWindowKey& right) noexcept {
    const bool equal = left.controller_instance == right.controller_instance &&
        left.projection_generation == right.projection_generation &&
        same_layout_authority(left.authority, right.authority) && same_prepared_text_key(left.text, right.text);
    return equal;
}
PreparedTextStatus validate_prepared_window(const PreparedWindowView& view, std::size_t& bytes) noexcept {
    const PreparedTextKey& key = view.key.text;
    if (view.key.controller_instance == 0 || view.key.projection_generation == 0 ||
        view.key.authority.session == 0 || view.key.authority.epoch == 0)
        return PreparedTextStatus::invalid_input;
    if (key.page.permitted.end.value >= key.page.permitted.begin.value &&
        key.page.permitted.end.value - key.page.permitted.begin.value > DocumentViewLimits::source_bytes)
        return PreparedTextStatus::budget_exceeded;
    const PreparedTextStatus key_status = validate_prepared_key(key);
    if (key_status != PreparedTextStatus::success) return key_status;
    constexpr std::size_t record_limit = PreparedTextLimits::metadata_records;
    if (view.paragraphs.size() > 512 || view.mappings.size() > record_limit ||
        view.endpoints.size() > record_limit - view.mappings.size() ||
        view.paragraphs.size() > record_limit - view.mappings.size() - view.endpoints.size())
        return PreparedTextStatus::budget_exceeded;
    const std::size_t metadata = sizeof(PreparedWindowInput) + view.mappings.size() * sizeof(DocumentMapSpan) +
        view.endpoints.size() * sizeof(PreparedSourceEndpoint) + view.paragraphs.size() * sizeof(PreparedWindowParagraph);
    if (metadata > PreparedTextLimits::metadata_bytes) return PreparedTextStatus::budget_exceeded;
    for (const PreparedWindowParagraph& row : view.paragraphs) {
        if (!row.proof.complete_begin || !row.proof.complete_end) return PreparedTextStatus::context_required;
    }
    if (view.display.size() != key.display_end.value - key.display_begin.value || !validate_utf8(view.display).valid())
        return PreparedTextStatus::invalid_input;
    if (view.paragraphs.empty() || key.page.viewport.anchor.value != key.source.begin.value)
        return PreparedTextStatus::context_required;
    const PreparedTextStatus mapping_status = window_mapping(view);
    if (mapping_status != PreparedTextStatus::success) return mapping_status;
    std::uint64_t source = key.source.begin.value;
    std::uint32_t display = key.display_begin.value;
    std::size_t endpoint = 0;
    std::size_t separator_mapping = 0;
    for (std::size_t index = 0; index < view.paragraphs.size(); ++index) {
        const PreparedWindowParagraph& row = view.paragraphs[index];
        if (!row.proof.complete_begin || !row.proof.complete_end) return PreparedTextStatus::context_required;
        if (row.content.begin.value != source || row.display_begin.value != display ||
            row.content.end.value < source || row.display_end.value < display ||
            row.content.end.value > key.source.end.value || row.display_end.value > key.display_end.value)
            return PreparedTextStatus::invalid_input;
        if (!certified_pair(view, endpoint, source, display) ||
            !certified_pair(view, endpoint, row.content.end.value, row.display_end.value))
            return PreparedTextStatus::context_required;
        // The existing A2 content profile excludes tabs and additional paragraph
        // separators. Atomic label spelling is ordinary UTF-8, never parsed as metadata.
        const std::string_view content = view.display.substr(display - key.display_begin.value, row.display_end.value - display);
        for (std::size_t offset = 0; offset < content.size(); ++offset) {
            const unsigned char byte = static_cast<unsigned char>(content[offset]);
            if ((byte >= 9 && byte <= 13) || (byte >= 0x1c && byte <= 0x1e) ||
                (byte == 0xc2 && offset + 1 < content.size() && static_cast<unsigned char>(content[offset + 1]) == 0x85) ||
                (byte == 0xe2 && offset + 2 < content.size() && static_cast<unsigned char>(content[offset + 1]) == 0x80 &&
                 (static_cast<unsigned char>(content[offset + 2]) == 0xa8 || static_cast<unsigned char>(content[offset + 2]) == 0xa9)))
                return PreparedTextStatus::unsupported_profile;
        }
        source = row.content.end.value;
        display = row.display_end.value;
        if (row.separator_source.begin.value != source || row.separator_begin.value != display)
            return PreparedTextStatus::invalid_input;
        if (row.separator == PreparedWindowSeparator::none) {
            if (index + 1 != view.paragraphs.size() || row.separator_source.end.value != source || row.separator_end.value != display)
                return PreparedTextStatus::invalid_input;
            if (row.content.begin.value == source && !view.complete_eof) return PreparedTextStatus::context_required;
        } else {
            if (row.separator != PreparedWindowSeparator::lf && row.separator != PreparedWindowSeparator::cr &&
                row.separator != PreparedWindowSeparator::crlf) return PreparedTextStatus::unsupported_profile;
            const std::uint64_t length = row.separator == PreparedWindowSeparator::crlf ? 2 : 1;
            if (row.separator_source.end.value > key.source.end.value || row.separator_source.end.value < source ||
                row.separator_source.end.value - source != length || row.separator_end.value <= display ||
                row.separator_end.value > key.display_end.value || index + 1 == view.paragraphs.size())
                return PreparedTextStatus::invalid_input;
            // No certified caret may split a consumed separator, including CRLF.
            if (endpoint + 1 >= view.endpoints.size() ||
                view.endpoints[endpoint + 1].source.value != row.separator_source.end.value ||
                view.endpoints[endpoint + 1].display.value != row.separator_end.value)
                return PreparedTextStatus::context_required;
            while (separator_mapping + 1 < view.mappings.size() &&
                view.mappings[separator_mapping].source.end.value <= source) ++separator_mapping;
            const DocumentMapSpan& map = view.mappings[separator_mapping];
            if (map.source.end.value < row.separator_source.end.value)
                return PreparedTextStatus::invalid_input;
            if (map.kind == DocumentMapKind::identity_utf8) {
                const std::string_view spelling = view.display.substr(display - key.display_begin.value,
                    row.separator_end.value - display);
                const std::string_view expected = row.separator == PreparedWindowSeparator::crlf ? "\r\n" :
                    row.separator == PreparedWindowSeparator::cr ? "\r" : "\n";
                if (spelling != expected) return PreparedTextStatus::invalid_input;
            } else if (map.source.begin.value != source || map.source.end.value != row.separator_source.end.value ||
                map.begin.value != display || map.end.value != row.separator_end.value)
                return PreparedTextStatus::invalid_input;
            source = row.separator_source.end.value;
            display = row.separator_end.value;
        }
    }
    if (source != key.source.end.value || display != key.display_end.value) return PreparedTextStatus::invalid_input;
    bytes = metadata + view.display.size();
    return PreparedTextStatus::success;
}
PreparedTextStatus own_prepared_window(const PreparedWindowView& view, std::shared_ptr<PreparedLedger> ledger,
    std::unique_ptr<const PreparedWindowInput>& output) {
    if (output) return PreparedTextStatus::busy;
    std::size_t bytes = 0;
    const PreparedTextStatus valid = validate_prepared_window(view, bytes);
    if (valid != PreparedTextStatus::success) return valid;
    PreparedReservation reservation{};
    const PreparedTextStatus reserved = reservation.acquire(std::move(ledger), PreparedResource::input, bytes);
    if (reserved != PreparedTextStatus::success) return reserved;
    try {
        std::unique_ptr<PreparedWindowInput> next = std::make_unique<PreparedWindowInput>();
        (*next).reservation = std::move(reservation);
        (*next).key = view.key;
        (*next).display_bytes = view.display.size();
        (*next).mapping_count = view.mappings.size();
        (*next).endpoint_count = view.endpoints.size();
        (*next).paragraph_count = view.paragraphs.size();
        (*next).charged_bytes = bytes;
        (*next).complete_eof = view.complete_eof;
        if (!view.display.empty()) {
            std::unique_ptr<char[]> display = std::make_unique<char[]>(view.display.size());
            std::copy(view.display.begin(), view.display.end(), display.get());
            (*next).display = std::move(display);
        }
        if (!view.mappings.empty()) {
            std::unique_ptr<DocumentMapSpan[]> mappings = std::make_unique<DocumentMapSpan[]>(view.mappings.size());
            std::copy(view.mappings.begin(), view.mappings.end(), mappings.get());
            (*next).mappings = std::move(mappings);
        }
        std::unique_ptr<PreparedSourceEndpoint[]> endpoints = std::make_unique<PreparedSourceEndpoint[]>(view.endpoints.size());
        std::copy(view.endpoints.begin(), view.endpoints.end(), endpoints.get());
        (*next).endpoints = std::move(endpoints);
        std::unique_ptr<PreparedWindowParagraph[]> paragraphs = std::make_unique<PreparedWindowParagraph[]>(view.paragraphs.size());
        std::copy(view.paragraphs.begin(), view.paragraphs.end(), paragraphs.get());
        (*next).paragraphs = std::move(paragraphs);
        output = std::move(next);
    } catch (const std::bad_alloc&) { return PreparedTextStatus::resource_failure; }
    return PreparedTextStatus::success;
}
}
