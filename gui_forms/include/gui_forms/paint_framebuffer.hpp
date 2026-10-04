#pragma once

#include "gui_forms/types/painter/painter.hpp"
#include <cstddef>
#include <cstdint>
#include <span>

namespace gui_forms {
class ImageRegistry;
enum class FramebufferChannelOrder { bgra, rgba };

// UI-thread-owned offscreen instance of the host's renderer. Pixels are
// premultiplied sRGB, with native channel order explicitly reported. The buffer
// survives begin/end; callers may restore cached pixels before drawing damage.
// A painter and pixel span are borrowed only for this object's lifetime.
class PaintFramebuffer {
public:
    virtual ~PaintFramebuffer() = default;
    virtual bool begin(const ImageRegistry& images, Rect damage) = 0;
    virtual void end() = 0;
    virtual Painter& painter() noexcept = 0;
    virtual std::span<std::byte> pixels() noexcept = 0;
    virtual std::size_t row_bytes() const noexcept = 0;
    virtual std::uint32_t width() const noexcept = 0;
    virtual std::uint32_t height() const noexcept = 0;
    virtual FramebufferChannelOrder channel_order() const noexcept = 0;
};
} // namespace gui_forms
