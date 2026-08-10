#pragma once

#include "gui_forms/binding/value/binding_value.hpp"
#include "gui_forms/event.hpp"

#include <functional>
#include <optional>
#include <stdexcept>
#include <utility>

namespace gui_forms::detail {

// Named property-registration adapters. Construction names the exact object,
// member, value type, and conversion policy; the templates only perform that
// already-authored decision for std::function-based registration seams.
template <typename Object, typename Value>
class BindingMemberGetter final {
public:
    using Member = Value Object::*;

    BindingMemberGetter(const Object& object, Member member) noexcept
        : object_(&object), member_(member) {}

    BindingValue operator()() const {
        return BindingValue{(*object_).*member_};
    }

private:
    const Object* object_{};
    Member member_{};
};

template <typename Object, typename Value>
class BindingMethodGetter final {
public:
    using Method = Value (Object::*)() const;

    BindingMethodGetter(const Object& object, Method method) noexcept
        : object_(&object), method_(method) {}

    BindingValue operator()() const {
        return BindingValue{((*object_).*method_)()};
    }

private:
    const Object* object_{};
    Method method_{};
};

template <typename Object, typename Value>
class BindingNoexceptMethodGetter final {
public:
    using Method = Value (Object::*)() const noexcept;

    BindingNoexceptMethodGetter(const Object& object, Method method) noexcept
        : object_(&object), method_(method) {}

    BindingValue operator()() const {
        return BindingValue{((*object_).*method_)()};
    }

private:
    const Object* object_{};
    Method method_{};
};

template <typename Object, typename Value>
class ConvertedPropertySetter final {
public:
    using Method = void (Object::*)(Value);

    ConvertedPropertySetter(Object& object, Method method,
                            BindingValueKind kind, const char* error) noexcept
        : object_(&object), method_(method), kind_(kind), error_(error) {}

    void operator()(const BindingValue& value) const {
        const std::optional<BindingValue> converted =
            convert_binding_value(value, kind_);
        if (!converted) throw std::invalid_argument(error_);
        ((*object_).*method_)(std::get<Value>(*converted));
    }

private:
    Object* object_{};
    Method method_{};
    BindingValueKind kind_{};
    const char* error_{};
};

template <typename Object, typename Value>
class DirectPropertySetter final {
public:
    using Method = void (Object::*)(Value);

    DirectPropertySetter(Object& object, Method method) noexcept
        : object_(&object), method_(method) {}

    void operator()(const BindingValue& value) const {
        ((*object_).*method_)(std::get<Value>(value));
    }

private:
    Object* object_{};
    Method method_{};
};

template <typename... EventArguments>
class PropertyChangeRelay final {
public:
    explicit PropertyChangeRelay(std::function<void()> changed)
        : changed_(std::move(changed)) {}

    void operator()(EventArguments...) const { changed_(); }

private:
    std::function<void()> changed_;
};

template <typename... EventArguments>
class EventChangeConnector final {
public:
    explicit EventChangeConnector(Event<EventArguments...>& event) noexcept
        : event_(&event) {}

    SubscriptionToken operator()(Component& owner,
                                 std::function<void()> changed) const {
        return (*event_).subscribe(
            owner,
            PropertyChangeRelay<EventArguments...>(std::move(changed)));
    }

private:
    Event<EventArguments...>* event_{};
};

} // namespace gui_forms::detail
