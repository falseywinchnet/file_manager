#pragma once

#include "gui_forms/component.hpp"
#include "gui_forms/event.hpp"

#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <variant>

namespace gui_forms {

enum class BindingValueKind : std::uint8_t {
    null,
    boolean,
    signed_integer,
    unsigned_integer,
    number,
    text,
};

using BindingValue = std::variant<std::monostate, bool, std::int64_t,
                                  std::uint64_t, double, std::string>;

[[nodiscard]] BindingValueKind binding_value_kind(
    const BindingValue& value) noexcept;
[[nodiscard]] std::string binding_value_to_string(const BindingValue& value);
[[nodiscard]] std::optional<bool> binding_value_to_bool(
    const BindingValue& value) noexcept;
[[nodiscard]] std::optional<std::int64_t> binding_value_to_signed(
    const BindingValue& value) noexcept;
[[nodiscard]] std::optional<std::uint64_t> binding_value_to_unsigned(
    const BindingValue& value) noexcept;
[[nodiscard]] std::optional<double> binding_value_to_number(
    const BindingValue& value) noexcept;
[[nodiscard]] std::optional<BindingValue> convert_binding_value(
    const BindingValue& value, BindingValueKind target_kind);

// A custom retained control registers these descriptors once during
// construction. Binding never uses C++ RTTI, native handles, or managed
// reflection: name resolution yields explicit typed get/set/change behavior.
struct BindableProperty final {
    using Getter = std::function<BindingValue()>;
    using Setter = std::function<void(const BindingValue&)>;
    using ChangeConnector = std::function<SubscriptionToken(
        Component&, std::function<void()>)>;

    std::string name;
    BindingValueKind kind{BindingValueKind::text};
    Getter get;
    Setter set;
    ChangeConnector connect_changed;
    bool readable{true};
    bool writable{true};
};

[[nodiscard]] std::string canonical_binding_name(std::string_view name);

} // namespace gui_forms
