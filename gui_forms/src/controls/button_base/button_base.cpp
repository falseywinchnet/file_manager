#include "gui_forms/controls/button_base/button_base.hpp"
#include "gui_forms/connected_controls.hpp"
#include "../basic/basic_control_rendering.hpp"
#include "gui_forms/detail/bound_member_function.hpp"
#include "gui_forms/detail/property_binding_adapters.hpp"
#include "gui_forms/text.hpp"
#include "gui_forms/window.hpp"

#include <algorithm>
#include <cmath>
#include <iterator>
#include <stdexcept>
#include <utility>
#include <vector>

namespace gui_forms {

namespace {
std::vector<std::string_view> button_text_lines(std::string_view text) {
    std::vector<std::string_view> result;
    if (text.empty()) return result;
    std::size_t start = 0;
    for (;;) {
        const std::size_t end = text.find('\n', start);
        std::string_view line = text.substr(start, end == std::string_view::npos ? end : end - start);
        if (!line.empty() && line.back() == '\r') line.remove_suffix(1);
        result.push_back(line);
        if (end == std::string_view::npos) break;
        start = end + 1;
    }
    return result;
}
}

ButtonBase::ButtonBase(StableId stable_id, std::string text)
    : Control(std::move(stable_id)), text_(std::move(text)) {
    set_focusable(true);
    set_cursor(CursorKind::hand);
    define_bindable_property({
        {"Text", BindingValueKind::text, "Appearance",
         "Text displayed by the button.", BindingValue{std::string{}},
         invalidation::text_content},
        detail::BindingMemberGetter<ButtonBase, std::string>(
            *this, &ButtonBase::text_),
        detail::ConvertedPropertySetter<ButtonBase, std::string>(
            *this, &ButtonBase::set_text, BindingValueKind::text,
            "ButtonBase.Text binding requires text"),
        detail::EventChangeConnector<const std::string&>(text_changed_), {}, {}});

    PropertyDescriptor font;
    font.name = "Font";
    font.kind = BindingValueKind::font;
    font.category = "Appearance";
    font.description = "Font used for button measurement and painting.";
    font.default_value = BindingValue{
        FontSpec{FontRole::control, 12.0, 400, false, 0.24}};
    font.invalidation_effects = Dirty::measure | Dirty::paint | Dirty::semantics;
    font.bindable = false;
    define_bindable_property({
        std::move(font),
        detail::BindingMemberGetter<ButtonBase, FontSpec>(
            *this, &ButtonBase::font_),
        detail::DirectPropertySetter<ButtonBase, FontSpec>(
            *this, &ButtonBase::set_font), {}, {}, {}});

    PropertyDescriptor image;
    image.name = "Image";
    image.kind = BindingValueKind::image;
    image.category = "Appearance";
    image.description = "Direct generational image resource for button content.";
    image.default_value = BindingValue{ImageId{}};
    image.invalidation_effects = Dirty::measure | Dirty::paint | Dirty::semantics;
    image.bindable = false;
    define_bindable_property({
        std::move(image),
        detail::BindingMemberGetter<ButtonBase, ImageId>(
            *this, &ButtonBase::image_),
        detail::DirectPropertySetter<ButtonBase, ImageId>(
            *this, &ButtonBase::set_image), {},
        detail::BoundMemberFunction<void (ButtonBase::*)()>(
            *this, &ButtonBase::clear_image),
        detail::BoundMemberFunction<bool (ButtonBase::*)() const noexcept>(
            *this, &ButtonBase::should_serialize_image)});

    PropertyDescriptor content_padding;
    content_padding.name = "ContentPadding";
    content_padding.kind = BindingValueKind::insets;
    content_padding.category = "Layout";
    content_padding.description =
        "Insets between the button frame and its image/text content.";
    content_padding.default_value = BindingValue{
        Insets{6.0, 4.0, 6.0, 4.0}};
    content_padding.invalidation_effects = Dirty::measure | Dirty::paint;
    content_padding.bindable = false;
    define_bindable_property({
        std::move(content_padding),
        detail::BindingMemberGetter<ButtonBase, Insets>(
            *this, &ButtonBase::content_padding_),
        detail::DirectPropertySetter<ButtonBase, Insets>(
            *this, &ButtonBase::set_content_padding), {}, {}, {}});
}

bool ButtonBase::should_serialize_image() const noexcept {
    return image_.value != 0U || image_index_ >= 0 || !image_key_.empty();
}

void ButtonBase::set_text(std::string text) {
    require_mutable();
    if (text_ == text) {
        return;
    }
    text_ = std::move(text);
    invalidate(Dirty::measure | Dirty::paint | Dirty::semantics);
    publish_change(text_changed_, text_);
}

void ButtonBase::set_font(FontSpec font) {
    require_mutable();
    if (!valid_font_spec(font)) {
        throw std::invalid_argument("Button font specification is invalid");
    }
    if (font_ == font) {
        return;
    }
    font_ = font;
    invalidate(Dirty::measure | Dirty::paint | Dirty::semantics);
}

void ButtonBase::set_text_line_spacing(double spacing) {
    require_mutable();
    if (!std::isfinite(spacing) || spacing < 0.75 || spacing > 3.0) {
        throw std::invalid_argument(
            "Button text line spacing must be finite and between 0.75 and 3.0");
    }
    if (text_line_spacing_ == spacing) return;
    text_line_spacing_ = spacing;
    invalidate(Dirty::measure | Dirty::paint | Dirty::semantics);
}

void ButtonBase::set_style(BasicControlStyle style) {
    require_mutable();
    if (style_override_ && *style_override_ == style) {
        return;
    }
    style_override_ = std::move(style);
    invalidate(Dirty::paint | Dirty::semantics);
}

const BasicControlStyle& ButtonBase::style() const noexcept {
    return style_override_ ? *style_override_ : effective_theme().basic_style();
}

void ButtonBase::clear_style() {
    require_mutable();
    if (!style_override_) return;
    style_override_.reset();
    invalidate(Dirty::style | Dirty::paint | Dirty::semantics);
}

void ButtonBase::set_visual_recipes(ControlStateRecipes recipes) {
    require_mutable();
    if (!valid_control_state_recipes(recipes)) {
        throw std::invalid_argument("Button visual recipes are invalid");
    }
    if (visual_recipes_override_ && *visual_recipes_override_ == recipes) {
        return;
    }
    visual_recipes_override_ = std::move(recipes);
    invalidate(Dirty::style | Dirty::paint | Dirty::semantics);
}

void ButtonBase::clear_visual_recipes() {
    require_mutable();
    if (!visual_recipes_override_) return;
    visual_recipes_override_.reset();
    invalidate(Dirty::style | Dirty::paint | Dirty::semantics);
}

void ButtonBase::set_connection_topology(
    std::optional<ConnectedControlTopology> topology) {
    require_mutable();
    if (topology && !valid_connected_control_topology(*topology)) {
        throw std::invalid_argument(
            "ButtonBase connection topology requires count >= 2 and index < count");
    }
    if (connection_topology_ == topology) return;
    connection_topology_ = topology;
    invalidate(Dirty::style | Dirty::paint | Dirty::semantics);
}

void ButtonBase::set_image(ImageId image) {
    require_mutable();
    if (image.value != 0U && window() != nullptr &&
        !(*window()).image_resources().find(image)) {
        throw std::invalid_argument("Button image ID is not live in its Window");
    }
    if (image_ == image && image_index_ == -1 && image_key_.empty()) return;
    image_ = image;
    image_index_ = -1;
    image_key_.clear();
    invalidate(Dirty::measure | Dirty::paint | Dirty::semantics);
}

void ButtonBase::clear_image() {
    set_image({});
}

void ButtonBase::set_image_list(std::shared_ptr<ImageList> image_list) {
    require_mutable();
    if (image_list && !(*image_list).is_alive()) {
        throw std::invalid_argument("Button requires a live ImageList");
    }
    if (image_list && window() != nullptr &&
        !(*image_list).belongs_to(*window())) {
        throw std::invalid_argument(
            "Button and ImageList must belong to the same Window");
    }
    if (image_list && image_index_ >= 0 &&
        static_cast<std::size_t>(image_index_) >= (*image_list).count()) {
        throw std::out_of_range(
            "Button image index is outside the assigned ImageList");
    }
    if (image_list_ == image_list) return;
    image_list_changed_.disconnect();
    image_list_ = std::move(image_list);
    if (image_list_) {
        image_list_changed_ = (*image_list_).changed().subscribe(
            *this,
            Delegate<const ImageListChange&>::bind<
                ButtonBase, &ButtonBase::on_image_list_changed>(*this));
    }
    invalidate(Dirty::measure | Dirty::paint | Dirty::semantics);
}

void ButtonBase::on_image_list_changed(const ImageListChange&) {
    if (is_alive()) {
        invalidate(Dirty::measure | Dirty::paint | Dirty::semantics);
    }
}

void ButtonBase::set_image_index(int image_index) {
    require_mutable();
    if (image_index < -1) {
        throw std::out_of_range("Button image index must be -1 or nonnegative");
    }
    if (image_index >= 0 && image_list_ &&
        static_cast<std::size_t>(image_index) >= (*image_list_).count()) {
        throw std::out_of_range("Button image index is outside its ImageList");
    }
    if (image_index_ == image_index && image_.value == 0U && image_key_.empty()) {
        return;
    }
    image_ = {};
    image_key_.clear();
    image_index_ = image_index;
    invalidate(Dirty::measure | Dirty::paint | Dirty::semantics);
}

void ButtonBase::set_image_key(std::string image_key) {
    require_mutable();
    if (image_key.size() > 256U) {
        throw std::invalid_argument("Button image key exceeds 256 bytes");
    }
    if (!image_key.empty() && !validate_utf8(image_key).valid()) {
        throw std::invalid_argument("Button image key must be valid UTF-8");
    }
    if (image_key_ == image_key && image_.value == 0U && image_index_ == -1) {
        return;
    }
    image_ = {};
    image_index_ = -1;
    image_key_ = std::move(image_key);
    invalidate(Dirty::measure | Dirty::paint | Dirty::semantics);
}

void ButtonBase::set_image_alignment(ContentAlignment alignment) {
    require_mutable();
    if (!valid_content_alignment(alignment)) {
        throw std::invalid_argument("invalid Button image alignment");
    }
    if (image_alignment_ == alignment) return;
    image_alignment_ = alignment;
    invalidate(Dirty::paint | Dirty::semantics);
}

void ButtonBase::set_text_alignment(ContentAlignment alignment) {
    require_mutable();
    if (!valid_content_alignment(alignment)) {
        throw std::invalid_argument("invalid Button text alignment");
    }
    if (text_alignment_ == alignment) return;
    text_alignment_ = alignment;
    invalidate(Dirty::paint | Dirty::semantics);
}

void ButtonBase::set_text_image_relation(TextImageRelation relation) {
    require_mutable();
    if (!valid_text_image_relation(relation)) {
        throw std::invalid_argument("invalid Button text/image relation");
    }
    if (text_image_relation_ == relation) return;
    text_image_relation_ = relation;
    invalidate(Dirty::measure | Dirty::paint | Dirty::semantics);
}

void ButtonBase::set_image_gap(double gap) {
    require_mutable();
    if (!std::isfinite(gap) || gap < 0.0 || gap > 64.0) {
        throw std::invalid_argument(
            "Button image gap must be finite and between zero and 64");
    }
    if (image_gap_ == gap) return;
    image_gap_ = gap;
    invalidate(Dirty::measure | Dirty::paint | Dirty::semantics);
}

void ButtonBase::set_content_padding(Insets padding) {
    require_mutable();
    const double values[] = {
        padding.left, padding.top, padding.right, padding.bottom};
    for (const double value : values) {
        if (!std::isfinite(value) || value < 0.0 || value > 128.0) {
            throw std::invalid_argument(
                "Button content padding must be finite and between zero and 128");
        }
    }
    if (content_padding_ == padding) return;
    content_padding_ = padding;
    invalidate(Dirty::measure | Dirty::paint);
}

void ButtonBase::set_use_mnemonic(bool value) {
    require_mutable();
    if (use_mnemonic_ == value) return;
    use_mnemonic_ = value;
    invalidate(Dirty::measure | Dirty::paint | Dirty::semantics);
}

std::string ButtonBase::display_text() const {
    return use_mnemonic_ ? parse_mnemonic_text(text_).display_text : text_;
}

bool ButtonBase::perform_click() {
    if (!prepare_command_activation()) return false;
    on_activate();
    return true;
}

bool ButtonBase::perform_dialog_command() { return perform_click(); }

void ButtonBase::on_attached_to_window() {
    Control::on_attached_to_window();
    if (image_list_ && !(*image_list_).belongs_to(*window())) {
        throw std::logic_error("Button cannot attach to a different ImageList Window");
    }
    if (image_.value != 0U && !(*window()).image_resources().find(image_)) {
        throw std::logic_error("Button cannot attach with a foreign or stale image ID");
    }
}

void ButtonBase::set_expanded_state(std::optional<bool> expanded) {
    require_mutable();
    if (expanded_state_ == expanded) return;
    expanded_state_ = expanded;
    invalidate(Dirty::paint | Dirty::semantics);
}

Size ButtonBase::measure(Size available) {
    const Rect requested = requested_bounds();
    const FontSpec font = effective_font(font_);
    const std::string display = display_text();
    const std::vector<std::string_view> lines = button_text_lines(display);
    double text_width = 0.0;
    for (const std::string_view line : lines) {
        text_width = std::max(text_width, estimated_text_width(line, font));
    }
    const double text_height = static_cast<double>(lines.size()) * font.size * text_line_spacing_;
    Size image_size{};
    if (image_.value != 0U) {
        if (window() != nullptr) {
            if (const std::optional<ImageResourceView> resource = (*window()).image_resources().find(image_)) {
                image_size = {static_cast<double>((*resource).metadata.width),
                              static_cast<double>((*resource).metadata.height)};
            }
        }
    } else if (image_list_ && (*image_list_).is_alive() &&
               (!image_key_.empty() || image_index_ >= 0)) {
        image_size = (*image_list_).image_size();
    }
    const bool has_text = text_width > 0.0;
    const bool has_image = image_size.width > 0.0 && image_size.height > 0.0;
    const double gap = has_text && has_image ? image_gap_ : 0.0;
    double content_width = text_width;
    double content_height = text_height;
    if (has_image) {
        switch (text_image_relation_) {
        case TextImageRelation::image_before_text:
        case TextImageRelation::text_before_image:
            content_width = image_size.width + gap + text_width;
            content_height = std::max(image_size.height, text_height);
            break;
        case TextImageRelation::image_above_text:
        case TextImageRelation::text_above_image:
            content_width = std::max(image_size.width, text_width);
            content_height = image_size.height + gap + text_height;
            break;
        case TextImageRelation::overlay:
            content_width = std::max(image_size.width, text_width);
            content_height = std::max(image_size.height, text_height);
            break;
        }
    }
    const double preferred_width = requested.width > 0.0
        ? requested.width
        : content_width + content_padding_.left + content_padding_.right + 10.0;
    const double preferred_height = requested.height > 0.0
        ? requested.height
        : std::max(24.0, content_height + content_padding_.top +
                                     content_padding_.bottom + 4.0);
    return {std::min(available.width, preferred_width),
            std::min(available.height, preferred_height)};
}

Rect ButtonBase::local_bounds() const noexcept {
    const Rect arranged = committed_arranged_bounds();
    return {0.0, 0.0, arranged.width, arranged.height};
}

void ButtonBase::paint_button_frame(Painter& painter, Rect bounds,
                                    bool default_cue) const {
    const BasicControlStyle& colors = style();
    paint_relief(painter, bounds, colors, pressed_visual());
    if (default_cue && bounds.width > 2.0 && bounds.height > 2.0) {
        painter.stroke_rect({0.5, 0.5, bounds.width - 1.0, bounds.height - 1.0},
                            colors.accent, 1.0);
    }
    if (focus_cue_visible()) {
        paint_focus(painter, bounds, colors.text);
    }
}

bool ButtonBase::focus_cue_visible() const noexcept {
    if (!focused_) return false;
    Window* owner = window();
    return owner == nullptr || (*owner).focused_control().get() != this ||
           (*owner).focus_cue_visible();
}

void ButtonBase::paint_button_text(Painter& painter, Rect bounds,
                                   std::string_view text) const {
    const BasicControlStyle& colors = style();
    const double offset = pressed_visual() ? 1.0 : 0.0;
    paint_button_content(painter, bounds, text,
                         effectively_enabled() ? colors.text
                                               : colors.disabled_text,
                         {offset, offset});
}

ImageListResolution ButtonBase::resolved_button_image(bool selected) const noexcept {
    ImageListResolution result;
    const ImageVisualState state = !effectively_enabled()
        ? ImageVisualState::disabled
        : pressed_visual() ? ImageVisualState::pressed
        : hovered_visual() ? ImageVisualState::hot
        : selected ? ImageVisualState::selected
                   : ImageVisualState::normal;
    result.requested_state = state;
    result.requested_scale = window() ? (*window()).scale() : 1.0;
    if (image_.value != 0U && window() != nullptr) {
        const std::optional<ImageResourceView> resource = (*window()).image_resources().find(image_);
        if (!resource) return result;
        result.image = image_;
        result.source_size = {static_cast<double>((*resource).metadata.width),
                              static_cast<double>((*resource).metadata.height)};
        result.resolved_state = ImageVisualState::normal;
        result.resolved_scale = 1.0;
        return result;
    }
    if (!image_list_ || !(*image_list_).is_alive() || window() == nullptr ||
        !(*image_list_).belongs_to(*window())) return result;
    if (!image_key_.empty()) {
        return (*image_list_).resolve(image_key_, state, (*window()).scale());
    }
    if (image_index_ >= 0) {
        return (*image_list_).resolve(static_cast<std::size_t>(image_index_), state,
                                    (*window()).scale());
    }
    return result;
}

void ButtonBase::paint_button_content(Painter& painter, Rect bounds,
                                      std::string_view text, Color foreground,
                                      Point offset, bool selected,
                                      bool command_alignment) const {
    const double command_leading = command_alignment ? 6.0 : 0.0;
    Rect content{
        bounds.x + content_padding_.left + command_leading,
        bounds.y + content_padding_.top,
        std::max(0.0, bounds.width - content_padding_.left -
                          content_padding_.right - command_leading),
        std::max(0.0, bounds.height - content_padding_.top -
                          content_padding_.bottom)};
    if (content.empty()) return;
    const FontSpec font = effective_font(font_);
    const std::vector<std::string_view> lines = button_text_lines(text);
    double text_width = 0.0;
    double line_height = font.size * text_line_spacing_;
    for (const std::string_view line : lines) {
        const Size measured = painter.measure_text_utf8(line, font);
        text_width = std::max(text_width, measured.width);
        line_height = std::max(line_height, measured.height);
    }
    const Size text_size{text_width, static_cast<double>(lines.size()) * line_height};
    const ImageListResolution image = resolved_button_image(selected);
    Size image_size{};
    if (image) {
        image_size = image_list_ && image_.value == 0U
            ? (*image_list_).image_size() : image.source_size;
    }
    const bool has_text = text_size.width > 0.0 && text_size.height > 0.0;
    const bool has_image = image && image_size.width > 0.0 &&
                           image_size.height > 0.0;
    Rect text_rect{};
    Rect image_rect{};
    const ContentAlignment group_alignment = command_alignment
        ? ContentAlignment::middle_left : text_alignment_;
    const double gap = has_text && has_image ? image_gap_ : 0.0;

    if (has_text && has_image && text_image_relation_ != TextImageRelation::overlay) {
        const bool horizontal =
            text_image_relation_ == TextImageRelation::image_before_text ||
            text_image_relation_ == TextImageRelation::text_before_image;
        const Size group_size = horizontal
            ? Size{image_size.width + gap + text_size.width,
                   std::max(image_size.height, text_size.height)}
            : Size{std::max(image_size.width, text_size.width),
                   image_size.height + gap + text_size.height};
        const Rect group = aligned_rect(content, group_size, group_alignment);
        if (horizontal) {
            const bool image_first =
                text_image_relation_ == TextImageRelation::image_before_text;
            const double image_x = image_first ? group.x
                                               : group.x + text_size.width + gap;
            const double text_x = image_first ? group.x + image_size.width + gap
                                              : group.x;
            image_rect = {image_x,
                          group.y + (group.height - image_size.height) * 0.5,
                          image_size.width, image_size.height};
            text_rect = {text_x,
                         group.y + (group.height - text_size.height) * 0.5,
                         text_size.width, text_size.height};
        } else {
            const bool image_first =
                text_image_relation_ == TextImageRelation::image_above_text;
            const double image_y = image_first ? group.y
                                               : group.y + text_size.height + gap;
            const double text_y = image_first ? group.y + image_size.height + gap
                                              : group.y;
            image_rect = {group.x + (group.width - image_size.width) * 0.5,
                          image_y, image_size.width, image_size.height};
            text_rect = {group.x + (group.width - text_size.width) * 0.5,
                         text_y, text_size.width, text_size.height};
        }
    } else {
        if (has_image) image_rect = aligned_rect(content, image_size, image_alignment_);
        if (has_text) text_rect = aligned_rect(content, text_size, group_alignment);
    }

    if (has_image && !image_rect.empty()) {
        const double opacity = !effectively_enabled() &&
                image.resolved_state != ImageVisualState::disabled
            ? 0.45 : 1.0;
        painter.draw_image(image.image,
                           {image_rect.x + offset.x, image_rect.y + offset.y,
                            image_rect.width, image_rect.height},
                           opacity);
    }
    if (has_text && !text_rect.empty()) {
        for (std::size_t index = 0; index < lines.size(); ++index) {
            const Size measured = painter.measure_text_utf8(lines[index], font);
            double x = text_rect.x;
            if (text_alignment_ == ContentAlignment::top_center ||
                text_alignment_ == ContentAlignment::middle_center ||
                text_alignment_ == ContentAlignment::bottom_center) {
                x += (text_rect.width - measured.width) * 0.5;
            } else if (text_alignment_ == ContentAlignment::top_right ||
                       text_alignment_ == ContentAlignment::middle_right ||
                       text_alignment_ == ContentAlignment::bottom_right) {
                x += text_rect.width - measured.width;
            }
            painter.draw_text_utf8(
                {x + offset.x, text_rect.y + offset.y + static_cast<double>(index) * line_height +
                    std::max(font.size, line_height * 0.82)},
                lines[index], font, foreground);
        }
    }
}

void ButtonBase::paint_themed_button(Painter& painter, Rect bounds,
                                     ControlVisualRole role,
                                     bool default_cue,
                                     bool command_alignment,
                                     bool selected) const {
    const ControlVisualContext context = visual_context(
        hovered_, pressed_visual(), selected, focus_cue_visible(), default_cue);
    const ControlVisualRecipe& recipe = resolve_visual_recipe(role, context);
    const Rect visual_bounds{
        bounds.x + recipe.visual_offset.x,
        bounds.y + recipe.visual_offset.y,
        bounds.width,
        bounds.height,
    };
    paint_connected_surface_material(
        painter, visual_bounds, recipe.material, connection_topology_);
    ControlVisualRecipe cue_recipe = recipe;
    // Connection shapes the physical stock only. Focus/default ownership is
    // still per control, so use a square per-segment cue instead of implying
    // that the whole joined instrument owns one focus identity.
    if (connection_topology_) cue_recipe.material.corner_radius = 0.0;
    paint_theme_cues(painter, visual_bounds, cue_recipe, context);
    const Point offset = context.surface == ControlSurfaceState::pressed
        ? recipe.pressed_content_offset : Point{};
    const std::string display = display_text();
    paint_button_content(painter, visual_bounds, display, recipe.text, offset, selected,
                         command_alignment);
}

const ControlVisualRecipe& ButtonBase::resolve_visual_recipe(
    ControlVisualRole role, ControlVisualContext context) const noexcept {
    if (visual_recipes_override_ && !context.high_contrast) {
        return (*visual_recipes_override_).resolve(context.surface);
    }
    return effective_theme().resolve(role, context);
}

Insets ButtonBase::resolved_visual_outsets(
    ControlVisualRole role, ControlVisualContext context) const noexcept {
    const ControlVisualRecipe& recipe = resolve_visual_recipe(role, context);
    Insets result = connected_surface_visual_outsets(
        recipe.material, connection_topology_);
    double cue_extent = 0.0;
    if (context.focused && recipe.authored_focus_outline &&
        recipe.focus_width > 0.0) {
        cue_extent = std::max(0.0, recipe.focus_offset + recipe.focus_width);
    }
    result.left = std::max(
        0.0, std::max(result.left, cue_extent) - recipe.visual_offset.x);
    result.top = std::max(
        0.0, std::max(result.top, cue_extent) - recipe.visual_offset.y);
    result.right = std::max(
        0.0, std::max(result.right, cue_extent) + recipe.visual_offset.x);
    result.bottom = std::max(
        0.0, std::max(result.bottom, cue_extent) + recipe.visual_offset.y);
    return result;
}

void ButtonBase::on_paint(Painter& painter, Rect) {
    const Rect bounds = local_bounds();
    const std::string display = display_text();
    if (has_style_override()) {
        paint_button_frame(painter, bounds, false);
        paint_button_text(painter, bounds, display);
    } else {
        paint_themed_button(painter, bounds, ControlVisualRole::button, false);
    }
}

Insets ButtonBase::visual_outsets() const noexcept {
    if (has_style_override()) return {};
    const ControlVisualContext context = visual_context(
        hovered_, pressed_visual(), false, focus_cue_visible(), false);
    return resolved_visual_outsets(ControlVisualRole::button, context);
}

void ButtonBase::on_pointer(PointerEvent& event) {
    if (event.action == PointerAction::enter || event.action == PointerAction::leave) {
        const bool next = event.action == PointerAction::enter;
        if (hovered_ != next) {
            hovered_ = next;
            invalidate(Dirty::style | Dirty::paint);
        }
    } else if (event.button == PointerButton::primary &&
               event.action == PointerAction::down) {
        pointer_engaged_ = true;
        pointer_pressed_ = true;
        invalidate(Dirty::paint);
        event.handled = true;
    } else if (event.action == PointerAction::move && pointer_engaged_) {
        const bool inside = absolute_bounds().contains(event.position);
        if (pointer_pressed_ != inside) {
            pointer_pressed_ = inside;
            invalidate(Dirty::paint);
        }
        event.handled = true;
    } else if (event.button == PointerButton::primary &&
               event.action == PointerAction::up && pointer_engaged_) {
        pointer_engaged_ = false;
        pointer_pressed_ = false;
        invalidate(Dirty::paint);
        event.handled = true;
    }
}

void ButtonBase::on_key(KeyEvent& event) {
    const bool activation_key = event.physical_key == PhysicalKey::space ||
                                event.physical_key == PhysicalKey::enter;
    if (!activation_key) {
        return;
    }
    if (event.action == KeyAction::down && !event.repeat && !keyboard_pressed_) {
        keyboard_key_ = event.physical_key;
        keyboard_pressed_ = true;
        invalidate(Dirty::paint);
        event.handled = true;
    } else if (event.action == KeyAction::up && keyboard_pressed_ &&
               keyboard_key_ == event.physical_key) {
        keyboard_key_ = 0;
        keyboard_pressed_ = false;
        invalidate(Dirty::paint);
        event.handled = true;
        on_activate();
    }
}

bool ButtonBase::mnemonic_matches(char32_t character) const noexcept {
    return use_mnemonic_ && is_mnemonic(character, text_);
}

bool ButtonBase::process_mnemonic_self(char32_t character) {
    if (!mnemonic_matches(character)) return false;
    // WinForms treats a recognized mnemonic as owned even when validation
    // prevents the resulting command from firing.
    static_cast<void>(perform_click());
    return true;
}

void ButtonBase::on_focus_changed(bool focused) {
    focused_ = focused;
    if (!focused) {
        keyboard_key_ = 0;
        keyboard_pressed_ = false;
    }
    invalidate(Dirty::paint | Dirty::semantics);
}

void ButtonBase::on_activate() {
    clicked_.emit(*this);
}

SemanticDescriptor ButtonBase::semantic_descriptor() const {
    SemanticDescriptor descriptor;
    descriptor.role = SemanticRole::button;
    descriptor.name = accessible_name().empty() ? display_text() : accessible_name();
    descriptor.description = accessible_description();
    descriptor.actions = {SemanticAction::focus, SemanticAction::press};
    if (visual_status() == ControlVisualStatus::pending) {
        descriptor.states |= SemanticState::busy;
    } else if (visual_status() == ControlVisualStatus::invalid) {
        descriptor.states |= SemanticState::invalid;
    }
    if (expanded_state_) {
        if (*expanded_state_) descriptor.states |= SemanticState::expanded;
        descriptor.actions.push_back(*expanded_state_ ? SemanticAction::collapse
                                                      : SemanticAction::expand);
    }
    descriptor.exposed = true;
    return descriptor;
}

bool ButtonBase::on_semantic_action(SemanticAction action, std::string_view value) {
    if (action == SemanticAction::press) {
        return perform_click();
    }
    if (expanded_state_ &&
        ((action == SemanticAction::expand && !*expanded_state_) ||
         (action == SemanticAction::collapse && *expanded_state_))) {
        on_activate();
        return true;
    }
    return Control::on_semantic_action(action, value);
}

} // namespace gui_forms
