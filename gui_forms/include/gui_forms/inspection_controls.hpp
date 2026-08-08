#pragma once

#include "gui_forms/input_controls.hpp"

#include <cstddef>
#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace gui_forms {

enum class PropertyEditorKind : std::uint8_t {
    read_only,
    text,
    choice,
    boolean,
    custom,
};

// Instance-owned formatting context. It deliberately does not mutate the C or
// process locale: two inspectors may display/edit different cultures in the
// same deterministic headless run.
struct PropertyConversionContext final {
    std::string culture_name{"invariant"};
    std::string decimal_separator{"."};
    std::string group_separator{","};
    bool use_grouping{};
    friend bool operator==(const PropertyConversionContext&,
                           const PropertyConversionContext&) = default;
};

// Renderer-neutral TypeConverter analogue. Formatting and parsing remain
// separate so an inspector can show a useful representation without implying
// that the value is editable. Registries are instance-owned and deterministic;
// there is no process-global mutable type-descriptor table.
struct PropertyValueConverter final {
    using Formatter = std::function<std::string(
        const BindingValue&, const PropertyDescriptor&)>;
    using Parser = std::function<std::optional<BindingValue>(
        std::string_view, const BindingValue&, const PropertyDescriptor&)>;
    using ContextFormatter = std::function<std::string(
        const BindingValue&, const PropertyDescriptor&,
        const PropertyConversionContext&)>;
    using ContextParser = std::function<std::optional<BindingValue>(
        std::string_view, const BindingValue&, const PropertyDescriptor&,
        const PropertyConversionContext&)>;

    Formatter format;
    Parser parse;
    ContextFormatter format_with_context;
    ContextParser parse_with_context;
};

class PropertyValueConverterRegistry final {
public:
    bool register_converter(std::string name, PropertyValueConverter converter);
    bool unregister_converter(std::string_view name);
    void map_kind(BindingValueKind kind, std::string converter_name);
    void clear_kind(BindingValueKind kind);
    [[nodiscard]] std::optional<std::string> converter_for(
        BindingValueKind kind) const;
    [[nodiscard]] const PropertyValueConverter* find(
        std::string_view name) const noexcept;
    [[nodiscard]] std::string format(const BindingValue& value,
                                     const PropertyDescriptor& descriptor) const;
    [[nodiscard]] std::optional<BindingValue> parse(
        std::string_view text, const BindingValue& current,
        const PropertyDescriptor& descriptor) const;
    [[nodiscard]] const PropertyConversionContext& context() const noexcept {
        return context_;
    }
    void set_context(PropertyConversionContext context);
    [[nodiscard]] static std::shared_ptr<PropertyValueConverterRegistry>
    create_default();

private:
    std::map<std::string, PropertyValueConverter> converters_;
    std::map<BindingValueKind, std::string> kind_mappings_;
    PropertyConversionContext context_;
};

struct PropertyEditorRequest final {
    std::string stable_id;
    std::string property_path;
    PropertyDescriptor descriptor;
    BindingValue value;
    bool writable{};
    bool top_level{};
};

struct PropertyEditorInputError final {
    std::string attempted_value;
    std::string message;
};

// Compact retained editor for a finite flags enum. The popup uses the same
// Window popup/focus-scope ownership as ComboBox while CheckedListBox permits
// multiple independent bit choices without converting the value through text.
class FlagsValueEditor final : public Panel {
public:
    FlagsValueEditor(StableId stable_id, PropertyEnumDescriptor descriptor,
                     PropertyEnumValue value);

    [[nodiscard]] const PropertyEnumDescriptor& descriptor() const noexcept {
        return descriptor_;
    }
    void set_descriptor(PropertyEnumDescriptor descriptor);
    [[nodiscard]] const PropertyEnumValue& value() const noexcept {
        return value_;
    }
    void set_value(PropertyEnumValue value);
    [[nodiscard]] bool dropped_down() const noexcept { return dropped_down_; }
    void set_dropped_down(bool dropped_down);
    [[nodiscard]] Event<const PropertyEnumValue&>& value_changed() noexcept {
        return value_changed_;
    }
    [[nodiscard]] Event<bool>& drop_down_changed() noexcept {
        return drop_down_changed_;
    }

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
    void open_drop_down();
    void close_drop_down();
    void on_popup_revoked();
    void synchronize_popup();
    void apply_popup_choice(std::size_t popup_index, CheckState state);

    PropertyEnumDescriptor descriptor_;
    PropertyEnumValue value_;
    std::shared_ptr<Panel> popup_layer_;
    std::shared_ptr<CheckedListBox> popup_list_;
    std::vector<std::size_t> popup_choice_indices_;
    std::uint64_t popup_scope_{};
    PopupToken popup_token_;
    SubscriptionToken popup_check_;
    SubscriptionToken popup_dismissal_;
    SubscriptionToken popup_revocation_;
    bool dropped_down_{};
    bool focused_{};
    bool synchronizing_popup_{};
    bool closing_popup_{};
    Event<const PropertyEnumValue&> value_changed_;
    Event<bool> drop_down_changed_;
};

// Retained color property editor. The text child owns ordinary selection,
// keyboard, clipboard, commit and cancellation behavior; this parent adds a
// checker-backed swatch, canonical #RRGGBBAA conversion and invalid feedback.
class ColorValueEditor final : public Panel {
public:
    explicit ColorValueEditor(StableId stable_id, Color value = {});
    void initialize_control_tree();

    [[nodiscard]] Color value() const noexcept { return value_; }
    void set_value(Color value);
    [[nodiscard]] std::shared_ptr<TextBox> editor() const noexcept {
        return editor_;
    }
    [[nodiscard]] Event<Color>& value_changed() noexcept {
        return value_changed_;
    }
    [[nodiscard]] Event<const PropertyEditorInputError&>& edit_failed() noexcept {
        return edit_failed_;
    }
    [[nodiscard]] static std::string format_value(Color value);
    [[nodiscard]] static std::optional<Color> parse_value(
        std::string_view text);

    [[nodiscard]] Size measure(Size available) override;
    void arrange(Rect final_bounds) override;
    void on_paint(Painter& painter, Rect local_damage) override;
    [[nodiscard]] SemanticDescriptor semantic_descriptor() const override;

protected:
    void on_dispose() noexcept override;

private:
    void commit(std::string_view text);
    void cancel();

    Color value_{};
    std::shared_ptr<TextBox> editor_;
    SubscriptionToken committed_;
    SubscriptionToken cancelled_;
    bool synchronizing_{};
    Event<Color> value_changed_;
    Event<const PropertyEditorInputError&> edit_failed_;
};

// The factory creates one ordinary retained control. PropertyList owns that
// control after installation; synchronize must not emit a user commit, while
// connect_committed returns a tokenized subscription owned by PropertyGrid.
struct PropertyEditorBinding final {
    Control::Ptr control;
    std::function<void(const BindingValue&)> synchronize;
    std::function<SubscriptionToken(
        Component&, std::function<void(BindingValue)>)> connect_committed;
    std::function<SubscriptionToken(
        Component&, std::function<void(const PropertyEditorInputError&)>)>
        connect_failed;
};

using PropertyEditorFactory = std::function<std::optional<PropertyEditorBinding>(
    const PropertyEditorRequest&)>;

class PropertyEditorRegistry final {
public:
    bool register_factory(std::string name, PropertyEditorFactory factory);
    bool unregister_factory(std::string_view name);
    void map_kind(BindingValueKind kind, std::string factory_name);
    void clear_kind(BindingValueKind kind);
    [[nodiscard]] std::optional<std::string> factory_for(
        BindingValueKind kind) const;
    [[nodiscard]] std::optional<PropertyEditorBinding> create(
        const PropertyEditorRequest& request) const;
    [[nodiscard]] static std::shared_ptr<PropertyEditorRegistry> create_default();

private:
    std::map<std::string, PropertyEditorFactory> factories_;
    std::map<BindingValueKind, std::string> kind_mappings_;
};

struct PropertyRowSpec final {
    PropertyRowSpec() = default;
    PropertyRowSpec(std::string authored_stable_id,
                    std::string authored_name,
                    std::string authored_value,
                    std::string authored_description = {},
                    PropertyEditorKind authored_editor =
                        PropertyEditorKind::read_only,
                    std::vector<std::string> authored_choices = {},
                    std::string authored_validation = {},
                    bool authored_enabled = true,
                    bool authored_required = false,
                    std::string authored_parent_id = {},
                    std::uint8_t authored_depth = 0,
                    bool authored_expandable = false,
                    bool authored_expanded = false,
                    bool authored_resettable = false,
                    bool authored_reset_enabled = false)
        : stable_id(std::move(authored_stable_id)),
          name(std::move(authored_name)), value(std::move(authored_value)),
          description(std::move(authored_description)),
          editor(authored_editor), choices(std::move(authored_choices)),
          validation_message(std::move(authored_validation)),
          parent_id(std::move(authored_parent_id)),
          depth(authored_depth), enabled(authored_enabled),
          required(authored_required), expandable(authored_expandable),
          expanded(authored_expanded), resettable(authored_resettable),
          reset_enabled(authored_reset_enabled) {}

    std::string stable_id;
    std::string name;
    std::string value;
    std::string description;
    PropertyEditorKind editor{PropertyEditorKind::read_only};
    std::vector<std::string> choices;
    std::string validation_message;
    std::string parent_id;
    std::uint8_t depth{};
    bool enabled{true};
    bool required{};
    bool expandable{};
    bool expanded{};
    bool resettable{};
    bool reset_enabled{};
};

struct PropertyGroupSpec final {
    std::string stable_id;
    std::string title;
    std::vector<PropertyRowSpec> rows;
    bool expanded{true};
};

struct PropertyValueChange final {
    std::string row_id;
    std::string previous_value;
    std::string current_value;
    bool committed{};
};

struct PropertyGroupChange final {
    std::string group_id;
    bool expanded{};
};

struct PropertyRowExpansionChange final {
    std::string row_id;
    bool expanded{};
};

struct PropertyResetRequest final {
    std::string row_id;
};

// Lightweight retained property/settings surface. Group and row identities
// remain stable while stock text/choice editors are owned as ordinary child
// controls. The PropertyList owns the only vertical scroll plane; consumers do
// not nest a second scroller around preview and property content.
class PropertyList final : public Panel {
public:
    explicit PropertyList(StableId stable_id);
    ~PropertyList() override;

    [[nodiscard]] const std::vector<PropertyGroupSpec>& groups() const noexcept;
    void set_groups(std::vector<PropertyGroupSpec> groups);
    bool set_value(std::string_view row_id, std::string value);
    bool set_description(std::string_view row_id, std::string description);
    bool set_validation(std::string_view row_id, std::string message);
    bool set_reset_enabled(std::string_view row_id, bool enabled);
    bool set_group_expanded(std::string_view group_id, bool expanded);
    bool set_row_expanded(std::string_view row_id, bool expanded);
    [[nodiscard]] std::optional<bool> row_expanded(
        std::string_view row_id) const;
    [[nodiscard]] std::optional<std::string> value(std::string_view row_id) const;
    [[nodiscard]] Control::Ptr editor(std::string_view row_id) const;
    // Replaces a stock row editor with an ordinary retained control. The list
    // assumes ownership only after validation succeeds and preserves the row's
    // existing layout, focus-reveal, disposal, and semantic participation.
    bool replace_editor(std::string_view row_id, Control::Ptr editor);
    [[nodiscard]] std::shared_ptr<Button> reset_button(
        std::string_view row_id) const;
    // Optional preview/summary content participates in this same scroll plane.
    // PropertyList assumes ownership after a successful assignment.
    void set_header_content(Control::Ptr content, double height);
    [[nodiscard]] Control::Ptr header_content() const noexcept;
    [[nodiscard]] double header_height() const noexcept;
    void set_header_height(double height);

    [[nodiscard]] double label_width() const noexcept;
    void set_label_width(double width);
    [[nodiscard]] double scroll_offset() const noexcept;
    void set_scroll_offset(double offset);
    [[nodiscard]] double content_height() const noexcept;

    [[nodiscard]] Event<const PropertyValueChange&>& value_changed() noexcept {
        return value_changed_;
    }
    [[nodiscard]] Event<const PropertyValueChange&>& value_committed() noexcept {
        return value_committed_;
    }
    [[nodiscard]] Event<const PropertyGroupChange&>& group_changed() noexcept {
        return group_changed_;
    }
    [[nodiscard]] Event<const PropertyRowExpansionChange&>&
    row_expansion_changed() noexcept {
        return row_expansion_changed_;
    }
    [[nodiscard]] Event<const PropertyResetRequest&>& reset_requested() noexcept {
        return reset_requested_;
    }

    [[nodiscard]] Size measure(Size available) override;
    void arrange(Rect final_bounds) override;
    void on_paint(Painter& painter, Rect local_damage) override;
    void on_pointer(PointerEvent& event) override;
    void on_key(KeyEvent& event) override;
    [[nodiscard]] SemanticDescriptor semantic_descriptor() const override;
    [[nodiscard]] std::vector<SemanticNode> semantic_virtual_children() const override;
    bool on_semantic_child_action(std::string_view stable_id,
                                  SemanticAction action,
                                  std::string_view value) override;

protected:
    void on_dispose() noexcept override;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
    Event<const PropertyValueChange&> value_changed_;
    Event<const PropertyValueChange&> value_committed_;
    Event<const PropertyGroupChange&> group_changed_;
    Event<const PropertyRowExpansionChange&> row_expansion_changed_;
    Event<const PropertyResetRequest&> reset_requested_;
};

enum class PropertySort : std::uint8_t {
    categorized,
    alphabetical,
};

struct PropertyGridValueChange final {
    std::string property_name;
    BindingValue previous_value;
    BindingValue current_value;
    PropertyValueOrigin origin{PropertyValueOrigin::computed};
    bool reset{};
};

struct PropertyGridEditError final {
    std::string property_name;
    std::string attempted_value;
    std::string message;
};

// Metadata-driven property inspector/editor. PropertyList remains the general
// authored settings surface; PropertyGrid projects a selected Control's inert
// descriptors through stock editors and the same executable registrations
// used by binding and future DML. Compound values and bounded immutable
// objects/collections expand through typed member/index paths; unsupported
// value families remain visibly read-only rather than round-tripping a
// diagnostic string as authored data.
class PropertyGrid final : public Panel {
public:
    explicit PropertyGrid(StableId stable_id);
    ~PropertyGrid() override;
    void initialize_control_tree();

    [[nodiscard]] Control::Ptr selected_object() const noexcept;
    void set_selected_object(Control::Ptr object);
    [[nodiscard]] std::vector<Control::Ptr> selected_objects() const;
    // Projects the common schema and applies edits as one transaction. If any
    // owner rejects the value, every previously changed owner is restored
    // before failure is published.
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
    // Automation/binding façades may submit culture-formatted text through the
    // same instance-owned converter used by the retained editor.
    bool try_set_property_text(std::string_view property_name,
                               std::string_view text);
    // Invokes the installed retained editor through its semantic Press action.
    // This is shared by accessibility, ABI automation, and physical pointer
    // activation; it never bypasses the editor's normal commit transaction.
    bool activate_property_editor(std::string_view property_name);
    bool reset_property(std::string_view property_name);
    // Collection mutations rebuild the immutable value tree and commit it
    // through the owning property's registered setter. Index paths use the
    // same stable syntax exposed by editor(), for example Items[2].
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
    property_value_changed() noexcept {
        return property_value_changed_;
    }
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
