#include WEB_FORMS_GENERATED_HEADER

#include "gui_forms/gui_forms.hpp"
#include "render/skia/raster/skia_raster.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <sstream>
#include <vector>

namespace {

using namespace gui_forms;

void require(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}

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

std::uint8_t unpremultiply(const std::uint8_t value,
                           const std::uint8_t alpha) noexcept {
    if (alpha == 0U) return 0U;
    return static_cast<std::uint8_t>(std::min(
        255U, (static_cast<unsigned>(value) * 255U + alpha / 2U) / alpha));
}

std::string inspection_with_raster_probes(
    gui_forms::VisualInspectionSnapshot snapshot,
    const gui_forms::render::SkiaRaster& raster) {
    std::string json = snapshot.to_json();
    while (!json.empty() && (json.back() == '\n' || json.back() == '\r' ||
                             json.back() == ' ' || json.back() == '\t')) {
        json.pop_back();
    }
    require(!json.empty() && json.back() == '}',
            "visual inspection JSON must be an object");
    json.pop_back();
    std::ostringstream probes;
    probes << ",\"raster_probes\":{";
    constexpr std::array<std::array<double, 2>, 8> positions{{
        {{0.15, 0.15}}, {{0.5, 0.15}}, {{0.85, 0.15}},
        {{0.15, 0.5}}, {{0.85, 0.5}},
        {{0.15, 0.85}}, {{0.5, 0.85}}, {{0.85, 0.85}},
    }};
    const auto* pixels = static_cast<const std::byte*>(raster.pixels());
    require(pixels != nullptr && raster.row_bytes() != 0U,
            "fidelity raster pixels must be readable");
    bool first_control = true;
    for (const auto& control : snapshot.controls) {
        if (control.stable_id.empty()) continue;
        if (!first_control) probes << ',';
        first_control = false;
        probes << '\"' << control.stable_id << "\":[";
        if (control.state.effectively_visible &&
            control.layout.absolute_bounds.width > 0.0 &&
            control.layout.absolute_bounds.height > 0.0) {
            bool first_probe = true;
            for (const auto& position : positions) {
                const auto& bounds = control.layout.absolute_bounds;
                const std::uint32_t x = static_cast<std::uint32_t>(std::clamp(
                    std::floor(bounds.x + bounds.width * position[0]), 0.0,
                    static_cast<double>(raster.pixel_width() - 1U)));
                const std::uint32_t y = static_cast<std::uint32_t>(std::clamp(
                    std::floor(bounds.y + bounds.height * position[1]), 0.0,
                    static_cast<double>(raster.pixel_height() - 1U)));
                const auto* pixel = pixels + y * raster.row_bytes() + x * 4U;
                const std::uint8_t blue = std::to_integer<std::uint8_t>(pixel[0]);
                const std::uint8_t green = std::to_integer<std::uint8_t>(pixel[1]);
                const std::uint8_t red = std::to_integer<std::uint8_t>(pixel[2]);
                const std::uint8_t alpha = std::to_integer<std::uint8_t>(pixel[3]);
                if (!first_probe) probes << ',';
                first_probe = false;
                probes << '[' << position[0] << ',' << position[1] << ",["
                       << static_cast<unsigned>(unpremultiply(red, alpha)) << ','
                       << static_cast<unsigned>(unpremultiply(green, alpha)) << ','
                       << static_cast<unsigned>(unpremultiply(blue, alpha)) << ','
                       << static_cast<unsigned>(alpha) << "]]";
            }
        }
        probes << ']';
    }
    probes << "}}";
    return json + probes.str();
}

} // namespace

int main(int argc, char** argv) {
    try {
        require(argc == 4 || argc == 8,
                "usage: native_fidelity_probe WIDTH HEIGHT FONT_DIRECTORY "
                "[TEXT_SCALE active|inactive reference|hover|pressed|focused|disabled TARGET|-]");
        const gui_forms::Size client{
            std::stod(argv[1]), std::stod(argv[2])};
        require(client.width > 0.0 && client.height > 0.0,
                "fidelity client size must be positive");
        auto form = WEB_FORMS_GENERATED_NAMESPACE::make_native_form();
        gui_forms::render::SkiaRaster raster;
        register_fidelity_fonts(raster, argv[3]);
        gui_forms::Window window(form.root_control(), client);
        WEB_FORMS_GENERATED_NAMESPACE::bind_native_resources(form, window);
        const double text_scale = argc == 8 ? std::stod(argv[4]) : 1.0;
        const bool active = argc != 8 || std::string_view(argv[5]) == "active";
        const std::string_view interaction =
            argc == 8 ? std::string_view(argv[6]) : std::string_view("reference");
        const std::string_view target_id =
            argc == 8 ? std::string_view(argv[7]) : std::string_view("-");
        require(text_scale >= 0.5 && text_scale <= 4.0,
                "fidelity text scale is outside the bounded range");
        require(active || (argc == 8 && std::string_view(argv[5]) == "inactive"),
                "fidelity active state must be active or inactive");
        require(interaction == "reference" || interaction == "hover" ||
                    interaction == "pressed" || interaction == "focused" ||
                    interaction == "disabled",
                "fidelity interaction state is outside the bounded vocabulary");
        window.set_text_scale(text_scale);
        window.set_active(active);
        window.set_text_metrics_provider(&raster);
        gui_forms::Control::Ptr target;
        if (interaction != "reference") {
            require(target_id != "-", "interactive fidelity state needs a target");
            target = window.find(target_id);
            require(static_cast<bool>(target), "fidelity interaction target is absent");
            if (interaction == "disabled") target->set_enabled(false);
        }
        window.perform_layout();
        if (target && interaction == "focused") {
            require(window.request_focus(target), "fidelity focus request failed");
        } else if (target && (interaction == "hover" || interaction == "pressed")) {
            const gui_forms::Rect bounds = target->absolute_bounds();
            const gui_forms::Point center{
                bounds.x + bounds.width * 0.5, bounds.y + bounds.height * 0.5};
            gui_forms::PointerEvent move;
            move.action = gui_forms::PointerAction::move;
            move.position = center;
            move.pointer_id = 1U;
            static_cast<void>(window.dispatch_pointer(move));
            if (interaction == "pressed") {
                gui_forms::PointerEvent down;
                down.action = gui_forms::PointerAction::down;
                down.button = gui_forms::PointerButton::primary;
                down.position = center;
                down.pointer_id = 1U;
                static_cast<void>(window.dispatch_pointer(down));
            }
        }
        require(raster.resize(client, 1.0),
                "Skia fidelity surface must allocate");
        require(raster.synchronize_images(window.image_resources()),
                "Skia fidelity resource synchronization failed");
        static_cast<void>(window.paint(
            raster, {0.0, 0.0, client.width, client.height}));
        gui_forms::VisualInspectionOptions options;
        options.include_text = true;
        std::cout << inspection_with_raster_probes(
            window.visual_inspection_snapshot(options), raster) << '\n';
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        std::cerr << "web_forms_native_fidelity_probe: " << error.what()
                  << '\n';
        return EXIT_FAILURE;
    }
}
