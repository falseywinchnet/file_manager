#include "gui_forms/document_view.hpp"
#include "gui_forms/detail/algorithm/binary_search.hpp"
#include "gui_forms/text/types/text_types.hpp"

#include <cmath>
#include <limits>
#include <span>
#include <string_view>
#include <utility>

namespace gui_forms {
namespace {

bool same_revision(const DocumentRevision& left, const DocumentRevision& right) noexcept {
    const bool equal = left.document == right.document && left.revision == right.revision;
    return equal;
}

bool same_request(const DocumentPageRequest& left, const DocumentPageRequest& right) noexcept {
    const bool revision_equal = same_revision(left.revision, right.revision);
    const bool equal = revision_equal && left.serial == right.serial &&
        left.permitted.begin.value == right.permitted.begin.value &&
        left.permitted.end.value == right.permitted.end.value &&
        left.viewport.anchor.value == right.viewport.anchor.value &&
        left.viewport.horizontal_dip == right.viewport.horizontal_dip;
    return equal;
}

bool scalar_boundary(std::string_view text, std::size_t offset) noexcept {
    if (offset > text.size()) { return false; }
    if (offset == text.size()) { return true; }
    const unsigned char byte = static_cast<unsigned char>(text[offset]);
    const unsigned char marker = byte & 0xc0U;
    const bool boundary = marker != 0x80U;
    return boundary;
}

DocumentViewStatus validate_page(const DocumentPage& page, SourceByteOffset document_size) noexcept {
    if (page.display_utf8.capacity() > DocumentViewLimits::display_capacity ||
        page.mapping.capacity() > DocumentViewLimits::mapping_capacity) {
        return DocumentViewStatus::budget_exceeded;
    }
    const SourceByteRange covered = page.covered;
    const SourceByteRange permitted = page.request.permitted;
    const std::uint64_t anchor = page.request.viewport.anchor.value;
    if (covered.begin.value > covered.end.value ||
        covered.begin.value < permitted.begin.value || covered.end.value > permitted.end.value ||
        anchor < covered.begin.value || anchor > covered.end.value) {
        return DocumentViewStatus::invalid_page;
    }
    if (covered.begin.value == covered.end.value) {
        if (covered.end.value != document_size.value || !page.display_utf8.empty() ||
            !page.mapping.empty()) {
            return DocumentViewStatus::invalid_page;
        }
        return DocumentViewStatus::success;
    }
    if (page.display_utf8.empty() || page.mapping.empty()) {
        return DocumentViewStatus::invalid_page;
    }
    const Utf8ValidationResult validation = validate_utf8(page.display_utf8);
    if (!validation.valid()) { return DocumentViewStatus::invalid_page; }
    std::uint64_t source_end = covered.begin.value;
    std::uint32_t display_end = 0;
    const std::size_t span_count = page.mapping.size();
    for (std::size_t index = 0; index < span_count; ++index) {
        const DocumentMapSpan& span = page.mapping[index];
        if (span.source.begin.value != source_end || span.begin.value != display_end ||
            span.source.end.value <= source_end || span.end.value <= display_end ||
            span.source.end.value > covered.end.value || span.end.value > page.display_utf8.size()) {
            return DocumentViewStatus::invalid_page;
        }
        const bool boundary = scalar_boundary(page.display_utf8, span.end.value);
        if (!boundary) { return DocumentViewStatus::invalid_page; }
        if (span.kind == DocumentMapKind::identity_utf8) {
            const std::uint64_t source_length = span.source.end.value - source_end;
            const std::uint32_t display_length = span.end.value - display_end;
            if (source_length != display_length) { return DocumentViewStatus::invalid_page; }
        } else if (span.kind != DocumentMapKind::atomic_token) {
            return DocumentViewStatus::invalid_page;
        }
        source_end = span.source.end.value;
        display_end = span.end.value;
    }
    if (source_end != covered.end.value || display_end != page.display_utf8.size()) {
        return DocumentViewStatus::invalid_page;
    }
    return DocumentViewStatus::success;
}

struct DisplayEndLess final {
    bool operator()(const DocumentMapSpan& span, DisplayByteOffset position) const noexcept {
        const bool less = span.end.value < position.value;
        return less;
    }
};
struct SourceEndLess final {
    bool operator()(const DocumentMapSpan& span, SourceByteOffset position) const noexcept {
        const bool less = span.source.end.value < position.value;
        return less;
    }
};

} // namespace

DocumentPage::DocumentPage(DocumentPage&& other) noexcept { swap(other); }

DocumentPage& DocumentPage::operator=(DocumentPage&& other) noexcept {
    if (this != &other) {
        DocumentPage previous{};
        previous.swap(*this);
        swap(other);
    }
    return *this;
}

void DocumentPage::swap(DocumentPage& other) noexcept {
    std::swap(request, other.request);
    std::swap(covered, other.covered);
    display_utf8.swap(other.display_utf8);
    mapping.swap(other.mapping);
}

DocumentViewStatus DocumentViewState::bind(DocumentRevision revision,
                                          SourceByteOffset document_size) noexcept {
    if (revision.document == 0 || revision.revision == 0) {
        return DocumentViewStatus::invalid_revision;
    }
    if (revision.document == revision_.document) {
        if (revision.revision < revision_.revision) { return DocumentViewStatus::stale; }
        if (revision.revision == revision_.revision) {
            if (document_size.value != document_size_.value) {
                return DocumentViewStatus::invalid_revision;
            }
            return DocumentViewStatus::success;
        }
    }
    revision_ = revision;
    document_size_ = document_size;
    viewport_ = DocumentViewport{};
    authorized_serial_ = 0;
    page_.reset();
    return DocumentViewStatus::success;
}

DocumentRequestResult DocumentViewState::request_page(DocumentViewport viewport,
                                                     SourceByteRange permitted) noexcept {
    DocumentRequestResult result{};
    if (revision_.document == 0) { return result; }
    const bool finite = std::isfinite(viewport.horizontal_dip);
    if (!finite || viewport.horizontal_dip < 0 || permitted.begin.value > permitted.end.value ||
        permitted.end.value > document_size_.value || viewport.anchor.value < permitted.begin.value ||
        viewport.anchor.value > permitted.end.value) {
        result.status = DocumentViewStatus::invalid_range;
        return result;
    }
    const std::uint64_t source_length = permitted.end.value - permitted.begin.value;
    if (source_length > DocumentViewLimits::source_bytes) {
        result.status = DocumentViewStatus::budget_exceeded;
        return result;
    }
    if (serial_ == std::numeric_limits<std::uint64_t>::max()) {
        result.status = DocumentViewStatus::serial_exhausted;
        return result;
    }
    std::size_t slot = 0;
    while (slot < slots_.size() && slots_[slot].has_value()) { ++slot; }
    if (slot == slots_.size()) {
        result.status = DocumentViewStatus::busy;
        return result;
    }
    ++serial_;
    const DocumentPageRequest request{
        .revision = revision_, .serial = serial_, .permitted = permitted, .viewport = viewport};
    slots_[slot] = request;
    authorized_serial_ = serial_;
    viewport_ = viewport;
    result.status = DocumentViewStatus::success;
    result.request = request;
    return result;
}

void DocumentViewState::cancel() noexcept { authorized_serial_ = 0; }

std::size_t DocumentViewState::find_slot(const DocumentPageRequest& request) const noexcept {
    const std::size_t count = slots_.size();
    for (std::size_t index = 0; index < count; ++index) {
        if (!slots_[index].has_value()) { continue; }
        const bool matching = same_request(*slots_[index], request);
        if (matching) { return index; }
    }
    return count;
}

DocumentViewStatus DocumentViewState::finish(const DocumentPageRequest& request) noexcept {
    const std::size_t slot = find_slot(request);
    if (slot == slots_.size()) { return DocumentViewStatus::stale; }
    slots_[slot].reset();
    if (authorized_serial_ == request.serial) { authorized_serial_ = 0; }
    return DocumentViewStatus::success;
}

DocumentViewStatus DocumentViewState::publish(DocumentPage&& incoming) noexcept {
    const std::size_t slot = find_slot(incoming.request);
    if (slot == slots_.size()) { return DocumentViewStatus::stale; }
    const bool current_revision = same_revision(incoming.request.revision, revision_);
    if (!current_revision) { return DocumentViewStatus::stale; }
    if (authorized_serial_ == 0) { return DocumentViewStatus::cancelled; }
    if (incoming.request.serial != authorized_serial_) { return DocumentViewStatus::stale; }
    const DocumentViewStatus validation = validate_page(incoming, document_size_);
    if (validation != DocumentViewStatus::success) { return validation; }
    page_.reset();
    page_.emplace(std::move(incoming));
    slots_[slot].reset();
    authorized_serial_ = 0;
    return DocumentViewStatus::success;
}

std::size_t DocumentViewState::pending_count() const noexcept {
    std::size_t count = 0;
    for (std::size_t index = 0; index < slots_.size(); ++index) {
        if (slots_[index].has_value()) { ++count; }
    }
    return count;
}

SourceMappingResult DocumentViewState::source_position(
    const DocumentPageRequest& page_token, DisplayByteOffset position) const noexcept {
    SourceMappingResult result{};
    if (!page_.has_value()) { return result; }
    const DocumentPage& current = *page_;
    const bool matching = same_request(current.request, page_token);
    if (!matching) { result.status = DocumentViewStatus::stale; return result; }
    if (position.value > current.display_utf8.size()) { return result; }
    if (current.mapping.empty()) {
        result.status = DocumentViewStatus::success;
        result.position = current.covered.begin;
        return result;
    }
    const std::span<const DocumentMapSpan> spans(current.mapping);
    const std::size_t index = detail::lower_bound_index(spans, position, DisplayEndLess{});
    const DocumentMapSpan& span = spans[index];
    std::uint64_t source = span.source.begin.value;
    if (position.value == span.end.value) {
        source = span.source.end.value;
    } else if (position.value != span.begin.value) {
        const bool boundary = scalar_boundary(current.display_utf8, position.value);
        if (span.kind == DocumentMapKind::atomic_token || !boundary) {
            result.status = DocumentViewStatus::invalid_boundary;
            return result;
        }
        const std::uint32_t relative = position.value - span.begin.value;
        source += relative;
    }
    result.status = DocumentViewStatus::success;
    result.position = SourceByteOffset(source);
    return result;
}

DisplayMappingResult DocumentViewState::display_position(
    const DocumentPageRequest& page_token, SourceByteOffset position) const noexcept {
    DisplayMappingResult result{};
    if (!page_.has_value()) { return result; }
    const DocumentPage& current = *page_;
    const bool matching = same_request(current.request, page_token);
    if (!matching) { result.status = DocumentViewStatus::stale; return result; }
    if (position.value < current.covered.begin.value || position.value > current.covered.end.value) {
        return result;
    }
    if (current.mapping.empty()) {
        result.status = DocumentViewStatus::success;
        result.position = DisplayByteOffset(0);
        return result;
    }
    const std::span<const DocumentMapSpan> spans(current.mapping);
    const std::size_t index = detail::lower_bound_index(spans, position, SourceEndLess{});
    const DocumentMapSpan& span = spans[index];
    std::uint32_t display = span.begin.value;
    if (position.value == span.source.end.value) {
        display = span.end.value;
    } else if (position.value != span.source.begin.value) {
        if (span.kind == DocumentMapKind::atomic_token) {
            result.status = DocumentViewStatus::invalid_boundary;
            return result;
        }
        const std::uint64_t relative = position.value - span.source.begin.value;
        display += static_cast<std::uint32_t>(relative);
        const bool boundary = scalar_boundary(current.display_utf8, display);
        if (!boundary) { result.status = DocumentViewStatus::invalid_boundary; return result; }
    }
    result.status = DocumentViewStatus::success;
    result.position = DisplayByteOffset(display);
    return result;
}

} // namespace gui_forms
