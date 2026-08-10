#include "gui_forms/commands/command_binding/command_binding.hpp"

#include <stdexcept>
#include <utility>

namespace gui_forms {

CommandBinding::CommandBinding(std::shared_ptr<Command> command,
                               std::shared_ptr<ButtonBase> button,
                               CommandBindingOptions options)
    : command_(std::move(command)), button_(std::move(button)), options_(options) {
    if (!command_ || !button_) {
        throw std::invalid_argument(
            "CommandBinding requires command and button owners");
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
                if (binding_options.synchronize_enabled) {
                    button->set_enabled(state.enabled);
                }
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
    if (options_.synchronize_enabled) button_->set_enabled(state.enabled);
    if (options_.synchronize_text) button_->set_text(state.text);
    if (options_.synchronize_visibility) button_->set_visible(state.visible);
    if (options_.synchronize_accessible_description) {
        button_->set_accessible_description(state.description);
    }
}

} // namespace gui_forms
