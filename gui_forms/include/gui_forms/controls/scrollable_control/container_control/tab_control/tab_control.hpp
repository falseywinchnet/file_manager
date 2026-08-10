#pragma once

#include "gui_forms/controls/scrollable_control/container_control/container_control.hpp"
#include "gui_forms/controls/scrollable_control/container_control/tab_control/tab_page/tab_page.hpp"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace gui_forms {

enum class TabAlignment : std::uint8_t {
    top,
    bottom,
    left,
    right,
};

enum class TabAppearance : std::uint8_t {
    normal,
    buttons,
    flat_buttons,
};

struct TabSelectionChange final {
    std::optional<std::size_t> old_index;
    std::optional<std::size_t> new_index;
};

class TabControl final : public ContainerControl {
public:
    explicit TabControl(StableId stable_id);

    void add_page(std::shared_ptr<TabPage> page);
    [[nodiscard]] std::shared_ptr<TabPage> remove_page(const TabPage& page);
    [[nodiscard]] std::vector<std::shared_ptr<TabPage>> pages() const;
    [[nodiscard]] std::size_t page_count() const;
    [[nodiscard]] std::shared_ptr<TabPage> page_at(std::size_t index) const;

    [[nodiscard]] std::optional<std::size_t> selected_index() const;
    [[nodiscard]] std::shared_ptr<TabPage> selected_tab() const noexcept {
        return selected_page_.lock();
    }
    void set_selected_index(std::size_t index);
    void set_selected_tab(const std::shared_ptr<TabPage>& page);
    [[nodiscard]] TabAlignment alignment() const noexcept { return alignment_; }
    void set_alignment(TabAlignment alignment);
    [[nodiscard]] TabAppearance appearance() const noexcept { return appearance_; }
    void set_appearance(TabAppearance appearance);
    [[nodiscard]] Size item_size() const noexcept { return item_size_; }
    void set_item_size(Size size);
    [[nodiscard]] const BasicControlStyle& style() const noexcept { return style_; }
    void set_style(BasicControlStyle style);
    [[nodiscard]] Rect tab_bounds(std::size_t index) const;
    [[nodiscard]] Rect display_bounds() const noexcept;
    [[nodiscard]] Event<const TabSelectionChange&>& selected_index_changed() noexcept {
        return selected_index_changed_;
    }

    [[nodiscard]] Size measure(Size available) override;
    void arrange(Rect final_bounds) override;
    void on_paint(Painter& painter, Rect local_damage) override;
    void on_pointer(PointerEvent& event) override;
    void on_key_preview(KeyEvent& event) override;
    void on_focus_changed(bool focused) override;
    [[nodiscard]] SemanticDescriptor semantic_descriptor() const override;
    [[nodiscard]] std::vector<SemanticNode> semantic_virtual_children() const override;
    bool on_semantic_child_action(std::string_view stable_id,
                                  SemanticAction action,
                                  std::string_view value) override;

private:
    [[nodiscard]] std::optional<std::size_t> index_of(
        const std::shared_ptr<TabPage>& page) const;
    void select_relative(int delta);
    void remember_page_focus(const std::shared_ptr<TabPage>& page);
    void restore_page_focus(const std::shared_ptr<TabPage>& page,
                            bool selection_owned_focus);
    void reconcile_pages();

    std::vector<std::weak_ptr<TabPage>> pages_;
    std::weak_ptr<TabPage> selected_page_;
    std::unordered_map<std::uint64_t, Control::WeakPtr> remembered_focus_;
    BasicControlStyle style_;
    Size item_size_{120.0, 30.0};
    FontSpec font_{FontRole::control, 11.0, 600, false, 0.24};
    TabAlignment alignment_{TabAlignment::top};
    TabAppearance appearance_{TabAppearance::normal};
    bool pointer_engaged_{};
    bool focused_{};
    Event<const TabSelectionChange&> selected_index_changed_;
};

} // namespace gui_forms
