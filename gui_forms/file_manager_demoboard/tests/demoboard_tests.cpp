#include "file_manager_demoboard/demoboard.hpp"
#include "file_manager_demoboard/fixture_model.hpp"
#include "gui_forms/gui_forms.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string_view>

namespace {

class NullPainter final : public gui_forms::Painter {
public:
    void save() override {}
    void restore() override {}
    void translate(gui_forms::Point) override {}
    void clip_rect(gui_forms::Rect) override {}
    void fill_rect(gui_forms::Rect, gui_forms::Color) override {}
    void stroke_rect(gui_forms::Rect, gui_forms::Color, double) override {}
    void draw_line(gui_forms::Point, gui_forms::Point, gui_forms::Color,
                   double) override {}
    void draw_text_utf8(gui_forms::Point, std::string_view,
                        gui_forms::FontSpec, gui_forms::Color) override {}
    void draw_image(gui_forms::ImageId, gui_forms::Rect, double) override {}
};

void require(bool condition, std::string_view message) {
    if (!condition) {
        std::cerr << message << '\n';
        std::exit(1);
    }
}

void require_bounds(gui_forms::Window& window, std::string_view id,
                    gui_forms::Rect expected) {
    const auto control = window.find(id);
    require(control != nullptr, std::string("missing control: ") + std::string(id));
    const gui_forms::Rect actual = control->committed_arranged_bounds();
    const auto close = [](double left, double right) {
        return std::abs(left - right) < 0.01;
    };
    if (!(close(actual.x, expected.x) && close(actual.y, expected.y) &&
          close(actual.width, expected.width) && close(actual.height, expected.height))) {
        std::cerr << id << " bounds were " << actual.x << ',' << actual.y << ','
                  << actual.width << ',' << actual.height << " expected "
                  << expected.x << ',' << expected.y << ',' << expected.width
                  << ',' << expected.height << '\n';
        std::exit(1);
    }
}

void print_paint_dirty(const gui_forms::Control::Ptr& control) {
    if (gui_forms::has_dirty(control->dirty(), gui_forms::Dirty::paint) ||
        gui_forms::has_dirty(control->subtree_dirty(), gui_forms::Dirty::paint)) {
        std::cerr << "paint-dirty " << control->stable_id().value() << '\n';
    }
    for (const auto& child : control->children()) print_paint_dirty(child);
}

bool semantic_contains(const std::vector<gui_forms::SemanticNode>& nodes,
                       std::string_view stable_id) {
    for (const auto& node : nodes) {
        if (node.stable_id == stable_id || semantic_contains(node.children, stable_id)) {
            return true;
        }
    }
    return false;
}

const gui_forms::SemanticNode* find_semantic(
    const std::vector<gui_forms::SemanticNode>& nodes,
    std::string_view stable_id) {
    for (const auto& node : nodes) {
        if (node.stable_id == stable_id) return &node;
        if (const auto* found = find_semantic(node.children, stable_id)) return found;
    }
    return nullptr;
}

gui_forms::Point virtual_item_center(const gui_forms::ObjectView& view,
                                     std::string_view stable_id) {
    const auto nodes = view.semantic_virtual_children();
    const auto found = std::find_if(nodes.begin(), nodes.end(),
        [stable_id](const gui_forms::SemanticNode& node) {
            return node.stable_id == stable_id;
        });
    require(found != nodes.end(), "virtual item required by interaction is unrealized");
    return {found->bounds.x + found->bounds.width * .5,
            found->bounds.y + found->bounds.height * .5};
}

void secondary_click(gui_forms::Window& window, gui_forms::Point point) {
    require(window.dispatch_pointer({gui_forms::PointerAction::down,
                                     gui_forms::PointerButton::secondary, point}) &&
                window.dispatch_pointer({gui_forms::PointerAction::up,
                                         gui_forms::PointerButton::secondary, point}),
            "secondary click must be handled by the object collection");
}

} // namespace

int main() {
    using file_manager_demoboard::FixtureCatalogue;
    const auto& catalogue = FixtureCatalogue::instance();
    require(catalogue.id() == "file-manager-demoboard-001", "catalogue ID drifted");
    require(catalogue.generation() == 86, "catalogue generation drifted");
    require(catalogue.project_objects().size() == 14, "project object fixture drifted");

    auto product = file_manager_demoboard::make_product_window();
    require_bounds(*product, "fm.title.identity", {0,0,1450,40});
    require_bounds(*product, "fm.ribbon.tabs", {0,40,1450,23});
    require_bounds(*product, "fm.ribbon.shelf", {0,63,1450,66});
    require_bounds(*product, "fm.navigation", {0,129,1450,40});
    require_bounds(*product, "fm.workspace", {0,169,1450,657});
    require_bounds(*product, "fm.status", {0,826,1450,24});
    require(product->find("fm.tree.view") != nullptr, "tree projection is absent");
    require(product->find("fm.selection.pane") != nullptr,
            "selection projection is absent");
    const auto semantic = product->semantic_snapshot();
    require(semantic.node_count != 0 && !semantic.roots.empty(),
            "semantic snapshot is empty");
    require(semantic_contains(semantic.roots, "fm.object.obj-facade-study"),
            "default selected virtual fixture object is absent from semantics");
    require(product->find("fm.title.material")->absolute_bounds() ==
                gui_forms::Rect{0,0,1450,40},
            "title material is not arranged in the title band");

    const auto terminal = std::dynamic_pointer_cast<gui_forms::Button>(
        product->find("fm.path.terminal"));
    require(terminal && product->request_focus(terminal),
            "path terminal must be a focusable retained popup owner");
    const gui_forms::Rect terminal_bounds = terminal->absolute_bounds();
    const gui_forms::Point terminal_center{
        terminal_bounds.x + terminal_bounds.width * .5,
        terminal_bounds.y + terminal_bounds.height * .5};
    require(product->dispatch_pointer({gui_forms::PointerAction::down,
                                       gui_forms::PointerButton::primary,
                                       terminal_center}) &&
                product->dispatch_pointer({gui_forms::PointerAction::up,
                                           gui_forms::PointerButton::primary,
                                           terminal_center}),
            "path terminal pointer activation must route through retained input");
    product->perform_layout();
    const auto matrix = product->find("fm.path.matrix");
    const auto matrix_layer = std::dynamic_pointer_cast<gui_forms::AnchoredPopupLayer>(
        product->find("fm.path.matrix.layer"));
    require(matrix && matrix_layer && terminal->expanded_state() == true &&
                matrix->absolute_bounds() == gui_forms::Rect{111,169,860,424} &&
                product->focused_control() ==
                    product->find("fm.path.matrix.current.tail"),
            "./ must open one edge-bounded matrix and contain focus at its open tail");
    const char* recent_ids[] = {
        "orchard-study", "legal-2026", "archive-material-library",
        "pictures-scans", "reference-materials"};
    for (const char* id : recent_ids) {
        require(product->find(std::string("fm.path.matrix.recent.") + id) != nullptr,
                "path matrix must expose all five exact recent full stacks");
    }
    require(product->find("fm.path.matrix.recent.sixth") == nullptr &&
                product->find("fm.path.matrix.current.segment.macintosh-hd") &&
                product->find("fm.path.matrix.current.segment.projects"),
            "path matrix must retain one full current drive stack and exactly five recents");

    std::dynamic_pointer_cast<gui_forms::Button>(
        product->find("fm.path.matrix.current.tail"))->perform_click();
    static_cast<void>(product->drain_posted_work());
    product->perform_layout();
    auto path_editor = std::dynamic_pointer_cast<gui_forms::TextBox>(
        product->find("fm.path.matrix.editor"));
    auto completions = std::dynamic_pointer_cast<gui_forms::ListBox>(
        product->find("fm.path.matrix.completions"));
    require(path_editor && completions && path_editor->visible() &&
                path_editor->text() == "/Users/quentin/Work/Projects/" &&
                path_editor->selection().empty() &&
                path_editor->selection().caret.value() == path_editor->text().size() &&
                product->focused_control() == path_editor,
            "open tail must become the real public TextBox in place with caret at end");
    path_editor->set_text("$PROJECTS/N");
    const auto completion_dispatch = product->dispatcher_snapshot();
    require(completion_dispatch.pending == 1U &&
                completion_dispatch.cancelled >= 1U,
            "a newer path query must cancel the prior deferred completion generation");
    static_cast<void>(product->drain_posted_work());
    require(completions->items().size() == 1U &&
                completions->items().front() ==
                    "/Users/quentin/Work/Projects/North Shore/" &&
                completions->item_stable_id(0U) ==
                    "fm.path.matrix.completion.north-shore" &&
                semantic_contains(product->semantic_snapshot().roots,
                                  "fm.path.matrix.completion.north-shore"),
            "fixture completion must expand variables and preserve exact option identity");
    require(product->dispatch_key({gui_forms::KeyAction::down,
                                   gui_forms::PhysicalKey::down}) &&
                product->focused_control() == path_editor &&
                product->dispatch_key({gui_forms::KeyAction::down,
                                       gui_forms::PhysicalKey::tab}) &&
                path_editor->text() ==
                    "/Users/quentin/Work/Projects/North Shore/" &&
                product->focused_control() == path_editor,
            "completion navigation and Tab acceptance must leave text focus in the editor");
    require(product->dispatch_key({gui_forms::KeyAction::down,
                                   gui_forms::PhysicalKey::enter}) &&
                !product->find("fm.path.matrix") &&
                terminal->expanded_state() == false &&
                std::dynamic_pointer_cast<gui_forms::Label>(
                    product->find("fm.title.text"))->text().find("North Shore") !=
                    std::string::npos,
            "Enter on a valid completion must close once and navigate atomically");
    std::dynamic_pointer_cast<gui_forms::Button>(
        product->find("fm.nav.back"))->perform_click();

    require(product->request_focus(terminal),
            "terminal must accept focus after valid path navigation");
    terminal->perform_click();
    std::dynamic_pointer_cast<gui_forms::Button>(
        product->find("fm.path.matrix.current.tail"))->perform_click();
    static_cast<void>(product->drain_posted_work());
    path_editor = std::dynamic_pointer_cast<gui_forms::TextBox>(
        product->find("fm.path.matrix.editor"));
    path_editor->set_text("$UNKNOWN/Projects");
    require(product->dispatch_key({gui_forms::KeyAction::down,
                                   gui_forms::PhysicalKey::enter}) &&
                product->find("fm.path.matrix") &&
                std::dynamic_pointer_cast<gui_forms::Label>(
                    product->find("fm.path.matrix.resolution"))
                        ->accessible_description().starts_with("Error:") &&
                std::dynamic_pointer_cast<gui_forms::Label>(
                    product->find("fm.status.authority"))->text().find(
                        "Unknown variable") != std::string::npos,
            "invalid path must remain editing with visible and semantic explanation");
    require(product->dispatch_key({gui_forms::KeyAction::down,
                                   gui_forms::PhysicalKey::escape}) &&
                product->find("fm.path.matrix") &&
                !path_editor->visible() &&
                product->focused_control() ==
                    product->find("fm.path.matrix.current.tail") &&
                product->dispatch_key({gui_forms::KeyAction::down,
                                       gui_forms::PhysicalKey::escape}) &&
                !product->find("fm.path.matrix") &&
                product->focused_control() == terminal,
            "Escape must first return to browse, then close and restore terminal focus");

    terminal->perform_click();
    product->perform_layout();
    require(product->dispatch_pointer({gui_forms::PointerAction::down,
                                       gui_forms::PointerButton::primary,
                                       {20.0, 220.0}}) &&
                !product->find("fm.path.matrix") &&
                product->focused_control() == terminal,
            "click-away must close the matrix and restore its owner focus");

    auto capture_product = file_manager_demoboard::make_product_window();
    require(file_manager_demoboard::apply_capture_state(
                *capture_product, "path-matrix-browse") &&
                capture_product->find("fm.path.matrix") &&
                !file_manager_demoboard::apply_capture_state(
                    *capture_product, "not-a-capture-state"),
            "capture-state entrypoint must route through public semantic actions");

    auto search_capture_product = file_manager_demoboard::make_product_window();
    require(file_manager_demoboard::apply_capture_state(
                *search_capture_product, "search-pinned") &&
                search_capture_product->find("fm.search.surface")->visible() &&
                !search_capture_product->find(
                    "fm.workspace.content_selection")->visible(),
            "Search capture state must use the same retained surface transition");

    auto search_product = file_manager_demoboard::make_product_window();
    require(file_manager_demoboard::set_product_surface(*search_product, "search"),
            "public surface transition must admit Search");
    search_product->perform_layout();
    const auto search_results =
        std::dynamic_pointer_cast<gui_forms::CorrespondenceView>(
            search_product->find("fm.search.results"));
    const auto search_surface = search_product->find("fm.search.surface");
    const auto folder_surface = search_product->find(
        "fm.workspace.content_selection");
    require(search_results && search_surface && folder_surface &&
                search_surface->visible() && !folder_surface->visible() &&
                search_results->items().size() == 7U &&
                search_results->children().empty() &&
                search_results->compact_height() == 45.0 &&
                search_results->expanded_height() == 126.0 &&
                search_results->selected_id() ==
                    "fm.result.result-invoice-text" &&
                search_results->pinned_id() ==
                    "fm.result.result-invoice-text" &&
                search_results->expanded("fm.result.result-invoice-text"),
            "Search must expose seven virtual correspondences with one exact default pin");
    const auto search_semantics = search_product->semantic_snapshot();
    require(semantic_contains(search_semantics.roots, "fm.tree.view") &&
                semantic_contains(search_semantics.roots,
                                  "fm.result.result-invoice-text") &&
                semantic_contains(search_semantics.roots,
                                  "fm.result.result-invoice-text.activate") &&
                semantic_contains(search_semantics.roots,
                                  "fm.result.result-invoice-text.percentage") &&
                semantic_contains(search_semantics.roots,
                                  "fm.result.result-invoice-text.metadata") &&
                semantic_contains(search_semantics.roots,
                                  "fm.result.result-invoice-text.excerpt") &&
                semantic_contains(search_semantics.roots,
                                  "fm.result.result-invoice-text.plugins") &&
                !semantic_contains(search_semantics.roots, "fm.selection.pane") &&
                !semantic_contains(search_semantics.roots, "fm.folder.objects"),
            "Search must retain the tree, expose evidence lanes, and remove Folder selection semantics");
    require(search_results->realized_count() <= 7U,
            "Search correspondence realization must remain bounded by its viewport");
    const auto first_bounds = search_results->item_bounds(
        "fm.result.result-invoice-text");
    const auto second_bounds = search_results->item_bounds(
        "fm.result.result-invoice-pdf");
    require(first_bounds && second_bounds && first_bounds->height == 126.0 &&
                second_bounds->height == 45.0 &&
                std::abs(second_bounds->y - (first_bounds->y + first_bounds->height)) <
                    0.01,
            "compact and expanded correspondence geometry must share one stable row stack");
    NullPainter search_painter;
    const gui_forms::DamageRegion search_startup_damage =
        search_product->take_damage();
    search_product->paint(search_painter, search_startup_damage.bounds());
    search_product->reset_activity_metrics();
    require(search_product->take_damage().empty() &&
                !search_product->needs_frame(),
            "settled Search must retain neither idle damage nor an active frame");
    require(search_product->perform_semantic_action(
                "fm.result.result-pages-offline",
                gui_forms::SemanticAction::expand),
            "unavailable Search evidence must remain inspectable");
    search_product->perform_layout();
    const gui_forms::DamageRegion offline_damage = search_product->take_damage();
    const gui_forms::Rect result_damage_bounds = offline_damage.bounds();
    const gui_forms::Rect result_control_bounds = search_results->absolute_bounds();
    const gui_forms::Rect result_status_bounds =
        search_product->find("fm.status.authority")->absolute_bounds();
    const auto contained_by = [](gui_forms::Rect inner, gui_forms::Rect outer) {
        return inner.x >= outer.x && inner.y >= outer.y &&
            inner.x + inner.width <= outer.x + outer.width + .01 &&
            inner.y + inner.height <= outer.y + outer.height + .01;
    };
    const bool damage_contained = !offline_damage.empty() &&
        std::all_of(offline_damage.rectangles().begin(),
                    offline_damage.rectangles().end(),
                    [&](gui_forms::Rect rect) {
                        return contained_by(rect, result_control_bounds) ||
                               contained_by(rect, result_status_bounds);
                    });
    require(damage_contained,
            "correspondence expansion damage must stay inside Search and its factual status cell");
    search_product->paint(search_painter, result_damage_bounds);
    const auto offline_semantics = search_product->semantic_snapshot();
    const auto* offline = find_semantic(
        offline_semantics.roots, "fm.result.result-pages-offline");
    require(offline && gui_forms::has_semantic_state(
                           offline->states, gui_forms::SemanticState::expanded) &&
                offline->description.find("source unavailable") != std::string::npos &&
                semantic_contains(offline->children,
                                  "fm.result.result-pages-offline.excerpt"),
            "offline correspondence must expand with explicit availability evidence");
    require(search_product->perform_semantic_action(
                "fm.result.result-pages-offline",
                gui_forms::SemanticAction::press) &&
                search_surface->visible() &&
                std::dynamic_pointer_cast<gui_forms::Label>(
                    search_product->find("fm.status.authority"))->text().find(
                        "source volume unavailable") != std::string::npos,
            "unavailable result activation must fail in place with an honest reason");
    require(search_product->request_focus(search_results) &&
                search_product->dispatch_key({gui_forms::KeyAction::down,
                                              gui_forms::PhysicalKey::home}) &&
                search_product->dispatch_key({gui_forms::KeyAction::down,
                                              gui_forms::PhysicalKey::down}) &&
                search_results->focused_id() ==
                    "fm.result.result-invoice-pdf" &&
                search_product->dispatch_key({gui_forms::KeyAction::down,
                                              gui_forms::PhysicalKey::space}) &&
                search_results->pinned_id() ==
                    "fm.result.result-invoice-pdf",
            "Search keyboard focus and Space pinning must be independent and deterministic");
    require(search_product->dispatch_key({gui_forms::KeyAction::down,
                                          gui_forms::PhysicalKey::enter}) &&
                folder_surface->visible() && !search_surface->visible() &&
                std::dynamic_pointer_cast<gui_forms::Label>(
                    search_product->find("fm.title.text"))->text().ends_with(
                        "Orchard Study") &&
                std::dynamic_pointer_cast<gui_forms::ObjectView>(
                    search_product->find("fm.folder.objects"))->selected_id() ==
                    "fm.object.search-invoice-pdf" &&
                std::dynamic_pointer_cast<gui_forms::Label>(
                    search_product->find("fm.selection.object_name"))->text() ==
                    "Invoice 0428.pdf",
            "Enter must activate a Search result through normal Folder navigation");

    auto debounced_search = file_manager_demoboard::make_product_window();
    const auto search_editor = std::dynamic_pointer_cast<gui_forms::TextBox>(
        debounced_search->find("fm.search.editor"));
    require(search_editor != nullptr, "Search requires the public navigation TextBox");
    search_editor->set_text("invoice");
    search_editor->set_text("invoice quartz");
    require(!debounced_search->find("fm.search.surface")->visible() &&
                std::dynamic_pointer_cast<gui_forms::Label>(
                    debounced_search->find("fm.status.authority"))->text().find(
                        "Search pending") != std::string::npos,
            "Search edits must remain deferred until the latest debounce generation");
    static_cast<void>(debounced_search->poll_frame_schedule(
        gui_forms::FrameClock::now() + std::chrono::seconds(1)));
    debounced_search->perform_layout();
    require(debounced_search->find("fm.search.surface")->visible() &&
                !debounced_search->find("fm.workspace.content_selection")->visible() &&
                std::dynamic_pointer_cast<gui_forms::Label>(
                    debounced_search->find("fm.status.summary"))->text() ==
                    "5 of 7 results shown · 1 unavailable source" &&
                std::dynamic_pointer_cast<gui_forms::Label>(
                    debounced_search->find("fm.status.authority"))->text() ==
                    "Local navigation ready · result generation 86",
            "only the latest debounced query may project the Search surface");
    debounced_search->resize({1800, 1050});
    debounced_search->perform_layout();
    debounced_search->perform_layout();
    const double wide_search_width =
        debounced_search->find("fm.search.results")->absolute_bounds().width;
    require(wide_search_width > 1500.0,
            "Search correspondence must fill a product window wider than startup");
    debounced_search->resize({480, 360});
    debounced_search->perform_layout();
    debounced_search->perform_layout();
    require(debounced_search->find("fm.search.results")->absolute_bounds().width >
                430.0 &&
                !semantic_contains(debounced_search->semantic_snapshot().roots,
                                   "fm.selection.pane"),
            "compact Search must preserve one useful evidence field without reintroducing Selection");

    auto criteria_product = file_manager_demoboard::make_product_window();
    require(file_manager_demoboard::set_product_surface(
                *criteria_product, "criteria"),
            "public surface transition must admit Criteria");
    criteria_product->perform_layout();
    const auto criteria_rack =
        std::dynamic_pointer_cast<gui_forms::InstrumentRack>(
            criteria_product->find("fm.criteria.console"));
    const auto criteria_objects =
        std::dynamic_pointer_cast<gui_forms::ObjectView>(
            criteria_product->find("fm.criteria.objects"));
    const auto criteria_apply =
        std::dynamic_pointer_cast<gui_forms::Button>(
            criteria_product->find("fm.criteria.apply"));
    const auto criteria_progress =
        std::dynamic_pointer_cast<gui_forms::ProgressBar>(
            criteria_product->find("fm.criteria.progress"));
    require(criteria_rack && criteria_objects && criteria_apply &&
                criteria_progress && criteria_rack->modules().size() == 3U &&
                criteria_objects->items().size() == 31U &&
                criteria_objects->selected_id() ==
                    "fm.object.obj-facade-study" &&
                criteria_product->find("fm.workspace.content_selection")->visible() &&
                !criteria_product->find("fm.folder.objects")->visible() &&
                criteria_product->find("fm.selection.pane")->visible(),
            "Criteria must retain three exact instruments, its virtual objects, and Selection");
    const auto criteria_semantics = criteria_product->semantic_snapshot();
    require(semantic_contains(criteria_semantics.roots,
                              "fm.criteria.module.criteria-kind-images.enable") &&
                semantic_contains(criteria_semantics.roots,
                                  "fm.criteria.module.criteria-modified-2026.operator") &&
                semantic_contains(criteria_semantics.roots,
                                  "fm.criteria.module.criteria-content-facade.value") &&
                semantic_contains(criteria_semantics.roots,
                                  "fm.criteria.module.criteria-content-facade.remove") &&
                semantic_contains(criteria_semantics.roots,
                                  "fm.criteria.apply") &&
                !semantic_contains(criteria_semantics.roots,
                                   "fm.folder.objects"),
            "Criteria controls and staged meaning must be exposed without hidden Folder semantics");
    require(criteria_product->perform_semantic_action(
                "fm.criteria.apply", gui_forms::SemanticAction::press) &&
                criteria_progress->animation_enabled() &&
                !criteria_apply->enabled(),
            "Apply must enter one explicit pending state and reject duplicate activation");
    const auto apply_clock = gui_forms::FrameClock::now() +
                             std::chrono::seconds(1);
    for (int step = 0; step < 4; ++step) {
        static_cast<void>(criteria_product->poll_frame_schedule(
            apply_clock + std::chrono::milliseconds(100 * step)));
    }
    require(!criteria_progress->animation_enabled() &&
                criteria_progress->value() == 100.0 &&
                criteria_objects->items().size() == 5U &&
                criteria_objects->selected_id() ==
                    "fm.object.obj-facade-study" &&
                std::dynamic_pointer_cast<gui_forms::Label>(
                    criteria_product->find("fm.status.summary"))->text() ==
                    "5 objects · 3 criteria applied" &&
                std::dynamic_pointer_cast<gui_forms::Label>(
                    criteria_product->find("fm.criteria.console.title"))->text().find(
                        "0 STAGED") != std::string::npos &&
                std::dynamic_pointer_cast<gui_forms::Label>(
                    criteria_product->find("fm.criteria.results.summary"))->text().starts_with(
                        "5 OBJECTS") &&
                std::dynamic_pointer_cast<gui_forms::Label>(
                    criteria_product->find("fm.status.authority"))->text() ==
                    "Local navigation ready · index 87 current",
            "Apply must complete deterministically while preserving surviving selection");

    auto criteria_edit = file_manager_demoboard::make_product_window();
    static_cast<void>(file_manager_demoboard::set_product_surface(
        *criteria_edit, "criteria"));
    const auto edit_rack = std::dynamic_pointer_cast<gui_forms::InstrumentRack>(
        criteria_edit->find("fm.criteria.console"));
    const auto kind_value = std::dynamic_pointer_cast<gui_forms::ComboBox>(
        criteria_edit->find("fm.criteria.module.criteria-kind-images.value"));
    require(edit_rack && kind_value, "Criteria live editor projection is absent");
    kind_value->set_selected_index(1U);
    require(edit_rack->modules()[0].state ==
                gui_forms::InstrumentModuleState::live &&
                std::dynamic_pointer_cast<gui_forms::Label>(
                    criteria_edit->find("fm.status.authority"))->text().find(
                        "Live criterion committed") != std::string::npos,
            "cheap choice edits must update the live projection immediately");
    require(criteria_edit->perform_semantic_action(
                "fm.criteria.add", gui_forms::SemanticAction::press),
            "bounded Add menu must open through the public semantic action");
    criteria_edit->perform_layout();
    const auto add_button = criteria_edit->find("fm.criteria.add");
    const auto add_popup = criteria_edit->find(
        "fm.criteria.add.menu.popup.panel.0");
    require(add_button && add_popup &&
                add_popup->absolute_bounds().x >=
                    add_button->absolute_bounds().x - 1.0 &&
                add_popup->absolute_bounds().y >=
                    add_button->absolute_bounds().y +
                    add_button->absolute_bounds().height - 1.0 &&
                criteria_edit->dispatch_key({gui_forms::KeyAction::down,
                                             gui_forms::PhysicalKey::enter}) &&
                edit_rack->modules().size() == 4U,
            "bounded Add menu must keyboard-activate a genuine module template");
    const std::string added_id = edit_rack->modules().back().stable_id;
    require(criteria_edit->request_focus(
                criteria_edit->find(added_id + ".remove")) &&
                criteria_edit->perform_semantic_action(
                    added_id + ".remove", gui_forms::SemanticAction::press) &&
                edit_rack->modules().size() == 3U &&
                criteria_edit->focused_control() != nullptr,
            "module removal must mutate the model and transfer focus predictably");

    auto compact_criteria = file_manager_demoboard::make_product_window();
    compact_criteria->resize({540.0, 500.0});
    compact_criteria->set_text_scale(2.25);
    static_cast<void>(file_manager_demoboard::set_product_surface(
        *compact_criteria, "criteria"));
    compact_criteria->perform_layout();
    compact_criteria->perform_layout();
    const auto compact_rack =
        std::dynamic_pointer_cast<gui_forms::InstrumentRack>(
            compact_criteria->find("fm.criteria.console"));
    const auto compact_first = compact_rack->module_bounds(
        "fm.criteria.module.criteria-kind-images");
    const auto compact_third = compact_rack->module_bounds(
        "fm.criteria.module.criteria-content-facade");
    require(compact_first && compact_third &&
                compact_third->y > compact_first->y &&
                compact_rack->content_height() >
                    compact_rack->committed_arranged_bounds().height,
            "Criteria must wrap and remain scroll-reachable under narrow large text");

    auto palette_product = file_manager_demoboard::make_product_window();
    require(file_manager_demoboard::apply_capture_state(
                *palette_product, "palettes"),
            "palette capture state must enter through the public surface selector");
    palette_product->perform_layout();
    const auto palette_surface = palette_product->find("fm.review.palettes");
    require(palette_surface && palette_surface->visible() &&
                !palette_product->find("fm.workspace.content_selection")->visible() &&
                !palette_product->find("fm.search.surface")->visible() &&
                !palette_product->find("fm.criteria.surface")->visible(),
            "palette review must replace the daily content surface without hidden overlap");
    constexpr const char* palette_ids[] = {
        "cobalt", "amethyst", "miami", "orchid", "aqua", "apricot",
        "mulberry", "viridian", "sapphire", "rose", "iris", "phosphor"};
    for (const char* palette_id : palette_ids) {
        const std::string card_id = std::string("fm.palette.") + palette_id;
        const auto card = std::dynamic_pointer_cast<gui_forms::Card>(
            palette_product->find(card_id));
        require(card && card->interactive() &&
                    palette_product->find(card_id + ".title") &&
                    palette_product->find(card_id + ".swatch") &&
                    palette_product->find(card_id + ".state"),
                "every atmosphere must be a public interactive Card with retained sections");
    }
    const auto sapphire_card = std::dynamic_pointer_cast<gui_forms::Card>(
        palette_product->find("fm.palette.sapphire"));
    const auto cobalt_card = std::dynamic_pointer_cast<gui_forms::Card>(
        palette_product->find("fm.palette.cobalt"));
    const auto palette_semantics = palette_product->semantic_snapshot();
    const auto* sapphire_semantic = find_semantic(
        palette_semantics.roots, "fm.palette.sapphire");
    require(sapphire_card && cobalt_card && sapphire_card->selected() &&
                !cobalt_card->selected() && sapphire_semantic &&
                gui_forms::has_semantic_state(
                    sapphire_semantic->states, gui_forms::SemanticState::selected),
            "Sapphire must begin as the visibly and semantically selected recipe");
    require(palette_product->perform_semantic_action(
                "fm.palette.cobalt", gui_forms::SemanticAction::press) &&
                cobalt_card->selected() && !sapphire_card->selected() &&
                std::dynamic_pointer_cast<gui_forms::Label>(
                    palette_product->find("fm.status.summary"))->text().find(
                        "Cobalt atmosphere") != std::string::npos,
            "palette activation must atomically move one selection and publish its status");
    palette_product->resize({1800.0, 1050.0});
    palette_product->perform_layout();
    palette_product->perform_layout();
    require(palette_surface->absolute_bounds().width > 1500.0 &&
                palette_product->find("fm.palette.phosphor")
                        ->absolute_bounds().x +
                        palette_product->find("fm.palette.phosphor")
                            ->absolute_bounds().width <=
                    palette_surface->absolute_bounds().x +
                        palette_surface->absolute_bounds().width,
            "palette cards must reflow inside the grown product surface");
    require(file_manager_demoboard::set_product_surface(
                *palette_product, "folder") &&
                !palette_surface->visible() &&
                palette_product->find("fm.workspace.content_selection")->visible(),
            "leaving a review laboratory must restore the ordinary Folder projection");

    auto dna_product = file_manager_demoboard::make_product_window();
    require(file_manager_demoboard::apply_capture_state(*dna_product, "dna"),
            "DNA capture state must enter through the public surface selector");
    dna_product->perform_layout();
    const auto dna_view = std::dynamic_pointer_cast<gui_forms::MasterDetailView>(
        dna_product->find("fm.review.dna"));
    require(dna_view && dna_view->visible() && dna_view->master() &&
                dna_view->detail() &&
                dna_view->effective_display_mode() ==
                    gui_forms::MasterDetailDisplayMode::side_by_side,
            "DNA must dogfood the public wide MasterDetailView composition");
    constexpr const char* decision_ids[]{
        "surface", "material", "provider", "similarity"};
    for (const char* decision_id : decision_ids) {
        const auto card = std::dynamic_pointer_cast<gui_forms::Card>(
            dna_product->find(std::string("fm.dna.") + decision_id));
        require(card && card->interactive() && card->header() && card->body() &&
                    card->footer(),
                "every DNA decision must be a retained interactive public Card");
    }
    require(dna_product->perform_semantic_action(
                "fm.dna.material", gui_forms::SemanticAction::press) &&
                std::dynamic_pointer_cast<gui_forms::Card>(
                    dna_product->find("fm.dna.material"))->selected() &&
                !std::dynamic_pointer_cast<gui_forms::Card>(
                    dna_product->find("fm.dna.surface"))->selected() &&
                std::dynamic_pointer_cast<gui_forms::Label>(
                    dna_product->find("fm.review.dna.heading"))->text() ==
                    "House material authority",
            "DNA activation must move selection and project the chosen record");
    dna_product->resize({680.0, 650.0});
    dna_product->perform_layout();
    dna_product->perform_layout();
    require(dna_view->effective_display_mode() ==
                gui_forms::MasterDetailDisplayMode::master_only,
            "DNA must collapse to its master navigation under compact width");
    require(dna_product->perform_semantic_action(
                "fm.dna.provider", gui_forms::SemanticAction::press),
            "compact DNA decision must accept semantic activation");
    dna_product->perform_layout();
    require(dna_view->effective_display_mode() ==
                gui_forms::MasterDetailDisplayMode::detail_only,
            "compact DNA activation must navigate to the detail presentation");
    dna_view->show_master();
    dna_product->perform_layout();
    require(dna_view->effective_display_mode() ==
                gui_forms::MasterDetailDisplayMode::master_only,
            "compact DNA detail must retain an explicit route back to master");

    NullPainter painter;
    const gui_forms::DamageRegion startup_damage = product->take_damage();
    require(!startup_damage.empty(), "startup did not publish retained damage");
    product->paint(painter, startup_damage.bounds());
    product->reset_activity_metrics();
    const gui_forms::DamageRegion idle_damage = product->take_damage();
    const auto idle = product->metrics_snapshot();
    if (!idle_damage.empty() || product->needs_frame() || idle.measure_passes != 0 ||
        idle.arrange_passes != 0 || idle.paint_passes != 0 ||
        idle.active_surface_count != 0) {
        std::cerr << "idle damage=" << idle_damage.area()
                  << " frame=" << product->needs_frame()
                  << " measure=" << idle.measure_passes
                  << " arrange=" << idle.arrange_passes
                  << " paint=" << idle.paint_passes
                  << " active=" << idle.active_surface_count << '\n';
        print_paint_dirty(product->root());
        const auto caption_bounds = product->find("fm.tree.caption")->absolute_bounds();
        std::cerr << "caption bounds=" << caption_bounds.x << ',' << caption_bounds.y
                  << ',' << caption_bounds.width << ',' << caption_bounds.height << '\n';
        require(false,
                "quiescent demoboard retained an idle layout, paint, or frame loop");
    }

    const auto objects = std::dynamic_pointer_cast<gui_forms::ObjectView>(
        product->find("fm.folder.objects"));
    const auto tree = std::dynamic_pointer_cast<gui_forms::TreeView>(
        product->find("fm.tree.view"));
    const auto view_command = std::dynamic_pointer_cast<gui_forms::Button>(
        product->find("fm.ribbon.view_mode"));
    const auto sort_command = std::dynamic_pointer_cast<gui_forms::Button>(
        product->find("fm.ribbon.sort"));
    const auto property_list = std::dynamic_pointer_cast<gui_forms::PropertyList>(
        product->find("fm.selection.properties"));
    const auto property_name = std::dynamic_pointer_cast<gui_forms::TextBox>(
        product->find("fm.property.name.editor"));
    const auto property_handler = std::dynamic_pointer_cast<gui_forms::ComboBox>(
        product->find("fm.property.handler.editor"));
    const auto preview_disclosure = std::dynamic_pointer_cast<gui_forms::Button>(
        product->find("fm.selection.preview_disclosure"));
    const auto selection_collapse = std::dynamic_pointer_cast<gui_forms::Button>(
        product->find("fm.selection.collapse"));
    const auto selection_split = std::dynamic_pointer_cast<gui_forms::SplitContainer>(
        product->find("fm.workspace.content_selection"));
    const auto preview = product->find("fm.selection.preview");
    require(objects && tree && view_command && sort_command && property_list &&
                property_name && property_handler && preview_disclosure && preview &&
                selection_collapse && selection_split,
            "D2 public controls are not present in the product tree");

    require(property_list->header_content() ==
                product->find("fm.selection.summary") &&
                property_list->header_height() == 212.0 &&
                property_list->content_height() >
                    property_list->header_height() &&
                property_name->parent() == property_list &&
                property_list->header_content()->parent() == property_list,
            "Selection preview and grouped properties must share one owned scroll plane");
    require(product->request_focus(property_name),
            "session Name property must accept focus");
    property_name->select_all();
    require(product->dispatch_text({"File Manager"}) &&
                product->dispatch_key({gui_forms::KeyAction::down,
                                       gui_forms::PhysicalKey::enter}) &&
                property_name->text() == "Facade Study.png" &&
                property_name->accessible_description().find("already exists") !=
                    std::string::npos &&
                std::dynamic_pointer_cast<gui_forms::Label>(
                    product->find("fm.status.authority"))->text().find(
                    "rename rejected") != std::string::npos,
            "duplicate session rename must roll back with inline accessible validation");
    property_name->select_all();
    require(product->dispatch_text({"Facade Final.png"}) &&
                product->dispatch_key({gui_forms::KeyAction::down,
                                       gui_forms::PhysicalKey::enter}) &&
                property_list->value("fm.property.name") == "Facade Final.png" &&
                property_name->accessible_description().find("Error:") ==
                    std::string::npos &&
                std::find_if(objects->items().begin(), objects->items().end(),
                    [](const gui_forms::ObjectViewItem& item) {
                        return item.stable_id == "fm.object.obj-facade-study" &&
                               item.name == "Facade Final.png";
                    }) != objects->items().end() &&
                std::dynamic_pointer_cast<gui_forms::Label>(
                    product->find("fm.selection.object_name"))->text() ==
                    "Facade Final.png",
            "valid session rename must update one coherent object/property projection");
    property_handler->set_selected_index(1U);
    require(property_list->value("fm.property.handler") == "Image Laboratory" &&
                std::dynamic_pointer_cast<gui_forms::Label>(
                    product->find("fm.status.authority"))->text().find(
                    "handler committed") != std::string::npos,
            "choice property must commit through the shared PropertyList model");
    preview_disclosure->perform_click();
    require(!preview->visible() && property_list->header_height() == 34.0 &&
                preview_disclosure->text() == "▶",
            "preview disclosure must collapse retained header content without replacing it");
    preview_disclosure->perform_click();
    require(preview->visible() && property_list->header_height() == 212.0 &&
                preview_disclosure->text() == "▼" &&
                property_list->header_content() ==
                    product->find("fm.selection.summary"),
            "preview disclosure must restore the same retained header subtree");
    selection_split->set_splitter_distance(0.0);
    product->perform_layout();
    require(selection_split->second_panel()->arranged_bounds().width <= 420.01 &&
                std::abs(property_list->committed_arranged_bounds().width -
                         selection_split->second_panel()->arranged_bounds().width) <
                    0.01,
            "widening Selection must stop at its useful content extent and fill it");
    selection_split->set_splitter_distance(935.0);
    product->perform_layout();
    const double inspector_extent =
        selection_split->second_panel()->arranged_bounds().width;
    selection_collapse->perform_click();
    product->perform_layout();
    require(selection_split->second_collapsed() &&
                !selection_split->second_panel()->visible(),
            "Selection caption affordance must collapse the reusable split pane");
    std::dynamic_pointer_cast<gui_forms::Button>(
        product->find("fm.ribbon.properties"))->perform_click();
    product->perform_layout();
    require(!selection_split->second_collapsed() &&
                selection_split->second_panel()->visible() &&
                std::abs(selection_split->second_panel()->arranged_bounds().width -
                         inspector_extent) < 0.01,
            "Selection-pane command must restore the remembered inspector extent");
    require(product->request_focus(objects) &&
                product->dispatch_key({gui_forms::KeyAction::down,
                                       gui_forms::PhysicalKey::f2}) &&
                product->focused_control() == property_name &&
                std::dynamic_pointer_cast<gui_forms::Label>(
                    product->find("fm.status.authority"))->text().find(
                    "Rename active") != std::string::npos,
            "F2 must route from the object field into the retained Name editor");
    require(product->dispatch_key({gui_forms::KeyAction::down,
                                   gui_forms::PhysicalKey::escape}),
            "Escape must cancel the active property rename editor");
    const auto inspection_semantics = product->semantic_snapshot();
    const auto* inspection_grid = find_semantic(
        inspection_semantics.roots, "fm.selection.properties");
    const auto* inspection_group = find_semantic(
        inspection_semantics.roots, "fm.property.group.editable");
    require(inspection_grid &&
                inspection_grid->role == gui_forms::SemanticRole::property_grid &&
                inspection_group &&
                inspection_group->role == gui_forms::SemanticRole::property_group,
            "Selection properties must expose reusable property grid/group semantics");

    view_command->perform_click();
    require(objects->view_mode() == gui_forms::ObjectViewMode::details &&
                std::dynamic_pointer_cast<gui_forms::Button>(
                    product->find("fm.status.view"))->text() == "Details  ▼" &&
                objects->selected_id() == "fm.object.obj-facade-study",
            "shared view command did not preserve selection across presentations");
    require(objects->on_semantic_child_action(
                "fm.object.obj-file-manager", gui_forms::SemanticAction::select, {}) &&
                std::dynamic_pointer_cast<gui_forms::Label>(
                    product->find("fm.selection.object_name"))->text() == "File Manager" &&
                std::dynamic_pointer_cast<gui_forms::Label>(
                    product->find("fm.status.summary"))->text().find("1.3G") !=
                    std::string::npos,
            "object selection did not update the shared Selection/status projection");
    objects->set_selected_ids({"fm.object.obj-file-manager",
                               "fm.object.obj-facade-study"},
                              "fm.object.obj-file-manager");
    require(objects->selected_ids().size() == 2U &&
                std::dynamic_pointer_cast<gui_forms::Label>(
                    product->find("fm.selection.object_name"))->text() ==
                    "2 objects selected" &&
                std::dynamic_pointer_cast<gui_forms::Label>(
                    product->find("fm.status.summary"))->text().starts_with(
                    "2 selected"),
            "multi-selection did not project one coherent Selection/status state");
    sort_command->perform_click();
    require(objects->items().front().stable_id ==
                "fm.object.obj-delivery-archive" &&
                objects->selected_id() == "fm.object.obj-file-manager" &&
                objects->selected_ids().size() == 2U &&
                sort_command->text() == "Sort Z→A  ▼",
            "shared sort command did not preserve the stable selection set");
    require(product->request_focus(objects),
            "object field must accept focus before context-menu exercise");
    secondary_click(*product,
                    virtual_item_center(*objects, "fm.object.obj-file-manager"));
    require(product->find("fm.context.object.popup.panel.0") != nullptr &&
                product->focus_scope_depth() == 1U &&
                objects->selected_ids().size() == 2U,
            "object context menu did not preserve the existing multiselection");
    require(product->dispatch_key({gui_forms::KeyAction::down,
                                   gui_forms::PhysicalKey::end}) &&
                product->dispatch_key({gui_forms::KeyAction::down,
                                       gui_forms::PhysicalKey::enter}) &&
                product->find("fm.context.object.popup.panel.0") == nullptr &&
                product->focus_scope_depth() == 0U &&
                product->focused_control() == objects &&
                std::dynamic_pointer_cast<gui_forms::Label>(
                    product->find("fm.status.authority"))->text().find(
                    "properties active") != std::string::npos,
            "keyboard context command did not execute and restore collection focus");

    const gui_forms::Rect object_bounds = objects->absolute_bounds();
    secondary_click(*product, {object_bounds.x + object_bounds.width - 6.0,
                               object_bounds.y + object_bounds.height - 6.0});
    require(product->find("fm.context.background.popup.panel.0") != nullptr &&
                objects->selected_ids().empty(),
            "background context must clear object selection and expose its own menu");
    require(product->dispatch_key({gui_forms::KeyAction::down,
                                   gui_forms::PhysicalKey::enter}) &&
                objects->items().size() == 15U &&
                objects->selected_id() == "fm.object.session-new-folder-1" &&
                std::dynamic_pointer_cast<gui_forms::Label>(
                    product->find("fm.selection.object_name"))->text() == "New Folder",
            "background New folder must create and project a session-only fixture");

    secondary_click(*product,
                    virtual_item_center(*objects, "fm.object.session-new-folder-1"));
    require(product->dispatch_key({gui_forms::KeyAction::down,
                                   gui_forms::PhysicalKey::home}) &&
                product->dispatch_key({gui_forms::KeyAction::down,
                                       gui_forms::PhysicalKey::down}) &&
                product->dispatch_key({gui_forms::KeyAction::down,
                                       gui_forms::PhysicalKey::down}) &&
                product->dispatch_key({gui_forms::KeyAction::down,
                                       gui_forms::PhysicalKey::down}) &&
                product->dispatch_key({gui_forms::KeyAction::down,
                                       gui_forms::PhysicalKey::down}) &&
                product->dispatch_key({gui_forms::KeyAction::down,
                                       gui_forms::PhysicalKey::down}) &&
                product->dispatch_key({gui_forms::KeyAction::down,
                                       gui_forms::PhysicalKey::down}) &&
                product->dispatch_key({gui_forms::KeyAction::down,
                                       gui_forms::PhysicalKey::enter}) &&
                objects->items().size() == 14U && objects->selected_ids().empty(),
            "destructive fixture command must hide only the selected session item");
    require(tree->on_semantic_child_action(
                "fm.tree.node.reference", gui_forms::SemanticAction::select, {}) &&
                std::dynamic_pointer_cast<gui_forms::Label>(
                    product->find("fm.status.authority"))->text().find("reference") !=
                    std::string::npos,
            "tree selection did not publish deterministic fixture navigation state");
    require(objects->items().size() == 3U &&
                std::dynamic_pointer_cast<gui_forms::Label>(
                    product->find("fm.title.text"))->text() ==
                    "File Manager  ·  Reference" &&
                std::dynamic_pointer_cast<gui_forms::Button>(
                    product->find("fm.nav.back"))->enabled() &&
                !std::dynamic_pointer_cast<gui_forms::Button>(
                    product->find("fm.nav.forward"))->enabled(),
            "tree navigation did not atomically project location objects and history");
    require(product->request_focus(objects) &&
                product->dispatch_key({gui_forms::KeyAction::down,
                                       gui_forms::PhysicalKey::left,
                                       gui_forms::Modifier::alt}) &&
                objects->items().size() == 14U &&
                std::dynamic_pointer_cast<gui_forms::Label>(
                    product->find("fm.title.text"))->text().ends_with("Projects") &&
                std::dynamic_pointer_cast<gui_forms::Button>(
                    product->find("fm.nav.forward"))->enabled(),
            "Alt+Left did not restore the exact retained Projects history entry");
    require(product->dispatch_key({gui_forms::KeyAction::down,
                                   gui_forms::PhysicalKey::right,
                                   gui_forms::Modifier::alt}) &&
                objects->items().size() == 3U &&
                std::dynamic_pointer_cast<gui_forms::Label>(
                    product->find("fm.title.text"))->text().ends_with("Reference"),
            "Alt+Right did not restore the forward location entry");
    require(product->dispatch_key({gui_forms::KeyAction::down,
                                   gui_forms::PhysicalKey::up,
                                   gui_forms::Modifier::alt}) &&
                objects->items().size() == 3U &&
                objects->items().front().name == "Projects" &&
                std::dynamic_pointer_cast<gui_forms::Label>(
                    product->find("fm.title.text"))->text().ends_with("Work") &&
                !std::dynamic_pointer_cast<gui_forms::Button>(
                    product->find("fm.nav.forward"))->enabled(),
            "Alt+Up did not navigate to the parent and clear the forward branch");

    const auto menu = std::dynamic_pointer_cast<gui_forms::MenuStrip>(
        product->find("fm.ribbon.tabs"));
    require(menu && product->perform_semantic_action(
                "fm.menu.help", gui_forms::SemanticAction::expand) &&
                product->find("fm.ribbon.tabs.menu.popup.panel.0") != nullptr &&
                product->dispatch_key({gui_forms::KeyAction::down,
                                       gui_forms::PhysicalKey::enter}) &&
                std::dynamic_pointer_cast<gui_forms::Label>(
                    product->find("fm.status.authority"))->text().find(
                    "Native GUI.Forms dogfood") != std::string::npos,
            "application menu semantic path did not execute its shared command");

    auto responsive = file_manager_demoboard::make_product_window();
    const auto responsive_outer =
        std::dynamic_pointer_cast<gui_forms::SplitContainer>(
            responsive->find("fm.workspace"));
    const auto responsive_inner =
        std::dynamic_pointer_cast<gui_forms::SplitContainer>(
            responsive->find("fm.workspace.content_selection"));
    const auto responsive_objects = responsive->find("fm.folder.objects");
    const auto title_band = responsive->find("fm.title.identity");
    const auto ribbon_band = responsive->find("fm.ribbon.shelf");
    const auto status_band = responsive->find("fm.status");
    require(responsive_outer && responsive_inner && responsive_objects &&
                title_band && ribbon_band && status_band,
            "responsive fixture requires both split policies and shell bands");
    const auto resize_twice = [&responsive](gui_forms::Size size) {
        responsive->resize(size);
        responsive->perform_layout();
        responsive->perform_layout();
    };
    resize_twice({1200, 760});
    require(!responsive_outer->first_collapsed() &&
                !responsive_inner->second_collapsed() && ribbon_band->visible(),
            "common size must retain both panes and the full command shelf");
    resize_twice({960, 680});
    require(!responsive_outer->first_collapsed() &&
                responsive_inner->second_collapsed() &&
                responsive_inner->second_collapse_origin() ==
                    gui_forms::SplitCollapseOrigin::automatic_accommodation,
            "narrow size must automatically collapse Selection before the tree");
    resize_twice({720, 520});
    require(!responsive_outer->first_collapsed() &&
                responsive_inner->second_collapsed() && !ribbon_band->visible(),
            "compact size must preserve the tree and move ribbon vocabulary to menus");
    resize_twice({480, 360});
    require(responsive_outer->first_collapsed() &&
                responsive_outer->first_collapse_origin() ==
                    gui_forms::SplitCollapseOrigin::automatic_accommodation &&
                responsive_inner->second_collapsed(),
            "severe size must collapse both secondary panes around one content field");
    resize_twice({300, 240});
    require(responsive_objects->committed_arranged_bounds().width > 250.0 &&
                responsive_objects->committed_arranged_bounds().height > 90.0,
            "extreme size must preserve a useful current-location content region");
    resize_twice({150, 150});
    require(!title_band->visible() && !ribbon_band->visible() &&
                !status_band->visible() &&
                responsive_objects->committed_arranged_bounds().width > 120.0 &&
                responsive_objects->committed_arranged_bounds().height > 60.0 &&
                responsive->find("fm.ribbon.tabs")->visible() &&
                responsive->find("fm.navigation")->visible(),
            "enforced minimum must retain menu, location controls, and one usable region");
    resize_twice({1450, 850});
    require(!responsive_outer->first_collapsed() &&
                !responsive_inner->second_collapsed() && title_band->visible() &&
                ribbon_band->visible() && status_band->visible(),
            "restoring reference size must restore only automatically hidden surfaces");
    const gui_forms::Rect reference_object_bounds =
        responsive_objects->committed_arranged_bounds();
    resize_twice({1800, 1050});
    const gui_forms::Rect expanded_selection =
        responsive->find("fm.selection.pane")->absolute_bounds();
    require(responsive->find("fm.title.identity")->committed_arranged_bounds().width ==
                1800.0 &&
                responsive->find("fm.workspace")->committed_arranged_bounds().width ==
                1800.0 &&
                responsive_objects->committed_arranged_bounds().width >
                    reference_object_bounds.width &&
                responsive_objects->committed_arranged_bounds().height >
                    reference_object_bounds.height &&
                std::abs(expanded_selection.x + expanded_selection.width -
                         1800.0) < 0.01,
            "growing beyond startup size must fill every shell band and expand the primary content field to the new window edges");
    resize_twice({1450, 850});

    const auto responsive_properties =
        std::dynamic_pointer_cast<gui_forms::PropertyList>(
            responsive->find("fm.selection.properties"));
    require(responsive_properties != nullptr,
            "large-text matrix requires the public Selection PropertyList");
    const double normal_property_content = responsive_properties->content_height();
    responsive->set_text_scale(1.25);
    responsive->perform_layout();
    responsive->perform_layout();
    require(responsive->find("fm.ribbon.tabs")->committed_arranged_bounds().height ==
                23.0 * 1.25 &&
                responsive->find("fm.navigation")->committed_arranged_bounds().height ==
                40.0 * 1.25 &&
                responsive_properties->content_height() > normal_property_content &&
                !responsive_outer->first_collapsed() &&
                !responsive_inner->second_collapsed(),
            "125% text must grow primary bands and inspector rows without premature collapse");
    responsive->set_text_scale(1.5);
    responsive->perform_layout();
    responsive->perform_layout();
    require(!responsive_outer->first_collapsed() &&
                responsive_inner->second_collapsed() && ribbon_band->visible(),
            "150% text must stack inspector content and collapse Selection before tree");
    responsive->set_text_scale(2.0);
    responsive->perform_layout();
    responsive->perform_layout();
    require(!responsive_outer->first_collapsed() &&
                responsive_inner->second_collapsed() && !ribbon_band->visible() &&
                responsive->find("fm.ribbon.tabs")->visible() &&
                responsive->find("fm.navigation")->visible(),
            "200% text must withdraw the ribbon while preserving menu, location, and tree");
    responsive->set_text_scale(2.25);
    responsive->perform_layout();
    responsive->perform_layout();
    require(responsive_outer->first_collapsed() &&
                responsive_inner->second_collapsed() &&
                responsive_outer->first_collapse_origin() ==
                    gui_forms::SplitCollapseOrigin::automatic_accommodation &&
                responsive_objects->committed_arranged_bounds().width > 1300.0,
            "225% text must collapse secondary panes and preserve the primary object field");
    responsive->set_text_scale(1.0);
    responsive->perform_layout();
    responsive->perform_layout();
    require(!responsive_outer->first_collapsed() &&
                !responsive_inner->second_collapsed() && ribbon_band->visible(),
            "returning to 100% must restore only automatically accommodated surfaces");

    auto controller = file_manager_demoboard::make_controller_window(responsive.get());
    const auto surface_choice = std::dynamic_pointer_cast<gui_forms::ComboBox>(
        controller->find("demo.surface.choice"));
    require(surface_choice != nullptr,
            "controller surface selector is absent");
    require(controller->find("demo.diagnostics") != nullptr,
            "controller capability report is absent");
    const auto text_scale = std::dynamic_pointer_cast<gui_forms::ComboBox>(
        controller->find("demo.accommodation.text_scale"));
    require(text_scale && text_scale->enabled(),
            "controller text-scale accommodation selector is absent");
    const auto sound = std::dynamic_pointer_cast<gui_forms::CheckBox>(
        controller->find("demo.accommodation.sound"));
    const auto reduced_motion = std::dynamic_pointer_cast<gui_forms::CheckBox>(
        controller->find("demo.accommodation.reduced_motion"));
    require(sound && sound->checked(),
            "controller semantic-sound policy toggle is absent");
    require(reduced_motion && !reduced_motion->checked(),
            "controller reduced-motion policy toggle is absent");
    surface_choice->set_selected_index(1U);
    require(responsive->find("fm.search.surface")->visible() &&
                !responsive->find("fm.workspace.content_selection")->visible(),
            "controller Search choice must reach the public product-surface transition");
    surface_choice->set_selected_index(2U);
    require(responsive->find("fm.criteria.surface")->visible() &&
                responsive->find("fm.workspace.content_selection")->visible() &&
                !responsive->find("fm.folder.objects")->visible(),
            "controller Criteria choice must reach the public retained surface transition");
    surface_choice->set_selected_index(3U);
    require(responsive->find("fm.review.palettes")->visible() &&
                !responsive->find("fm.workspace.content_selection")->visible() &&
                !responsive->find("fm.search.surface")->visible() &&
                !responsive->find("fm.criteria.surface")->visible(),
            "controller Palettes choice must reach the public Card laboratory");
    surface_choice->set_selected_index(6U);
    require(responsive->find("fm.review.dna")->visible() &&
                !responsive->find("fm.review.palettes")->visible() &&
                !responsive->find("fm.workspace.content_selection")->visible(),
            "controller DNA choice must reach the public MasterDetail laboratory");
    surface_choice->set_selected_index(0U);
    require(!responsive->find("fm.search.surface")->visible() &&
                !responsive->find("fm.review.palettes")->visible() &&
                !responsive->find("fm.review.dna")->visible() &&
                responsive->find("fm.workspace.content_selection")->visible(),
            "controller Folder choice must restore the retained Folder projection");
    text_scale->set_selected_index(4U);
    require(responsive->presentation_settings().text_scale == 2.25,
            "controller text-scale selector did not reach the product presentation contract");
    text_scale->set_selected_index(0U);
    require(responsive->presentation_settings().text_scale == 1.0,
            "controller did not restore the product to 100% text");
    const auto semantic_before_sound_toggle = responsive->semantic_snapshot();
    sound->set_checked(false);
    require(!responsive->presentation_settings().sound_enabled &&
                responsive->semantic_snapshot().node_count ==
                    semantic_before_sound_toggle.node_count,
            "sounds-off must preserve the product semantic state");
    sound->set_checked(true);
    reduced_motion->set_checked(true);
    require(responsive->presentation_settings().reduced_motion,
            "controller reduced-motion toggle did not reach the product presentation contract");
    reduced_motion->set_checked(false);

    std::cout << "demoboard fixture, reference geometry, semantics, and controller pass\n";
}
