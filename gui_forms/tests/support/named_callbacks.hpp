#pragma once

#include <cstddef>
#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <utility>

namespace gui_forms::test_support {

// Small callback vocabulary for tests whose point is the event or ownership
// behavior, not the spelling of a closure. Signatures remain explicit at each
// call site, so the event contract is still visible to the reader.
template <typename Counter, typename... Arguments>
class IncrementCounter final {
public:
    explicit IncrementCounter(Counter& counter) noexcept : counter_(counter) {}

    void operator()(Arguments...) const { ++counter_; }

private:
    Counter& counter_;
};

template <typename Value>
class RecordValue final {
public:
    explicit RecordValue(Value& target) noexcept : target_(target) {}

    void operator()(const Value& value) const { target_ = value; }

private:
    Value& target_;
};

template <typename Container, typename Argument>
class PushBack final {
public:
    explicit PushBack(Container& target) noexcept : target_(target) {}

    void operator()(Argument value) const { target_.push_back(value); }

private:
    Container& target_;
};

template <typename Container, typename Value, typename... Arguments>
class PushConstant final {
public:
    PushConstant(Container& target, Value value) noexcept
        : target_(target), value_(std::move(value)) {}

    void operator()(Arguments...) const { target_.push_back(value_); }

private:
    Container& target_;
    Value value_;
};

template <typename... Arguments>
class SetFlag final {
public:
    explicit SetFlag(bool& flag, bool value = true) noexcept
        : flag_(flag), value_(value) {}

    void operator()(Arguments...) const { flag_ = value_; }

private:
    bool& flag_;
    bool value_;
};

template <typename... Arguments>
class AppendLiteral final {
public:
    AppendLiteral(std::string& target, std::string_view text) noexcept
        : target_(target), text_(text) {}

    void operator()(Arguments...) const { target_.append(text_); }

private:
    std::string& target_;
    std::string_view text_;
};

template <typename Counter>
class IncrementWhenTrue final {
public:
    explicit IncrementWhenTrue(Counter& counter) noexcept : counter_(counter) {}

    void operator()(bool value) const {
        if (value) ++counter_;
    }

private:
    Counter& counter_;
};

template <typename Counter>
class CountSuccessfulCommand final {
public:
    explicit CountSuccessfulCommand(Counter& counter) noexcept
        : counter_(counter) {}

    bool operator()() const {
        ++counter_;
        return true;
    }

private:
    Counter& counter_;
};

template <typename ControlType, typename... Arguments>
class DisposeCapturedControl final {
public:
    explicit DisposeCapturedControl(std::shared_ptr<ControlType> control) noexcept
        : control_(std::move(control)) {}

    void operator()(Arguments...) const {
        if ((*control_).is_alive()) (*control_).dispose();
    }

private:
    std::shared_ptr<ControlType> control_;
};

template <typename... IgnoredArguments>
class IgnoreArgumentsThenInvoke final {
public:
    explicit IgnoreArgumentsThenInvoke(std::function<void()> callback) noexcept
        : callback_(std::move(callback)) {}

    void operator()(IgnoredArguments...) const { callback_(); }

private:
    std::function<void()> callback_;
};

} // namespace gui_forms::test_support
