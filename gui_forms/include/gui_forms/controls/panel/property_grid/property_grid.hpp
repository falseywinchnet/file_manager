#pragma once

#include "gui_forms/controls/panel/property_list/property_list.hpp"
#include "gui_forms/inspection/property_editor_registry/property_editor_registry.hpp"
#include "gui_forms/inspection/property_value_converter_registry/property_value_converter_registry.hpp"

#include <memory>
#include <optional>
#include <string_view>
#include <vector>

namespace gui_forms {

class PropertyGrid final : public Panel {
public:
    static constexpr bool initialize_tree_after_construction = true;
    explicit PropertyGrid(StableId stable_id);
    ~PropertyGrid() override;
    void initialize_control_tree();

    [[nodiscard]] Control::Ptr selected_object() const noexcept;
    void set_selected_object(Control::Ptr object);
    [[nodiscard]] std::vector<Control::Ptr> selected_objects() const;
    void set_selected_objects(std::vector<Control::Ptr> objects);
    [[nodiscard]] PropertySort property_sort() const noexcept;
    void set_property_sort(PropertySort sort);
    void refresh_properties();

    [[nodiscard]] std::shared_ptr<PropertyList> property_list() const noexcept;
    [[nodiscard]] std::shared_ptr<PropertyValueConverterRegistry>
    converter_registry() const noexcept;
    void set_converter_registry(
        std::shared_ptr<PropertyValueConverterRegistry> registry);
    [[nodiscard]] std::shared_ptr<PropertyEditorRegistry>
    editor_registry() const noexcept;
    void set_editor_registry(std::shared_ptr<PropertyEditorRegistry> registry);
    [[nodiscard]] Control::Ptr editor(std::string_view property_name) const;
    [[nodiscard]] std::shared_ptr<Button> reset_button(
        std::string_view property_name) const;
    [[nodiscard]] std::optional<PropertyDescriptor> selected_descriptor(
        std::string_view property_name) const;
    [[nodiscard]] std::optional<PropertyValueOrigin> selected_origin(
        std::string_view property_name) const;
    bool set_property_expanded(std::string_view property_name, bool expanded);
    [[nodiscard]] std::optional<bool> property_expanded(
        std::string_view property_name) const;

    bool try_set_property_value(std::string_view property_name,
                                BindingValue value);
    bool try_set_property_text(std::string_view property_name,
                               std::string_view text);
    bool activate_property_editor(std::string_view property_name);
    bool reset_property(std::string_view property_name);
    bool insert_collection_item(std::string_view property_name,
                                std::size_t index, BindingValue value);
    bool remove_collection_item(std::string_view property_name,
                                std::size_t index);
    bool move_collection_item(std::string_view property_name,
                              std::size_t from, std::size_t to);
    [[nodiscard]] std::optional<PropertyGridEditError> last_error() const;

    [[nodiscard]] Event<Control::Ptr>& selected_object_changed() noexcept {
        return selected_object_changed_;
    }
    [[nodiscard]] Event<const PropertyGridValueChange&>&
    property_value_changed() noexcept { return property_value_changed_; }
    [[nodiscard]] Event<const PropertyGridEditError&>& edit_failed() noexcept {
        return edit_failed_;
    }

    [[nodiscard]] Size measure(Size available) override;
    void arrange(Rect final_bounds) override;
    [[nodiscard]] SemanticDescriptor semantic_descriptor() const override;

protected:
    void on_dispose() noexcept override;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
    Event<Control::Ptr> selected_object_changed_;
    Event<const PropertyGridValueChange&> property_value_changed_;
    Event<const PropertyGridEditError&> edit_failed_;
};

} // namespace gui_forms
