#include "about_view.hpp"

#include <utility>

namespace file_manager {
namespace {

gui_forms::SurfaceMaterial title_material() {
    using gui_forms::Color;
    using gui_forms::GradientStop;
    using gui_forms::MaterialFillLayer;
    gui_forms::SurfaceMaterial material{};
    material.fills = {
        MaterialFillLayer::linear_css_angle(92.0, {
            GradientStop{0.0, Color::rgba(23, 52, 127)},
            GradientStop{0.55, Color::rgba(58, 104, 203)},
            GradientStop{1.0, Color::rgba(217, 104, 114)},
        }),
        MaterialFillLayer::radial({0.18, -0.90}, {0.42, 1.35}, {
            GradientStop{0.0, Color::rgba(146, 217, 255, 116)},
            GradientStop{1.0, Color::rgba(146, 217, 255, 0)},
        }),
        MaterialFillLayer::radial({0.62, 1.60}, {0.38, 1.10}, {
            GradientStop{0.0, Color::rgba(214, 178, 255, 106)},
            GradientStop{1.0, Color::rgba(214, 178, 255, 0)},
        }),
        MaterialFillLayer::radial({0.95, 1.0}, {0.28, 0.80}, {
            GradientStop{0.0, Color::rgba(255, 195, 142, 100)},
            GradientStop{1.0, Color::rgba(255, 195, 142, 0)},
        }),
    };
    material.border_edges.bottom =
        gui_forms::MaterialBorder{Color::rgba(23, 45, 105), 1.0};
    return material;
}

} // namespace

AboutView::AboutView()
    : root_(std::make_shared<gui_forms::ScaledPanel>(
          gui_forms::StableId("file-manager.about"))),
      title_bar_(std::make_shared<gui_forms::Control>(
          gui_forms::StableId("file-manager.about.title"))),
      title_(std::make_shared<gui_forms::Label>(
          gui_forms::StableId("file-manager.about.title.name"),
          "About File Manager")),
      heading_(std::make_shared<gui_forms::Label>(
          gui_forms::StableId("file-manager.about.heading"), "File Manager")),
      version_(std::make_shared<gui_forms::Label>(
          gui_forms::StableId("file-manager.about.version"),
          "0.001-alpha · development build")),
      description_(std::make_shared<gui_forms::Label>(
          gui_forms::StableId("file-manager.about.description"),
          "One local place at a time, with protected filesystem authority and inspectable service boundaries.")),
      authority_(std::make_shared<gui_forms::Label>(
          gui_forms::StableId("file-manager.about.authority"),
          "AUTHORITY  Local filesystem · bounded Home and Volumes navigation")),
      composition_(std::make_shared<gui_forms::Label>(
          gui_forms::StableId("file-manager.about.composition"),
          "COMPOSITION  Web.Forms source → retained GUI.Forms C++")),
      close_(std::make_shared<gui_forms::Button>(
          gui_forms::StableId("file-manager.about.close"), "Close")) {
    (*root_).set_requested_bounds({0, 0, 540, 330});
    (*root_).set_design_size({540, 330});
    (*root_).set_background(gui_forms::Color::rgba(231, 237, 246));
    (*root_).set_border_style(gui_forms::BorderStyle::line);

    (*title_bar_).set_authored_surface_material(title_material());
    (*title_bar_).set_accessible_name("About window title bar");
    (*title_).set_font({gui_forms::FontRole::control, 15.0, 700, false});
    (*title_).set_foreground(gui_forms::Color::rgba(255, 255, 255));
    (*title_).set_hit_test_transparent(true);
    (*heading_).set_font({gui_forms::FontRole::control, 25.0, 700, false});
    (*heading_).set_foreground(gui_forms::Color::rgba(29, 45, 75));
    (*version_).set_font({gui_forms::FontRole::control, 11.0, 700, false});
    (*version_).set_foreground(gui_forms::Color::rgba(62, 97, 163));
    (*description_).set_text_wrapping(gui_forms::TextWrapping::word);
    (*description_).set_maximum_lines(3);
    (*description_).set_font({gui_forms::FontRole::control, 12.0, 400, false});
    (*authority_).set_font({gui_forms::FontRole::control, 10.0, 700, false});
    (*composition_).set_font({gui_forms::FontRole::control, 10.0, 700, false});

    (*root_).add_at(title_bar_, {1, 1, 538, 40});
    (*root_).add_at(title_, {76, 9, 250, 23});
    (*root_).add_at(heading_, {28, 66, 360, 38});
    (*root_).add_at(version_, {30, 108, 360, 22});
    (*root_).add_at(description_, {30, 146, 480, 58});
    (*root_).add_at(authority_, {30, 218, 480, 22});
    (*root_).add_at(composition_, {30, 244, 480, 22});
    (*root_).add_at(close_, {432, 280, 78, 32});

    gui_forms::SubscriptionToken close_connection = (*close_).clicked().subscribe(
        std::bind_front(&AboutView::close_clicked, this));
    subscriptions_.push_back(std::move(close_connection));
}

std::shared_ptr<gui_forms::Control> AboutView::root_control() const {
    return root_;
}

void AboutView::close_clicked(gui_forms::ButtonBase&) {
    if (hide_) hide_();
}

void AboutView::bind_hide(std::function<void()> hide) {
    hide_ = std::move(hide);
}

} // namespace file_manager
