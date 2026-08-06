#include "gui_forms/inspection_controls.hpp"

#include "gui_forms/text.hpp"
#include "gui_forms/window.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <unordered_set>
#include <utility>

namespace gui_forms {
namespace {

constexpr double property_group_height = 25.0;
constexpr double property_row_height = 29.0;
constexpr double property_editor_height = 34.0;
constexpr double property_validation_height = 18.0;
constexpr double property_padding = 8.0;
constexpr double property_gap = 7.0;

std::uint64_t property_virtual_runtime_id(std::string_view id) noexcept {
    std::uint64_t hash = 1469598103934665603ULL;
    for (const unsigned char byte : id) {
        hash ^= byte;
        hash *= 1099511628211ULL;
    }
    return hash | (1ULL << 63U);
}

void require_property_text(std::string_view value, const char* field) {
    if (!validate_utf8(value).valid()) {
        throw std::invalid_argument(std::string("PropertyList ") + field +
                                    " must be valid UTF-8");
    }
}

} // namespace

struct PropertyList::Impl final {
    struct RowState final {
        std::size_t group_index{};
        std::size_t row_index{};
        Control::Ptr editor;
        SubscriptionToken value_subscription;
        SubscriptionToken commit_subscription;
        SubscriptionToken cancel_subscription;
        SubscriptionToken focus_subscription;
        std::string committed_value;
        Rect row_bounds{};
        Rect name_bounds{};
        Rect value_bounds{};
        Rect validation_bounds{};
    };

    explicit Impl(PropertyList& public_owner) : owner(public_owner) {}

    [[nodiscard]] double scale() const noexcept {
        return owner.effective_text_scale();
    }

    PropertyRowSpec& spec(RowState& state) {
        return groups[state.group_index].rows[state.row_index];
    }
    const PropertyRowSpec& spec(const RowState& state) const {
        return groups[state.group_index].rows[state.row_index];
    }

    RowState* find_row(std::string_view id) noexcept {
        const auto found = std::find_if(rows.begin(), rows.end(),
            [this, id](const RowState& row) { return spec(row).stable_id == id; });
        return found == rows.end() ? nullptr : &*found;
    }
    const RowState* find_row(std::string_view id) const noexcept {
        const auto found = std::find_if(rows.begin(), rows.end(),
            [this, id](const RowState& row) { return spec(row).stable_id == id; });
        return found == rows.end() ? nullptr : &*found;
    }

    void clear_editors() noexcept {
        for (RowState& row : rows) {
            if (!row.editor || row.editor->parent().get() != &owner) continue;
            Control::Ptr removed = owner.remove_child(row.editor->runtime_id());
            if (removed && removed->is_alive()) removed->dispose();
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
                    auto editor = make_control<TextBox>(
                        StableId(row.stable_id + ".editor"), row.value);
                    editor->set_font({FontRole::content, 10.0, 400, false});
                    editor->set_accessible_name(row.name);
                    editor->set_accessible_description(row.description);
                    editor->set_enabled(row.enabled);
                    state.editor = editor;
                    owner.add_child(editor);
                } else if (row.editor == PropertyEditorKind::choice) {
                    auto editor = make_control<ComboBox>(
                        StableId(row.stable_id + ".editor"));
                    editor->set_items(row.choices);
                    const auto selected = std::find(row.choices.begin(),
                                                    row.choices.end(), row.value);
                    if (selected != row.choices.end()) {
                        editor->set_selected_index(static_cast<std::size_t>(
                            std::distance(row.choices.begin(), selected)));
                    }
                    editor->set_font({FontRole::content, 10.0, 400, false});
                    editor->set_accessible_name(row.name);
                    editor->set_accessible_description(row.description);
                    editor->set_enabled(row.enabled);
                    state.editor = editor;
                    owner.add_child(editor);
                }
                rows.push_back(std::move(state));
                connect_row(rows.back());
            }
        }
    }

    void connect_row(RowState& state) {
        if (!state.editor) return;
        const std::string row_id = spec(state).stable_id;
        if (const auto text = std::dynamic_pointer_cast<TextBox>(state.editor)) {
            state.value_subscription = text->text_changed().subscribe(
                owner, [this, row_id](const std::string& value) {
                    if (synchronizing) return;
                    RowState* row = find_row(row_id);
                    if (!row) return;
                    PropertyRowSpec& model = spec(*row);
                    const std::string previous = model.value;
                    model.value = value;
                    owner.value_changed_.emit(
                        {row_id, previous, model.value, false});
                });
            state.commit_subscription = text->committed().subscribe(
                owner, [this, row_id](const std::string& value) {
                    RowState* row = find_row(row_id);
                    if (!row) return;
                    PropertyRowSpec& model = spec(*row);
                    const std::string previous = row->committed_value;
                    row->committed_value = value;
                    model.value = value;
                    if (model.required && value.empty()) {
                        model.validation_message = model.name + " is required";
                    }
                    recompute_geometry();
                    owner.value_committed_.emit(
                        {row_id, previous, value, true});
                    owner.invalidate(Dirty::measure | Dirty::layout | Dirty::paint |
                                     Dirty::semantics);
                });
            state.cancel_subscription = text->cancelled().subscribe(
                owner, [this, row_id] {
                    RowState* row = find_row(row_id);
                    if (!row) return;
                    synchronizing = true;
                    if (const auto editor =
                            std::dynamic_pointer_cast<TextBox>(row->editor)) {
                        editor->set_text(row->committed_value);
                    }
                    spec(*row).value = row->committed_value;
                    synchronizing = false;
                    owner.invalidate(Dirty::paint | Dirty::semantics);
                });
        } else if (const auto choice =
                       std::dynamic_pointer_cast<ComboBox>(state.editor)) {
            state.value_subscription = choice->selected_index_changed().subscribe(
                owner, [this, row_id, weak_choice = std::weak_ptr<ComboBox>(choice)](
                    std::optional<std::size_t>) {
                    if (synchronizing) return;
                    RowState* row = find_row(row_id);
                    const auto editor = weak_choice.lock();
                    if (!row || !editor) return;
                    PropertyRowSpec& model = spec(*row);
                    const std::string previous = row->committed_value;
                    model.value = std::string(editor->selected_text());
                    row->committed_value = model.value;
                    PropertyValueChange change{row_id, previous, model.value, true};
                    owner.value_changed_.emit(change);
                    owner.value_committed_.emit(change);
                });
        }
        state.focus_subscription = state.editor->focus_observed().subscribe(
            owner, [this, row_id](bool focused) {
                if (focused) ensure_visible(row_id);
            });
    }

    void recompute_geometry() {
        const double width = std::max(0.0, owner.committed_arranged_bounds().width);
        const double viewport = std::max(0.0, owner.committed_arranged_bounds().height);
        const double s = scale();
        const double padding = property_padding * s;
        const double gap = property_gap * s;
        const double scaled_label_width = label_width * s;
        const double validation_height = property_validation_height * s;
        const bool stacked = width < std::max(230.0 * s,
                                              scaled_label_width + 116.0 * s);
        double y{header_height};
        for (std::size_t group_index = 0; group_index < groups.size(); ++group_index) {
            y += property_group_height * s;
            if (!groups[group_index].expanded) continue;
            for (RowState& state : rows) {
                if (state.group_index != group_index) continue;
                const PropertyRowSpec& row = spec(state);
                const bool editable = row.editor != PropertyEditorKind::read_only;
                double height = (editable ? property_editor_height
                                          : property_row_height) * s;
                if (stacked) height += (editable ? 21.0 : 13.0) * s;
                if (!row.validation_message.empty()) height += validation_height;
                state.row_bounds = {0.0, y, width, height};
                if (stacked) {
                    state.name_bounds = {padding, y + 3.0 * s,
                                         std::max(0.0, width - padding * 2.0),
                                         18.0 * s};
                    state.value_bounds = {padding, y + 21.0 * s,
                        std::max(0.0, width - padding * 2.0),
                        (editable ? 28.0 : 19.0) * s};
                } else {
                    state.name_bounds = {padding, y + 5.0 * s,
                                         scaled_label_width, 21.0 * s};
                    state.value_bounds = {
                        padding + scaled_label_width + gap, y + 3.0 * s,
                        std::max(0.0, width - padding * 2.0 -
                                          scaled_label_width - gap),
                        (editable ? 28.0 : 22.0) * s};
                }
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
            if (!state.editor) continue;
            const bool expanded = groups[state.group_index].expanded;
            state.editor->set_visible(expanded);
            if (expanded) {
                owner.set_child_layout(state.editor,
                    {state.value_bounds.x,
                     state.value_bounds.y - scroll_offset,
                     state.value_bounds.width, state.value_bounds.height});
            }
        }
    }

    void ensure_visible(std::string_view row_id) {
        RowState* row = find_row(row_id);
        if (!row) return;
        const double viewport = owner.committed_arranged_bounds().height;
        if (row->row_bounds.y < scroll_offset) {
            scroll_offset = row->row_bounds.y;
        } else if (row->row_bounds.y + row->row_bounds.height >
                   scroll_offset + viewport) {
            scroll_offset = row->row_bounds.y + row->row_bounds.height - viewport;
        }
        scroll_offset = std::clamp(scroll_offset, 0.0,
            std::max(0.0, content_height - viewport));
        arrange_editors();
        owner.invalidate(Dirty::paint | Dirty::hit_test | Dirty::semantics);
    }

    std::optional<std::size_t> group_at(Point absolute) const noexcept {
        const Rect bounds = owner.absolute_bounds();
        if (!bounds.contains(absolute)) return {};
        const double y_target = absolute.y - bounds.y + scroll_offset;
        const double group_height = property_group_height * scale();
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

    PropertyList& owner;
    std::vector<PropertyGroupSpec> groups;
    std::vector<RowState> rows;
    Control::Ptr header;
    double header_height{};
    double label_width{76.0};
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
    return impl_->groups;
}

void PropertyList::set_groups(std::vector<PropertyGroupSpec> groups) {
    require_mutable();
    std::unordered_set<std::string> identities;
    for (const auto& group : groups) {
        require_property_text(group.stable_id, "group ID");
        require_property_text(group.title, "group title");
        if (group.stable_id.empty() || group.title.empty() ||
            !identities.insert(group.stable_id).second) {
            throw std::invalid_argument(
                "PropertyList groups require unique nonempty identities and titles");
        }
        for (const auto& row : group.rows) {
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
            if (row.editor == PropertyEditorKind::choice && row.choices.empty()) {
                throw std::invalid_argument(
                    "PropertyList choice rows require at least one choice");
            }
            for (const auto& choice : row.choices) {
                require_property_text(choice, "choice");
            }
        }
    }
    impl_->groups = std::move(groups);
    impl_->scroll_offset = 0.0;
    impl_->rebuild_editors();
    impl_->recompute_geometry();
    invalidate(Dirty::measure | Dirty::layout | Dirty::paint | Dirty::hit_test |
               Dirty::semantics);
}

bool PropertyList::set_value(std::string_view row_id, std::string value) {
    require_mutable();
    require_property_text(value, "row value");
    Impl::RowState* row = impl_->find_row(row_id);
    if (!row) return false;
    PropertyRowSpec& model = impl_->spec(*row);
    if (model.value == value && row->committed_value == value) return true;
    impl_->synchronizing = true;
    model.value = value;
    row->committed_value = value;
    if (const auto text = std::dynamic_pointer_cast<TextBox>(row->editor)) {
        text->set_text(value);
    } else if (const auto choice = std::dynamic_pointer_cast<ComboBox>(row->editor)) {
        const auto found = std::find(model.choices.begin(), model.choices.end(), value);
        choice->set_selected_index(found == model.choices.end()
            ? std::optional<std::size_t>{}
            : std::optional<std::size_t>{static_cast<std::size_t>(
                std::distance(model.choices.begin(), found))});
    }
    impl_->synchronizing = false;
    invalidate(Dirty::paint | Dirty::semantics);
    return true;
}

bool PropertyList::set_validation(std::string_view row_id, std::string message) {
    require_mutable();
    require_property_text(message, "validation message");
    Impl::RowState* row = impl_->find_row(row_id);
    if (!row) return false;
    PropertyRowSpec& model = impl_->spec(*row);
    if (model.validation_message == message) return true;
    model.validation_message = std::move(message);
    if (row->editor) {
        std::string description = model.description;
        if (!model.validation_message.empty()) {
            if (!description.empty()) description += " · ";
            description += "Error: " + model.validation_message;
        }
        row->editor->set_accessible_description(std::move(description));
    }
    impl_->recompute_geometry();
    impl_->arrange_editors();
    invalidate(Dirty::measure | Dirty::layout | Dirty::paint | Dirty::hit_test |
               Dirty::semantics);
    return true;
}

bool PropertyList::set_group_expanded(std::string_view id, bool expanded) {
    require_mutable();
    const auto found = std::find_if(impl_->groups.begin(), impl_->groups.end(),
        [id](const PropertyGroupSpec& group) { return group.stable_id == id; });
    if (found == impl_->groups.end()) return false;
    if (found->expanded == expanded) return true;
    found->expanded = expanded;
    impl_->recompute_geometry();
    impl_->arrange_editors();
    invalidate(Dirty::measure | Dirty::layout | Dirty::paint | Dirty::hit_test |
               Dirty::semantics);
    group_changed_.emit({found->stable_id, expanded});
    return true;
}

std::optional<std::string> PropertyList::value(std::string_view id) const {
    const Impl::RowState* row = impl_->find_row(id);
    return row ? std::optional<std::string>{impl_->spec(*row).value}
               : std::optional<std::string>{};
}

Control::Ptr PropertyList::editor(std::string_view id) const {
    const Impl::RowState* row = impl_->find_row(id);
    return row ? row->editor : Control::Ptr{};
}

void PropertyList::set_header_content(Control::Ptr content, double height) {
    require_mutable();
    if (!std::isfinite(height) || height < 0.0 || height > 4096.0 ||
        (!content && height != 0.0)) {
        throw std::invalid_argument(
            "PropertyList header requires content and a 0 through 4096 height");
    }
    if (impl_->header == content && impl_->header_height == height) return;
    if (impl_->header && impl_->header->parent().get() == this) {
        Control::Ptr previous = remove_child(impl_->header->runtime_id());
        if (previous && previous->is_alive()) previous->dispose();
    }
    impl_->header = std::move(content);
    impl_->header_height = height;
    if (impl_->header) add_child(impl_->header);
    impl_->recompute_geometry();
    impl_->arrange_editors();
    invalidate(Dirty::measure | Dirty::layout | Dirty::paint | Dirty::hit_test |
               Dirty::semantics);
}

Control::Ptr PropertyList::header_content() const noexcept {
    return impl_->header;
}

double PropertyList::header_height() const noexcept { return impl_->header_height; }

void PropertyList::set_header_height(double height) {
    require_mutable();
    if (!std::isfinite(height) || height < 0.0 || height > 4096.0 ||
        (!impl_->header && height != 0.0)) {
        throw std::invalid_argument("PropertyList header height must be 0 through 4096");
    }
    if (impl_->header_height == height) return;
    impl_->header_height = height;
    impl_->recompute_geometry();
    impl_->arrange_editors();
    invalidate(Dirty::measure | Dirty::layout | Dirty::paint | Dirty::hit_test |
               Dirty::semantics);
}

double PropertyList::label_width() const noexcept { return impl_->label_width; }

void PropertyList::set_label_width(double width) {
    require_mutable();
    if (!std::isfinite(width) || width < 40.0 || width > 320.0) {
        throw std::invalid_argument("PropertyList label width must be 40 through 320");
    }
    if (impl_->label_width == width) return;
    impl_->label_width = width;
    impl_->recompute_geometry();
    impl_->arrange_editors();
    invalidate(Dirty::measure | Dirty::layout | Dirty::paint | Dirty::hit_test |
               Dirty::semantics);
}

double PropertyList::scroll_offset() const noexcept { return impl_->scroll_offset; }

void PropertyList::set_scroll_offset(double offset) {
    require_mutable();
    if (!std::isfinite(offset)) {
        throw std::invalid_argument("PropertyList scroll offset must be finite");
    }
    const double maximum = std::max(0.0, impl_->content_height -
                                          committed_arranged_bounds().height);
    const double next = std::clamp(offset, 0.0, maximum);
    if (next == impl_->scroll_offset) return;
    impl_->scroll_offset = next;
    impl_->arrange_editors();
    invalidate(Dirty::paint | Dirty::hit_test | Dirty::semantics);
}

double PropertyList::content_height() const noexcept { return impl_->content_height; }

Size PropertyList::measure(Size available) {
    return {available.width, std::min(available.height,
        std::max(property_group_height * effective_text_scale(),
                 impl_->content_height))};
}

void PropertyList::arrange(Rect final_bounds) {
    arrange_self(final_bounds);
    impl_->recompute_geometry();
    impl_->arrange_editors();
}

void PropertyList::on_paint(Painter& painter, Rect) {
    const Rect bounds{0.0, 0.0, committed_arranged_bounds().width,
                      committed_arranged_bounds().height};
    const BasicControlStyle style;
    const double s = effective_text_scale();
    const double group_height = property_group_height * s;
    const FontSpec group_glyph = effective_font(
        {FontRole::control, 8.5, 600, false});
    const FontSpec group_font = effective_font(
        {FontRole::control, 8.5, 700, false, 0.24});
    const FontSpec name_font = effective_font(
        {FontRole::content, 9.5, 600, false});
    const FontSpec value_font = effective_font(
        {FontRole::content, 9.5, 400, false});
    const FontSpec validation_font = effective_font(
        {FontRole::content, 8.5, 600, false});
    painter.fill_rect(bounds, background());
    double y = impl_->header_height - impl_->scroll_offset;
    for (std::size_t group_index = 0;
         group_index < impl_->groups.size(); ++group_index) {
        const PropertyGroupSpec& group = impl_->groups[group_index];
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
        for (const Impl::RowState& row_state : impl_->rows) {
            if (row_state.group_index != group_index) continue;
            const PropertyRowSpec& row = impl_->spec(row_state);
            const Rect name{row_state.name_bounds.x,
                            row_state.name_bounds.y - impl_->scroll_offset,
                            row_state.name_bounds.width,
                            row_state.name_bounds.height};
            const Rect value_bounds{row_state.value_bounds.x,
                                    row_state.value_bounds.y - impl_->scroll_offset,
                                    row_state.value_bounds.width,
                                    row_state.value_bounds.height};
            painter.draw_text_utf8({name.x, name.y + 15.0 * s}, row.name,
                                   name_font, style.disabled_text);
            if (row.editor == PropertyEditorKind::read_only) {
                painter.draw_text_utf8({value_bounds.x,
                                        value_bounds.y + 15.0 * s},
                    row.value, value_font,
                    row.enabled ? style.text : style.disabled_text);
            }
            if (!row.validation_message.empty()) {
                const Rect validation{row_state.validation_bounds.x,
                    row_state.validation_bounds.y - impl_->scroll_offset,
                    row_state.validation_bounds.width,
                    row_state.validation_bounds.height};
                painter.draw_text_utf8({validation.x,
                                        validation.y + 13.0 * s},
                    "! " + row.validation_message,
                    validation_font,
                    Color::rgba(151, 45, 43));
            }
            painter.draw_line({property_padding * s,
                               row_state.row_bounds.y + row_state.row_bounds.height -
                                   impl_->scroll_offset - 1.0},
                              {std::max(property_padding * s,
                                        bounds.width - property_padding * s),
                               row_state.row_bounds.y + row_state.row_bounds.height -
                                   impl_->scroll_offset - 1.0},
                              Color::rgba(211, 219, 229), 1.0);
            y += row_state.row_bounds.height;
        }
    }
    if (impl_->content_height > bounds.height && bounds.height > 12.0) {
        const double ratio = bounds.height / impl_->content_height;
        const double thumb_height = std::max(18.0, bounds.height * ratio);
        const double maximum = impl_->content_height - bounds.height;
        const double thumb_y = maximum <= 0.0 ? 0.0 :
            (bounds.height - thumb_height) * impl_->scroll_offset / maximum;
        painter.fill_rect({std::max(0.0, bounds.width - 4.0), thumb_y,
                           3.0, thumb_height}, style.border);
    }
}

void PropertyList::on_pointer(PointerEvent& event) {
    if (event.action == PointerAction::wheel && event.wheel_delta.y != 0.0) {
        set_scroll_offset(impl_->scroll_offset +
            (event.wheel_delta.y > 0.0 ? -54.0 : 54.0) *
                effective_text_scale());
        event.handled = true;
        return;
    }
    if (event.action == PointerAction::down &&
        event.button == PointerButton::primary) {
        const auto group = impl_->group_at(event.position);
        if (group) {
            static_cast<void>(set_group_expanded(
                impl_->groups[*group].stable_id,
                !impl_->groups[*group].expanded));
            event.handled = true;
        }
    }
}

void PropertyList::on_key(KeyEvent& event) {
    if (event.action != KeyAction::down) return;
    double next = impl_->scroll_offset;
    if (event.physical_key == PhysicalKey::home) next = 0.0;
    else if (event.physical_key == PhysicalKey::end) next = impl_->content_height;
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
    const double group_height = property_group_height * effective_text_scale();
    double y = impl_->header_height - impl_->scroll_offset;
    for (std::size_t group_index = 0;
         group_index < impl_->groups.size(); ++group_index) {
        const PropertyGroupSpec& group = impl_->groups[group_index];
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
        for (const Impl::RowState& row_state : impl_->rows) {
            if (row_state.group_index != group_index) continue;
            const PropertyRowSpec& row = impl_->spec(row_state);
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
                    absolute.y + row_state.row_bounds.y - impl_->scroll_offset,
                    absolute.width, row_state.row_bounds.height};
                node.states = SemanticState::visible;
                if (row.enabled) node.states |= SemanticState::enabled;
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
    const auto found = std::find_if(impl_->groups.begin(), impl_->groups.end(),
        [id](const PropertyGroupSpec& group) { return group.stable_id == id; });
    if (found == impl_->groups.end()) return false;
    if (action == SemanticAction::focus) {
        if (window()) static_cast<void>(window()->request_focus(shared_from_this()));
        return true;
    }
    if (action == SemanticAction::expand || action == SemanticAction::collapse) {
        return set_group_expanded(id, action == SemanticAction::expand);
    }
    return false;
}

void PropertyList::on_dispose() noexcept {
    impl_->clear_editors();
    if (impl_->header && impl_->header->parent().get() == this) {
        Control::Ptr removed = remove_child(impl_->header->runtime_id());
        if (removed && removed->is_alive()) removed->dispose();
    }
    impl_->header.reset();
}

} // namespace gui_forms
