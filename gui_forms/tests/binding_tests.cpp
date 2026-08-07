#include "gui_forms/gui_forms.hpp"

#include <cstdlib>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

using namespace gui_forms;

void require(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}

struct Fixture final {
    Fixture()
        : root(make_control<Panel>(StableId("binding.root"))),
          editor(make_control<TextBox>(StableId("binding.editor"))),
          enabled(make_control<CheckBox>(StableId("binding.enabled"), "Enabled")),
          gain(make_control<TrackBar>(StableId("binding.gain"))),
          caption(make_control<Label>(StableId("binding.caption"))),
          window(root, {640.0, 240.0}),
          source(std::make_shared<BindingSource>(window)) {
        root->set_requested_bounds({0.0, 0.0, 640.0, 240.0});
        editor->set_requested_bounds({20.0, 20.0, 220.0, 32.0});
        enabled->set_requested_bounds({20.0, 70.0, 180.0, 28.0});
        gain->set_requested_bounds({20.0, 115.0, 300.0, 32.0});
        caption->set_requested_bounds({350.0, 20.0, 240.0, 32.0});
        root->add_child(editor);
        root->add_child(enabled);
        root->add_child(gain);
        root->add_child(caption);
        source->set_records({
            {"record.alpha", {{"name", std::string("Alpha")},
                               {"enabled", true}, {"gain", 24.0}}},
            {"record.beta", {{"name", std::string("Beta")},
                              {"enabled", false}, {"gain", 72.0}}},
        });
    }

    std::shared_ptr<Panel> root;
    std::shared_ptr<TextBox> editor;
    std::shared_ptr<CheckBox> enabled;
    std::shared_ptr<TrackBar> gain;
    std::shared_ptr<Label> caption;
    Window window;
    std::shared_ptr<BindingSource> source;
};

void test_value_conversion_is_strict_and_invariant() {
    require(binding_value_kind(BindingValue{}) == BindingValueKind::null &&
                binding_value_to_bool(std::string("TRUE")) == true &&
                binding_value_to_signed(std::string("-42")) == -42 &&
                binding_value_to_unsigned(std::string("42")) == 42U &&
                binding_value_to_number(std::string("12.5")) == 12.5,
            "binding scalar conversion must cover null, Boolean, integer, and invariant number values");
    require(!binding_value_to_unsigned(std::string("-1")) &&
                !binding_value_to_number(std::string("12.5ms")) &&
                !binding_value_to_signed(2.5) &&
                canonical_binding_name("  DataMember  ") == "datamember",
            "binding conversion must reject partial, negative-unsigned, and fractional-integer input");
}

void test_multi_control_two_way_currency_and_event_order() {
    Fixture fixture;
    BindingOptions immediate;
    immediate.data_source_update_mode =
        DataSourceUpdateMode::on_property_changed;
    const auto text = fixture.editor->data_bindings().add(
        "Text", fixture.source, "Name", immediate);
    const auto check = fixture.enabled->data_bindings().add(
        "Checked", fixture.source, "Enabled", immediate);
    const auto range = fixture.gain->data_bindings().add(
        "Value", fixture.source, "Gain", immediate);
    const auto label = fixture.caption->data_bindings().add(
        "Text", fixture.source, "Name");
    require(text && check && range && label &&
                fixture.editor->text() == "Alpha" && fixture.enabled->checked() &&
                fixture.gain->value() == 24.0 && fixture.caption->text() == "Alpha",
            "adding stock bindings must immediately project the current record into every control kind");

    fixture.editor->set_text("Alpha edited");
    fixture.enabled->set_checked(false);
    fixture.gain->set_value(31.0);
    require(fixture.source->current_field("name") ==
                std::optional<BindingValue>{std::string("Alpha edited")} &&
                fixture.source->current_field("enabled") ==
                    std::optional<BindingValue>{false} &&
                fixture.source->current_field("gain") ==
                    std::optional<BindingValue>{31.0},
            "OnPropertyChanged bindings must commit control mutations to the current retained record");

    std::vector<std::string> order;
    auto position = fixture.source->position_changed().subscribe(
        [&](std::ptrdiff_t value) {
            require(fixture.editor->text() == "Beta",
                    "binding propagation must precede public PositionChanged");
            order.push_back("position:" + std::to_string(value));
        });
    auto current = fixture.source->current_changed().subscribe(
        [&] { order.push_back("current"); });
    auto item = fixture.source->current_item_changed().subscribe(
        [&] { order.push_back("item"); });
    require(fixture.source->currency_manager().set_position(1) &&
                fixture.source->currency_manager().current()->stable_id ==
                    "record.beta" && fixture.editor->text() == "Beta" &&
                !fixture.enabled->checked() && fixture.gain->value() == 72.0 &&
                order == std::vector<std::string>{
                    "position:1", "current", "item"},
            "currency movement must update controls then emit the chosen deterministic event order");
    static_cast<void>(position);
    static_cast<void>(current);
    static_cast<void>(item);
}

void test_update_modes_suspension_and_explicit_transfers() {
    Fixture fixture;
    BindingOptions validation;
    validation.data_source_update_mode = DataSourceUpdateMode::on_validation;
    const auto binding = fixture.editor->data_bindings().add(
        "Text", fixture.source, "Name", validation);
    fixture.editor->set_text("pending");
    require(fixture.source->current_field("name") ==
                std::optional<BindingValue>{std::string("Alpha")},
            "OnValidation must retain an editor-side pending value");
    require(binding->validate() && fixture.source->current_field("name") ==
                std::optional<BindingValue>{std::string("pending")},
            "explicit validation must commit an OnValidation binding");

    fixture.source->currency_manager().suspend_binding();
    fixture.source->set_current_field("name", std::string("suspended"));
    fixture.source->set_current_field("name", std::string("coalesced"));
    require(fixture.editor->text() == "pending" &&
                fixture.source->snapshot().suspended_mutations == 2U &&
                fixture.source->currency_manager().binding_suspended(),
            "SuspendBinding must retain logical mutations without pushing controls");
    fixture.source->currency_manager().resume_binding();
    require(fixture.editor->text() == "coalesced" &&
                !fixture.source->binding_suspended(),
            "ResumeBinding must publish one current reset after suspended mutations");

    Fixture never_fixture;
    BindingOptions never;
    never.control_update_mode = ControlUpdateMode::never;
    never.data_source_update_mode = DataSourceUpdateMode::never;
    const auto never_binding = never_fixture.editor->data_bindings().add(
        "Text", never_fixture.source, "Name", never);
    require(never_fixture.editor->text().empty(),
            "ControlUpdateMode.Never must suppress the initial automatic read");
    require(never_binding->read_value() && never_fixture.editor->text() == "Alpha",
            "ReadValue must remain an explicit transfer under Never");
    never_fixture.editor->set_text("manual");
    require(never_fixture.source->current_field("name") ==
                std::optional<BindingValue>{std::string("Alpha")} &&
                never_binding->write_value() &&
                never_fixture.source->current_field("name") ==
                    std::optional<BindingValue>{std::string("manual")},
            "WriteValue must remain an explicit transfer under Never");
}

void test_format_parse_failure_rollback_and_completion() {
    auto root = make_control<Panel>(StableId("format.root"));
    auto editor = make_control<TextBox>(StableId("format.editor"));
    root->add_child(editor);
    Window window(root, {320.0, 120.0});
    auto source = std::make_shared<BindingSource>(window);
    source->set_records({{"format.record", {{"amount", 12.5}}}});
    BindingOptions options;
    options.formatting_enabled = true;
    options.format_string = "F2";
    options.data_source_update_mode = DataSourceUpdateMode::on_property_changed;
    const auto binding = editor->data_bindings().add(
        "Text", source, "Amount", options);
    require(editor->text() == "12.50",
            "invariant fixed formatting must project numeric data into text");

    std::vector<BindingCompleteState> completions;
    std::vector<std::string> errors;
    auto completed = binding->binding_complete().subscribe(
        [&](BindingCompleteEvent& event) { completions.push_back(event.state); });
    auto data_error = source->data_error().subscribe(
        [&](const std::string& error) { errors.push_back(error); });
    editor->set_text("not-a-number");
    require(source->current_field("amount") ==
                std::optional<BindingValue>{12.5} &&
                !completions.empty() &&
                completions.back() == BindingCompleteState::exception &&
                errors.size() == 1U,
            "failed parsing must retain the authoritative source and publish completion plus DataError");

    auto parser = binding->parse().subscribe([](BindingConvertEvent& event) {
        if (const auto* text = std::get_if<std::string>(&event.value);
            text && *text == "$13.25") {
            event.value = 13.25;
            event.handled = true;
        }
    });
    editor->set_text("$13.25");
    require(source->current_field("amount") ==
                std::optional<BindingValue>{13.25},
            "a handled Parse event must provide a typed source value without reflection");
    static_cast<void>(completed);
    static_cast<void>(data_error);
    static_cast<void>(parser);
}

void test_edit_transactions_list_mutation_and_context() {
    Fixture fixture;
    BindingManagerBase& manager = fixture.source->currency_manager();
    require(manager.count() == 2U && manager.current() == fixture.source->current() &&
                fixture.source->begin_edit() &&
                fixture.source->set_current_field("name", std::string("draft")),
            "editable current record must establish a deterministic snapshot");
    manager.cancel_current_edit();
    require(fixture.source->current_field("name") ==
                std::optional<BindingValue>{std::string("Alpha")} &&
                !fixture.source->snapshot().editing,
            "CancelEdit must restore the exact retained record snapshot");
    fixture.source->set_current_field("name", std::string("committed"));
    manager.end_current_edit();
    manager.cancel_current_edit();
    require(fixture.source->current_field("name") ==
                std::optional<BindingValue>{std::string("committed")},
            "EndEdit must retire rollback state without changing committed fields");

    const std::size_t inserted = fixture.source->insert(
        1U, {"record.middle", {{"name", std::string("Middle")}}});
    require(inserted == 1U && fixture.source->count() == 3U &&
                fixture.source->find("name", std::string("Middle")) == 1U,
            "BindingSource must support stable insertion and field search");
    require(manager.remove_at(1U) &&
                fixture.source->count() == 2U,
            "BindingSource removal must normalize currency without stale identities");

    BindingContext context(fixture.window);
    std::vector<bool> changes;
    auto observed = context.collection_changed().subscribe(
        [&](const BindingContextChange& change) { changes.push_back(change.added); });
    require(&context.manager(fixture.source) ==
                &fixture.source->currency_manager() &&
                context.contains(*fixture.source) && context.size() == 1U &&
                context.remove(*fixture.source) &&
                changes == std::vector<bool>{true, false},
            "BindingContext must own a real same-window manager registry and ordered change events");
    static_cast<void>(observed);
}

void test_collection_uniqueness_base_properties_and_cleanup() {
    Fixture fixture;
    fixture.source->set_current_field("visible", false);
    fixture.source->set_current_field("enabled_property", false);
    fixture.source->set_current_field("control_name", std::string("bound.editor"));
    const auto visible = fixture.editor->data_bindings().add(
        "Visible", fixture.source, "visible");
    const auto enabled = fixture.editor->data_bindings().add(
        "Enabled", fixture.source, "enabled_property");
    const auto name = fixture.editor->data_bindings().add(
        "Name", fixture.source, "control_name");
    require(visible && enabled && name && !fixture.editor->visible() &&
                !fixture.editor->enabled() && fixture.editor->name() == "bound.editor",
            "base Control visibility, enabled state, and name must be first-class bindable properties");

    bool duplicate_rejected = false;
    try {
        static_cast<void>(fixture.editor->data_bindings().add(
            "text", fixture.source, "name"));
        static_cast<void>(fixture.editor->data_bindings().add(
            "TEXT", fixture.source, "name"));
    } catch (const std::invalid_argument&) {
        duplicate_rejected = true;
    }
    require(duplicate_rejected,
            "ControlBindingsCollection must reject duplicate canonical target properties");

    const auto cleanup = fixture.caption->data_bindings().add(
        "Text", fixture.source, "Name");
    fixture.caption->dispose();
    require(cleanup->is_disposed(),
            "disposing a target Control must synchronously dispose its bindings");
    fixture.source->dispose();
    require(!visible->active() && !enabled->active() && !name->active(),
            "disposing a BindingSource must deactivate every surviving endpoint without dangling access");
}

void test_choice_numeric_defaults_manager_transfer_and_completion_order() {
    auto root = make_control<Panel>(StableId("advanced.root"));
    auto combo = make_control<ComboBox>(StableId("advanced.combo"));
    auto numeric = make_control<NumericUpDown>(StableId("advanced.numeric"));
    auto deferred = make_control<TextBox>(StableId("advanced.deferred"));
    combo->set_items({"Local", "Archive", "Plugins"});
    root->add_child(combo);
    root->add_child(numeric);
    root->add_child(deferred);
    Window window(root, {480.0, 180.0});
    auto source = std::make_shared<BindingSource>(window);
    source->set_records({
        {"advanced.record", {{"choice", std::int64_t{1}},
                              {"amount", 42.0},
                              {"name", std::string("Alpha")}}},
    });

    BindingOptions formatted_immediate;
    formatted_immediate.formatting_enabled = true;
    formatted_immediate.data_source_update_mode =
        DataSourceUpdateMode::on_property_changed;
    const auto choice = combo->data_bindings().add(
        "SelectedIndex", source, "choice", formatted_immediate);
    const auto amount = numeric->data_bindings().add(
        "Value", source, "amount", formatted_immediate);
    require(combo->selected_index() == 1U && numeric->value() == 42.0,
            "ComboBox.SelectedIndex and NumericUpDown.Value must be stock bindable properties");

    bool binding_saw_committed_control = false;
    bool manager_saw_committed_control = false;
    auto binding_complete = choice->binding_complete().subscribe(
        [&](BindingCompleteEvent& event) {
            if (event.context == BindingCompleteContext::control_update &&
                event.state == BindingCompleteState::success) {
                binding_saw_committed_control = combo->selected_index() == 2U;
            }
        });
    auto manager_complete = source->currency_manager().binding_complete().subscribe(
        [&](BindingCompleteEvent& event) {
            if (event.binding == choice.get() &&
                event.context == BindingCompleteContext::control_update &&
                event.state == BindingCompleteState::success) {
                manager_saw_committed_control = combo->selected_index() == 2U;
            }
        });
    source->set_current_field("choice", std::int64_t{2});
    require(binding_saw_committed_control && manager_saw_committed_control,
            "BindingComplete must run after the destination commit and propagate through the currency manager");

    bool source_was_committed_before_completion = false;
    auto source_complete = amount->binding_complete().subscribe(
        [&](BindingCompleteEvent& event) {
            if (event.context == BindingCompleteContext::data_source_update &&
                event.state == BindingCompleteState::success) {
                source_was_committed_before_completion =
                    source->current_field("amount") ==
                        std::optional<BindingValue>{64.0};
            }
        });
    numeric->set_value(64.0);
    require(source_was_committed_before_completion,
            "data-source BindingComplete must observe the already committed retained record");

    deferred->data_bindings().set_default_data_source_update_mode(
        DataSourceUpdateMode::on_property_changed);
    const auto defaulted = deferred->data_bindings().add(
        "Text", source, "name");
    deferred->set_text("Default immediate");
    require(source->current_field("name") ==
                std::optional<BindingValue>{std::string("Default immediate")},
            "the no-options Add overload must use the collection default update mode");

    BindingOptions explicit_validation;
    explicit_validation.data_source_update_mode =
        DataSourceUpdateMode::on_validation;
    defaulted->dispose();
    deferred->data_bindings().clear();
    const auto validation = deferred->data_bindings().add(
        "Text", source, "name", explicit_validation);
    deferred->set_text("Still pending");
    require(source->current_field("name") ==
                std::optional<BindingValue>{std::string("Default immediate")},
            "an explicit OnValidation option must not be overwritten by the collection default");
    require(source->currency_manager().pull_data() &&
                source->current_field("name") ==
                    std::optional<BindingValue>{std::string("Still pending")},
            "CurrencyManager.PullData must transfer every live target into its source");
    source->set_current_field("name", std::string("Pushed"));
    deferred->set_text("Local pending");
    require(source->currency_manager().push_data() &&
                deferred->text() == "Pushed",
            "CurrencyManager.PushData must refresh every live target from retained currency");

    BindingContext context(window);
    std::vector<bool> context_changes;
    auto context_observer = context.collection_changed().subscribe(
        [&](const BindingContextChange& change) {
            context_changes.push_back(change.added);
        });
    context.add(source);
    source->dispose();
    require(context.size() == 0U &&
                context_changes == std::vector<bool>{true, false},
            "BindingContext must eagerly remove a disposed source and publish one removal");
    static_cast<void>(binding_complete);
    static_cast<void>(manager_complete);
    static_cast<void>(source_complete);
    static_cast<void>(validation);
    static_cast<void>(context_observer);
}

} // namespace

int main() {
    try {
        test_value_conversion_is_strict_and_invariant();
        test_multi_control_two_way_currency_and_event_order();
        test_update_modes_suspension_and_explicit_transfers();
        test_format_parse_failure_rollback_and_completion();
        test_edit_transactions_list_mutation_and_context();
        test_collection_uniqueness_base_properties_and_cleanup();
        test_choice_numeric_defaults_manager_transfer_and_completion_order();
        std::cout << "gui_forms_binding_tests: all tests passed\n";
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        std::cerr << "gui_forms_binding_tests: " << error.what() << '\n';
        return EXIT_FAILURE;
    }
}
