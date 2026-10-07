#include "gui_forms/event.hpp"

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <iostream>

namespace {

class Counter final : public gui_forms::Component {
public:
    void add(std::uint64_t value) noexcept { total_ += value; }
    [[nodiscard]] std::uint64_t total() const noexcept { return total_; }

private:
    std::uint64_t total_{};
};

struct DispatchSample final {
    std::int64_t nanoseconds{};
    std::uint64_t callbacks{};
};

[[nodiscard]] DispatchSample measure(gui_forms::Event<std::uint64_t>& event,
                                     const Counter& counter,
                                     const std::size_t iterations) {
    const std::uint64_t before = counter.total();
    const std::chrono::steady_clock::time_point start =
        std::chrono::steady_clock::now();
    for (std::size_t index = 0U; index < iterations; ++index) {
        event.emit(1U);
    }
    const std::chrono::steady_clock::time_point finish =
        std::chrono::steady_clock::now();
    const std::chrono::nanoseconds elapsed =
        std::chrono::duration_cast<std::chrono::nanoseconds>(finish - start);
    const DispatchSample sample{elapsed.count(), counter.total() - before};
    return sample;
}

void print_sample(const char* kind, const std::size_t trial,
                  const std::size_t iterations, const DispatchSample sample) {
    const double per_emit = static_cast<double>(sample.nanoseconds) /
                            static_cast<double>(iterations);
    std::cout << kind << ',' << trial << ',' << iterations << ','
              << sample.nanoseconds << ',' << per_emit << ','
              << sample.callbacks << '\n';
}

} // namespace

int main() {
    constexpr std::size_t iterations = 1000000U;
    constexpr std::size_t trials = 7U;
    Counter delegate_owner{};
    gui_forms::Event<std::uint64_t> delegate_event{};
    const gui_forms::Delegate<std::uint64_t> handler =
        gui_forms::Delegate<std::uint64_t>::bind<Counter, &Counter::add>(
            delegate_owner);
    const gui_forms::SubscriptionToken token =
        delegate_event.subscribe(delegate_owner, handler);
    static_cast<void>(measure(delegate_event, delegate_owner, 10000000U));

    // The same source can measure the preceding revision's existing dispatch
    // without requiring that revision to declare the new helper.
#ifndef GUI_FORMS_EVENT_LAB_BASELINE
    Counter owned_owner{};
    gui_forms::Event<std::uint64_t> owned_event{};
    gui_forms::on(owned_event, owned_owner, &Counter::add);
    static_cast<void>(measure(owned_event, owned_owner, 10000000U));
#endif

    std::cout << "component_bytes," << sizeof(gui_forms::Component) << '\n';
    std::cout << "kind,trial,iterations,total_ns,ns_per_emit,callbacks\n";
    for (std::size_t trial = 0U; trial < trials; ++trial) {
        const DispatchSample delegate_sample =
            measure(delegate_event, delegate_owner, iterations);
        print_sample("caller_owned_delegate", trial, iterations, delegate_sample);
        if (delegate_sample.callbacks != iterations) {
            return 1;
        }
#ifndef GUI_FORMS_EVENT_LAB_BASELINE
        const DispatchSample owned_sample =
            measure(owned_event, owned_owner, iterations);
        print_sample("owner_held_member", trial, iterations, owned_sample);
        if (owned_sample.callbacks != iterations) {
            return 1;
        }
#endif
    }
    if (!token.connected()) {
        return 1;
    }
    return 0;
}
