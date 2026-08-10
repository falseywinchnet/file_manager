#pragma once

#include "gui_forms/binding/types/binding_contract_types.hpp"

#include <cstdint>
#include <string>
#include <string_view>

namespace gui_forms::detail {

void validate_binding_options(const BindingOptions& options);
void bump_counter(std::uint64_t& value) noexcept;
[[nodiscard]] BindingValue format_binding_value(
    const BindingValue& value, std::string_view format);
[[nodiscard]] std::string binding_failure_text(
    std::string_view direction, std::string_view property,
    std::string_view member);

} // namespace gui_forms::detail
