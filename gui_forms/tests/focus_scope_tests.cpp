#include "gui_forms/basic_controls.hpp"
#include "gui_forms/window.hpp"

#include <cstdlib>
#include <array>
#include <iostream>
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

struct FocusFixture final {
    FocusFixture() {
        outside->set_requested_bounds({0.0, 0.0, 80.0, 24.0});
        popup->set_requested_bounds({0.0, 30.0, 160.0, 70.0});
        first->set_requested_bounds({4.0, 4.0, 70.0, 24.0});
        second->set_requested_bounds({82.0, 4.0, 70.0, 24.0});
        popup->add_child(first);
        popup->add_child(second);
        root->add_child(outside);
        root->add_child(popup);
        window = std::make_unique<Window>(root, Size{200.0, 120.0});
        window->perform_layout();
    }

    std::shared_ptr<Panel> root = make_control<Panel>(StableId("focus.root"));
    std::shared_ptr<Button> outside =
        make_control<Button>(StableId("focus.outside"), "Outside");
    std::shared_ptr<Panel> popup =
        make_control<Panel>(StableId("focus.popup"));
    std::shared_ptr<Button> first =
        make_control<Button>(StableId("focus.popup.first"), "First");
    std::shared_ptr<Button> second =
        make_control<Button>(StableId("focus.popup.second"), "Second");
    std::unique_ptr<Window> window;
};

void test_containment_and_tab_traversal() {
    FocusFixture fixture;
    require(fixture.window->request_focus(fixture.outside),
            "fixture outside control must accept focus");
    const FocusScopeId scope = fixture.window->begin_focus_scope(
        fixture.popup, fixture.first);
    require(scope && fixture.window->focus_scope_depth() == 1U &&
                fixture.window->active_focus_scope_root() == fixture.popup &&
                fixture.window->focused_control() == fixture.first,
            "scope entry must publish identity and focus the preferred descendant");
    require(!fixture.window->request_focus(fixture.outside) &&
                fixture.window->focused_control() == fixture.first,
            "contained scope must reject focus leakage to an outside control");

    KeyEvent tab;
    tab.action = KeyAction::down;
    tab.physical_key = PhysicalKey::tab;
    require(fixture.window->dispatch_key(tab) &&
                fixture.window->focused_control() == fixture.second,
            "Tab must traverse forward inside the active scope");
    require(fixture.window->dispatch_key(tab) &&
                fixture.window->focused_control() == fixture.first,
            "scope traversal must wrap in stable retained-tree order");
    tab.modifiers = Modifier::shift;
    require(fixture.window->dispatch_key(tab) &&
                fixture.window->focused_control() == fixture.second,
            "Shift+Tab must traverse backward inside the active scope");

    require(fixture.window->end_focus_scope(scope) &&
                fixture.window->focused_control() == fixture.outside &&
                fixture.window->focus_scope_depth() == 0U,
            "scope exit must restore the focus present at entry");
    const MetricsSnapshot metrics = fixture.window->metrics_snapshot();
    require(metrics.focus_scopes_opened == 1U &&
                metrics.focus_scopes_closed == 1U &&
                metrics.focus_scope_restorations == 1U &&
                metrics.focus_scope_rejections == 1U &&
                metrics.maximum_focus_scope_depth == 1U,
            "focus-scope behavior must be structurally observable in metrics");
}

void test_nested_out_of_order_close_restores_outer_history() {
    auto root = make_control<Panel>(StableId("nested.root"));
    auto base = make_control<Button>(StableId("nested.base"), "Base");
    auto outer = make_control<Panel>(StableId("nested.outer"));
    auto outer_field =
        make_control<Button>(StableId("nested.outer.field"), "Outer");
    auto inner = make_control<Panel>(StableId("nested.inner"));
    auto inner_field =
        make_control<Button>(StableId("nested.inner.field"), "Inner");
    inner->add_child(inner_field);
    outer->add_child(outer_field);
    outer->add_child(inner);
    root->add_child(base);
    root->add_child(outer);
    Window window(root, {240.0, 140.0});
    require(window.request_focus(base), "nested fixture base focus must succeed");

    std::vector<std::string> changes;
    auto changed = window.focus_scope_changed().subscribe(
        [&changes](const FocusScopeChange& change) {
            changes.push_back(std::string(change.opened ? "open:" : "close:") +
                              change.stable_id);
        });
    const FocusScopeId outer_scope =
        window.begin_focus_scope(outer, outer_field);
    const FocusScopeId inner_scope =
        window.begin_focus_scope(inner, inner_field);
    require(window.focused_control() == inner_field &&
                window.focus_scope_depth() == 2U,
            "nested scope must own focus at the deepest active level");
    require(window.end_focus_scope(outer_scope) &&
                window.focused_control() == inner_field &&
                window.focus_scope_depth() == 1U,
            "out-of-order outer close must not steal focus from a live inner scope");
    require(window.end_focus_scope(inner_scope) &&
                window.focused_control() == base &&
                window.focus_scope_depth() == 0U,
            "closing the inner scope must collapse tombstones and restore outer history");
    require(changes == std::vector<std::string>{
                "open:nested.outer", "open:nested.inner",
                "close:nested.outer", "close:nested.inner"},
            "nested scope events must remain deterministic under out-of-order close");
}

void test_owner_unavailable_cleanup_and_nesting_guard() {
    FocusFixture fixture;
    require(fixture.window->request_focus(fixture.outside),
            "cleanup fixture outside focus must succeed");
    FocusScopeCloseReason close_reason = FocusScopeCloseReason::explicit_close;
    bool saw_close = false;
    auto changed = fixture.window->focus_scope_changed().subscribe(
        [&](const FocusScopeChange& change) {
            if (!change.opened) {
                close_reason = change.close_reason;
                saw_close = true;
            }
        });
    static_cast<void>(fixture.window->begin_focus_scope(
        fixture.popup, fixture.first));

    bool rejected_foreign_nested_scope = false;
    try {
        static_cast<void>(fixture.window->begin_focus_scope(fixture.outside));
    } catch (const std::logic_error&) {
        rejected_foreign_nested_scope = true;
    }
    require(rejected_foreign_nested_scope,
            "a nested scope must remain inside the containing retained subtree");

    fixture.popup->set_visible(false);
    require(saw_close &&
                close_reason == FocusScopeCloseReason::owner_unavailable &&
                fixture.window->focus_scope_depth() == 0U &&
                fixture.window->focused_control() == fixture.outside,
            "hiding a scope root must close it and restore eligible outside focus");
}

void test_ui_thread_enforcement_and_empty_focus_traversal() {
    FocusFixture fixture;
    KeyEvent tab;
    tab.action = KeyAction::down;
    tab.physical_key = PhysicalKey::tab;
    require(fixture.window->dispatch_key(tab) &&
                fixture.window->focused_control() == fixture.outside,
            "Tab must establish initial focus when no control is active");

    bool rejected = false;
    std::thread worker([&] {
        try {
            static_cast<void>(fixture.window->begin_focus_scope(fixture.popup));
        } catch (const std::logic_error&) {
            rejected = true;
        }
    });
    worker.join();
    require(rejected && fixture.window->focus_scope_depth() == 0U &&
                fixture.window->metrics_snapshot().rejected_wrong_thread_operations == 1U,
            "focus-scope mutation must preserve renderer-free UI-thread enforcement");
}

void test_bounded_nesting() {
    FocusFixture fixture;
    std::array<FocusScopeId, maximum_focus_scope_depth> scopes{};
    for (FocusScopeId& scope : scopes) {
        scope = fixture.window->begin_focus_scope(fixture.popup);
    }
    bool rejected = false;
    try {
        static_cast<void>(fixture.window->begin_focus_scope(fixture.popup));
    } catch (const std::length_error&) {
        rejected = true;
    }
    require(rejected &&
                fixture.window->focus_scope_depth() == maximum_focus_scope_depth,
            "focus-scope nesting must stop at its declared portable bound");
    for (auto scope = scopes.rbegin(); scope != scopes.rend(); ++scope) {
        require(fixture.window->end_focus_scope(*scope),
                "bounded focus scopes must close in reverse order");
    }
    require(fixture.window->focus_scope_depth() == 0U &&
                fixture.window->metrics_snapshot().maximum_focus_scope_depth ==
                    maximum_focus_scope_depth,
            "bounded nesting depth must be observable and fully recoverable");
}

void test_open_callback_may_close_without_late_focus() {
    FocusFixture fixture;
    require(fixture.window->request_focus(fixture.outside),
            "reentrant fixture outside focus must succeed");
    auto changed = fixture.window->focus_scope_changed().subscribe(
        [&](const FocusScopeChange& change) {
            if (change.opened) {
                static_cast<void>(fixture.window->end_focus_scope(change.scope));
            }
        });
    const FocusScopeId scope = fixture.window->begin_focus_scope(
        fixture.popup, fixture.first);
    require(scope && fixture.window->focus_scope_depth() == 0U &&
                fixture.window->focused_control() == fixture.outside &&
                !fixture.window->end_focus_scope(scope),
            "a synchronous open handler may close the scope without a late focus leak");
}

} // namespace

int main() {
    try {
        test_containment_and_tab_traversal();
        test_nested_out_of_order_close_restores_outer_history();
        test_owner_unavailable_cleanup_and_nesting_guard();
        test_ui_thread_enforcement_and_empty_focus_traversal();
        test_bounded_nesting();
        test_open_callback_may_close_without_late_focus();
        std::cout << "gui_forms_focus_scope_tests: all tests passed\n";
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        std::cerr << "gui_forms_focus_scope_tests: " << error.what() << '\n';
        return EXIT_FAILURE;
    }
}
