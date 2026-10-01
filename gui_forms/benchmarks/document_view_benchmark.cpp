#include "gui_forms/document_view.hpp"

#include <algorithm>
#include <array>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace {
using namespace gui_forms;
using Clock = std::chrono::steady_clock;

void require(bool condition, const char* message) {
    if (!condition) { throw std::runtime_error(message); }
}

double elapsed_ns(Clock::time_point begin, Clock::time_point end) {
    const Clock::duration elapsed = end - begin;
    const std::chrono::duration<double, std::nano> nanoseconds(elapsed);
    const double value = nanoseconds.count();
    return value;
}

void report(std::string_view operation, std::uint32_t count, std::vector<double>& samples) {
    std::sort(samples.begin(), samples.end());
    const std::size_t last = samples.size() - 1;
    const std::size_t p50 = last * 50 / 100;
    const std::size_t p95 = last * 95 / 100;
    const std::size_t p99 = last * 99 / 100;
    std::cout << operation << ',' << count << ',' << samples.size() << ','
              << samples[p50] << ',' << samples[p95] << ',' << samples[p99] << ','
              << samples[last] << '\n';
}

// Independent deliberately linear endpoint oracle for the all-token workload.
SourceByteOffset reference_position(const DocumentPage& page, DisplayByteOffset position) {
    const std::size_t count = page.mapping.size();
    for (std::size_t index = 0; index < count; ++index) {
        const DocumentMapSpan& span = page.mapping[index];
        if (position.value == span.begin.value) { return span.source.begin; }
        if (position.value == span.end.value) { return span.source.end; }
    }
    throw std::runtime_error("reference position outside admitted endpoints");
}

DocumentPage make_page(const DocumentPageRequest& token, std::uint32_t count) {
    DocumentPage page{};
    page.request = token;
    page.covered = token.permitted;
    const std::size_t display_size = static_cast<std::size_t>(count) * 9;
    page.display_utf8.reserve(display_size);
    page.mapping.reserve(count);
    for (std::uint32_t index = 0; index < count; ++index) {
        page.display_utf8.append("[BYTE FF]");
        const std::uint32_t source_end = index + 1;
        const std::uint32_t display_begin = index * 9;
        const std::uint32_t display_end = display_begin + 9;
        const SourceByteRange source{SourceByteOffset(index), SourceByteOffset(source_end)};
        const DocumentMapSpan span{.source = source, .begin = DisplayByteOffset(display_begin),
            .end = DisplayByteOffset(display_end), .kind = DocumentMapKind::atomic_token};
        page.mapping.push_back(span);
    }
    return page;
}

void measure(std::uint32_t count) {
    constexpr std::size_t publication_samples = 101;
    constexpr std::size_t lookup_samples = 1001;
    const std::size_t count_size = static_cast<std::size_t>(count);
    const std::size_t endpoint_count = count_size + 1;
    DocumentViewState state{};
    const DocumentViewStatus binding = state.bind({1, 1}, SourceByteOffset(count));
    require(binding == DocumentViewStatus::success, "bind");
    const SourceByteRange permitted{SourceByteOffset(0), SourceByteOffset(count)};
    const DocumentViewport viewport{};
    std::vector<double> requests(publication_samples, 0);
    std::vector<double> publications(publication_samples, 0);
    std::vector<double> binary(lookup_samples, 0);
    std::vector<double> linear(lookup_samples, 0);
    DocumentPageRequest last_token{};
    double first_publication = 0;
    for (std::size_t index = 0; index <= publication_samples; ++index) {
        const Clock::time_point request_start = Clock::now();
        const DocumentRequestResult request = state.request_page(viewport, permitted);
        const Clock::time_point request_end = Clock::now();
        require(request.status == DocumentViewStatus::success, "request");
        last_token = *request.request;
        DocumentPage page = make_page(last_token, count);
        const char* original_display = page.display_utf8.data();
        const DocumentMapSpan* original_mapping = page.mapping.data();
        const Clock::time_point publish_start = Clock::now();
        const DocumentViewStatus publication = state.publish(std::move(page));
        const Clock::time_point publish_end = Clock::now();
        require(publication == DocumentViewStatus::success, "publish");
        const std::optional<DocumentPage>& published = state.page();
        const DocumentPage& retained = *published;
        const char* retained_display = retained.display_utf8.data();
        const DocumentMapSpan* retained_mapping = retained.mapping.data();
        require(original_display == retained_display && original_mapping == retained_mapping,
                "publication transfers payload allocations");
        const double publish_ns = elapsed_ns(publish_start, publish_end);
        if (index == 0) { first_publication = publish_ns; }
        else {
            const std::size_t sample = index - 1;
            requests[sample] = elapsed_ns(request_start, request_end);
            publications[sample] = publish_ns;
        }
    }
    const std::optional<DocumentPage>& published = state.page();
    const DocumentPage& page = *published;
    // Correctness first, outside timing. Every token endpoint, both directions.
    for (std::uint32_t index = 0; index <= count; ++index) {
        const std::uint32_t display = index * 9;
        const SourceMappingResult actual = state.source_position(last_token, DisplayByteOffset(display));
        require(actual.status == DocumentViewStatus::success && (*actual.position).value == index,
                "binary endpoint oracle");
        const DisplayMappingResult reverse = state.display_position(last_token, SourceByteOffset(index));
        require(reverse.status == DocumentViewStatus::success && (*reverse.position).value == display,
                "reverse endpoint oracle");
    }
    std::uint64_t checksum = 0;
    for (std::size_t index = 0; index < lookup_samples; ++index) {
        const std::size_t shuffled = (index * 7919) % endpoint_count;
        const std::uint32_t source = static_cast<std::uint32_t>(shuffled);
        const std::uint32_t display = source * 9;
        const DisplayByteOffset position(display);
        const SourceByteOffset expected = reference_position(page, position);
        const SourceMappingResult actual = state.source_position(last_token, position);
        require(actual.status == DocumentViewStatus::success && (*actual.position).value == expected.value,
                "independent linear reference");
    }
    // Warm resident pages; per-operation clock overhead is included. Building
    // producer pages is excluded, prior-page destruction is inside publication.
    for (std::size_t index = 0; index < lookup_samples; ++index) {
        const std::size_t shuffled = (index * 7919) % endpoint_count;
        const std::uint32_t source = static_cast<std::uint32_t>(shuffled);
        const std::uint32_t display = source * 9;
        const DisplayByteOffset position(display);
        const Clock::time_point binary_start = Clock::now();
        const SourceMappingResult actual = state.source_position(last_token, position);
        const Clock::time_point binary_end = Clock::now();
        const Clock::time_point linear_start = Clock::now();
        const SourceByteOffset reference = reference_position(page, position);
        const Clock::time_point linear_end = Clock::now();
        binary[index] = elapsed_ns(binary_start, binary_end);
        linear[index] = elapsed_ns(linear_start, linear_end);
        checksum += (*actual.position).value;
        checksum += reference.value;
    }
    report("request_warm", count, requests);
    report("publish_replace_warm", count, publications);
    report("map_binary_warm", count, binary);
    report("map_linear_reference_warm", count, linear);
    const std::size_t mapping_bytes = page.mapping.capacity() * sizeof(DocumentMapSpan);
    const std::size_t payload_bytes = mapping_bytes + page.display_utf8.capacity();
    const std::size_t three_payload_bytes = payload_bytes * 3;
    const std::size_t two_source_bytes = count_size * 2;
    const std::size_t metadata_bytes = sizeof(DocumentViewState) +
        2 * sizeof(DocumentPage) + 2 * sizeof(std::string) +
        sizeof(DocumentViewport) + sizeof(SourceByteRange);
    const std::size_t bounded_bytes = three_payload_bytes + two_source_bytes + metadata_bytes;
    std::cout << "metadata," << count << ",first_publish_ns=" << first_publication
              << ",display_capacity=" << page.display_utf8.capacity()
              << ",mapping_bytes=" << mapping_bytes << ",checksum=" << checksum
              << ",model_size=" << sizeof(DocumentViewState)
              << ",three_payload_bytes=" << three_payload_bytes
              << ",two_source_bytes=" << two_source_bytes
              << ",metadata_bytes=" << metadata_bytes
              << ",accounted_bytes=" << bounded_bytes << '\n';
}

} // namespace

int main() {
    try {
        std::cout << "operation,map_spans,samples,p50_ns,p95_ns,p99_ns,worst_ns\n";
        const std::array<std::uint32_t, 3> sizes{64, 4096, 65536};
        for (std::size_t index = 0; index < sizes.size(); ++index) { measure(sizes[index]); }
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return EXIT_FAILURE;
    }
}
