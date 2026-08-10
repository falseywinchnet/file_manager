#include "gui_forms/binding/value/binding_value.hpp"

namespace gui_forms {

PropertyCollectionValue::operator bool() const noexcept {
    return static_cast<bool>(data_);
}

std::string_view PropertyCollectionValue::item_type_name() const noexcept {
    return data_ ? std::string_view(data_->item_type_name) : std::string_view{};
}

BindingValueKind PropertyCollectionValue::item_kind() const noexcept {
    return data_ ? data_->item_kind : BindingValueKind::null;
}

bool operator==(const PropertyCollectionValue& left,
                const PropertyCollectionValue& right) noexcept {
    if (left.data_ == right.data_) return true;
    return left.data_ && right.data_ &&
        left.data_->item_type_name == right.data_->item_type_name &&
        left.data_->item_kind == right.data_->item_kind &&
        left.data_->items == right.data_->items;
}

} // namespace gui_forms
