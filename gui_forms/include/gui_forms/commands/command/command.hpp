#pragma once

#include "gui_forms/commands/types/command_types.hpp"
#include "gui_forms/component/component/component.hpp"
#include "gui_forms/event.hpp"

#include <string>
#include <string_view>

namespace gui_forms {

// Shared authority for menu, shortcut, context, semantic, and test surfaces.
class Command final : public Component {
public:
    Command(std::string stable_id, std::string text);

    [[nodiscard]] const std::string& stable_id() const noexcept {
        return stable_id_;
    }
    [[nodiscard]] const CommandState& state() const noexcept { return state_; }
    void set_text(std::string text);
    void set_description(std::string description);
    void set_icon_id(std::string icon_id);
    void set_shortcut(std::string shortcut);
    void set_mnemonic(std::string mnemonic);
    void set_key_tip(std::string key_tip);
    void set_availability_reason(std::string reason);
    void set_enabled(bool enabled);
    void set_visible(bool visible);
    void set_checked(bool checked);
    void set_default_action(bool is_default);
    void set_destructive(bool destructive);
    bool execute(std::string_view source_id = {});

    [[nodiscard]] Event<const CommandState&>& state_changed() noexcept {
        return state_changed_;
    }
    [[nodiscard]] Event<const CommandInvocation&>& invoked() noexcept {
        return invoked_;
    }

private:
    void publish_state();

    std::string stable_id_;
    CommandState state_;
    std::uint64_t next_sequence_{1U};
    Event<const CommandState&> state_changed_;
    Event<const CommandInvocation&> invoked_;
};

} // namespace gui_forms
