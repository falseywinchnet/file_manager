#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace gui_forms::tests {

class FakeMonotonicClock final {
public:
    [[nodiscard]] std::uint64_t now_nanoseconds() const noexcept { return now_; }
    void advance(std::uint64_t nanoseconds) noexcept { now_ += nanoseconds; }

private:
    std::uint64_t now_{};
};

class TraceRecorder final {
public:
    void record(std::string line);
    [[nodiscard]] std::string text() const;

private:
    std::vector<std::string> lines_;
};

[[nodiscard]] std::string canonical_lifecycle_trace();

} // namespace gui_forms::tests
