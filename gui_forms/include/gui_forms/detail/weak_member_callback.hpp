#pragma once

#include <memory>
#include <utility>

namespace gui_forms::detail {

// Owning-event adapter for callbacks that must not prolong the target beyond
// its retained owner. Locking the weak identity keeps it alive only for the
// duration of a named member operation.
template <typename MemberFunction>
class WeakMemberCallback;

template <typename Object, typename... Arguments>
class WeakMemberCallback<void (Object::*)(Arguments...)> final {
public:
    using Method = void (Object::*)(Arguments...);

    WeakMemberCallback(std::weak_ptr<Object> object, Method method) noexcept
        : object_(std::move(object)), method_(method) {}

    void operator()(Arguments... arguments) const {
        const std::shared_ptr<Object> object = object_.lock();
        if (object) {
            ((*object).*method_)(std::forward<Arguments>(arguments)...);
        }
    }

private:
    std::weak_ptr<Object> object_;
    Method method_{};
};

template <typename Object, typename... Arguments>
class WeakMemberCallback<void (Object::*)(Arguments...) noexcept> final {
public:
    using Method = void (Object::*)(Arguments...) noexcept;

    WeakMemberCallback(std::weak_ptr<Object> object, Method method) noexcept
        : object_(std::move(object)), method_(method) {}

    void operator()(Arguments... arguments) const noexcept {
        const std::shared_ptr<Object> object = object_.lock();
        if (object) {
            ((*object).*method_)(std::forward<Arguments>(arguments)...);
        }
    }

private:
    std::weak_ptr<Object> object_;
    Method method_{};
};

} // namespace gui_forms::detail
