#pragma once

#include "gui_forms/input_controls.hpp"
#include "gui_forms/instrument/instrument_types.hpp"

#include <memory>
#include <optional>
#include <string_view>
#include <vector>

namespace gui_forms {

// A bounded retained rack for compact editing instruments.
class InstrumentRack final : public Panel {
public:
    explicit InstrumentRack(StableId stable_id);
    ~InstrumentRack() override;

    [[nodiscard]] const std::vector<InstrumentModuleSpec>& modules() const noexcept;
    void set_modules(std::vector<InstrumentModuleSpec> modules);
    [[nodiscard]] Control::Ptr field_editor(std::string_view module_id,
                                            std::string_view field_id) const;
    [[nodiscard]] std::optional<Rect> module_bounds(
        std::string_view module_id) const noexcept;

    bool set_module_enabled(std::string_view module_id, bool enabled);
    bool set_module_state(std::string_view module_id,
                          InstrumentModuleState state,
                          std::string status_text);
    bool set_field_value(std::string_view module_id,
                         std::string_view field_id,
                         std::string value);
    bool set_field_validation(std::string_view module_id,
                              std::string_view field_id,
                              std::string message);

    void set_action_content(Control::Ptr content,
                            double minimum_width = 170.0);
    [[nodiscard]] Control::Ptr action_content() const noexcept;

    [[nodiscard]] double module_width() const noexcept;
    void set_module_width(double width);
    [[nodiscard]] double module_height() const noexcept;
    void set_module_height(double height);
    [[nodiscard]] double rack_gap() const noexcept;
    void set_rack_gap(double gap);
    [[nodiscard]] double content_height() const noexcept;
    [[nodiscard]] double preferred_height(double available_width) const;
    [[nodiscard]] double scroll_offset() const noexcept;
    void set_scroll_offset(double offset);

    [[nodiscard]] Event<const InstrumentFieldChange&>& field_changed() noexcept {
        return field_changed_;
    }
    [[nodiscard]] Event<const InstrumentFieldChange&>& field_committed() noexcept {
        return field_committed_;
    }
    [[nodiscard]] Event<const InstrumentModuleToggle&>& module_toggled() noexcept {
        return module_toggled_;
    }
    [[nodiscard]] Event<const InstrumentModuleRequest&>& remove_requested() noexcept {
        return remove_requested_;
    }
    [[nodiscard]] Event<const InstrumentModuleMoveRequest&>&
    move_requested() noexcept { return move_requested_; }

    [[nodiscard]] Size measure(Size available) override;
    void arrange(Rect final_bounds) override;
    void on_pointer(PointerEvent& event) override;
    void on_pointer_bubble(PointerEvent& event) override;
    void on_key_preview(KeyEvent& event) override;
    [[nodiscard]] SemanticDescriptor semantic_descriptor() const override;

protected:
    void on_dispose() noexcept override;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
    Event<const InstrumentFieldChange&> field_changed_;
    Event<const InstrumentFieldChange&> field_committed_;
    Event<const InstrumentModuleToggle&> module_toggled_;
    Event<const InstrumentModuleRequest&> remove_requested_;
    Event<const InstrumentModuleMoveRequest&> move_requested_;
};

} // namespace gui_forms
