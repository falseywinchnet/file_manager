#include "gui_forms/gui_forms.hpp"

#include <algorithm>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string_view>
#include <vector>

namespace {

using namespace gui_forms;

void require(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}

Color first_fill(const ControlVisualRecipe& recipe) {
    const MaterialFillLayer& fill = recipe.material.fills.front();
    return fill.kind == MaterialFillKind::solid ? fill.color
                                                : fill.stops.front().color;
}

class ThemePainter final : public Painter {
public:
    void save() override {}
    void restore() override {}
    void translate(Point) override {}
    void clip_rect(Rect) override {}
    void clip_rounded_rect(Rect, double) override {}
    void fill_rect(Rect, Color color) override {
        last_fill = color;
        fills.push_back(color);
    }
    void fill_rounded_rect(Rect, double, Color color) override {
        last_fill = color;
        fills.push_back(color);
    }
    void stroke_rect(Rect, Color, double) override {}
    void stroke_rounded_rect(Rect, double, Color color, double) override {
        last_stroke = color;
    }
    void fill_linear_gradient(Rect, Point, Point,
                              std::span<const GradientStop> stops) override {
        ++gradients;
        first_gradient = stops.front().color;
        last_gradient = stops.back().color;
    }
    void fill_radial_gradient(Rect, Point, Size,
                              std::span<const GradientStop>) override {}
    void draw_box_shadow(Rect, double, Point, double, double, Color) override {}
    void draw_line(Point, Point, Color, double) override {}
    void draw_text_utf8(Point, std::string_view, FontSpec, Color color) override {
        text = color;
    }
    void draw_image(ImageId, Rect, double) override {}

    unsigned gradients{};
    Color first_gradient{};
    Color last_gradient{};
    Color last_fill{};
    Color last_stroke{};
    Color text{};
    std::vector<Color> fills;
};

bool contains_color(const ThemePainter& painter, Color color) {
    return std::find(painter.fills.begin(), painter.fills.end(), color) !=
           painter.fills.end();
}

class ObserveCommittedTheme final {
public:
    explicit ObserveCommittedTheme(unsigned& changes) : changes_(changes) {}

    void operator()(const Theme& theme) const {
        require(theme.id() == "test-atomic",
                "theme event must observe the committed replacement");
        ++changes_;
    }

private:
    unsigned& changes_;
};

void set_solid_recipe(ThemeDefinition& definition, ControlVisualRole role,
                      ControlSurfaceState state, bool selected, Color color) {
    ControlRoleRecipes& recipes =
        definition.roles[static_cast<std::size_t>(role)];
    ControlVisualRecipe& recipe =
        (selected ? recipes.selected : recipes.ordinary)
            [static_cast<std::size_t>(state)];
    recipe.material.fills = {MaterialFillLayer::solid(color)};
}

void test_default_theme_has_complete_state_matrix() {
    const std::shared_ptr<const Theme> theme = default_theme();
    require(theme && (*theme).id() == "windows-professional",
            "default theme must have stable professional identity");
    for (std::size_t role = 0; role < control_visual_role_count; ++role) {
        for (std::size_t state = 0; state < control_surface_state_count; ++state) {
            for (bool selected : {false, true}) {
                for (bool high_contrast : {false, true}) {
                    ControlVisualContext context;
                    context.surface = static_cast<ControlSurfaceState>(state);
                    context.selected = selected;
                    context.high_contrast = high_contrast;
                    const ControlVisualRecipe& recipe = (*theme).resolve(
                        static_cast<ControlVisualRole>(role), context);
                    require(valid_surface_material(recipe.material),
                            "every role/state combination must own a valid material");
                }
            }
        }
    }
    ControlVisualContext normal;
    ControlVisualContext hot;
    hot.surface = ControlSurfaceState::hot;
    require(first_fill((*theme).resolve(ControlVisualRole::button, normal)) !=
                first_fill((*theme).resolve(ControlVisualRole::button, hot)),
            "button hot state must be visually distinct from normal");
    const ThemeStructureTokens& structure = (*theme).structure();
    require(structure.spacing.micro <= structure.spacing.section &&
                structure.geometry.splitter_width <=
                    structure.geometry.splitter_hit_width &&
                structure.typography.control.role == FontRole::control &&
                structure.typography.field.role == FontRole::content &&
                structure.motion.quick <= structure.motion.standard,
            "default theme must expose ordered renderer-neutral structural tokens");
}

void test_structural_tokens_are_validated_atomically() {
    ThemeDefinition invalid = windows_professional_theme_definition();
    invalid.id = "invalid-structure";
    invalid.structure.spacing.large = invalid.structure.spacing.xsmall;
    bool rejected{};
    try {
        static_cast<void>(Theme::create(invalid));
    } catch (const std::invalid_argument&) {
        rejected = true;
    }
    require(rejected,
            "unordered structural spacing tokens must be rejected");

    invalid = windows_professional_theme_definition();
    invalid.structure.typography.title.size =
        std::numeric_limits<double>::quiet_NaN();
    rejected = false;
    try {
        static_cast<void>(Theme::create(std::move(invalid)));
    } catch (const std::invalid_argument&) {
        rejected = true;
    }
    require(rejected,
            "invalid structural typography must be rejected before publication");
}

void test_window_theme_replacement_and_control_inheritance_are_atomic() {
    std::shared_ptr<gui_forms::Panel> root = make_control<Panel>(StableId("theme.root"));
    std::shared_ptr<gui_forms::Button> child = make_control<Button>(StableId("theme.child"), "Apply");
    (*child).set_requested_bounds({20.0, 20.0, 90.0, 28.0});
    (*root).add_child(child);
    Window window(root, {160.0, 80.0});
    window.perform_layout();
    static_cast<void>(window.take_damage());

    ThemeDefinition custom_definition = windows_professional_theme_definition();
    custom_definition.id = "test-atomic";
    ControlVisualRecipe& custom_normal = custom_definition
        .roles[static_cast<std::size_t>(ControlVisualRole::button)]
        .ordinary[static_cast<std::size_t>(ControlSurfaceState::normal)];
    custom_normal.material.fills = {
        MaterialFillLayer::solid(Color::rgba(10, 20, 30))};
    const std::shared_ptr<const Theme> custom =
        Theme::create(std::move(custom_definition));

    unsigned changes{};
    SubscriptionToken token = window.theme_changed().subscribe(
        ObserveCommittedTheme(changes));
    window.set_theme(custom);
    window.set_theme(custom);
    require(changes == 1U && (*child).effective_theme().id() == "test-atomic" &&
                !window.take_damage().empty(),
            "window replacement must be atomic, silent on identity, and inherited");

    const std::shared_ptr<const Theme> local = default_theme();
    (*root).set_theme_override(local);
    require((*child).effective_theme().id() == "windows-professional",
            "ancestor override must be inherited by descendants");
    (*root).clear_theme_override();
    require((*child).effective_theme().id() == "test-atomic",
            "clearing override must restore window inheritance");

    bool rejected{};
    try {
        window.set_theme({});
    } catch (const std::invalid_argument&) {
        rejected = true;
    }
    require(rejected && window.theme().id() == "test-atomic" && changes == 1U,
            "null replacement must fail without changing the active theme");
}

void test_button_routes_hover_and_status_through_theme_recipes() {
    std::shared_ptr<gui_forms::Panel> root = make_control<Panel>(StableId("theme.states.root"));
    std::shared_ptr<gui_forms::Button> button = make_control<Button>(StableId("theme.states.button"), "Run");
    (*button).set_requested_bounds({20.0, 20.0, 100.0, 30.0});
    (*button).set_default_button(true);
    (*root).add_child(button);
    Window window(root, {160.0, 80.0});
    window.perform_layout();
    static_cast<void>(window.take_damage());

    ThemePainter painter;
    window.paint(painter, {0.0, 0.0, 160.0, 80.0});
    const Color normal = painter.first_gradient;

    PointerEvent move;
    move.action = PointerAction::move;
    move.position = {30.0, 30.0};
    static_cast<void>(window.dispatch_pointer(move));
    require((*button).hovered_visual(),
            "window hover routing must update themed button state");
    window.paint(painter, window.take_damage().bounds());
    require(painter.first_gradient != normal,
            "hover must select the hot theme recipe");

    (*button).set_visual_status(ControlVisualStatus::invalid);
    const ControlVisualContext invalid = (*button).visual_context(
        true, false, false, false, true);
    require(invalid.surface == ControlSurfaceState::invalid,
            "invalid state must take precedence over hover");
    (*button).set_enabled(false);
    require((*button).visual_context(true).surface == ControlSurfaceState::disabled,
            "disabled state must take precedence over validation state");
    (*button).set_enabled(true);
    (*button).set_visual_status(ControlVisualStatus::normal);
    window.set_active(false);
    require((*button).visual_context().surface == ControlSurfaceState::deactivated,
            "inactive windows must resolve deactivated recipes");

    PresentationSettings settings = window.presentation_settings();
    settings.high_contrast = true;
    window.set_presentation_settings(settings);
    const ControlVisualContext high = (*button).visual_context();
    require(high.high_contrast,
            "presentation accommodation must select high-contrast recipes");
}

void test_editor_selection_menu_and_progress_roles_reach_stock_controls() {
    constexpr Color editor_color = Color::rgba(17, 31, 47);
    constexpr Color selection_color = Color::rgba(53, 79, 101);
    constexpr Color menu_color = Color::rgba(71, 97, 119);
    constexpr Color progress_track = Color::rgba(89, 113, 137);
    constexpr Color progress_fill = Color::rgba(107, 131, 157);
    ThemeDefinition definition = windows_professional_theme_definition();
    definition.id = "stock-role-routing";
    set_solid_recipe(definition, ControlVisualRole::editor,
                     ControlSurfaceState::normal, false, editor_color);
    set_solid_recipe(definition, ControlVisualRole::selection,
                     ControlSurfaceState::normal, true, selection_color);
    set_solid_recipe(definition, ControlVisualRole::menu_item,
                     ControlSurfaceState::hot, false, menu_color);
    set_solid_recipe(definition, ControlVisualRole::progress,
                     ControlSurfaceState::normal, false, progress_track);
    set_solid_recipe(definition, ControlVisualRole::progress,
                     ControlSurfaceState::normal, true, progress_fill);

    std::shared_ptr<gui_forms::Panel> root = make_control<Panel>(StableId("theme.stock.root"));
    std::shared_ptr<gui_forms::TextBox> editor = make_control<TextBox>(StableId("theme.stock.editor"), "alpha");
    (*editor).set_requested_bounds({8.0, 8.0, 140.0, 28.0});
    std::shared_ptr<gui_forms::ListBox> list = make_control<ListBox>(StableId("theme.stock.list"));
    (*list).set_requested_bounds({8.0, 42.0, 140.0, 58.0});
    (*list).set_items({"one", "two"});
    (*list).select_index(0U);
    std::shared_ptr<gui_forms::ComboBox> combo = make_control<ComboBox>(StableId("theme.stock.combo"));
    (*combo).set_requested_bounds({156.0, 8.0, 136.0, 28.0});
    (*combo).set_items({"one", "two"});
    (*combo).set_selected_index(0U);
    std::shared_ptr<gui_forms::ProgressBar> progress = make_control<ProgressBar>(StableId("theme.stock.progress"));
    (*progress).set_requested_bounds({156.0, 46.0, 136.0, 20.0});
    (*progress).set_value(50.0);
    (*progress).set_visual_style(ProgressBarVisualStyle::continuous);
    std::shared_ptr<gui_forms::MenuStrip> menu = make_control<MenuStrip>(StableId("theme.stock.menu"));
    (*menu).set_requested_bounds({8.0, 108.0, 284.0, 28.0});
    std::shared_ptr<gui_forms::Command> menu_command = std::make_shared<Command>("theme.stock.open", "Open");
    (*menu).set_items({{"file", "File",
                      {{"open", MenuItemKind::command, menu_command, "Open", {}}}}});
    (*root).add_child(editor);
    (*root).add_child(list);
    (*root).add_child(combo);
    (*root).add_child(progress);
    (*root).add_child(menu);
    Window window(root, {300.0, 144.0});
    window.set_theme(Theme::create(std::move(definition)));
    window.perform_layout();
    PointerEvent hover{PointerAction::move, PointerButton::none,
                       {20.0, 120.0}};
    static_cast<void>(window.dispatch_pointer(hover));

    ThemePainter painter;
    window.paint(painter, {0.0, 0.0, 300.0, 144.0});
    require(contains_color(painter, editor_color) &&
                contains_color(painter, selection_color) &&
                contains_color(painter, menu_color) &&
                contains_color(painter, progress_track) &&
                contains_color(painter, progress_fill),
            "editor, selection, menu-item, and progress recipes must reach stock paint");
}

} // namespace

int main() {
    try {
        test_default_theme_has_complete_state_matrix();
        test_structural_tokens_are_validated_atomically();
        test_window_theme_replacement_and_control_inheritance_are_atomic();
        test_button_routes_hover_and_status_through_theme_recipes();
        test_editor_selection_menu_and_progress_roles_reach_stock_controls();
        std::cout << "gui_forms_theme_tests: all tests passed\n";
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        std::cerr << "gui_forms_theme_tests: " << error.what() << '\n';
        return EXIT_FAILURE;
    }
}
