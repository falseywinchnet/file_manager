#include "gui_forms/binding/value/binding_value.hpp"

namespace gui_forms {

PropertyObjectValue::operator bool() const noexcept {
    return static_cast<bool>(data_);
}

std::string_view PropertyObjectValue::type_name() const noexcept {
    return data_ ? std::string_view((*data_).type_name) : std::string_view{};
}

std::span<const PropertyObjectMember> PropertyObjectValue::members() const noexcept {
    return data_ ? std::span<const PropertyObjectMember>((*data_).members)
                 : std::span<const PropertyObjectMember>{};
}

bool operator==(const PropertyObjectValue& left,
                const PropertyObjectValue& right) noexcept {
    if (left.data_ == right.data_) return true;
    return left.data_ && right.data_ &&
        (*left.data_).type_name == (*right.data_).type_name &&
        (*left.data_).members == (*right.data_).members;
}

} // namespace gui_forms
