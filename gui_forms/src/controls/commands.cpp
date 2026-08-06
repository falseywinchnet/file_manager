#include "gui_forms/commands.hpp"

#include "gui_forms/text.hpp"

#include <stdexcept>
#include <utility>

namespace gui_forms {

Command::Command(std::string stable_id, std::string text)
    : stable_id_(std::move(stable_id)) {
    if (stable_id_.empty() || !validate_utf8(stable_id_).valid() ||
        !validate_utf8(text).valid()) {
        throw std::invalid_argument("Command identity and text must be valid UTF-8");
    }
    state_.text = std::move(text);
}

void Command::publish_state() {
    ++state_.generation;
    state_changed_.emit(state_);
}

void Command::set_text(std::string text) {
    if (!validate_utf8(text).valid()) {
        throw std::invalid_argument("Command text must be valid UTF-8");
    }
    if (state_.text == text) return;
    state_.text = std::move(text);
    publish_state();
}

namespace {

void require_command_text(std::string_view value, const char* field) {
    if (!validate_utf8(value).valid()) {
        throw std::invalid_argument(std::string("Command ") + field +
                                    " must be valid UTF-8");
    }
}

} // namespace

#define GUI_FORMS_COMMAND_STRING_SETTER(method, member, field_name) \
void Command::method(std::string value) {                           \
    require_command_text(value, field_name);                        \
    if (state_.member == value) return;                             \
    state_.member = std::move(value);                               \
    publish_state();                                                \
}

GUI_FORMS_COMMAND_STRING_SETTER(set_description, description, "description")
GUI_FORMS_COMMAND_STRING_SETTER(set_icon_id, icon_id, "icon ID")
GUI_FORMS_COMMAND_STRING_SETTER(set_shortcut, shortcut, "shortcut")
GUI_FORMS_COMMAND_STRING_SETTER(set_mnemonic, mnemonic, "mnemonic")
GUI_FORMS_COMMAND_STRING_SETTER(set_key_tip, key_tip, "key tip")
GUI_FORMS_COMMAND_STRING_SETTER(set_availability_reason, availability_reason,
                                "availability reason")

#undef GUI_FORMS_COMMAND_STRING_SETTER

void Command::set_enabled(bool enabled) {
    if (state_.enabled == enabled) return;
    state_.enabled = enabled;
    publish_state();
}

void Command::set_visible(bool visible) {
    if (state_.visible == visible) return;
    state_.visible = visible;
    publish_state();
}

void Command::set_checked(bool checked) {
    if (state_.checked == checked) return;
    state_.checked = checked;
    publish_state();
}

void Command::set_default_action(bool is_default) {
    if (state_.default_action == is_default) return;
    state_.default_action = is_default;
    publish_state();
}

void Command::set_destructive(bool destructive) {
    if (state_.destructive == destructive) return;
    state_.destructive = destructive;
    publish_state();
}

bool Command::execute(std::string_view source_id) {
    if (!is_alive() || !state_.enabled || !validate_utf8(source_id).valid()) return false;
    CommandInvocation invocation{stable_id_, std::string(source_id), next_sequence_++};
    invoked_.emit(invocation);
    return true;
}

CommandBinding::CommandBinding(std::shared_ptr<Command> command,
                               std::shared_ptr<ButtonBase> button,
                               CommandBindingOptions options)
    : command_(std::move(command)), button_(std::move(button)), options_(options) {
    if (!command_ || !button_) {
        throw std::invalid_argument("CommandBinding requires command and button owners");
    }
    apply(command_->state());
    const std::weak_ptr<Command> weak_command = command_;
    click_ = button_->clicked().subscribe(
        [weak_command](ButtonBase& source) {
            if (const auto command = weak_command.lock()) {
                static_cast<void>(command->execute(source.stable_id().value()));
            }
        });
    const std::weak_ptr<ButtonBase> weak_button = button_;
    const CommandBindingOptions binding_options = options_;
    state_ = command_->state_changed().subscribe(
        [weak_button, binding_options](const CommandState& state) {
            if (const auto button = weak_button.lock()) {
                button->set_enabled(state.enabled);
                if (binding_options.synchronize_text) button->set_text(state.text);
                if (binding_options.synchronize_visibility) {
                    button->set_visible(state.visible);
                }
                if (binding_options.synchronize_accessible_description) {
                    button->set_accessible_description(state.description);
                }
            }
        });
}

void CommandBinding::apply(const CommandState& state) {
    button_->set_enabled(state.enabled);
    if (options_.synchronize_text) button_->set_text(state.text);
    if (options_.synchronize_visibility) button_->set_visible(state.visible);
    if (options_.synchronize_accessible_description) {
        button_->set_accessible_description(state.description);
    }
}

} // namespace gui_forms
