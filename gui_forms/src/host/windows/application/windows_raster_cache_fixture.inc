// Private native DIB fixture. Included only in the lifecycle test executable.
// Captured pixels are compared outside timed intervals; no host window is used
// by the brush workload, so this does not measure presentation or input latency.
struct DibRasterCacheFixture final {
    static void require(const bool accepted, const char* message) {
        if (!accepted) throw std::runtime_error(message);
    }
    static void begin(DibPainter& painter, const double scale = 1.0) {
        require(painter.resize({800.0, 500.0}, scale), "brush fixture resize");
        require(painter.begin_frame(), "brush fixture candidate");
    }
    static void background(DibPainter& painter, const std::uint32_t color) {
        const std::size_t count = static_cast<std::size_t>(painter.width_) *
            static_cast<std::size_t>(painter.height_);
        std::fill_n(painter.pixels_, count, color);
    }
    static std::vector<std::uint32_t> capture(const DibPainter& painter) {
        const std::size_t count = static_cast<std::size_t>(painter.width_) *
            static_cast<std::size_t>(painter.height_);
        const std::vector<std::uint32_t> result(painter.pixels_, painter.pixels_ + count);
        return result;
    }
    static void compare(const DibPainter& painter, const std::vector<std::uint32_t>& expected) {
        const std::size_t count = static_cast<std::size_t>(painter.width_) *
            static_cast<std::size_t>(painter.height_);
        require(count == expected.size(), "brush oracle extent");
        for (std::size_t index = 0U; index < count; ++index) {
            require(painter.pixels_[index] == expected[index], "brush oracle pixel mismatch");
        }
    }
    static void scene(DibPainter& painter, const bool shadows, const std::size_t variant) {
        painter.save();
        if (variant == 1U) painter.clip_rounded_rect({13.2, 10.7, 701.3, 420.6}, 23.5);
        if (variant == 2U) painter.translate({0.375, -1.25});
        if (shadows) {
            Color color = Color::rgba(20, 30, 40, 173);
            if (variant == 3U) color = Color::rgba(70, 110, 150, 1);
            if (variant == 4U) color = Color::rgba(70, 110, 150, 173);
            const double radius = variant == 5U ? 27.75 : 19.25;
            const double blur = variant == 6U ? 9.0 : 14.0;
            painter.draw_box_shadow({90.25, 75.5, 520.0, 270.0}, radius, {3.25, 8.5}, blur, 2.5, color);
            painter.draw_box_shadow({130.5, 117.25, 130.75, 60.5}, 7.5, {-2.5, 2.25}, 0.0, -1.0, color);
        } else {
            const std::array<GradientStop, 3U> stops{{{0.0, Color::rgba(20, 40, 70, 210)},
                {0.4, Color::rgba(100, 130, 170, 100)}, {1.0, Color::rgba(5, 15, 35, 0)}}};
            painter.fill_radial_gradient({0.0, 0.0, 800.0, 500.0}, {320.25, 220.5}, {480.0, 340.0}, stops);
            for (std::size_t index = 0U; index < 40U; ++index) {
                const double x = 15.25 + static_cast<double>(index % 10U) * 72.0;
                const double y = 20.5 + static_cast<double>(index / 10U) * 82.0;
                painter.fill_radial_gradient({x, y, 48.0, 48.0}, {x + 19.0, y + 17.0}, {33.0, 31.0}, stops);
            }
            painter.fill_linear_gradient_spread({30.0, 400.0, 650.0, 38.0}, {33.25, 410.5},
                {140.75, 431.0}, stops, GradientSpreadMode::reflect);
        }
        painter.restore();
    }
    static void correctness() {
        DibPainter reused{};
        // Preserve populated caches across scale changes in both directions.
        for (const double scale : std::array<double, 3U>{1.0, 1.25, 1.0}) {
            begin(reused, scale);
            for (std::size_t pass = 0U; pass < 3U; ++pass) {
                for (std::size_t variant = 0U; variant < 7U; ++variant) {
                    for (const bool shadows : std::array<bool, 2U>{true, false}) {
                        DibPainter fresh{};
                        fresh.brush_cache_disabled_ = true;
                        begin(fresh, scale);
                        // Two transparent passes cover construction and reuse.
                        // Compare shadows alone: later gradients would overwrite
                        // destination alpha and hide alpha-zero/skip mistakes.
                        const std::uint32_t backdrop = pass < 2U ? 0x00123456U : 0xff987654U;
                        background(fresh, backdrop);
                        background(reused, backdrop);
                        scene(fresh, shadows, variant);
                        const std::vector<std::uint32_t> expected = capture(fresh);
                        scene(reused, shadows, variant);
                        compare(reused, expected);
                    }
                }
            }
        }
    }
    static void byte_limits() {
        DibPainter painter{};
        begin(painter);
        const std::array<GradientStop, 2U> stops{{{0.0, Color::rgba(1, 2, 3, 255)},
            {1.0, Color::rgba(4, 5, 6, 0)}}};
        // Each brush is admissible alone. The third forces byte-budget FIFO
        // retirement while the entry count remains well below its ceiling.
        for (std::size_t index = 0U; index < 3U; ++index) {
            const double position = static_cast<double>(index);
            const DibPainter::GradientRaster* gradient = painter.gradient_raster({0, 0, 2048, 1024},
                {position, 2.25}, {3.0, 4.0}, stops, GradientSpreadMode::pad, true);
            require(gradient != nullptr, "large gradient admission");
            const DibPainter::ShadowRaster* shadow = painter.shadow_raster({0, 0, 1024, 1024},
                {position, 0.0, 4.0, 5.0}, 1.0, 0.75, 173U);
            require(shadow != nullptr, "large shadow admission");
            const std::size_t retained = std::min(index + 1U, std::size_t{2U});
            require(painter.gradient_cache_pixels_ == retained * 2097152U,
                "gradient pixel accounting at byte limit");
            require(painter.shadow_cache_samples_ == retained * 1048576U,
                "shadow sample accounting at byte limit");
        }
        require(painter.gradient_cache_.size() == 2U && painter.gradient_cache_.front().first.x == 1.0,
            "gradient byte budget retires oldest brush");
        require(painter.shadow_cache_.size() == 2U && painter.shadow_cache_.front().shadow.x == 1.0,
            "shadow byte budget retires oldest brush");
    }
    static void limits() {
        DibPainter painter{};
        begin(painter);
        const std::array<GradientStop, 2U> stops{{{0.0, Color::rgba(1, 2, 3, 255)},
            {1.0, Color::rgba(4, 5, 6, 0)}}};
        const DibPainter::PixelRect area{0, 0, 8, 8};
        for (std::size_t index = 0U; index < 140U; ++index) {
            const double position = static_cast<double>(index) / 8.0;
            const DibPainter::GradientRaster* gradient = painter.gradient_raster(area,
                {position, 2.25}, {3.0, 4.0}, stops, GradientSpreadMode::pad, true);
            require(gradient != nullptr, "bounded gradient admission");
            const DibPainter::ShadowRaster* shadow = painter.shadow_raster(area,
                {position, 0.0, 4.0, 5.0}, 1.0, 0.75, 173U);
            require(shadow != nullptr, "bounded shadow admission");
        }
        require(painter.gradient_cache_.size() == 128U && painter.gradient_cache_pixels_ == 128U * 64U,
            "gradient entry bound and retirement");
        require(painter.shadow_cache_.size() == 64U && painter.shadow_cache_samples_ == 64U * 64U,
            "shadow entry bound and retirement");
        const DibPainter::GradientRaster* oversized_gradient = painter.gradient_raster({0, 0, 4096, 1025},
            {1.0, 2.0}, {3.0, 4.0}, stops, GradientSpreadMode::pad, true);
        const DibPainter::ShadowRaster* oversized_shadow = painter.shadow_raster({0, 0, 4096, 513},
            {0.0, 0.0, 4.0, 5.0}, 1.0, 0.75, 173U);
        require(oversized_gradient == nullptr && oversized_shadow == nullptr,
            "oversized brush uses uncached path before allocation");
        require(painter.gradient_cache_pixels_ == 128U * 64U && painter.shadow_cache_samples_ == 64U * 64U,
            "oversized refusal preserves existing cache storage");
        DibPainter fallback{};
        begin(fallback);
        fallback.fail_shadow_allocation_ = true;
        background(fallback, 0x00123456U);
        scene(fallback, true, 3U);
        const std::vector<std::uint32_t> failed_pixels = capture(fallback);
        require(fallback.shadow_cache_samples_ == 0U, "injected failed allocation owns no samples");
        fallback.fail_shadow_allocation_ = false;
        fallback.brush_cache_disabled_ = true;
        background(fallback, 0x00123456U);
        scene(fallback, true, 3U);
        compare(fallback, failed_pixels);
    }
    static void benchmark(const bool shadows, const bool cold = false) {
        DibPainter painter{};
        begin(painter);
        background(painter, 0xff123456U);
        scene(painter, shadows, 0U);
        const std::vector<std::uint32_t> expected = capture(painter);
        constexpr std::size_t samples = 40U;
        std::array<double, samples> elapsed{};
        const std::size_t previous_builds = painter.gradient_builds_;
        for (std::size_t index = 0U; index < samples; ++index) {
            if (cold) {
                painter.gradient_cache_.clear();
                painter.gradient_cache_pixels_ = 0U;
                painter.shadow_cache_.clear();
                painter.shadow_cache_samples_ = 0U;
            }
            background(painter, 0xff123456U);
            const std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
            scene(painter, shadows, 0U);
            const std::chrono::steady_clock::time_point finish = std::chrono::steady_clock::now();
            const std::chrono::duration<double, std::milli> interval = finish - start;
            elapsed[index] = interval.count();
            compare(painter, expected);
        }
        std::sort(elapsed.begin(), elapsed.end());
        const char* name = shadows ? "shadow" : "gradients_42";
        if (cold) name = shadows ? "shadow_cold" : "gradients_42_cold";
        const std::size_t builds = painter.gradient_builds_ - previous_builds;
        std::printf("%s,%zu,%.6f,%.6f,%.6f,%.6f,%zu,%zu\n", name, samples,
            elapsed[19U], elapsed[37U], elapsed[39U], elapsed[39U], builds, painter.gradient_cache_pixels_);
    }
    static void run() {
        correctness();
        limits();
        byte_limits();
        if (environment_flag("GUI_FORMS_BENCH_BRUSH_CACHE")) {
            std::puts("workload,samples,p50_ms,p95_ms,p99_ms,max_ms,gradient_builds,gradient_pixels");
            benchmark(false);
            benchmark(true);
            benchmark(false, true);
            benchmark(true, true);
        }
    }
};
