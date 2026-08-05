#include "gui_forms/gui_forms.hpp"

#include <exception>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>
#include <vector>

namespace {

using namespace gui_forms;

void require(bool condition, const char* message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

class LifecycleProbe final : public Control {
public:
    LifecycleProbe(StableId id, std::vector<std::string>& trace)
        : Control(std::move(id)), trace_(&trace) {}

    bool saw_bound_subtree{};
    bool saw_unbound_subtree{};

protected:
    void on_attached_to_window() override {
        saw_bound_subtree = attached();
        for (const Control::Ptr& child : children()) {
            saw_bound_subtree = saw_bound_subtree && child->attached();
        }
        trace_->push_back("attach:" + std::string(stable_id().value()));
    }

    void on_detached_from_window() noexcept override {
        saw_unbound_subtree = !attached();
        for (const Control::Ptr& child : children()) {
            saw_unbound_subtree = saw_unbound_subtree && !child->attached();
        }
        trace_->push_back("detach:" + std::string(stable_id().value()));
    }

private:
    std::vector<std::string>* trace_{};
};

class ThrowingAttachControl final : public Control {
public:
    explicit ThrowingAttachControl(StableId id) : Control(std::move(id)) {}

    bool detached_after_failure{};

protected:
    void on_attached_to_window() override {
        throw std::runtime_error("declared attach failure");
    }
    void on_detached_from_window() noexcept override {
        detached_after_failure = !attached();
    }
};

void test_attach_detach_order_and_observable_state() {
    std::vector<std::string> trace;
    auto root = make_control<Panel>(StableId("lifecycle.root"));
    auto parent = make_control<LifecycleProbe>(StableId("lifecycle.parent"), trace);
    auto child = make_control<LifecycleProbe>(StableId("lifecycle.child"), trace);
    parent->add_child(child);
    root->add_child(parent);
    Window window(root, {200.0, 100.0});

    require(trace == std::vector<std::string>{"attach:lifecycle.parent",
                                              "attach:lifecycle.child"} &&
                parent->saw_bound_subtree && child->saw_bound_subtree,
            "attach callbacks must be parent-first after the complete subtree is bound");

    trace.clear();
    Control::Ptr detached = root->remove_child(parent->runtime_id());
    require(detached == parent &&
                trace == std::vector<std::string>{"detach:lifecycle.child",
                                                  "detach:lifecycle.parent"} &&
                parent->saw_unbound_subtree && child->saw_unbound_subtree,
            "detach callbacks must be child-first after the complete subtree is unbound");
}

void test_user_control_load_once_across_reparent() {
    auto root = make_control<Panel>(StableId("user.root"));
    auto left = make_control<Panel>(StableId("user.left"));
    auto right = make_control<Panel>(StableId("user.right"));
    auto composed = make_control<UserControl>(StableId("user.composed"));
    auto field = make_control<Label>(StableId("user.composed.field"), "ready");
    composed->add_child(field);
    left->add_child(composed);
    root->add_child(left);
    root->add_child(right);

    std::uint64_t loads = 0;
    auto load = composed->loaded().subscribe([&loads] { ++loads; });
    Window window(root, {200.0, 100.0});
    require(loads == 1 && composed->is_loaded() && composed->is_attached() &&
                composed->attachment_count() == 1,
            "UserControl must load once on its first successful attachment");

    right->add_child(composed);
    require(composed->parent() == right && loads == 1 && composed->is_attached() &&
                composed->attachment_count() == 2,
            "same-window reparent must detach and reattach without repeating Load");

    static_cast<void>(right->remove_child(composed->runtime_id()));
    require(!composed->is_attached() && loads == 1,
            "detached UserControl must retain its once-per-lifetime loaded state");
}

void test_failed_attach_rolls_back_binding_and_identity() {
    auto root = make_control<Panel>(StableId("rollback.root"));
    Window window(root, {100.0, 60.0});
    auto failing = make_control<ThrowingAttachControl>(StableId("rollback.child"));
    bool threw = false;
    try {
        root->add_child(failing);
    } catch (const std::runtime_error&) {
        threw = true;
    }
    require(threw && !failing->attached() && !failing->parent() &&
                failing->detached_after_failure && !window.find("rollback.child"),
            "failed attachment must roll back binding, parentage, and stable-ID registration");

    auto replacement = make_control<Label>(StableId("rollback.child"), "replacement");
    root->add_child(replacement);
    require(window.find("rollback.child") == replacement,
            "a stable ID from a failed attachment must be immediately reusable");

    auto composed = make_control<UserControl>(StableId("rollback.composed"));
    auto nested_failure = make_control<ThrowingAttachControl>(
        StableId("rollback.composed.failure"));
    composed->add_child(nested_failure);
    std::uint64_t loads = 0;
    auto load = composed->loaded().subscribe([&loads] { ++loads; });
    bool nested_threw = false;
    try {
        root->add_child(composed);
    } catch (const std::runtime_error&) {
        nested_threw = true;
    }
    require(nested_threw && loads == 1 && composed->is_loaded() &&
                composed->attachment_count() == 0 && !composed->attached(),
            "a later descendant failure cannot un-observe Load but must not commit attachment");
    static_cast<void>(composed->remove_child(nested_failure->runtime_id()));
    root->add_child(composed);
    require(loads == 1 && composed->is_loaded() &&
                composed->attachment_count() == 1,
            "retry after subtree rollback must commit attachment without repeating Load");
}

void test_lifecycle_callbacks_cannot_mutate_tree_structure() {
    auto root = make_control<Panel>(StableId("guard.root"));
    Window window(root, {100.0, 60.0});
    auto composed = make_control<UserControl>(StableId("guard.composed"));
    bool structural_rejection = false;
    auto load = composed->loaded().subscribe([&] {
        try {
            static_cast<void>(root->remove_child(composed->runtime_id()));
        } catch (const std::logic_error&) {
            structural_rejection = true;
            throw;
        }
    });
    bool attach_rejected = false;
    try {
        root->add_child(composed);
    } catch (const std::logic_error&) {
        attach_rejected = true;
    }
    require(structural_rejection && attach_rejected && !composed->attached() &&
                !composed->parent() && !window.find("guard.composed"),
            "lifecycle callbacks must reject structural mutation and roll back attachment");
}

void test_nested_initialization_batches_invalidation_only() {
    auto label = make_control<Label>(StableId("init.label"), "initial");
    label->set_requested_bounds({0.0, 0.0, 120.0, 24.0});
    Window window(label, {120.0, 24.0});
    window.perform_layout();
    window.reset_activity_metrics();

    std::uint64_t text_events = 0;
    std::uint64_t completions = 0;
    Dirty completed_dirty = Dirty::none;
    bool completed_subtree = false;
    auto text = label->text_changed().subscribe(
        [&text_events](const std::string&) { ++text_events; });
    auto complete = label->initialization_completed().subscribe(
        [&](Dirty dirty, bool subtree) {
            ++completions;
            completed_dirty = dirty;
            completed_subtree = subtree;
        });

    label->begin_init();
    label->set_text("first");
    label->begin_init();
    label->set_text("second");
    label->invalidate_subtree(Dirty::semantics);
    label->end_init();
    require(text_events == 2 && completions == 0 &&
                window.metrics_snapshot().mutations == 0,
            "nested initialization must preserve synchronous property events while deferring dirtiness");
    label->end_init();

    require(completions == 1 && completed_subtree &&
                has_dirty(completed_dirty, invalidation::text_content) &&
                has_dirty(completed_dirty, Dirty::semantics) &&
                window.metrics_snapshot().mutations == 1,
            "outer EndInit must publish and apply one coalesced declared invalidation");

    bool underflow = false;
    try {
        label->end_init();
    } catch (const std::logic_error&) {
        underflow = true;
    }
    require(underflow, "EndInit without BeginInit must be rejected");
}

void test_initialization_disposal_and_thread_guards() {
    auto root = make_control<Panel>(StableId("init.dispose.root"));
    auto label = make_control<Label>(StableId("init.dispose.label"), "alive");
    root->add_child(label);
    Window window(root, {100.0, 40.0});
    std::uint64_t completions = 0;
    auto complete = label->initialization_completed().subscribe(
        [&](Dirty, bool) { ++completions; });
    label->begin_init();
    label->set_text("pending");
    label->dispose();
    require(label->is_disposed() && completions == 0,
            "disposal during initialization must discard pending completion work");

    auto attached = make_control<Label>(StableId("init.thread"), "thread");
    Window thread_window(attached, {100.0, 24.0});
    bool rejected = false;
    std::thread worker([&] {
        try {
            attached->begin_init();
        } catch (const std::logic_error&) {
            rejected = true;
        }
    });
    worker.join();
    require(rejected && attached->initialization_depth() == 0,
            "initialization scopes must preserve attached-control UI-thread enforcement");
}

} // namespace

int main() {
    try {
        test_attach_detach_order_and_observable_state();
        test_user_control_load_once_across_reparent();
        test_failed_attach_rolls_back_binding_and_identity();
        test_lifecycle_callbacks_cannot_mutate_tree_structure();
        test_nested_initialization_batches_invalidation_only();
        test_initialization_disposal_and_thread_guards();
        std::cout << "lifecycle controls tests passed\n";
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        std::cerr << "lifecycle controls test failure: " << error.what() << '\n';
        return EXIT_FAILURE;
    }
}
