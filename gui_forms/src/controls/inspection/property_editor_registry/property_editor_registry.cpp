#include "gui_forms/inspection/property_editor_registry/property_editor_registry.hpp"

#include "gui_forms/controls/panel/color_value_editor/color_value_editor.hpp"
#include "gui_forms/controls/panel/flags_value_editor/flags_value_editor.hpp"
#include "../property_service_utilities.hpp"

#include <algorithm>
#include <bit>
#include <limits>
#include <stdexcept>
#include <utility>

namespace gui_forms {

using detail::property_service_name;

bool PropertyEditorRegistry::register_factory(
    std::string name, PropertyEditorFactory factory) {
    const std::string canonical = property_service_name(name);
    if (!factory) {
        throw std::invalid_argument(
            "Property editor factories require a callback");
    }
    return factories_.emplace(canonical, std::move(factory)).second;
}

bool PropertyEditorRegistry::unregister_factory(std::string_view name) {
    const std::string canonical = property_service_name(name);
    const bool removed = factories_.erase(canonical) != 0U;
    if (removed) {
        std::erase_if(kind_mappings_, [&canonical](const auto& item) {
            return item.second == canonical;
        });
    }
    return removed;
}

void PropertyEditorRegistry::map_kind(BindingValueKind kind,
                                      std::string factory_name) {
    const std::string canonical = property_service_name(factory_name);
    if (!factories_.contains(canonical)) {
        throw std::invalid_argument(
            "Property editor kind mapping requires a registered factory");
    }
    kind_mappings_.insert_or_assign(kind, canonical);
}

void PropertyEditorRegistry::clear_kind(BindingValueKind kind) {
    kind_mappings_.erase(kind);
}

std::optional<std::string> PropertyEditorRegistry::factory_for(
    BindingValueKind kind) const {
    const KindMap::const_iterator found = kind_mappings_.find(kind);
    return found == kind_mappings_.end()
        ? std::optional<std::string>{}
        : std::optional<std::string>{(*found).second};
}

std::optional<PropertyEditorBinding> PropertyEditorRegistry::create(
    const PropertyEditorRequest& request) const {
    if (!request.writable) return {};
    const bool explicit_service = request.top_level &&
        !request.descriptor.editor_name.empty();
    std::string service = explicit_service
        ? request.descriptor.editor_name : std::string{};
    if (service.empty()) {
        const std::optional<std::string> mapped = factory_for(binding_value_kind(request.value));
        if (mapped) service = *mapped;
    }
    if (service.empty()) return {};
    const FactoryMap::const_iterator found =
        factories_.find(canonical_binding_name(service));
    if (found == factories_.end()) {
        if (explicit_service) {
            throw std::invalid_argument(
                "Declared property editor factory is not registered");
        }
        return {};
    }
    std::optional<gui_forms::PropertyEditorBinding> result = (*found).second(request);
    if (!result) return {};
    if (!(*result).control || !(*(*result).control).is_alive() ||
        (*(*result).control).parent() || (*(*result).control).attached_window() ||
        !(*result).synchronize || !(*result).connect_committed) {
        throw std::invalid_argument(
            "Property editor factory returned an invalid retained binding");
    }
    return result;
}

std::shared_ptr<PropertyEditorRegistry> PropertyEditorRegistry::create_default() {
    std::shared_ptr<gui_forms::PropertyEditorRegistry> result = std::make_shared<PropertyEditorRegistry>();
    static_cast<void>((*result).register_factory(
        "numeric-up-down", [](const PropertyEditorRequest& request)
            -> std::optional<PropertyEditorBinding> {
            const std::optional<double> number = binding_value_to_number(request.value);
            if (!number) return {};
            std::shared_ptr<gui_forms::NumericUpDown> editor = make_control<NumericUpDown>(StableId(request.stable_id));
            (*editor).set_range(std::numeric_limits<double>::lowest(),
                              std::numeric_limits<double>::max());
            (*editor).set_increment(0.1);
            (*editor).set_decimal_places(4U);
            (*editor).set_value(*number);
            (*editor).set_enabled(request.writable);
            (*editor).set_accessible_name(request.property_path);
            (*editor).set_accessible_description(request.descriptor.description);
            std::shared_ptr<bool> synchronizing = std::make_shared<bool>(false);
            PropertyEditorBinding binding;
            binding.control = editor;
            binding.synchronize =
                [weak = std::weak_ptr<NumericUpDown>(editor), synchronizing](
                    const BindingValue& value) {
                    const std::shared_ptr<gui_forms::NumericUpDown> retained = weak.lock();
                    const std::optional<double> converted = binding_value_to_number(value);
                    if (!retained || !converted) return;
                    *synchronizing = true;
                    (*retained).set_value(*converted);
                    *synchronizing = false;
                };
            binding.connect_committed =
                [weak = std::weak_ptr<NumericUpDown>(editor), synchronizing](
                    Component& owner,
                    std::function<void(BindingValue)> committed) {
                    const std::shared_ptr<gui_forms::NumericUpDown> retained = weak.lock();
                    return retained
                        ? (*retained).value_changed().subscribe(
                              owner,
                              [synchronizing,
                               committed = std::move(committed)](double value) {
                                  if (!*synchronizing) {
                                      committed(BindingValue{value});
                                  }
                              })
                        : SubscriptionToken{};
                };
            return binding;
        }));
    static_cast<void>((*result).register_factory(
        "flags-value", [](const PropertyEditorRequest& request)
            -> std::optional<PropertyEditorBinding> {
            if (!request.descriptor.enumeration ||
                !(*request.descriptor.enumeration).flags) return {};
            const gui_forms::PropertyEnumValue* value = std::get_if<PropertyEnumValue>(&request.value);
            if (!value) return {};
            const bool has_bit = std::any_of(
                (*request.descriptor.enumeration).choices.begin(),
                (*request.descriptor.enumeration).choices.end(),
                [](const PropertyEnumChoice& choice) {
                    return choice.value > 0 && std::has_single_bit(
                        static_cast<std::uint64_t>(choice.value));
                });
            if (!has_bit) return {};
            std::shared_ptr<gui_forms::FlagsValueEditor> editor = make_control<FlagsValueEditor>(
                StableId(request.stable_id), *request.descriptor.enumeration,
                *value);
            (*editor).set_enabled(request.writable);
            (*editor).set_accessible_name(request.property_path);
            (*editor).set_accessible_description(request.descriptor.description);
            std::shared_ptr<bool> synchronizing = std::make_shared<bool>(false);
            PropertyEditorBinding binding;
            binding.control = editor;
            binding.synchronize =
                [weak = std::weak_ptr<FlagsValueEditor>(editor), synchronizing](
                    const BindingValue& value) {
                    const std::shared_ptr<gui_forms::FlagsValueEditor> retained = weak.lock();
                    const gui_forms::PropertyEnumValue* flags = std::get_if<PropertyEnumValue>(&value);
                    if (!retained || !flags) return;
                    *synchronizing = true;
                    (*retained).set_value(*flags);
                    *synchronizing = false;
                };
            binding.connect_committed =
                [weak = std::weak_ptr<FlagsValueEditor>(editor), synchronizing](
                    Component& owner,
                    std::function<void(BindingValue)> committed) {
                    const std::shared_ptr<gui_forms::FlagsValueEditor> retained = weak.lock();
                    return retained
                        ? (*retained).value_changed().subscribe(
                              owner,
                              [synchronizing,
                               committed = std::move(committed)](
                                  const PropertyEnumValue& value) {
                                  if (!*synchronizing) {
                                      committed(BindingValue{value});
                                  }
                              })
                        : SubscriptionToken{};
                };
            return binding;
        }));
    static_cast<void>((*result).register_factory(
        "color-value", [](const PropertyEditorRequest& request)
            -> std::optional<PropertyEditorBinding> {
            const gui_forms::Color* value = std::get_if<Color>(&request.value);
            if (!value) return {};
            std::shared_ptr<gui_forms::ColorValueEditor> editor = make_control<ColorValueEditor>(
                StableId(request.stable_id), *value);
            (*editor).set_enabled(request.writable);
            (*editor).set_accessible_name(request.property_path);
            (*editor).set_accessible_description(request.descriptor.description);
            if ((*editor).editor()) {
                (*(*editor).editor()).set_accessible_name(request.property_path);
                (*(*editor).editor()).set_accessible_description(
                    request.descriptor.description);
            }
            std::shared_ptr<bool> synchronizing = std::make_shared<bool>(false);
            PropertyEditorBinding binding;
            binding.control = editor;
            binding.synchronize =
                [weak = std::weak_ptr<ColorValueEditor>(editor), synchronizing](
                    const BindingValue& value) {
                    const std::shared_ptr<gui_forms::ColorValueEditor> retained = weak.lock();
                    const gui_forms::Color* color = std::get_if<Color>(&value);
                    if (!retained || !color) return;
                    *synchronizing = true;
                    (*retained).set_value(*color);
                    *synchronizing = false;
                };
            binding.connect_committed =
                [weak = std::weak_ptr<ColorValueEditor>(editor), synchronizing](
                    Component& owner,
                    std::function<void(BindingValue)> committed) {
                    const std::shared_ptr<gui_forms::ColorValueEditor> retained = weak.lock();
                    return retained
                        ? (*retained).value_changed().subscribe(
                              owner,
                              [synchronizing,
                               committed = std::move(committed)](Color value) {
                                  if (!*synchronizing) {
                                      committed(BindingValue{value});
                                  }
                              })
                        : SubscriptionToken{};
                };
            binding.connect_failed =
                [weak = std::weak_ptr<ColorValueEditor>(editor)](
                    Component& owner,
                    std::function<void(const PropertyEditorInputError&)> failed) {
                    const std::shared_ptr<gui_forms::ColorValueEditor> retained = weak.lock();
                    return retained
                        ? (*retained).edit_failed().subscribe(owner,
                              std::move(failed))
                        : SubscriptionToken{};
                };
            return binding;
        }));
    (*result).map_kind(BindingValueKind::number, "numeric-up-down");
    (*result).map_kind(BindingValueKind::enumeration, "flags-value");
    (*result).map_kind(BindingValueKind::color, "color-value");
    return result;
}

} // namespace gui_forms
