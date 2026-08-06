#pragma once

#include "gui_forms/basic_controls.hpp"
#include "gui_forms/component.hpp"
#include "gui_forms/event.hpp"

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>

namespace gui_forms {

struct CommandState final {
    std::string text;
    std::string description;
    std::string icon_id;
    std::string shortcut;
    std::string mnemonic;
    std::string key_tip;
    std::string availability_reason;
    bool enabled{true};
    bool visible{true};
    bool checked{};
    bool default_action{};
    bool destructive{};
    std::uint64_t generation{};
};

struct CommandInvocation final {
    std::string command_id;
    std::string source_id;
    std::uint64_t sequence{};
};

// Shared command authority for ribbon, menu, shortcut, context, semantic, and
// test presentations. A presentation requests execute; the command publishes
// one ordered invocation only when enabled.
class Command final : public Component {
public:
    Command(std::string stable_id, std::string text);

    [[nodiscard]] const std::string& stable_id() const noexcept { return stable_id_; }
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

struct CommandBindingOptions final {
    bool synchronize_text{true};
    bool synchronize_visibility{true};
    bool synchronize_accessible_description{true};
};

// RAII binding between one shared command and one ButtonBase presentation.
// Destroying the binding disconnects both directions deterministically.
class CommandBinding final {
public:
    CommandBinding(std::shared_ptr<Command> command,
                   std::shared_ptr<ButtonBase> button,
                   CommandBindingOptions options = {});
    ~CommandBinding() = default;
    CommandBinding(CommandBinding&&) noexcept = default;
    CommandBinding& operator=(CommandBinding&&) noexcept = default;
    CommandBinding(const CommandBinding&) = delete;
    CommandBinding& operator=(const CommandBinding&) = delete;

    [[nodiscard]] const std::shared_ptr<Command>& command() const noexcept {
        return command_;
    }
    [[nodiscard]] const std::shared_ptr<ButtonBase>& button() const noexcept {
        return button_;
    }

private:
    void apply(const CommandState& state);

    std::shared_ptr<Command> command_;
    std::shared_ptr<ButtonBase> button_;
    CommandBindingOptions options_;
    SubscriptionToken click_;
    SubscriptionToken state_;
};

} // namespace gui_forms
