#include "visual_inspector_lab.hpp"

#include "gui_forms/gui_forms.hpp"

#include <cstdlib>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>

namespace {

using namespace gui_forms;

void require(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}

class NullPainter final : public Painter {
public:
    void save() override {}
    void restore() override {}
    void translate(Point) override {}
    void clip_rect(Rect) override {}
    void fill_rect(Rect, Color) override {}
    void stroke_rect(Rect, Color, double) override {}
    void draw_line(Point, Point, Color, double) override {}
    void draw_text_utf8(Point, std::string_view, FontSpec, Color) override {}
    void draw_image(ImageId, Rect, double) override {}
};

} // namespace

int main() {
    try {
        std::unique_ptr<Window> window =
            gui_forms::visual_inspector_lab::make_visual_inspector_lab();
        NullPainter painter;
        static_cast<void>((*window).paint(
            painter, {0.0, 0.0, 1160.0, 700.0}));
        const VisualInspectionSnapshot snapshot =
            (*window).visual_inspection_snapshot();
        const VisualControlInspection* card =
            snapshot.find("visual-inspector.specimen-card");
        const VisualControlInspection* text =
            snapshot.find("visual-inspector.specimen-title");
        const VisualControlInspection* clipped =
            snapshot.find("visual-inspector.clipped-child");
        const VisualControlInspection* overlay =
            snapshot.find("visual-inspector.overlay");
        require(card && card->authored_material && card->display_chunk_current &&
                    (*card->authored_material).fills.size() == 3U,
                "lab must dogfood ordered authored material inspection");
        require(text && text->display_chunk_current,
                "lab must dogfood committed resolved text inspection");
        require(clipped && clipped->layout.clipped_by_ancestor,
                "lab must dogfood ancestor clip inspection");
        require(overlay && overlay->paint_plane == PaintPlane::overlay &&
                    overlay->state.hit_test_transparent,
                "lab overlay must remain pointer-transparent and ordered last");
        require(snapshot.total_controls >= 20U,
                "lab must contain a representative retained inspection surface");
        const std::string semantics = (*window).semantic_snapshot().to_json();
        require(semantics.find("Visual state inspector") != std::string::npos &&
                    semantics.find("visual-inspector.specimen-card") !=
                        std::string::npos,
                "lab inspector must expose its target and current visual summary to accessibility");
        require(semantics.find("visual-inspector.overlay") == std::string::npos,
                "pointer-transparent visual geometry overlay must remain absent from accessibility");
        std::cout << "gui_forms_visual_inspector_lab_tests: all tests passed\n";
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        std::cerr << "gui_forms_visual_inspector_lab_tests: " << error.what()
                  << '\n';
        return EXIT_FAILURE;
    }
}
