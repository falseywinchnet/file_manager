#pragma once

#include <string>
#include <string_view>

namespace gui_forms {

// Immutable authored identity shared by retained lookup, semantics, traces,
// and public construction seams. Runtime identity remains a separate concern.
class StableId final {
public:
    explicit StableId(std::string value);
    [[nodiscard]] std::string_view value() const noexcept { return value_; }
    friend bool operator==(const StableId& left,
                           const StableId& right) noexcept {
        return left.value_ == right.value_;
    }

private:
    std::string value_;
};

} // namespace gui_forms
