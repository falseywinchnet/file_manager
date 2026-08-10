#pragma once

#include <utility>

namespace gui_forms::detail {

// Explicit adapter for APIs that deliberately retain std::function but bind a
// named member operation. The adapter owns nothing: the enclosing registration
// or callback contract remains responsible for the target lifetime.
template <typename MemberFunction>
class BoundMemberFunction;

template <typename Result, typename Object, typename... Arguments>
class BoundMemberFunction<Result (Object::*)(Arguments...)> final {
public:
    using Method = Result (Object::*)(Arguments...);

    constexpr BoundMemberFunction(Object& object, Method method) noexcept
        : object_(&object), method_(method) {}

    Result operator()(Arguments... arguments) const {
        return ((*object_).*method_)(
            std::forward<Arguments>(arguments)...);
    }

private:
    Object* object_{};
    Method method_{};
};

template <typename Result, typename Object, typename... Arguments>
class BoundMemberFunction<Result (Object::*)(Arguments...) const> final {
public:
    using Method = Result (Object::*)(Arguments...) const;

    constexpr BoundMemberFunction(const Object& object, Method method) noexcept
        : object_(&object), method_(method) {}

    Result operator()(Arguments... arguments) const {
        return ((*object_).*method_)(
            std::forward<Arguments>(arguments)...);
    }

private:
    const Object* object_{};
    Method method_{};
};

template <typename Result, typename Object, typename... Arguments>
class BoundMemberFunction<Result (Object::*)(Arguments...) noexcept> final {
public:
    using Method = Result (Object::*)(Arguments...) noexcept;

    constexpr BoundMemberFunction(Object& object, Method method) noexcept
        : object_(&object), method_(method) {}

    Result operator()(Arguments... arguments) const noexcept {
        return ((*object_).*method_)(
            std::forward<Arguments>(arguments)...);
    }

private:
    Object* object_{};
    Method method_{};
};

template <typename Result, typename Object, typename... Arguments>
class BoundMemberFunction<
    Result (Object::*)(Arguments...) const noexcept> final {
public:
    using Method = Result (Object::*)(Arguments...) const noexcept;

    constexpr BoundMemberFunction(const Object& object, Method method) noexcept
        : object_(&object), method_(method) {}

    Result operator()(Arguments... arguments) const noexcept {
        return ((*object_).*method_)(
            std::forward<Arguments>(arguments)...);
    }

private:
    const Object* object_{};
    Method method_{};
};

} // namespace gui_forms::detail
