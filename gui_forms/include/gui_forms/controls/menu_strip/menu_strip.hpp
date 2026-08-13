#pragma once

#include "gui_forms/commands.hpp"
#include "gui_forms/component.hpp"
#include "gui_forms/control.hpp"
#include "gui_forms/event.hpp"

#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "gui_forms/components/context_menu/context_menu.hpp"

namespace gui_forms {
struct MenuStripItemSpec final {
    std::string stable_id;
    std::string text;
    std::vector<MenuItemSpec> items;
    bool enabled{true};
    bool visible{true};
};

struct MenuStripInvocation final {
    std::string menu_strip_id;
    std::string top_level_id;
    MenuItemInvocation item;
};

// Retained top-level application menu. The bar stays in ordinary layout; its
// active menu is presented by ContextMenu as a separate window popup root.
// Pointer, keyboard, and semantic activation converge on the same shared
// Command instances contained by MenuItemSpec.
class MenuStrip final : public Control {
public:
    explicit MenuStrip(StableId stable_id);
    ~MenuStrip() override;

    [[nodiscard]] const std::vector<MenuStripItemSpec>& items() const noexcept {
        return items_;
    }
    void set_items(std::vector<MenuStripItemSpec> items);
    [[nodiscard]] bool use_mnemonic() const noexcept { return use_mnemonic_; }
    void set_use_mnemonic(bool value);
    [[nodiscard]] double item_padding() const noexcept { return item_padding_; }
    void set_item_padding(double padding);
    [[nodiscard]] std::optional<std::size_t> active_index() const noexcept {
        return active_index_;
    }
    [[nodiscard]] std::string_view selected_item_id() const noexcept {
        return selected_item_id_;
    }
    void set_selected_item_id(std::string_view stable_id);
    bool open(std::size_t index);
    void close() noexcept;
    [[nodiscard]] bool is_open() const noexcept;

    [[nodiscard]] Event<const MenuStripInvocation&>& item_invoked() noexcept {
        return item_invoked_;
    }
    [[nodiscard]] Event<std::optional<std::size_t>>& open_changed() noexcept {
        return open_changed_;
    }

    [[nodiscard]] Size measure(Size available) override;
    void on_paint(Painter& painter, Rect local_damage) override;
    void on_pointer(PointerEvent& event) override;
    void on_key(KeyEvent& event) override;
    void on_focus_changed(bool focused) override;
    [[nodiscard]] SemanticDescriptor semantic_descriptor() const override;
    [[nodiscard]] std::vector<SemanticNode> semantic_virtual_children() const override;
    bool on_semantic_child_action(std::string_view stable_id,
                                  SemanticAction action,
                                  std::string_view value) override;

protected:
    void on_dispose() noexcept override;
    [[nodiscard]] bool mnemonic_matches(
        char32_t character) const noexcept override;
    bool process_mnemonic_self(char32_t character) override;

private:
    [[nodiscard]] std::vector<Rect> item_bounds() const;
    [[nodiscard]] std::optional<std::size_t> index_at(Point window_point) const;
    [[nodiscard]] std::optional<std::size_t> next_enabled(
        std::size_t start, int direction) const;
    bool navigate_root(int direction);
    bool handle_popup_pointer(const PointerEvent& event);
    void on_popup_item_invoked(const MenuItemInvocation& invocation);
    void on_popup_open_changed(bool open_state);
    void set_hot(std::optional<std::size_t> index);

    std::vector<MenuStripItemSpec> items_;
    std::unique_ptr<ContextMenu> popup_;
    std::optional<std::size_t> active_index_;
    std::optional<std::size_t> hot_index_;
    std::string selected_item_id_;
    bool focused_{};
    bool switching_{};
    bool use_mnemonic_{true};
    double item_padding_{11.0};
    SubscriptionToken popup_invoked_;
    SubscriptionToken popup_changed_;
    Event<const MenuStripInvocation&> item_invoked_;
    Event<std::optional<std::size_t>> open_changed_;
};

} // namespace gui_forms
