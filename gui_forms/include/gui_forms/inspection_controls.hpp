#pragma once

#include "gui_forms/input_controls.hpp"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace gui_forms {

enum class PropertyEditorKind : std::uint8_t {
    read_only,
    text,
    choice,
};

struct PropertyRowSpec final {
    std::string stable_id;
    std::string name;
    std::string value;
    std::string description;
    PropertyEditorKind editor{PropertyEditorKind::read_only};
    std::vector<std::string> choices;
    std::string validation_message;
    bool enabled{true};
    bool required{};
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
    bool set_validation(std::string_view row_id, std::string message);
    bool set_group_expanded(std::string_view group_id, bool expanded);
    [[nodiscard]] std::optional<std::string> value(std::string_view row_id) const;
    [[nodiscard]] Control::Ptr editor(std::string_view row_id) const;
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
};

} // namespace gui_forms
