#include "file_manager_demoboard/demoboard.hpp"

#include "file_manager_demoboard/fixture_model.hpp"
#include "gui_forms/gui_forms.hpp"

#include <array>
#include <memory>
#include <string>
#include <vector>

namespace file_manager_demoboard {

namespace {

struct ControllerLifetime final {
    std::vector<gui_forms::SubscriptionToken> subscriptions;
};

} // namespace

std::unique_ptr<gui_forms::Window> make_controller_window(
    gui_forms::Window* product) {
    using namespace gui_forms;
    auto root = std::make_shared<FlowLayoutPanel>(StableId("demo.controller.window"));
    root->set_flow_direction(FlowDirection::top_down);
    root->set_wrap_contents(false);
    root->set_padding({10,10,10,10});

    auto title = std::make_shared<Label>(StableId("demo.controller.title"),
                                         "DEMOboard controller · catalogue 001 / generation 86");
    title->set_requested_bounds({0,0,430,30});
    title->set_font({FontRole::control,12,700,false,.25});
    root->add_child(title);

    auto surface = std::make_shared<ComboBox>(StableId("demo.surface.choice"));
    surface->set_requested_bounds({0,0,430,30});
    surface->set_items({"Folder", "Search", "Criteria", "Palettes", "Icons", "Styles", "DNA"});
    surface->set_selected_index(0);
    root->add_child(surface);

    auto condition = std::make_shared<ComboBox>(StableId("demo.fixture.condition"));
    condition->set_requested_bounds({0,0,430,30});
    condition->set_items({"Normal", "Offline volume", "Stale index", "Preview unavailable",
                          "Transfer running", "Conflict", "Empty folder", "Million items"});
    condition->set_selected_index(0);
    root->add_child(condition);

    auto accommodation = std::make_shared<Label>(
        StableId("demo.accommodation.title"),
        "ACCOMMODATION · LOGICAL TEXT / HOST DENSITY SEPARATE");
    accommodation->set_requested_bounds({0,0,430,24});
    accommodation->set_font({FontRole::control,9,700,false,.28});
    root->add_child(accommodation);

    auto text_scale = std::make_shared<ComboBox>(
        StableId("demo.accommodation.text_scale"));
    text_scale->set_requested_bounds({0,0,430,30});
    text_scale->set_accessible_name("Product text size");
    text_scale->set_accessible_description(
        "Changes logical text measurement and large-text accommodation without "
        "changing the host display scale");
    text_scale->set_items({"100% text", "125% text", "150% text",
                           "200% text", "225% text"});
    text_scale->set_selected_index(0);
    root->add_child(text_scale);

    auto density = std::make_shared<Label>(
        StableId("demo.accommodation.display_scale"),
        "Display scale: host-derived · text scale: 100%");
    density->set_requested_bounds({0,0,430,24});
    density->set_font({FontRole::content,10,400,false});
    density->set_accessible_name("Presentation scale status");
    root->add_child(density);

    auto sound = std::make_shared<CheckBox>(
        StableId("demo.accommodation.sound"), "Semantic state sounds");
    sound->set_requested_bounds({0,0,430,28});
    sound->set_checked(true);
    sound->set_accessible_description(
        "Mutes presentation only; visual state, semantics, and feedback trace remain identical");
    root->add_child(sound);

    auto reduced_motion = std::make_shared<CheckBox>(
        StableId("demo.accommodation.reduced_motion"), "Reduced motion");
    reduced_motion->set_requested_bounds({0,0,430,28});
    reduced_motion->set_accessible_description(
        "Keeps live animation active at a calmer cadence and reduced excursion");
    root->add_child(reduced_motion);

    auto report = std::make_shared<ListBox>(StableId("demo.diagnostics"));
    report->set_requested_bounds({0,0,430,330});
    std::vector<std::string> rows;
    for (const auto& capability : initial_capability_report()) {
        rows.push_back(capability.id + " · " + capability.state + " · " + capability.detail);
    }
    report->set_items(std::move(rows));
    report->set_item_height(32);
    root->add_child(report);

    auto reset = std::make_shared<Button>(StableId("demo.reset"), "Reset to reference");
    reset->set_requested_bounds({0,0,180,30});
    reset->set_default_button(true);
    root->add_child(reset);

    auto lifetime = std::make_shared<ControllerLifetime>();
    if (product != nullptr) {
        static constexpr std::array<double, 5> scales{
            1.0, 1.25, 1.5, 2.0, 2.25};
        lifetime->subscriptions.push_back(
            surface->selected_index_changed().subscribe(
                *root, [product](std::optional<std::size_t> index) {
                    if (!index) return;
                    if (*index == 0U) {
                        static_cast<void>(set_product_surface(*product, "folder"));
                    } else if (*index == 1U) {
                        static_cast<void>(set_product_surface(*product, "search"));
                    } else if (*index == 2U) {
                        static_cast<void>(set_product_surface(*product, "criteria"));
                    } else if (*index == 3U) {
                        static_cast<void>(set_product_surface(*product, "palettes"));
                    } else if (*index == 6U) {
                        static_cast<void>(set_product_surface(*product, "dna"));
                    }
                }));
        lifetime->subscriptions.push_back(
            text_scale->selected_index_changed().subscribe(
                *root, [product, density = std::weak_ptr<Label>(density)](
                    std::optional<std::size_t> index) {
                    if (!index || *index >= std::size(scales)) return;
                    product->set_text_scale(scales[*index]);
                    if (const auto status = density.lock()) {
                        status->set_text(
                            "Display scale: host-derived · text scale: " +
                            std::to_string(static_cast<int>(scales[*index] * 100.0)) +
                            "%");
                    }
                }));
        lifetime->subscriptions.push_back(reset->clicked().subscribe(
            *root, [product,
                    surface_selector = std::weak_ptr<ComboBox>(surface),
                    selector = std::weak_ptr<ComboBox>(text_scale),
                    density = std::weak_ptr<Label>(density),
                    sound = std::weak_ptr<CheckBox>(sound),
                    reduced = std::weak_ptr<CheckBox>(reduced_motion)](ButtonBase&) {
                product->set_presentation_settings({});
                static_cast<void>(set_product_surface(*product, "folder"));
                if (const auto choice = surface_selector.lock()) {
                    choice->set_selected_index(0U);
                }
                if (const auto choice = selector.lock()) {
                    choice->set_selected_index(0U);
                }
                if (const auto status = density.lock()) {
                    status->set_text(
                        "Display scale: host-derived · text scale: 100%");
                }
                if (const auto toggle = sound.lock()) toggle->set_checked(true);
                if (const auto toggle = reduced.lock()) toggle->set_checked(false);
            }));
        lifetime->subscriptions.push_back(sound->checked_changed().subscribe(
            *root, [product](bool enabled) {
                PresentationSettings settings = product->presentation_settings();
                settings.sound_enabled = enabled;
                product->set_presentation_settings(settings);
            }));
        lifetime->subscriptions.push_back(
            reduced_motion->checked_changed().subscribe(
                *root, [product](bool reduced) {
                    PresentationSettings settings = product->presentation_settings();
                    settings.reduced_motion = reduced;
                    product->set_presentation_settings(settings);
                }));
    } else {
        text_scale->set_enabled(false);
        text_scale->set_accessible_description(
            "Attach a product window to exercise text accommodation");
        density->set_text("Display scale: host-derived · product not attached");
        sound->set_enabled(false);
        reduced_motion->set_enabled(false);
    }

    auto window = std::make_unique<Window>(root, Size{470,700});
    window->perform_layout();
    root->set_tag(std::move(lifetime));
    return window;
}

} // namespace file_manager_demoboard
