#pragma once

#include "gui_forms/resources.hpp"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <span>

namespace gui_forms::render {

class SkiaRaster final : public Painter {
public:
    SkiaRaster();
    ~SkiaRaster() override;
    SkiaRaster(const SkiaRaster&) = delete;
    SkiaRaster& operator=(const SkiaRaster&) = delete;

    bool resize(Size logical_size, double scale);
    void begin_frame(const DamageRegion& damage);
    void end_frame();

    [[nodiscard]] bool register_typeface(FontRole role,
                                         std::uint16_t weight,
                                         bool italic,
                                         std::span<const std::byte> encoded);
    [[nodiscard]] bool synchronize_images(const ImageRegistry& registry);
    [[nodiscard]] const void* pixels() const noexcept;
    [[nodiscard]] std::size_t row_bytes() const noexcept;
    [[nodiscard]] std::uint32_t pixel_width() const noexcept;
    [[nodiscard]] std::uint32_t pixel_height() const noexcept;
    [[nodiscard]] std::size_t byte_size() const noexcept;

    void save() override;
    void restore() override;
    void translate(Point offset) override;
    void clip_rect(Rect rect) override;
    void fill_rect(Rect rect, Color color) override;
    void stroke_rect(Rect rect, Color color, double width) override;
    void draw_line(Point from, Point to, Color color, double width) override;
    void draw_text_utf8(Point origin,
                        std::string_view text,
                        FontSpec font,
                        Color color) override;
    [[nodiscard]] Size measure_text_utf8(std::string_view text,
                                         FontSpec font) override;
    void draw_image(ImageId image, Rect destination, double opacity) override;

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace gui_forms::render
