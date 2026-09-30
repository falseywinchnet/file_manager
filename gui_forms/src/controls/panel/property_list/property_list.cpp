#include "gui_forms/controls/panel/property_list/property_list.hpp"

#include "property_list_utilities.hpp"
#include "gui_forms/text.hpp"
#include "gui_forms/window.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>
#include <utility>

namespace gui_forms {

using detail::property_editor_height;
using detail::property_gap;
using detail::property_group_height;
using detail::property_padding;
using detail::property_row_height;
using detail::property_validation_height;
using detail::property_virtual_runtime_id;
using detail::require_property_text;

struct PropertyList::Impl final {
    struct RowState final {
        std::size_t group_index{};
        std::size_t row_index{};
        Control::Ptr editor;
        std::shared_ptr<Button> reset_button;
        SubscriptionToken value_subscription;
        SubscriptionToken commit_subscription;
        SubscriptionToken cancel_subscription;
        SubscriptionToken focus_subscription;
        SubscriptionToken reset_subscription;
        SubscriptionToken reset_focus_subscription;
        std::string committed_value;
        Rect row_bounds{};
        Rect name_bounds{};
        Rect value_bounds{};
        Rect reset_bounds{};
        Rect validation_bounds{};
    };
    using RowList = std::vector<RowState>;
    using GroupList = std::vector<PropertyGroupSpec>;

    struct TextValueChanged final {
        Impl* implementation{};
        std::string row_id;
        void operator()(const std::string& value) const {
            if ((*implementation).synchronizing) return;
            RowState* row = (*implementation).find_row(row_id);
            if (row == nullptr) return;
            PropertyRowSpec& model = (*implementation).spec(*row);
            const std::string previous = model.value;
            model.value = value;
            (*implementation).owner.publish_change(
                (*implementation).owner.value_changed_,
                PropertyValueChange{row_id, previous, model.value, false});
        }
    };

    struct TextCommitted final {
        Impl* implementation{};
        std::string row_id;
        void operator()(const std::string& value) const {
            RowState* row = (*implementation).find_row(row_id);
            if (row == nullptr) return;
            PropertyRowSpec& model = (*implementation).spec(*row);
            const std::string previous = (*row).committed_value;
            (*row).committed_value = value;
            model.value = value;
            if (model.required && value.empty()) {
                model.validation_message = model.name + " is required";
            }
            (*implementation).recompute_geometry();
            (*implementation).owner.value_committed_.emit(
                {row_id, previous, value, true});
            (*implementation).owner.invalidate(
                Dirty::measure | Dirty::layout | Dirty::paint |
                Dirty::semantics);
        }
    };

    struct TextCancelled final {
        Impl* implementation{};
        std::string row_id;
        void operator()() const {
            RowState* row = (*implementation).find_row(row_id);
            if (row == nullptr) return;
            (*implementation).synchronizing = true;
            const std::shared_ptr<TextBox> editor =
                std::dynamic_pointer_cast<TextBox>((*row).editor);
            if (editor) (*editor).set_text((*row).committed_value);
            (*implementation).spec(*row).value = (*row).committed_value;
            (*implementation).synchronizing = false;
            (*implementation).owner.invalidate(
                Dirty::paint | Dirty::semantics);
        }
    };

    struct ChoiceChanged final {
        Impl* implementation{};
        std::string row_id;
        std::weak_ptr<ComboBox> choice;
        void operator()(std::optional<std::size_t>) const {
            if ((*implementation).synchronizing) return;
            RowState* row = (*implementation).find_row(row_id);
            const std::shared_ptr<ComboBox> editor = choice.lock();
            if (row == nullptr || !editor) return;
            PropertyRowSpec& model = (*implementation).spec(*row);
            const std::string previous = (*row).committed_value;
            model.value = std::string((*editor).selected_text());
            (*row).committed_value = model.value;
            const PropertyValueChange change{
                row_id, previous, model.value, true};
            (*implementation).owner.publish_change(
                (*implementation).owner.value_changed_, change);
            (*implementation).owner.value_committed_.emit(change);
        }
    };

    struct CheckChanged final {
        Impl* implementation{};
        std::string row_id;
        std::weak_ptr<CheckBox> check;
        void operator()(bool checked) const {
            if ((*implementation).synchronizing) return;
            RowState* row = (*implementation).find_row(row_id);
            const std::shared_ptr<CheckBox> editor = check.lock();
            if (row == nullptr || !editor) return;
            PropertyRowSpec& model = (*implementation).spec(*row);
            const std::string previous = (*row).committed_value;
            model.value = checked ? "True" : "False";
            (*row).committed_value = model.value;
            (*editor).set_text(model.value);
            const PropertyValueChange change{
                row_id, previous, model.value, true};
            (*implementation).owner.publish_change(
                (*implementation).owner.value_changed_, change);
            (*implementation).owner.value_committed_.emit(change);
        }
    };

    struct EnsureVisible final {
        Impl* implementation{};
        std::string row_id;
        void operator()(bool focused) const {
            if (focused) (*implementation).ensure_visible(row_id);
        }
    };

    struct ResetClicked final {
        Impl* implementation{};
        std::string row_id;
        void operator()(ButtonBase&) const {
            (*implementation).owner.reset_requested_.emit({row_id});
        }
    };

    explicit Impl(PropertyList& public_owner) : owner(public_owner) {}

    [[nodiscard]] double scale() const noexcept {
        return owner.effective_text_scale();
    }

    [[nodiscard]] double line_height() const {
        return owner.resolve_text_layout_utf8(
            "Mg", owner.effective_font(font)).logical_size.height / scale();
    }

    [[nodiscard]] double group_extent() const {
        return std::max(property_group_height, line_height() + 8.0) * scale();
    }

    void apply_editor_font(const Control::Ptr& editor) const {
        if (!font_overridden || !editor) return;
        if (const std::shared_ptr<TextBox> text =
                std::dynamic_pointer_cast<TextBox>(editor)) {
            (*text).set_font(font);
        } else if (const std::shared_ptr<NumericUpDown> numeric =
                       std::dynamic_pointer_cast<NumericUpDown>(editor)) {
            const std::shared_ptr<TextBox> text = (*numeric).editor();
            if (text) (*text).set_font(font);
        } else if (const std::shared_ptr<ComboBox> choice =
                       std::dynamic_pointer_cast<ComboBox>(editor)) {
            (*choice).set_font(font);
        } else if (const std::shared_ptr<ButtonBase> button =
                       std::dynamic_pointer_cast<ButtonBase>(editor)) {
            (*button).set_font(font);
        }
    }

    PropertyRowSpec& spec(RowState& state) {
        return groups[state.group_index].rows[state.row_index];
    }
    const PropertyRowSpec& spec(const RowState& state) const {
        return groups[state.group_index].rows[state.row_index];
    }

    RowState* find_row(std::string_view id) noexcept {
        for (RowState& row : rows) {
            if (spec(row).stable_id == id) return &row;
        }
        return nullptr;
    }
    const RowState* find_row(std::string_view id) const noexcept {
        for (const RowState& row : rows) {
            if (spec(row).stable_id == id) return &row;
        }
        return nullptr;
    }

    [[nodiscard]] bool row_visible(const RowState& state) const noexcept {
        const PropertyRowSpec* current = &spec(state);
        std::size_t remaining = groups[state.group_index].rows.size();
        while (!(*current).parent_id.empty() && remaining-- > 0U) {
            const RowState* parent = find_row((*current).parent_id);
            if (!parent || (*parent).group_index != state.group_index ||
                !spec(*parent).expandable || !spec(*parent).expanded) {
                return false;
            }
            current = &spec(*parent);
        }
        return (*current).parent_id.empty();
    }

    void clear_editors() noexcept {
        for (RowState& row : rows) {
            if (row.editor && (*row.editor).parent().get() == &owner) {
                Control::Ptr removed = owner.remove_child((*row.editor).runtime_id());
                if (removed && (*removed).is_alive()) (*removed).dispose();
            }
            if (row.reset_button && (*row.reset_button).parent().get() == &owner) {
                Control::Ptr removed = owner.remove_child(
                    (*row.reset_button).runtime_id());
                if (removed && (*removed).is_alive()) (*removed).dispose();
            }
        }
        rows.clear();
    }

    void rebuild_editors() {
        clear_editors();
        for (std::size_t group_index = 0; group_index < groups.size(); ++group_index) {
            for (std::size_t row_index = 0;
                 row_index < groups[group_index].rows.size(); ++row_index) {
                PropertyRowSpec& row = groups[group_index].rows[row_index];
                RowState state;
                state.group_index = group_index;
                state.row_index = row_index;
                state.committed_value = row.value;
                if (row.editor == PropertyEditorKind::text) {
                    std::shared_ptr<gui_forms::TextBox> editor = make_control<TextBox>(
                        StableId(row.stable_id + ".editor"), row.value);
                    (*editor).set_font({FontRole::content, 10.0, 400, false});
                    (*editor).set_accessible_name(row.name);
                    (*editor).set_accessible_description(row.description);
                    (*editor).set_enabled(row.enabled);
                    state.editor = editor;
                    owner.add_child(editor);
                } else if (row.editor == PropertyEditorKind::choice) {
                    std::shared_ptr<gui_forms::ComboBox> editor = make_control<ComboBox>(
                        StableId(row.stable_id + ".editor"));
                    (*editor).set_items(row.choices);
                    const std::vector<std::string>::iterator selected =
                        std::find(row.choices.begin(),
                                                    row.choices.end(), row.value);
                    if (selected != row.choices.end()) {
                        (*editor).set_selected_index(static_cast<std::size_t>(
                            std::distance(row.choices.begin(), selected)));
                    }
                    (*editor).set_font({FontRole::content, 10.0, 400, false});
                    (*editor).set_accessible_name(row.name);
                    (*editor).set_accessible_description(row.description);
                    (*editor).set_enabled(row.enabled);
                    state.editor = editor;
                    owner.add_child(editor);
                } else if (row.editor == PropertyEditorKind::boolean) {
                    const bool checked = binding_value_to_bool(
                        BindingValue{row.value}).value_or(false);
                    std::shared_ptr<gui_forms::CheckBox> editor = make_control<CheckBox>(
                        StableId(row.stable_id + ".editor"),
                        checked ? "True" : "False");
                    (*editor).set_checked(checked);
                    (*editor).set_font({FontRole::content, 10.0, 400, false});
                    (*editor).set_accessible_name(row.name);
                    (*editor).set_accessible_description(row.description);
                    (*editor).set_enabled(row.enabled);
                    state.editor = editor;
                    owner.add_child(editor);
                }
                if (row.resettable) {
                    std::shared_ptr<gui_forms::Button> reset = make_control<Button>(
                        StableId(row.stable_id + ".reset"), "Reset");
                    (*reset).set_font({FontRole::control, 8.5, 600, false});
                    (*reset).set_enabled(row.enabled && row.reset_enabled);
                    (*reset).set_accessible_name("Reset " + row.name);
                    (*reset).set_accessible_description(
                        "Restore " + row.name + " to its declared default");
                    state.reset_button = reset;
                    owner.add_child(reset);
                }
                apply_editor_font(state.editor);
                apply_editor_font(state.reset_button);
                rows.push_back(std::move(state));
                connect_row(rows.back());
            }
        }
    }

    void connect_row(RowState& state) {
        const std::string row_id = spec(state).stable_id;
        if (const std::shared_ptr<gui_forms::TextBox> text = std::dynamic_pointer_cast<TextBox>(state.editor)) {
            state.value_subscription = (*text).text_changed().subscribe(
                owner, TextValueChanged{this, row_id});
            state.commit_subscription = (*text).committed().subscribe(
                owner, TextCommitted{this, row_id});
            state.cancel_subscription = (*text).cancelled().subscribe(
                owner, TextCancelled{this, row_id});
        } else if (const std::shared_ptr<gui_forms::ComboBox> choice =
                       std::dynamic_pointer_cast<ComboBox>(state.editor)) {
            state.value_subscription = (*choice).selected_index_changed().subscribe(
                owner, ChoiceChanged{
                    this, row_id, std::weak_ptr<ComboBox>(choice)});
        } else if (const std::shared_ptr<gui_forms::CheckBox> check =
                       std::dynamic_pointer_cast<CheckBox>(state.editor)) {
            state.value_subscription = (*check).checked_changed().subscribe(
                owner, CheckChanged{
                    this, row_id, std::weak_ptr<CheckBox>(check)});
        }
        if (state.editor) {
            state.focus_subscription = (*state.editor).focus_observed().subscribe(
                owner, EnsureVisible{this, row_id});
        }
        if (state.reset_button) {
            state.reset_subscription = (*state.reset_button).clicked().subscribe(
                owner, ResetClicked{this, row_id});
            state.reset_focus_subscription =
                (*state.reset_button).focus_observed().subscribe(
                owner, EnsureVisible{this, row_id});
        }
    }

    void recompute_geometry() {
        const double width = std::max(0.0, owner.committed_arranged_bounds().width);
        const double viewport = std::max(0.0, owner.committed_arranged_bounds().height);
        const double s = scale();
        const double line = line_height();
        const double row_extra = std::max(row_height, line + 10.0) - property_row_height;
        const double stacked_extra = std::max(21.0, line + 6.0) - 21.0;
        const double padding = property_padding * s;
        const double gap = property_gap * s;
        const double scaled_label_width = label_width * s;
        const double validation_height = std::max(property_validation_height, line + 4.0) * s;
        const bool stacked = width < std::max(230.0 * s,
                                              scaled_label_width + 116.0 * s);
        double y{header_height};
        for (std::size_t group_index = 0; group_index < groups.size(); ++group_index) {
            y += group_extent();
            if (!groups[group_index].expanded) continue;
            for (RowState& state : rows) {
                if (state.group_index != group_index) continue;
                const PropertyRowSpec& row = spec(state);
                if (!row_visible(state)) {
                    state.row_bounds = {};
                    state.name_bounds = {};
                    state.value_bounds = {};
                    state.reset_bounds = {};
                    state.validation_bounds = {};
                    continue;
                }
                const bool editable = row.editor != PropertyEditorKind::read_only;
                const bool interactive = editable || row.resettable;
                double height = ((interactive ? property_editor_height
                                              : property_row_height) + row_extra) * s;
                if (stacked) height += ((interactive ? 21.0 : 13.0) + stacked_extra) * s;
                if (!row.validation_message.empty()) height += validation_height;
                state.row_bounds = {0.0, y, width, height};
                const double indent = static_cast<double>(row.depth) * 15.0 * s +
                    (row.expandable ? 14.0 * s : 0.0);
                const double reset_width = row.resettable ? 48.0 * s : 0.0;
                const double reset_gap = row.resettable ? 5.0 * s : 0.0;
                if (stacked) {
                    state.name_bounds = {padding + indent, y + 3.0 * s,
                                         std::max(0.0, width - padding * 2.0 -
                                                           indent),
                                         (18.0 + stacked_extra) * s};
                    state.value_bounds = {padding, y + (21.0 + stacked_extra) * s,
                        std::max(0.0, width - padding * 2.0 - reset_width -
                                          reset_gap),
                        ((interactive ? 28.0 : 19.0) + row_extra) * s};
                } else {
                    state.name_bounds = {padding + indent, y + 5.0 * s,
                                         std::max(0.0, scaled_label_width - indent),
                                         (21.0 + row_extra) * s};
                    state.value_bounds = {
                        padding + scaled_label_width + gap, y + 3.0 * s,
                        std::max(0.0, width - padding * 2.0 -
                                          scaled_label_width - gap - reset_width -
                                          reset_gap),
                        ((interactive ? 28.0 : 22.0) + row_extra) * s};
                }
                state.reset_bounds = row.resettable
                    ? Rect{std::max(padding, width - padding - reset_width),
                           state.value_bounds.y, reset_width,
                           state.value_bounds.height}
                    : Rect{};
                state.validation_bounds = {
                    state.value_bounds.x,
                    y + height - validation_height,
                    state.value_bounds.width, validation_height};
                y += height;
            }
        }
        content_height = y;
        scroll_offset = std::clamp(scroll_offset, 0.0,
            std::max(0.0, content_height - viewport));
    }

    void arrange_editors() {
        if (header) {
            owner.set_child_layout(header,
                {0.0, -scroll_offset,
                 std::max(0.0, owner.committed_arranged_bounds().width),
                 header_height});
        }
        for (RowState& state : rows) {
            const bool visible = groups[state.group_index].expanded &&
                row_visible(state);
            if (state.editor) {
                (*state.editor).set_visible(visible);
            }
            if (state.reset_button) {
                (*state.reset_button).set_visible(visible);
            }
            if (visible && state.editor) {
                owner.set_child_layout(state.editor,
                    {state.value_bounds.x,
                     state.value_bounds.y - scroll_offset,
                     state.value_bounds.width, state.value_bounds.height});
            }
            if (visible && state.reset_button) {
                owner.set_child_layout(state.reset_button,
                    {state.reset_bounds.x,
                     state.reset_bounds.y - scroll_offset,
                     state.reset_bounds.width, state.reset_bounds.height});
            }
        }
    }

    void ensure_visible(std::string_view row_id) {
        RowState* row = find_row(row_id);
        if (!row) return;
        const double viewport = owner.committed_arranged_bounds().height;
        if ((*row).row_bounds.y < scroll_offset) {
            scroll_offset = (*row).row_bounds.y;
        } else if ((*row).row_bounds.y + (*row).row_bounds.height >
                   scroll_offset + viewport) {
            scroll_offset = (*row).row_bounds.y + (*row).row_bounds.height - viewport;
        }
        scroll_offset = std::clamp(scroll_offset, 0.0,
            std::max(0.0, content_height - viewport));
        arrange_editors();
        owner.invalidate(Dirty::paint | Dirty::hit_test | Dirty::semantics);
    }

    std::optional<std::size_t> group_at(Point absolute) const {
        const Rect bounds = owner.absolute_bounds();
        if (!bounds.contains(absolute)) return {};
        const double y_target = absolute.y - bounds.y + scroll_offset;
        const double group_height = group_extent();
        double y{header_height};
        for (std::size_t group_index = 0; group_index < groups.size(); ++group_index) {
            if (y_target >= y && y_target < y + group_height) {
                return group_index;
            }
            y += group_height;
            if (!groups[group_index].expanded) continue;
            for (const RowState& state : rows) {
                if (state.group_index == group_index) y += state.row_bounds.height;
            }
        }
        return {};
    }

    RowState* disclosure_at(Point absolute) noexcept {
        const Rect bounds = owner.absolute_bounds();
        if (!bounds.contains(absolute)) return nullptr;
        const Point local{absolute.x - bounds.x,
                          absolute.y - bounds.y + scroll_offset};
        for (RowState& state : rows) {
            const PropertyRowSpec& row = spec(state);
            if (!groups[state.group_index].expanded || !row_visible(state) ||
                !row.expandable || !state.row_bounds.contains(local)) {
                continue;
            }
            const double disclosure_right = state.name_bounds.x;
            const double disclosure_left = std::max(
                0.0, disclosure_right - 16.0 * scale());
            if (local.x >= disclosure_left &&
                local.x <= disclosure_right + state.name_bounds.width) {
                return &state;
            }
        }
        return nullptr;
    }

    PropertyList& owner;
    GroupList groups;
    RowList rows;
    Control::Ptr header;
    double header_height{};
    double label_width{76.0};
    FontSpec font{FontRole::content, 9.5, 400, false};
    double row_height{property_row_height};
    bool font_overridden{};
    double scroll_offset{};
    double content_height{};
    bool synchronizing{};
};

PropertyList::PropertyList(StableId stable_id)
    : Panel(std::move(stable_id)), impl_(std::make_unique<Impl>(*this)) {
    set_background(Color::rgba(244, 247, 251));
    set_border_style(BorderStyle::none);
    set_focusable(true);
    set_paint_plane(PaintPlane::control);
}

PropertyList::~PropertyList() = default;

const std::vector<PropertyGroupSpec>& PropertyList::groups() const noexcept {
    return (*impl_).groups;
}

void PropertyList::set_groups(std::vector<PropertyGroupSpec> groups) {
    require_mutable();
    std::unordered_set<std::string> identities;
    for (const gui_forms::PropertyGroupSpec& group : groups) {
        require_property_text(group.stable_id, "group ID");
        require_property_text(group.title, "group title");
        if (group.stable_id.empty() || group.title.empty() ||
            !identities.insert(group.stable_id).second) {
            throw std::invalid_argument(
                "PropertyList groups require unique nonempty identities and titles");
        }
        std::unordered_map<std::string, const PropertyRowSpec*> prior_rows;
        for (const gui_forms::PropertyRowSpec& row : group.rows) {
            require_property_text(row.stable_id, "row ID");
            require_property_text(row.name, "row name");
            require_property_text(row.value, "row value");
            require_property_text(row.description, "row description");
            require_property_text(row.validation_message, "validation message");
            if (row.stable_id.empty() || row.name.empty() ||
                !identities.insert(row.stable_id).second) {
                throw std::invalid_argument(
                    "PropertyList rows require unique nonempty identities and names");
            }
            if (row.depth > 8U ||
                (row.expanded && !row.expandable) ||
                (row.reset_enabled && !row.resettable)) {
                throw std::invalid_argument(
                    "PropertyList row hierarchy/reset state is inconsistent");
            }
            if (row.parent_id.empty()) {
                if (row.depth != 0U) {
                    throw std::invalid_argument(
                        "PropertyList root rows must have depth zero");
                }
            } else {
                const std::unordered_map<
                    std::string, const PropertyRowSpec*>::iterator parent =
                    prior_rows.find(row.parent_id);
                if (parent == prior_rows.end() ||
                    !(*(*parent).second).expandable ||
                    row.depth != (*(*parent).second).depth + 1U) {
                    throw std::invalid_argument(
                        "PropertyList child rows require an earlier expandable parent at the preceding depth");
                }
            }
            if (row.editor == PropertyEditorKind::choice && row.choices.empty()) {
                throw std::invalid_argument(
                    "PropertyList choice rows require at least one choice");
            }
            for (const std::string& choice : row.choices) {
                require_property_text(choice, "choice");
            }
            if (row.editor == PropertyEditorKind::choice && !row.value.empty() &&
                std::find(row.choices.begin(), row.choices.end(), row.value) ==
                    row.choices.end()) {
                throw std::invalid_argument(
                    "PropertyList choice value must be empty or one declared choice");
            }
            if (row.editor == PropertyEditorKind::boolean &&
                !binding_value_to_bool(BindingValue{row.value})) {
                throw std::invalid_argument(
                    "PropertyList Boolean value must be True, False, 1, or 0");
            }
            prior_rows.emplace(row.stable_id, &row);
        }
    }
    (*impl_).groups = std::move(groups);
    (*impl_).scroll_offset = 0.0;
    (*impl_).rebuild_editors();
    (*impl_).recompute_geometry();
    invalidate(Dirty::measure | Dirty::layout | Dirty::paint | Dirty::hit_test |
               Dirty::semantics);
}

bool PropertyList::set_value(std::string_view row_id, std::string value) {
    require_mutable();
    require_property_text(value, "row value");
    Impl::RowState* row = (*impl_).find_row(row_id);
    if (!row) return false;
    PropertyRowSpec& model = (*impl_).spec(*row);
    if (model.editor == PropertyEditorKind::choice && !value.empty() &&
        std::find(model.choices.begin(), model.choices.end(), value) ==
            model.choices.end()) {
        throw std::invalid_argument(
            "PropertyList choice value must be empty or one declared choice");
    }
    if (model.editor == PropertyEditorKind::boolean &&
        !binding_value_to_bool(BindingValue{value})) {
        throw std::invalid_argument(
            "PropertyList Boolean value must be True, False, 1, or 0");
    }
    if (model.value == value && (*row).committed_value == value) return true;
    (*impl_).synchronizing = true;
    model.value = value;
    (*row).committed_value = value;
    if (const std::shared_ptr<gui_forms::TextBox> text = std::dynamic_pointer_cast<TextBox>((*row).editor)) {
        (*text).set_text(value);
    } else if (const std::shared_ptr<gui_forms::ComboBox> choice = std::dynamic_pointer_cast<ComboBox>((*row).editor)) {
        const std::vector<std::string>::iterator found =
            std::find(model.choices.begin(), model.choices.end(), value);
        (*choice).set_selected_index(found == model.choices.end()
            ? std::optional<std::size_t>{}
            : std::optional<std::size_t>{static_cast<std::size_t>(
                std::distance(model.choices.begin(), found))});
    } else if (const std::shared_ptr<gui_forms::CheckBox> check =
                   std::dynamic_pointer_cast<CheckBox>((*row).editor)) {
        const bool checked = binding_value_to_bool(
            BindingValue{value}).value_or(false);
        (*check).set_checked(checked);
        (*check).set_text(checked ? "True" : "False");
    }
    (*impl_).synchronizing = false;
    invalidate(Dirty::paint | Dirty::semantics);
    return true;
}

bool PropertyList::set_description(std::string_view row_id,
                                   std::string description) {
    require_mutable();
    require_property_text(description, "row description");
    Impl::RowState* row = (*impl_).find_row(row_id);
    if (!row) return false;
    PropertyRowSpec& model = (*impl_).spec(*row);
    if (model.description == description) return true;
    model.description = std::move(description);
    if ((*row).editor) {
        std::string accessible = model.description;
        if (!model.validation_message.empty()) {
            if (!accessible.empty()) accessible += " · ";
            accessible += "Error: " + model.validation_message;
        }
        (*(*row).editor).set_accessible_description(std::move(accessible));
    }
    invalidate(Dirty::paint | Dirty::semantics | Dirty::accessibility);
    return true;
}

bool PropertyList::set_validation(std::string_view row_id, std::string message) {
    require_mutable();
    require_property_text(message, "validation message");
    Impl::RowState* row = (*impl_).find_row(row_id);
    if (!row) return false;
    PropertyRowSpec& model = (*impl_).spec(*row);
    if (model.validation_message == message) return true;
    model.validation_message = std::move(message);
    if ((*row).editor) {
        std::string description = model.description;
        if (!model.validation_message.empty()) {
            if (!description.empty()) description += " · ";
            description += "Error: " + model.validation_message;
        }
        (*(*row).editor).set_accessible_description(std::move(description));
    }
    (*impl_).recompute_geometry();
    (*impl_).arrange_editors();
    invalidate(Dirty::measure | Dirty::layout | Dirty::paint | Dirty::hit_test |
               Dirty::semantics);
    return true;
}

bool PropertyList::set_reset_enabled(std::string_view row_id, bool enabled) {
    require_mutable();
    Impl::RowState* row = (*impl_).find_row(row_id);
    if (!row || !(*impl_).spec(*row).resettable || !(*row).reset_button) return false;
    PropertyRowSpec& model = (*impl_).spec(*row);
    if (model.reset_enabled == enabled) return true;
    model.reset_enabled = enabled;
    (*(*row).reset_button).set_enabled(model.enabled && enabled);
    invalidate(Dirty::paint | Dirty::semantics | Dirty::accessibility);
    return true;
}

bool PropertyList::set_group_expanded(std::string_view id, bool expanded) {
    require_mutable();
    Impl::GroupList::iterator found = (*impl_).groups.begin();
    while (found != (*impl_).groups.end() && (*found).stable_id != id) {
        ++found;
    }
    if (found == (*impl_).groups.end()) return false;
    if ((*found).expanded == expanded) return true;
    (*found).expanded = expanded;
    (*impl_).recompute_geometry();
    (*impl_).arrange_editors();
    invalidate(Dirty::measure | Dirty::layout | Dirty::paint | Dirty::hit_test |
               Dirty::semantics);
    publish_change(group_changed_,
                   PropertyGroupChange{(*found).stable_id, expanded});
    return true;
}

bool PropertyList::set_row_expanded(std::string_view id, bool expanded) {
    require_mutable();
    Impl::RowState* row = (*impl_).find_row(id);
    if (!row || !(*impl_).spec(*row).expandable) return false;
    PropertyRowSpec& model = (*impl_).spec(*row);
    if (model.expanded == expanded) return true;
    model.expanded = expanded;
    (*impl_).recompute_geometry();
    (*impl_).arrange_editors();
    invalidate(Dirty::measure | Dirty::layout | Dirty::paint | Dirty::hit_test |
               Dirty::semantics);
    publish_change(row_expansion_changed_,
                   PropertyRowExpansionChange{model.stable_id, expanded});
    return true;
}

std::optional<bool> PropertyList::row_expanded(std::string_view id) const {
    const Impl::RowState* row = (*impl_).find_row(id);
    return row && (*impl_).spec(*row).expandable
        ? std::optional<bool>{(*impl_).spec(*row).expanded}
        : std::optional<bool>{};
}

std::optional<std::string> PropertyList::value(std::string_view id) const {
    const Impl::RowState* row = (*impl_).find_row(id);
    return row ? std::optional<std::string>{(*impl_).spec(*row).value}
               : std::optional<std::string>{};
}

Control::Ptr PropertyList::editor(std::string_view id) const {
    const Impl::RowState* row = (*impl_).find_row(id);
    return row ? (*row).editor : Control::Ptr{};
}

bool PropertyList::replace_editor(std::string_view id, Control::Ptr editor) {
    require_mutable();
    Impl::RowState* row = (*impl_).find_row(id);
    if (!row) return false;
    if (!editor || !(*editor).is_alive() || (*editor).parent() ||
        (*editor).attached_window()) {
        throw std::invalid_argument(
            "PropertyList replacement editor must be an unattached live control");
    }
    PropertyRowSpec& model = (*impl_).spec(*row);
    if (!model.enabled) (*editor).set_enabled(false);
    if ((*editor).accessible_name().empty()) {
        (*editor).set_accessible_name(model.name);
    }
    if ((*editor).accessible_description().empty()) {
        (*editor).set_accessible_description(model.description);
    }

    // Attach first so an identity/lifecycle failure leaves the existing editor
    // and its subscriptions intact.
    (*impl_).apply_editor_font(editor);
    add_child(editor);
    Control::Ptr previous = (*row).editor;
    (*row).value_subscription.disconnect();
    (*row).commit_subscription.disconnect();
    (*row).cancel_subscription.disconnect();
    (*row).focus_subscription.disconnect();
    (*row).editor = editor;
    model.editor = PropertyEditorKind::custom;
    (*row).focus_subscription = (*editor).focus_observed().subscribe(
        *this, Impl::EnsureVisible{impl_.get(), std::string(id)});
    if (previous && (*previous).parent().get() == this) {
        Control::Ptr removed = remove_child((*previous).runtime_id());
        if (removed && (*removed).is_alive()) (*removed).dispose();
    }
    (*impl_).recompute_geometry();
    (*impl_).arrange_editors();
    invalidate(Dirty::measure | Dirty::layout | Dirty::paint | Dirty::hit_test |
               Dirty::semantics);
    return true;
}

std::shared_ptr<Button> PropertyList::reset_button(std::string_view id) const {
    const Impl::RowState* row = (*impl_).find_row(id);
    return row ? (*row).reset_button : std::shared_ptr<Button>{};
}

void PropertyList::set_header_content(Control::Ptr content, double height) {
    require_mutable();
    if (!std::isfinite(height) || height < 0.0 || height > 4096.0 ||
        (!content && height != 0.0)) {
        throw std::invalid_argument(
            "PropertyList header requires content and a 0 through 4096 height");
    }
    if ((*impl_).header == content && (*impl_).header_height == height) return;
    if ((*impl_).header && (*(*impl_).header).parent().get() == this) {
        Control::Ptr previous = remove_child((*(*impl_).header).runtime_id());
        if (previous && (*previous).is_alive()) (*previous).dispose();
    }
    (*impl_).header = std::move(content);
    (*impl_).header_height = height;
    if ((*impl_).header) add_child((*impl_).header);
    (*impl_).recompute_geometry();
    (*impl_).arrange_editors();
    invalidate(Dirty::measure | Dirty::layout | Dirty::paint | Dirty::hit_test |
               Dirty::semantics);
}

Control::Ptr PropertyList::header_content() const noexcept {
    return (*impl_).header;
}

double PropertyList::header_height() const noexcept { return (*impl_).header_height; }

void PropertyList::set_header_height(double height) {
    require_mutable();
    if (!std::isfinite(height) || height < 0.0 || height > 4096.0 ||
        (!(*impl_).header && height != 0.0)) {
        throw std::invalid_argument("PropertyList header height must be 0 through 4096");
    }
    if ((*impl_).header_height == height) return;
    (*impl_).header_height = height;
    (*impl_).recompute_geometry();
    (*impl_).arrange_editors();
    invalidate(Dirty::measure | Dirty::layout | Dirty::paint | Dirty::hit_test |
               Dirty::semantics);
}

double PropertyList::label_width() const noexcept { return (*impl_).label_width; }

FontSpec PropertyList::font() const noexcept { return (*impl_).font; }

void PropertyList::set_font(FontSpec font) {
    require_mutable();
    if (!valid_font_spec(font)) {
        throw std::invalid_argument("PropertyList font specification is invalid");
    }
    if ((*impl_).font_overridden && (*impl_).font == font) return;
    (*impl_).font = font;
    (*impl_).font_overridden = true;
    for (const Impl::RowState& row : (*impl_).rows) {
        (*impl_).apply_editor_font(row.editor);
        (*impl_).apply_editor_font(row.reset_button);
    }
    (*impl_).recompute_geometry();
    (*impl_).arrange_editors();
    invalidate(Dirty::measure | Dirty::layout | Dirty::paint |
               Dirty::hit_test | Dirty::semantics);
}

double PropertyList::row_height() const noexcept { return (*impl_).row_height; }

void PropertyList::set_row_height(double height) {
    require_mutable();
    if (!std::isfinite(height) || height <= 0.0 || height > 4096.0) {
        throw std::invalid_argument("PropertyList row height must be positive and at most 4096");
    }
    if ((*impl_).row_height == height) return;
    (*impl_).row_height = height;
    (*impl_).recompute_geometry();
    (*impl_).arrange_editors();
    invalidate(Dirty::measure | Dirty::layout | Dirty::paint |
               Dirty::hit_test | Dirty::semantics);
}

void PropertyList::set_label_width(double width) {
    require_mutable();
    if (!std::isfinite(width) || width < 40.0 || width > 320.0) {
        throw std::invalid_argument("PropertyList label width must be 40 through 320");
    }
    if ((*impl_).label_width == width) return;
    (*impl_).label_width = width;
    (*impl_).recompute_geometry();
    (*impl_).arrange_editors();
    invalidate(Dirty::measure | Dirty::layout | Dirty::paint | Dirty::hit_test |
               Dirty::semantics);
}

double PropertyList::scroll_offset() const noexcept { return (*impl_).scroll_offset; }

void PropertyList::set_scroll_offset(double offset) {
    require_mutable();
    if (!std::isfinite(offset)) {
        throw std::invalid_argument("PropertyList scroll offset must be finite");
    }
    const double maximum = std::max(0.0, (*impl_).content_height -
                                          committed_arranged_bounds().height);
    const double next = std::clamp(offset, 0.0, maximum);
    if (next == (*impl_).scroll_offset) return;
    (*impl_).scroll_offset = next;
    (*impl_).arrange_editors();
    invalidate(Dirty::paint | Dirty::hit_test | Dirty::semantics);
}

double PropertyList::content_height() const noexcept { return (*impl_).content_height; }

Size PropertyList::measure(Size available) {
    return {available.width, std::min(available.height,
        std::max((*impl_).group_extent(),
                 (*impl_).content_height))};
}

void PropertyList::arrange(Rect final_bounds) {
    arrange_self(final_bounds);
    (*impl_).recompute_geometry();
    (*impl_).arrange_editors();
}

void PropertyList::on_paint(Painter& painter, Rect) {
    const Rect bounds{0.0, 0.0, committed_arranged_bounds().width,
                      committed_arranged_bounds().height};
    const BasicControlStyle style;
    const double s = effective_text_scale();
    const double group_height = (*impl_).group_extent();
    const FontSpec group_glyph = effective_font((*impl_).font_overridden
        ? (*impl_).font : FontSpec{FontRole::control, 8.5, 600, false});
    FontSpec group_font = effective_font((*impl_).font_overridden
        ? (*impl_).font : FontSpec{FontRole::control, 8.5, 700, false, 0.24});
    group_font.weight = std::max<std::uint16_t>(group_font.weight, 700U);
    const FontSpec name_font = effective_font((*impl_).font_overridden
        ? (*impl_).font : FontSpec{FontRole::content, 9.5, 600, false});
    const FontSpec value_font = effective_font((*impl_).font);
    const FontSpec validation_font = effective_font((*impl_).font_overridden
        ? (*impl_).font : FontSpec{FontRole::content, 8.5, 600, false});
    painter.fill_rect(bounds, background());
    double y = (*impl_).header_height - (*impl_).scroll_offset;
    for (std::size_t group_index = 0;
         group_index < (*impl_).groups.size(); ++group_index) {
        const PropertyGroupSpec& group = (*impl_).groups[group_index];
        painter.fill_rect({0.0, y, bounds.width, group_height},
                          Color::rgba(216, 224, 236));
        painter.draw_line({0.0, y + group_height - 1.0},
                          {bounds.width, y + group_height - 1.0},
                          style.border, 1.0);
        const double group_baseline = y + std::max(group_font.size,
                                                    group_height * 0.5 +
                                                        group_font.size * 0.35);
        painter.draw_text_utf8({7.0 * s, group_baseline},
                               group.expanded ? "▼" : "▶",
                               group_glyph, style.text);
        painter.draw_text_utf8({22.0 * s, group_baseline}, group.title,
                               group_font,
                               style.text);
        y += group_height;
        if (!group.expanded) continue;
        for (const Impl::RowState& row_state : (*impl_).rows) {
            if (row_state.group_index != group_index) continue;
            const PropertyRowSpec& row = (*impl_).spec(row_state);
            if (!(*impl_).row_visible(row_state)) continue;
            const Rect name{row_state.name_bounds.x,
                            row_state.name_bounds.y - (*impl_).scroll_offset,
                            row_state.name_bounds.width,
                            row_state.name_bounds.height};
            const Rect value_bounds{row_state.value_bounds.x,
                                    row_state.value_bounds.y - (*impl_).scroll_offset,
                                    row_state.value_bounds.width,
                                    row_state.value_bounds.height};
            if (row.expandable) {
                painter.draw_text_utf8({std::max(2.0 * s, name.x - 13.0 * s),
                                        name.y + std::max(15.0 * s, name_font.size)},
                    row.expanded ? "▼" : "▶", group_glyph, style.text);
            }
            painter.draw_text_utf8({name.x, name.y + std::max(15.0 * s, name_font.size)}, row.name,
                                   name_font, style.disabled_text);
            if (row.editor == PropertyEditorKind::read_only) {
                painter.draw_text_utf8({value_bounds.x,
                                        value_bounds.y + std::max(15.0 * s, value_font.size)},
                    row.value, value_font,
                    row.enabled ? style.text : style.disabled_text);
            }
            if (!row.validation_message.empty()) {
                const Rect validation{row_state.validation_bounds.x,
                    row_state.validation_bounds.y - (*impl_).scroll_offset,
                    row_state.validation_bounds.width,
                    row_state.validation_bounds.height};
                painter.draw_text_utf8({validation.x,
                                        validation.y + std::max(13.0 * s, validation_font.size)},
                    "! " + row.validation_message,
                    validation_font,
                    Color::rgba(151, 45, 43));
            }
            painter.draw_line({property_padding * s,
                               row_state.row_bounds.y + row_state.row_bounds.height -
                                   (*impl_).scroll_offset - 1.0},
                              {std::max(property_padding * s,
                                        bounds.width - property_padding * s),
                               row_state.row_bounds.y + row_state.row_bounds.height -
                                   (*impl_).scroll_offset - 1.0},
                              Color::rgba(211, 219, 229), 1.0);
            y += row_state.row_bounds.height;
        }
    }
    if ((*impl_).content_height > bounds.height && bounds.height > 12.0) {
        const double ratio = bounds.height / (*impl_).content_height;
        const double thumb_height = std::max(18.0, bounds.height * ratio);
        const double maximum = (*impl_).content_height - bounds.height;
        const double thumb_y = maximum <= 0.0 ? 0.0 :
            (bounds.height - thumb_height) * (*impl_).scroll_offset / maximum;
        painter.fill_rect({std::max(0.0, bounds.width - 4.0), thumb_y,
                           3.0, thumb_height}, style.border);
    }
}

void PropertyList::on_pointer(PointerEvent& event) {
    if (event.action == PointerAction::wheel && event.wheel_delta.y != 0.0) {
        set_scroll_offset((*impl_).scroll_offset +
            (event.wheel_delta.y > 0.0 ? -54.0 : 54.0) *
                effective_text_scale());
        event.handled = true;
        return;
    }
    if (event.action == PointerAction::down &&
        event.button == PointerButton::primary) {
        if (Impl::RowState* row = (*impl_).disclosure_at(event.position)) {
            const PropertyRowSpec& model = (*impl_).spec(*row);
            static_cast<void>(set_row_expanded(model.stable_id,
                                               !model.expanded));
            event.handled = true;
            return;
        }
        const std::optional<std::size_t> group = (*impl_).group_at(event.position);
        if (group) {
            static_cast<void>(set_group_expanded(
                (*impl_).groups[*group].stable_id,
                !(*impl_).groups[*group].expanded));
            event.handled = true;
        }
    }
}

void PropertyList::on_key(KeyEvent& event) {
    if (event.action != KeyAction::down) return;
    double next = (*impl_).scroll_offset;
    if (event.physical_key == PhysicalKey::home) next = 0.0;
    else if (event.physical_key == PhysicalKey::end) next = (*impl_).content_height;
    else if (event.physical_key == PhysicalKey::page_up) {
        next -= committed_arranged_bounds().height;
    } else if (event.physical_key == PhysicalKey::page_down) {
        next += committed_arranged_bounds().height;
    } else return;
    set_scroll_offset(next);
    event.handled = true;
}

SemanticDescriptor PropertyList::semantic_descriptor() const {
    SemanticDescriptor descriptor;
    descriptor.role = SemanticRole::property_grid;
    descriptor.name = accessible_name().empty() ? "Properties" : accessible_name();
    descriptor.description = accessible_description();
    descriptor.actions = {SemanticAction::focus};
    descriptor.exposed = true;
    descriptor.include_descendants = true;
    return descriptor;
}

std::vector<SemanticNode> PropertyList::semantic_virtual_children() const {
    std::vector<SemanticNode> nodes;
    const Rect absolute = absolute_bounds();
    const double group_height = (*impl_).group_extent();
    double y = (*impl_).header_height - (*impl_).scroll_offset;
    for (std::size_t group_index = 0;
         group_index < (*impl_).groups.size(); ++group_index) {
        const PropertyGroupSpec& group = (*impl_).groups[group_index];
        SemanticNode header;
        header.stable_id = group.stable_id;
        header.runtime_id = property_virtual_runtime_id(header.stable_id);
        header.role = SemanticRole::property_group;
        header.name = group.title;
        header.bounds = {absolute.x, absolute.y + y, absolute.width,
                         group_height};
        header.states = SemanticState::visible | SemanticState::focusable |
                        SemanticState::enabled;
        if (group.expanded) header.states |= SemanticState::expanded;
        header.actions = {SemanticAction::focus,
            group.expanded ? SemanticAction::collapse : SemanticAction::expand};
        nodes.push_back(std::move(header));
        y += group_height;
        if (!group.expanded) continue;
        for (const Impl::RowState& row_state : (*impl_).rows) {
            if (row_state.group_index != group_index) continue;
            const PropertyRowSpec& row = (*impl_).spec(row_state);
            if (!(*impl_).row_visible(row_state)) continue;
            if (row.editor == PropertyEditorKind::read_only) {
                SemanticNode node;
                node.stable_id = row.stable_id;
                node.runtime_id = property_virtual_runtime_id(node.stable_id);
                node.role = SemanticRole::property_row;
                node.name = row.name;
                node.value = row.value;
                node.description = row.validation_message.empty()
                    ? row.description : row.description + " · Error: " +
                                            row.validation_message;
                node.bounds = {absolute.x,
                    absolute.y + row_state.row_bounds.y - (*impl_).scroll_offset,
                    absolute.width, row_state.row_bounds.height};
                node.states = SemanticState::visible;
                if (row.enabled) node.states |= SemanticState::enabled;
                if (row.expandable) {
                    if (row.expanded) node.states |= SemanticState::expanded;
                    node.actions.push_back(row.expanded
                        ? SemanticAction::collapse : SemanticAction::expand);
                }
                nodes.push_back(std::move(node));
            }
            y += row_state.row_bounds.height;
        }
    }
    return nodes;
}

bool PropertyList::on_semantic_child_action(std::string_view id,
                                            SemanticAction action,
                                            std::string_view) {
    Impl::GroupList::iterator found = (*impl_).groups.begin();
    while (found != (*impl_).groups.end() && (*found).stable_id != id) {
        ++found;
    }
    if (found != (*impl_).groups.end()) {
        if (action == SemanticAction::focus) {
            if (window()) {
                static_cast<void>((*window()).request_focus(shared_from_this()));
            }
            return true;
        }
        if (action == SemanticAction::expand ||
            action == SemanticAction::collapse) {
            return set_group_expanded(id, action == SemanticAction::expand);
        }
        return false;
    }
    const Impl::RowState* row = (*impl_).find_row(id);
    if (row && (*impl_).spec(*row).expandable &&
        (action == SemanticAction::expand ||
         action == SemanticAction::collapse)) {
        return set_row_expanded(id, action == SemanticAction::expand);
    }
    return false;
}

void PropertyList::on_dispose() noexcept {
    // Component::dispose marks the control disposing before entering this
    // callback. Control::on_dispose owns the disposal-safe child detach path.
    (*impl_).rows.clear();
    (*impl_).header.reset();
    Control::on_dispose();
}

} // namespace gui_forms
