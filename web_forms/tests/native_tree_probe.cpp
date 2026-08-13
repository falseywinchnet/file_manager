#include "standard_shell_sapphire.gui_tree.wf.hpp"

#include "gui_forms/gui_forms.hpp"
#ifdef WEB_FORMS_FIDELITY_SKIA
#include "render/skia/raster/skia_raster.hpp"
#endif

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

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

#ifdef WEB_FORMS_FIDELITY_SKIA
std::vector<std::byte> read_font(const std::string& path) {
    std::ifstream input(path, std::ios::binary | std::ios::ate);
    require(input.good(), "fidelity font must be readable");
    const std::streamsize size = input.tellg();
    require(size > 0, "fidelity font must not be empty");
    input.seekg(0, std::ios::beg);
    std::vector<std::byte> result(static_cast<std::size_t>(size));
    require(input.read(reinterpret_cast<char*>(result.data()), size).good(),
            "fidelity font bytes must be complete");
    return result;
}

bool register_font(gui_forms::render::SkiaRaster& raster,
                   std::string_view directory,
                   const char* name,
                   FontRole role,
                   std::uint16_t weight,
                   bool italic = false) {
    const std::vector<std::byte> bytes =
        read_font(std::string(directory) + "/" + name);
    return raster.register_typeface(role, weight, italic, bytes);
}

bool register_fallback(gui_forms::render::SkiaRaster& raster,
                       std::string_view directory,
                       const char* name) {
    const std::vector<std::byte> bytes =
        read_font(std::string(directory) + "/" + name);
    return raster.register_fallback_typeface(400, false, bytes);
}

void register_fidelity_fonts(gui_forms::render::SkiaRaster& raster,
                             std::string_view directory) {
    require(
        register_font(raster, directory, "PortsmouthRapids.ttf",
                      FontRole::control, 400) &&
        register_font(raster, directory, "PortsmouthRapids-Bold.ttf",
                      FontRole::control, 700) &&
        register_font(raster, directory, "Carlito-Regular.ttf",
                      FontRole::content, 400) &&
        register_font(raster, directory, "Carlito-Bold.ttf",
                      FontRole::content, 700) &&
        register_font(raster, directory, "Carlito-Italic.ttf",
                      FontRole::content, 400, true) &&
        register_font(raster, directory, "Carlito-BoldItalic.ttf",
                      FontRole::content, 700, true) &&
        register_font(raster, directory, "Cousine-Regular.ttf",
                      FontRole::monospace, 400) &&
        register_font(raster, directory, "Cousine-Bold.ttf",
                      FontRole::monospace, 700) &&
        register_fallback(raster, directory, "NotoSansCJKjp-Regular.otf") &&
        register_fallback(raster, directory, "NotoEmoji-Regular.ttf"),
        "fidelity probe requires the complete pinned font pack");
}
#endif

} // namespace

int main(int argc, char** argv) {
    try {
        web_forms_generated_standard_shell_sapphire::NativeForm form =
            web_forms_generated_standard_shell_sapphire::make_native_form();
        require(form.root_control() == form.standard_app,
                "typed root must be the retained body control");
        require(form.standard_app->stable_id().value() ==
                    std::string_view("standard-app"),
                "root StableId must preserve source identity");
        require(form.standard_app->children().size() == 1U &&
                    form.standard_app->children()[0] == form.standard_app_shell,
                "body must own the shell rather than overlay it");
        require(form.standard_app_shell->children().size() == 4U,
                "shell must retain title, menu, workspace, and status in DOM order");
        require(form.standard_app_shell_workspace->children().size() == 2U,
                "workspace grid must retain its sidebar and content cells");
        require(form.standard_app_shell_workspace_content_cards->children().size() ==
                    2U,
                "card grid must retain both authored cards");
        require(form.standard_app_shell_workspace_content_heading_new->stable_id().value() ==
                    std::string_view(
                        "standard-app.shell.workspace.content.heading.new"),
                "typed command member must preserve its hierarchical id");

        gui_forms::Window window(form.root_control(), {1080.0, 720.0});
        web_forms_generated_standard_shell_sapphire::bind_native_resources(
            form, window);
#ifdef WEB_FORMS_FIDELITY_SKIA
        std::unique_ptr<gui_forms::render::SkiaRaster> raster;
        if (argc >= 2 && std::string_view(argv[1]) == "--snapshot") {
            raster = std::make_unique<gui_forms::render::SkiaRaster>();
            register_fidelity_fonts(
                *raster,
                argc >= 3 ? std::string_view(argv[2])
                          : std::string_view("gui_forms/assets/fonts"));
            window.set_text_metrics_provider(raster.get());
        }
#endif
        window.perform_layout();
        const gui_forms::Rect shell =
            form.standard_app_shell->committed_arranged_bounds();
        require(shell.x == 34.0 && shell.y == 34.0 && shell.width == 1012.0 &&
                    shell.height >= 650.0,
                "CSS padding, horizontal auto margins, width, and min-height must compose");
        const gui_forms::Rect workspace =
            form.standard_app_shell_workspace->committed_arranged_bounds();
        const gui_forms::Rect sidebar =
            form.standard_app_shell_workspace_sidebar->committed_arranged_bounds();
        const gui_forms::Rect content =
            form.standard_app_shell_workspace_content->committed_arranged_bounds();
        if (!(workspace.width == shell.width && sidebar.width == 210.0 &&
              content.x == 210.0 &&
              content.width == workspace.width - 210.0)) {
            std::cerr << "workspace=" << workspace.x << ',' << workspace.y << ','
                      << workspace.width << ',' << workspace.height
                      << " sidebar=" << sidebar.x << ',' << sidebar.y << ','
                      << sidebar.width << ',' << sidebar.height
                      << " content=" << content.x << ',' << content.y << ','
                      << content.width << ',' << content.height << '\n';
        }
        require(workspace.width == shell.width && sidebar.width == 210.0 &&
                    content.x == 210.0 &&
                    content.width == workspace.width - 210.0,
                "grid tracks must relax from 210px plus one fraction");
        if (argc == 2 && std::string_view(argv[1]) == "--snapshot") {
#ifdef WEB_FORMS_FIDELITY_SKIA
            require(raster != nullptr && raster->resize({1080.0, 720.0}, 1.0),
                    "Skia fidelity surface must allocate");
            static_cast<void>(window.paint(
                *raster, {0.0, 0.0, 1080.0, 720.0}));
#else
            NullPainter painter;
            static_cast<void>(window.paint(painter, {0.0, 0.0, 1080.0, 720.0}));
#endif
            gui_forms::VisualInspectionOptions options;
            options.include_text = true;
            std::cout << window.visual_inspection_snapshot(options).to_json()
                      << '\n';
            return EXIT_SUCCESS;
        }
        std::cout << "web_forms_native_tree_probe: all tests passed\n";
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        std::cerr << "web_forms_native_tree_probe: " << error.what() << '\n';
        return EXIT_FAILURE;
    }
}
