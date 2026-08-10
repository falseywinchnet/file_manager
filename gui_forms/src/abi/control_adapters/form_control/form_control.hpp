#pragma once

#include "../support/abi_control_adapter_support.hpp"

namespace gui_forms::abi::detail {

// A form is a retained panel plus a preview seam for focus-scope commands.
// Keeping this in the ABI adapter lets the portable Window route keys once,
// before the focused child, without teaching the core about WinForms dialog
// buttons or managed callback types.
class FormControl final : public gui_forms::Panel {
public:
    ~FormControl() override;

    explicit FormControl(StableId stable_id)
        : Panel(std::move(stable_id)) {}

    [[nodiscard]] gui_forms::Event<RasterKeySample&>& key_preview() noexcept {
        return key_preview_;
    }

    void on_key_preview(gui_forms::KeyEvent& event) override {
        RasterKeySample sample{
            event.action == gui_forms::KeyAction::down
                ? GF_EVENT_KEY_DOWN : GF_EVENT_KEY_UP,
            event.physical_key,
            static_cast<std::uint32_t>(event.modifiers),
            event.repeat,
            false,
        };
        key_preview_.emit(sample);
        event.handled = sample.handled;
    }

private:
    gui_forms::Event<RasterKeySample&> key_preview_;
};

} // namespace gui_forms::abi::detail

