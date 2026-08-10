#include "gui_forms/live_surface/frame/live_surface_frame.hpp"

#include "../state/live_surface_state.hpp"

#include <utility>

namespace gui_forms {

LiveSurfaceFrame::LiveSurfaceFrame(
    std::shared_ptr<const detail::LiveSurfaceBuffer> buffer,
    std::uint64_t epoch, std::uint64_t generation, Rect damage) noexcept
    : buffer_(std::move(buffer)), epoch_(epoch), generation_(generation),
      damage_(damage) {}

std::uint32_t LiveSurfaceFrame::width() const noexcept {
    return buffer_ ? (*buffer_).description.width : 0U;
}

std::uint32_t LiveSurfaceFrame::height() const noexcept {
    return buffer_ ? (*buffer_).description.height : 0U;
}

std::uint64_t LiveSurfaceFrame::row_bytes() const noexcept {
    return buffer_ ? (*buffer_).row_bytes : 0U;
}

LiveSurfacePixelFormat LiveSurfaceFrame::pixel_format() const noexcept {
    return buffer_ ? (*buffer_).description.pixel_format
                   : LiveSurfacePixelFormat::bgra32_premultiplied_srgb;
}

std::span<const std::byte> LiveSurfaceFrame::pixels() const noexcept {
    return buffer_ ? std::span<const std::byte>((*buffer_).pixels)
                   : std::span<const std::byte>{};
}

} // namespace gui_forms
