// Pixel-by-pixel reference for the former Windows shape kernels. Keep this
// deliberately scalar: optimized row spans and cached materials compare here.
static bool reference_inside(Point sample, Rect rect, double radius) {
    if (!rect.contains(sample)) return false;
    radius = std::clamp(radius, 0.0, std::min(rect.width, rect.height) * 0.5);
    const double dx = sample.x - std::clamp(sample.x, rect.x + radius, rect.right() - radius);
    const double dy = sample.y - std::clamp(sample.y, rect.y + radius, rect.bottom() - radius);
    return dx * dx + dy * dy <= radius * radius;
}

static Color reference_gradient(std::span<const GradientStop> stops, double amount) {
    amount = std::clamp(amount, 0.0, 1.0);
    for (std::size_t index = 1; index < stops.size(); ++index) {
        if (amount > stops[index].offset) continue;
        const Color first = stops[index - 1].color;
        const Color second = stops[index].color;
        const double width = stops[index].offset - stops[index - 1].offset;
        const double local = width <= 0.0 ? 1.0 : (amount - stops[index - 1].offset) / width;
        return Color::rgba(
            static_cast<std::uint8_t>(std::lround(first.red + (double(second.red) - first.red) * local)),
            static_cast<std::uint8_t>(std::lround(first.green + (double(second.green) - first.green) * local)),
            static_cast<std::uint8_t>(std::lround(first.blue + (double(second.blue) - first.blue) * local)),
            static_cast<std::uint8_t>(std::lround(first.alpha + (double(second.alpha) - first.alpha) * local)));
    }
    return stops.back().color;
}

static void shape_scanline_equivalence() {
    const Color background = Color::rgba(29, 43, 61);
    const Point translation{2.3, -1.7};
    const Rect shape{3.2, 4.1, 41.4, 29.7};
    const Rect absolute{shape.x + translation.x, shape.y + translation.y, shape.width, shape.height};
    const Rect clip{0.4, 1.2, 57.8, 43.2};
    const Rect rounded_clip{1.8, 2.2, 53.4, 40.7};
    for (const double scale : {1.0, 1.5, 2.0}) {
        for (const bool rounded : {false, true}) {
            for (const std::uint8_t alpha : {std::uint8_t(119), std::uint8_t(255)}) {
                host::DibPainter painter;
                require(painter.resize({64, 48}, scale));
                const Color fill = Color::rgba(193, 71, 117, alpha);
                std::array<GradientStop, 3> stops{{{0.0, fill},
                    {0.35, Color::rgba(22, 173, 91, alpha)}, {1.0, Color::rgba(73, 31, 219, alpha)}}};
                for (int kind = 0; kind < 7; ++kind) {
                    const Point start{8.0, 4.0};
                    const Point end = kind == 3 ? Point{8.0, 32.0} : kind == 4 ? Point{41.0, 4.0} : Point{41.0, 32.0};
                    const Point center{25.0, 15.0};
                    for (int repeat = 0; repeat < 4; ++repeat) {
                        stops[1].color = repeat < 2 ? Color::rgba(22, 173, 91, alpha) : Color::rgba(132, 47, 181, alpha);
                        painter.begin_frame();
                        painter.fill_rect({0, 0, 64, 48}, background);
                        painter.clip_rect(clip);
                        if (rounded) painter.clip_rounded_rect(rounded_clip, 7.3);
                        painter.translate(translation);
                        if (kind == 0) painter.fill_rect(shape, fill);
                        else if (kind == 1) painter.fill_rounded_rect(shape, 8.2, fill);
                        else if (kind == 2) painter.stroke_rounded_rect(shape, 8.2, fill, 2.3);
                        else if (kind == 6) painter.fill_radial_gradient(shape, center, {19, 8}, stops);
                        else painter.fill_linear_gradient_spread(shape, start, end, stops, GradientSpreadMode::reflect);
                        require(painter.save_bmp(L"shape-reference.bmp"));
                        std::ifstream file("shape-reference.bmp", std::ios::binary);
                        const std::vector<unsigned char> bytes((std::istreambuf_iterator<char>(file)), {});
                        file.close();
                        const int width = static_cast<int>(64 * scale);
                        const int height = static_cast<int>(48 * scale);
                        const Rect clipping = rounded ? Rect::intersection(clip, rounded_clip) : clip;
                        const Rect bounds = Rect::intersection(absolute, clipping);
                        for (int y = 0; y < height; ++y) {
                            for (int x = 0; x < width; ++x) {
                                const Point sample{(x + 0.5) / scale, (y + 0.5) / scale};
                                bool covered = x >= std::floor(bounds.x * scale) && x < std::ceil(bounds.right() * scale) &&
                                    y >= std::floor(bounds.y * scale) && y < std::ceil(bounds.bottom() * scale) && clipping.contains(sample);
                                if (rounded) covered = covered && reference_inside(sample, rounded_clip, 7.3);
                                if (kind == 1 || kind == 2) covered = covered && reference_inside(sample, absolute, 8.2);
                                if (kind == 2) covered = covered && !reference_inside(sample,
                                    {absolute.x + 2.3, absolute.y + 2.3, absolute.width - 4.6, absolute.height - 4.6}, 5.9);
                                Color color = fill;
                                if (kind >= 3 && covered) {
                                    double amount{};
                                    if (kind == 6) {
                                        const double rx = (sample.x - center.x - translation.x) / 19.0;
                                        const double ry = (sample.y - center.y - translation.y) / 8.0;
                                        amount = std::sqrt(rx * rx + ry * ry);
                                    } else {
                                        const double dx = end.x - start.x;
                                        const double dy = end.y - start.y;
                                        amount = ((sample.x - start.x - translation.x) * dx +
                                            (sample.y - start.y - translation.y) * dy) / (dx * dx + dy * dy);
                                        amount = std::fmod(amount, 2.0);
                                        if (amount < 0.0) amount += 2.0;
                                        if (amount > 1.0) amount = 2.0 - amount;
                                    }
                                    color = reference_gradient(stops, amount);
                                }
                                const unsigned a = covered ? color.alpha : 0U;
                                const std::array<unsigned, 3> expected{{
                                    (color.blue * a + background.blue * (255U - a) + 127U) / 255U,
                                    (color.green * a + background.green * (255U - a) + 127U) / 255U,
                                    (color.red * a + background.red * (255U - a) + 127U) / 255U}};
                                const std::size_t offset = 54U + (static_cast<std::size_t>(y) * width + x) * 4U;
                                for (std::size_t channel = 0; channel < 3; ++channel) {
                                    if (bytes[offset + channel] != expected[channel]) {
                                        std::cerr << "shape mismatch kind=" << kind << " scale=" << scale << " x=" << x << " y=" << y << '\n';
                                        require(false);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    std::remove("shape-reference.bmp");
}
