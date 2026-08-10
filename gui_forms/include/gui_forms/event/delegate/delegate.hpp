#pragma once

#include <utility>

namespace gui_forms {

// A Delegate binds invocation only. It neither owns nor extends the lifetime of
// its target; Event subscription and Component revocation remain separate.
// Binding stores one object/context pointer and one compile-time-selected thunk.
template <typename... Arguments>
class Delegate final {
public:
    using Thunk = void (*)(void*, Arguments...);

    constexpr Delegate() noexcept = default;

    template <typename Object, void (Object::*Method)(Arguments...)>
    [[nodiscard]] static constexpr Delegate bind(Object& object) noexcept {
        return Delegate(static_cast<void*>(&object),
                        &invoke_member<Object, Method>);
    }

    template <typename Object, void (Object::*Method)(Arguments...) const>
    [[nodiscard]] static constexpr Delegate bind(const Object& object) noexcept {
        return Delegate(static_cast<void*>(const_cast<Object*>(&object)),
                        &invoke_const_member<Object, Method>);
    }

    template <void (*Function)(Arguments...)>
    [[nodiscard]] static constexpr Delegate bind() noexcept {
        return Delegate(nullptr, &invoke_function<Function>);
    }

    [[nodiscard]] explicit constexpr operator bool() const noexcept {
        return thunk_ != nullptr;
    }

    constexpr void operator()(Arguments... arguments) const {
        if (thunk_ != nullptr) {
            thunk_(context_, std::forward<Arguments>(arguments)...);
        }
    }

private:
    constexpr Delegate(void* context, Thunk thunk) noexcept
        : context_(context), thunk_(thunk) {}

    template <typename Object, void (Object::*Method)(Arguments...)>
    static void invoke_member(void* context, Arguments... arguments) {
        Object& object = *static_cast<Object*>(context);
        (object.*Method)(std::forward<Arguments>(arguments)...);
    }

    template <typename Object, void (Object::*Method)(Arguments...) const>
    static void invoke_const_member(void* context, Arguments... arguments) {
        const Object& object = *static_cast<const Object*>(context);
        (object.*Method)(std::forward<Arguments>(arguments)...);
    }

    template <void (*Function)(Arguments...)>
    static void invoke_function(void*, Arguments... arguments) {
        Function(std::forward<Arguments>(arguments)...);
    }

    void* context_{};
    Thunk thunk_{};
};

} // namespace gui_forms
