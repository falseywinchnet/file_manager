#include "gui_forms/commands/command_binding/command_binding.hpp"

#include <stdexcept>
#include <utility>

namespace gui_forms {
namespace {

struct ExecuteBoundCommand final {
    std::weak_ptr<Command> command;

    void operator()(ButtonBase& source) const {
        if (const std::shared_ptr<Command> retained = command.lock()) {
            static_cast<void>((*retained).execute(source.stable_id().value()));
        }
    }
};

struct SynchronizeBoundButton final {
    std::weak_ptr<ButtonBase> button;
    CommandBindingOptions options;

    void operator()(const CommandState& state) const {
        const std::shared_ptr<ButtonBase> retained = button.lock();
        if (!retained) return;
        if (options.synchronize_enabled) (*retained).set_enabled(state.enabled);
        if (options.synchronize_text) (*retained).set_text(state.text);
        if (options.synchronize_visibility) {
            (*retained).set_visible(state.visible);
        }
        if (options.synchronize_accessible_description) {
            (*retained).set_accessible_description(state.description);
        }
    }
};

} // namespace

CommandBinding::CommandBinding(std::shared_ptr<Command> command,
                               std::shared_ptr<ButtonBase> button,
                               CommandBindingOptions options)
    : command_(std::move(command)), button_(std::move(button)), options_(options) {
    if (!command_ || !button_) {
        throw std::invalid_argument(
            "CommandBinding requires command and button owners");
    }
    apply((*command_).state());
    click_ = (*button_).clicked().subscribe(
        ExecuteBoundCommand{command_});
    state_ = (*command_).state_changed().subscribe(
        SynchronizeBoundButton{button_, options_});
}

void CommandBinding::apply(const CommandState& state) {
    if (options_.synchronize_enabled) (*button_).set_enabled(state.enabled);
    if (options_.synchronize_text) (*button_).set_text(state.text);
    if (options_.synchronize_visibility) (*button_).set_visible(state.visible);
    if (options_.synchronize_accessible_description) {
        (*button_).set_accessible_description(state.description);
    }
}

} // namespace gui_forms
