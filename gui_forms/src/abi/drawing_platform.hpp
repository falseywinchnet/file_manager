#ifndef GUI_FORMS_DRAWING_PLATFORM_HPP
#define GUI_FORMS_DRAWING_PLATFORM_HPP

#include "gui_forms/drawing.hpp"
#include "gui_forms/drawing_c_api.h"

#include <cstdint>
#include <memory>

namespace gui_drawing::abi::platform {

struct CapturedSurface final {
    std::unique_ptr<Bitmap> bitmap;
    RectF bounds;
};

gd_result export_hbitmap(Bitmap& bitmap, Color background, std::uintptr_t& output);
gd_result import_hbitmap(std::uintptr_t source, std::unique_ptr<Bitmap>& output);
gd_result capture_surface(std::uintptr_t source, std::uint32_t kind,
                          CapturedSurface& output);
gd_result present_surface(std::uintptr_t destination, std::uint32_t kind,
                          Bitmap& bitmap);
gd_result acquire_hdc(Bitmap& bitmap, std::uintptr_t& output,
                      std::uint64_t& lease_token);
gd_result release_hdc(Bitmap& bitmap, std::uint64_t lease_token);
bool has_hdc_lease(Bitmap& bitmap);

} // namespace gui_drawing::abi::platform

#endif
