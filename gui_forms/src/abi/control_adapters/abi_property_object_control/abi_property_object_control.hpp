#pragma once

#include "../support/abi_control_adapter_support.hpp"

namespace gui_forms::abi::detail {

// A foreign object is represented to the native property engine as an
// ordinary retained Control whose registrations happen to dispatch through a
// bounded C callback table. The proxy is intentionally nonvisual and never
// retains a foreign pointer other than the caller-owned callback context.
class AbiPropertyObjectControl final : public Control {
    struct PropertyState;

public:
    ~AbiPropertyObjectControl() override;

    explicit AbiPropertyObjectControl(StableId stable_id)
        : Control(std::move(stable_id)) {
        clear_bindable_properties();
    }

    void define(const gf_property_descriptor_v1& authored,
                const gf_property_callbacks_v1& callbacks) {
        require_mutable();
        constexpr std::size_t minimum_callback_size =
            offsetof(gf_property_callbacks_v1, parse) +
            sizeof(gf_property_parse_callback);
        if (authored.struct_size < sizeof(gf_property_descriptor_v1) ||
            callbacks.struct_size < minimum_callback_size) {
            throw std::invalid_argument(
                "property proxy definitions require complete size-prefixed records");
        }
        std::shared_ptr<gui_forms::abi::detail::AbiPropertyObjectControl::PropertyState> state = std::make_shared<PropertyState>();
        std::memcpy(&(*state).callbacks, &callbacks,
                    std::min<std::size_t>(callbacks.struct_size,
                                          sizeof((*state).callbacks)));
        (*state).descriptor = copy_descriptor(authored);
        if ((*state).descriptor.readable && callbacks.get == nullptr) {
            throw std::invalid_argument(
                "readable foreign properties require a getter callback");
        }
        if ((*state).descriptor.writable && callbacks.set == nullptr) {
            throw std::invalid_argument(
                "writable foreign properties require a setter callback");
        }
        if ((*state).descriptor.resettable && callbacks.reset == nullptr) {
            throw std::invalid_argument(
                "resettable foreign properties require a reset callback");
        }
        if (!(*state).descriptor.editor_name.empty() &&
            (*state).callbacks.edit == nullptr) {
            throw std::invalid_argument(
                "foreign property editor identities require an edit callback");
        }
        const std::string canonical = gui_forms::canonical_binding_name(
            (*state).descriptor.name);
        if (properties_.contains(canonical)) {
            throw std::invalid_argument(
                "foreign property names must be unique ignoring case");
        }

        gui_forms::PropertyRegistration registration;
        registration.descriptor = (*state).descriptor;
        if (registration.descriptor.readable) {
            registration.get = [state] { return (*state).get(); };
        }
        if (registration.descriptor.writable) {
            registration.set = [state](const gui_forms::BindingValue& value) {
                (*state).set(value);
                (*state).changed.emit();
            };
        }
        if (registration.descriptor.change_notifications) {
            registration.connect_changed = [state](
                gui_forms::Component& owner, std::function<void()> changed) {
                return (*state).changed.subscribe(owner, std::move(changed));
            };
        }
        if (registration.descriptor.resettable) {
            registration.reset = [state] {
                (*state).reset();
                (*state).changed.emit();
            };
        }
        if (callbacks.should_serialize != nullptr) {
            registration.should_serialize = [state] {
                return (*state).should_serialize();
            };
            registration.origin = [state] {
                return (*state).should_serialize()
                    ? gui_forms::PropertyValueOrigin::local
                    : gui_forms::PropertyValueOrigin::defaulted;
            };
        }
        define_bindable_property(std::move(registration));
        properties_.emplace(canonical, std::move(state));
    }

    void notify_changed(std::string_view name) {
        require_mutable();
        const auto found = properties_.find(
            gui_forms::canonical_binding_name(name));
        if (found == properties_.end()) {
            throw std::invalid_argument(
                "foreign property change names must identify a definition");
        }
        (*(*found).second).changed.emit();
    }

    void install_converters(gui_forms::PropertyValueConverterRegistry& target) {
        for (const std::pair<const std::string,
                 std::shared_ptr<PropertyState>>& property : properties_) {
            const std::shared_ptr<PropertyState>& state = property.second;
            const std::string& name = (*state).descriptor.converter_name;
            if (name.empty() || target.find(name) != nullptr ||
                (*state).callbacks.format == nullptr) {
                continue;
            }
            gui_forms::PropertyValueConverter converter;
            if ((*state).callbacks.format != nullptr) {
                converter.format = [weak = std::weak_ptr<PropertyState>(state)](
                    const gui_forms::BindingValue& value,
                    const gui_forms::PropertyDescriptor&) {
                    const std::shared_ptr<gui_forms::abi::detail::AbiPropertyObjectControl::PropertyState> current = weak.lock();
                    if (!current) {
                        throw std::logic_error(
                            "foreign property converter outlived its proxy");
                    }
                    return (*current).format(value);
                };
            }
            converter.parse = [weak = std::weak_ptr<PropertyState>(state)](
                std::string_view text, const gui_forms::BindingValue&,
                const gui_forms::PropertyDescriptor&) {
                const std::shared_ptr<gui_forms::abi::detail::AbiPropertyObjectControl::PropertyState> current = weak.lock();
                if (!current) {
                    throw std::logic_error(
                        "foreign property converter outlived its proxy");
                }
                return (*current).parse(text);
            };
            if (!target.register_converter(name, std::move(converter))) {
                throw std::logic_error(
                    "foreign property converter identity collision");
            }
        }
    }

    void install_editors(gui_forms::PropertyEditorRegistry& target,
                         std::set<std::string>& installed) {
        for (const std::pair<const std::string,
                 std::shared_ptr<PropertyState>>& property : properties_) {
            const std::shared_ptr<PropertyState>& state = property.second;
            const std::string& name = (*state).descriptor.editor_name;
            if (name.empty() || (*state).callbacks.edit == nullptr ||
                !installed.insert(name).second) {
                continue;
            }
            const bool registered = target.register_factory(
                name, [weak = std::weak_ptr<PropertyState>(state)](
                          const gui_forms::PropertyEditorRequest& request)
                    -> std::optional<gui_forms::PropertyEditorBinding> {
                    const std::shared_ptr<gui_forms::abi::detail::AbiPropertyObjectControl::PropertyState> retained = weak.lock();
                    if (!retained) {
                        throw std::logic_error(
                            "foreign property editor outlived its proxy");
                    }
                    std::shared_ptr<gui_forms::Button> button = gui_forms::make_control<gui_forms::Button>(
                        StableId(request.stable_id));
                    auto current = std::make_shared<gui_forms::BindingValue>(
                        request.value);
                    std::shared_ptr<gui_forms::Event<const gui_forms::PropertyEditorInputError &>> failures = std::make_shared<
                        gui_forms::Event<const gui_forms::PropertyEditorInputError&>>();
                    const auto update_text = [weak_button =
                            std::weak_ptr<gui_forms::Button>(button), weak](
                            const gui_forms::BindingValue& value) {
                        const std::shared_ptr<gui_forms::Button> editor = weak_button.lock();
                        const std::shared_ptr<gui_forms::abi::detail::AbiPropertyObjectControl::PropertyState> state = weak.lock();
                        if (!editor || !state) return;
                        std::string display = (*state).format(value);
                        if (display.size() > 96U) {
                            display.resize(93U);
                            display += "...";
                        }
                        (*editor).set_text(display.empty()
                            ? std::string("Edit \xE2\x80\xA6")
                            : display + "  \xE2\x80\xA6");
                    };
                    update_text(*current);
                    (*button).set_accessible_name(request.property_path);
                    (*button).set_accessible_description(
                        request.descriptor.description);
                    (*button).set_enabled(request.writable);

                    gui_forms::PropertyEditorBinding binding;
                    binding.control = button;
                    binding.synchronize = [current, update_text](
                        const gui_forms::BindingValue& value) {
                        *current = value;
                        update_text(value);
                    };
                    binding.connect_committed =
                        [weak_button = std::weak_ptr<gui_forms::Button>(button),
                         weak, current, failures](
                            gui_forms::Component& owner,
                            std::function<void(gui_forms::BindingValue)> committed) {
                            const std::shared_ptr<gui_forms::Button> editor = weak_button.lock();
                            if (!editor) return gui_forms::SubscriptionToken{};
                            return (*editor).clicked().subscribe(
                                owner, [weak, current, failures,
                                        committed = std::move(committed)](
                                           gui_forms::ButtonBase&) {
                                    const std::shared_ptr<gui_forms::abi::detail::AbiPropertyObjectControl::PropertyState> state = weak.lock();
                                    if (!state) return;
                                    try {
                                        committed((*state).edit(*current));
                                    } catch (const std::exception& error) {
                                        const gui_forms::PropertyEditorInputError failure{
                                            (*state).format(*current), error.what()};
                                        (*failures).emit(failure);
                                    }
                                });
                        };
                    binding.connect_failed =
                        [failures](gui_forms::Component& owner,
                                   std::function<void(
                                       const gui_forms::PropertyEditorInputError&)>
                                       failed) {
                            return (*failures).subscribe(owner, std::move(failed));
                        };
                    return binding;
                });
            if (!registered) {
                throw std::logic_error(
                    "foreign property editor identity collision");
            }
        }
    }

private:
    static constexpr std::uint64_t maximum_callback_text_bytes =
        1024ULL * 1024ULL;

    static std::string copy_text(gf_string_view view,
                                 std::size_t maximum,
                                 std::string_view field) {
        if ((view.size != 0U && view.data == nullptr) ||
            view.size > maximum ||
            view.size > static_cast<std::uint64_t>(
                std::numeric_limits<std::size_t>::max())) {
            throw std::invalid_argument(
                std::string(field) + " is not a bounded string view");
        }
        std::string result;
        if (view.size != 0U) {
            result.assign(view.data, static_cast<std::size_t>(view.size));
        }
        if (result.find('\0') != std::string::npos ||
            !gui_forms::validate_utf8(result).valid()) {
            throw std::invalid_argument(
                std::string(field) + " must contain valid UTF-8 without NUL");
        }
        return result;
    }

    static gui_forms::BindingValueKind native_kind(std::uint32_t kind) {
        switch (kind) {
        case GF_PROPERTY_BOOLEAN:
            return gui_forms::BindingValueKind::boolean;
        case GF_PROPERTY_SIGNED_INTEGER:
            return gui_forms::BindingValueKind::signed_integer;
        case GF_PROPERTY_UNSIGNED_INTEGER:
            return gui_forms::BindingValueKind::unsigned_integer;
        case GF_PROPERTY_NUMBER:
            return gui_forms::BindingValueKind::number;
        case GF_PROPERTY_TEXT:
            return gui_forms::BindingValueKind::text;
        case GF_PROPERTY_COLOR:
            return gui_forms::BindingValueKind::color;
        case GF_PROPERTY_ENUMERATION:
            return gui_forms::BindingValueKind::enumeration;
        default:
            throw std::invalid_argument(
                "foreign property kind is not supported by ABI 0.23");
        }
    }

    static std::uint32_t abi_kind(gui_forms::BindingValueKind kind) {
        switch (kind) {
        case gui_forms::BindingValueKind::null: return GF_PROPERTY_NULL;
        case gui_forms::BindingValueKind::boolean: return GF_PROPERTY_BOOLEAN;
        case gui_forms::BindingValueKind::signed_integer:
            return GF_PROPERTY_SIGNED_INTEGER;
        case gui_forms::BindingValueKind::unsigned_integer:
            return GF_PROPERTY_UNSIGNED_INTEGER;
        case gui_forms::BindingValueKind::number: return GF_PROPERTY_NUMBER;
        case gui_forms::BindingValueKind::text: return GF_PROPERTY_TEXT;
        case gui_forms::BindingValueKind::color: return GF_PROPERTY_COLOR;
        case gui_forms::BindingValueKind::enumeration:
            return GF_PROPERTY_ENUMERATION;
        default:
            throw std::invalid_argument(
                "property value cannot cross the ABI 0.23 scalar channel");
        }
    }

    static gf_property_value to_abi(const gui_forms::BindingValue& value) {
        gf_property_value result{};
        result.kind = abi_kind(gui_forms::binding_value_kind(value));
        if (const bool* item = std::get_if<bool>(&value)) {
            result.boolean_value = *item ? 1U : 0U;
        } else if (const long long* item = std::get_if<std::int64_t>(&value)) {
            result.signed_value = *item;
        } else if (const unsigned long long* item = std::get_if<std::uint64_t>(&value)) {
            result.unsigned_value = *item;
        } else if (const double* item = std::get_if<double>(&value)) {
            result.number_value = *item;
        } else if (const std::string* item = std::get_if<std::string>(&value)) {
            result.text_value = {(*item).data(), (*item).size()};
        } else if (const gui_forms::Color* item = std::get_if<gui_forms::Color>(&value)) {
            result.color_argb =
                (static_cast<std::uint32_t>((*item).alpha) << 24U) |
                (static_cast<std::uint32_t>((*item).red) << 16U) |
                (static_cast<std::uint32_t>((*item).green) << 8U) |
                static_cast<std::uint32_t>((*item).blue);
        } else if (const gui_forms::PropertyEnumValue* item =
                       std::get_if<gui_forms::PropertyEnumValue>(&value)) {
            result.signed_value = (*item).value;
            result.text_value = {(*item).name.data(), (*item).name.size()};
        }
        return result;
    }

    static gui_forms::BindingValue from_abi(
        const gf_property_value& value, std::string text,
        const gui_forms::PropertyDescriptor& descriptor) {
        if (value.kind == GF_PROPERTY_NULL) {
            return gui_forms::BindingValue{};
        }
        const gui_forms::BindingValueKind expected = native_kind(value.kind);
        if (expected != descriptor.kind) {
            throw std::invalid_argument(
                "foreign callback returned a value outside its declared kind");
        }
        switch (value.kind) {
        case GF_PROPERTY_BOOLEAN:
            if (value.boolean_value > 1U) {
                throw std::invalid_argument(
                    "foreign Boolean callback returned a non-Boolean value");
            }
            return gui_forms::BindingValue{value.boolean_value != 0U};
        case GF_PROPERTY_SIGNED_INTEGER:
            return gui_forms::BindingValue{value.signed_value};
        case GF_PROPERTY_UNSIGNED_INTEGER:
            return gui_forms::BindingValue{value.unsigned_value};
        case GF_PROPERTY_NUMBER:
            if (!std::isfinite(value.number_value)) {
                throw std::invalid_argument(
                    "foreign numeric callback returned a non-finite value");
            }
            return gui_forms::BindingValue{value.number_value};
        case GF_PROPERTY_TEXT:
            return gui_forms::BindingValue{std::move(text)};
        case GF_PROPERTY_COLOR:
            return gui_forms::BindingValue{color_from_argb(value.color_argb)};
        case GF_PROPERTY_ENUMERATION: {
            std::string name = std::move(text);
            if (name.empty() && descriptor.enumeration) {
                const auto found = std::find_if(
                    (*descriptor.enumeration).choices.begin(),
                    (*descriptor.enumeration).choices.end(),
                    [&](const gui_forms::PropertyEnumChoice& choice) {
                        return choice.value == value.signed_value;
                    });
                if (found != (*descriptor.enumeration).choices.end()) {
                    name = (*found).name;
                }
            }
            return gui_forms::BindingValue{gui_forms::PropertyEnumValue{
                descriptor.enumeration ? (*descriptor.enumeration).type_name
                                       : std::string{},
                std::move(name), value.signed_value}};
        }
        default:
            throw std::invalid_argument(
                "foreign callback returned an unsupported property kind");
        }
    }

    static std::string callback_text(
        const std::function<std::uint32_t(char*, std::uint64_t,
                                          std::uint64_t*)>& invoke,
        std::string_view operation) {
        std::vector<char> buffer(256U);
        for (std::size_t attempt = 0U; attempt < 2U; ++attempt) {
            std::uint64_t required = 0U;
            const std::uint32_t result = invoke(
                buffer.data(), buffer.size(), &required);
            if (required > maximum_callback_text_bytes) {
                throw std::invalid_argument(
                    std::string(operation) + " exceeded the 1 MiB text bound");
            }
            if ((result == GF_ERROR_BUFFER_TOO_SMALL ||
                 required > buffer.size()) && attempt == 0U) {
                buffer.resize(std::max<std::size_t>(
                    1U, static_cast<std::size_t>(required)));
                continue;
            }
            if (result != GF_OK) {
                throw std::runtime_error(
                    std::string(operation) + " callback failed with result " +
                    std::to_string(result));
            }
            if (required > buffer.size()) {
                throw std::invalid_argument(
                    std::string(operation) + " callback reported an unstable size");
            }
            std::string text(buffer.data(), static_cast<std::size_t>(required));
            if (text.find('\0') != std::string::npos ||
                !gui_forms::validate_utf8(text).valid()) {
                throw std::invalid_argument(
                    std::string(operation) + " callback returned invalid UTF-8");
            }
            return text;
        }
        throw std::runtime_error(
            std::string(operation) + " callback did not stabilize");
    }

    static gui_forms::PropertyDescriptor copy_descriptor(
        const gf_property_descriptor_v1& authored) {
        gui_forms::PropertyDescriptor result;
        result.name = copy_text(authored.name, 256U, "property name");
        result.category = copy_text(authored.category, 256U, "property category");
        result.description = copy_text(
            authored.description, 4096U, "property description");
        result.converter_name = copy_text(
            authored.converter_name, 256U, "property converter name");
        result.editor_name = copy_text(
            authored.editor_name, 256U, "property editor name");
        result.kind = native_kind(authored.kind);
        result.readable =
            (authored.flags & GF_PROPERTY_READABLE) != 0U;
        result.writable =
            (authored.flags & GF_PROPERTY_WRITABLE) != 0U;
        result.browsable =
            (authored.flags & GF_PROPERTY_BROWSABLE) != 0U;
        result.nullable =
            (authored.flags & GF_PROPERTY_NULLABLE) != 0U;
        result.standard_values_exclusive =
            (authored.flags & GF_PROPERTY_STANDARD_VALUES_EXCLUSIVE) != 0U;
        result.resettable =
            (authored.flags & GF_PROPERTY_RESETTABLE) != 0U;
        result.change_notifications =
            (authored.flags & GF_PROPERTY_CHANGE_NOTIFICATIONS) != 0U;
        result.invalidation_effects = gui_forms::Dirty::paint |
                                      gui_forms::Dirty::semantics;
        if (result.name.empty() || (!result.readable && !result.writable)) {
            throw std::invalid_argument(
                "foreign property definitions require a name and access mode");
        }
        if (authored.enum_choice_count >
                gui_forms::maximum_property_enum_choices ||
            (authored.enum_choice_count != 0U &&
             authored.enum_choices == nullptr)) {
            throw std::invalid_argument(
                "foreign enum choices are missing or unbounded");
        }
        if (result.kind == gui_forms::BindingValueKind::enumeration) {
            std::shared_ptr<gui_forms::PropertyEnumDescriptor> enumeration =
                std::make_shared<gui_forms::PropertyEnumDescriptor>();
            (*enumeration).type_name = copy_text(
                authored.enum_type_name, 256U, "property enum type");
            (*enumeration).flags =
                (authored.flags & GF_PROPERTY_ENUM_FLAGS) != 0U;
            (*enumeration).choices.reserve(
                static_cast<std::size_t>(authored.enum_choice_count));
            for (std::uint64_t index = 0U;
                 index < authored.enum_choice_count; ++index) {
                (*enumeration).choices.push_back({
                    copy_text(authored.enum_choices[index].name,
                              gui_forms::maximum_property_enum_text_bytes,
                              "property enum choice"),
                    authored.enum_choices[index].value});
            }
            result.enumeration = std::move(enumeration);
        } else if (authored.enum_choice_count != 0U ||
                   authored.enum_type_name.size != 0U) {
            throw std::invalid_argument(
                "only enumeration properties may define enum metadata");
        }
        if (authored.standard_value_count >
                gui_forms::maximum_property_standard_values ||
            (authored.standard_value_count != 0U &&
             authored.standard_values == nullptr)) {
            throw std::invalid_argument(
                "foreign standard values are missing or unbounded");
        }
        result.standard_values.reserve(
            static_cast<std::size_t>(authored.standard_value_count));
        for (std::uint64_t index = 0U;
             index < authored.standard_value_count; ++index) {
            const gf_property_value& value = authored.standard_values[index];
            const std::string value_text = copy_text(
                value.text_value, maximum_callback_text_bytes,
                "property standard value");
            result.standard_values.push_back(from_abi(
                value, value_text, result));
        }
        return result;
    }

    struct PropertyState final {
        gui_forms::PropertyDescriptor descriptor;
        gf_property_callbacks_v1 callbacks{};
        gui_forms::Event<> changed;

        [[nodiscard]] gui_forms::BindingValue get() const {
            gf_property_value value{};
            const std::string text = callback_text(
                [&](char* buffer, std::uint64_t capacity,
                    std::uint64_t* required) {
                    return callbacks.get(callbacks.context, &value, buffer,
                                         capacity, required);
                }, "property getter");
            return from_abi(value, text, descriptor);
        }

        void set(const gui_forms::BindingValue& value) const {
            gf_property_value native = to_abi(value);
            const std::uint32_t result = callbacks.set(
                callbacks.context, &native);
            if (result != GF_OK) {
                throw std::runtime_error(
                    "property setter callback failed with result " +
                    std::to_string(result));
            }
        }

        void reset() const {
            const std::uint32_t result = callbacks.reset(callbacks.context);
            if (result != GF_OK) {
                throw std::runtime_error(
                    "property reset callback failed with result " +
                    std::to_string(result));
            }
        }

        [[nodiscard]] bool should_serialize() const {
            std::uint32_t result_value = 0U;
            const std::uint32_t result = callbacks.should_serialize(
                callbacks.context, &result_value);
            if (result != GF_OK || result_value > 1U) {
                throw std::runtime_error(
                    "property serialization callback failed or returned a non-Boolean value");
            }
            return result_value != 0U;
        }

        [[nodiscard]] std::string format(
            const gui_forms::BindingValue& value) const {
            gf_property_value native = to_abi(value);
            return callback_text(
                [&](char* buffer, std::uint64_t capacity,
                    std::uint64_t* required) {
                    return callbacks.format(callbacks.context, &native,
                                            buffer, capacity, required);
                }, "property formatter");
        }

        [[nodiscard]] std::optional<gui_forms::BindingValue> parse(
            std::string_view text) const {
            if (callbacks.parse == nullptr) return {};
            gf_property_value value{};
            const gf_string_view input{text.data(), text.size()};
            try {
                const std::string value_text = callback_text(
                    [&](char* buffer, std::uint64_t capacity,
                        std::uint64_t* required) {
                        return callbacks.parse(callbacks.context, input, &value,
                                               buffer, capacity, required);
                    }, "property parser");
                return from_abi(value, value_text, descriptor);
            } catch (const std::invalid_argument&) {
                return {};
            } catch (const std::runtime_error&) {
                return {};
            }
        }

        [[nodiscard]] gui_forms::BindingValue edit(
            const gui_forms::BindingValue& current) const {
            if (callbacks.edit == nullptr) {
                throw std::logic_error(
                    "foreign property editor callback is unavailable");
            }
            gf_property_value input = to_abi(current);
            gf_property_value output{};
            const std::string value_text = callback_text(
                [&](char* buffer, std::uint64_t capacity,
                    std::uint64_t* required) {
                    return callbacks.edit(callbacks.context, &input, &output,
                                          buffer, capacity, required);
                }, "property editor");
            return from_abi(output, value_text, descriptor);
        }
    };

    std::map<std::string, std::shared_ptr<PropertyState>> properties_;
};

} // namespace gui_forms::abi::detail
