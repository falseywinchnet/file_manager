#pragma once

#include "gui_forms/controls/panel/list_box/list_box.hpp"
#include "gui_forms/window.hpp"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace gui_forms {

class ComboBox final : public Panel {
public:
    explicit ComboBox(StableId stable_id);

    [[nodiscard]] std::span<const std::string> items() const noexcept {
        return items_;
    }
    void set_items(std::vector<std::string> items);
    void add_item(std::string item);
    [[nodiscard]] std::optional<std::size_t> selected_index() const noexcept {
        return selected_index_;
    }
    void set_selected_index(std::optional<std::size_t> index);
    [[nodiscard]] std::string_view selected_text() const noexcept;
    [[nodiscard]] std::string_view placeholder_text() const noexcept {
        return placeholder_;
    }
    void set_placeholder_text(std::string text);
    [[nodiscard]] bool dropped_down() const noexcept { return dropped_down_; }
    void set_dropped_down(bool dropped_down);
    [[nodiscard]] std::size_t maximum_drop_down_items() const noexcept {
        return maximum_drop_down_items_;
    }
    void set_maximum_drop_down_items(std::size_t count);
    // Zero tracks the control width. A positive value supplies the desired
    // popup width while host placement still constrains it to the work area.
    [[nodiscard]] double drop_down_width() const noexcept {
        return drop_down_width_;
    }
    void set_drop_down_width(double width);
    [[nodiscard]] FontSpec font() const noexcept { return font_; }
    void set_font(FontSpec font);

    [[nodiscard]] Event<std::optional<std::size_t>>& selected_index_changed() noexcept {
        return selected_index_changed_;
    }
    [[nodiscard]] Event<bool>& drop_down_changed() noexcept {
        return drop_down_changed_;
    }
    [[nodiscard]] Event<>& items_changed() noexcept { return items_changed_; }

    void on_paint(Painter& painter, Rect local_damage) override;
    void on_pointer(PointerEvent& event) override;
    void on_key(KeyEvent& event) override;
    void on_focus_changed(bool focused) override;
    [[nodiscard]] SemanticDescriptor semantic_descriptor() const override;
    bool on_semantic_action(SemanticAction action,
                            std::string_view value) override;

protected:
    void on_detached_from_window() noexcept override;

private:
    [[nodiscard]] BindingValue items_property_value() const;
    void set_items_property(const BindingValue& value);
    void reset_items_property();
    [[nodiscard]] bool should_serialize_items_property() const noexcept;
    [[nodiscard]] BindingValue selected_index_property_value() const;
    void set_selected_index_property(const BindingValue& value);
    [[nodiscard]] BindingValue text_property_value() const;
    void set_text_property(const BindingValue& value);
    void reset_text_property();
    [[nodiscard]] bool should_serialize_text_property() const noexcept;
    void popup_selection_changed(const ListSelectionChange& change);
    void open_drop_down();
    void close_drop_down();
    void commit_popup_selection(std::size_t index);
    void on_popup_revoked();

    std::vector<std::string> items_;
    std::optional<std::size_t> selected_index_;
    std::string placeholder_;
    FontSpec font_{FontRole::content, 12.0, 400, false};
    std::size_t maximum_drop_down_items_{8U};
    double drop_down_width_{};
    bool dropped_down_{};
    bool focused_{};
    std::shared_ptr<Panel> popup_layer_;
    std::shared_ptr<ListBox> popup_list_;
    std::uint64_t popup_scope_{};
    PopupToken popup_token_;
    SubscriptionToken popup_selection_;
    SubscriptionToken popup_activation_;
    SubscriptionToken popup_dismissal_;
    SubscriptionToken popup_revocation_;
    Event<std::optional<std::size_t>> selected_index_changed_;
    Event<bool> drop_down_changed_;
    Event<> items_changed_;
    bool closing_popup_{};
};

} // namespace gui_forms
