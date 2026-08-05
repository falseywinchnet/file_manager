#include "gui_forms/drawing_c_api.h"

#include "gui_forms/drawing.hpp"
#include "drawing_platform.hpp"

#include <algorithm>
#include <cstring>
#include <limits>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>
#include <vector>

namespace {

thread_local gd_result last_error_code = GD_OK;
thread_local std::string last_error_message;

gd_result fail(gd_result result, std::string message) noexcept {
    last_error_code = result;
    try {
        last_error_message = std::move(message);
    } catch (...) {
        last_error_message.clear();
    }
    return result;
}

template <typename Operation>
gd_result translate(Operation&& operation) noexcept {
    try {
        return operation();
    } catch (const std::length_error& error) {
        return fail(GD_ERROR_LIMIT_EXCEEDED, error.what());
    } catch (const std::invalid_argument& error) {
        return fail(GD_ERROR_INVALID_ARGUMENT, error.what());
    } catch (const std::logic_error& error) {
        return fail(GD_ERROR_INVALID_ARGUMENT, error.what());
    } catch (const std::exception& error) {
        return fail(GD_ERROR_INTERNAL, error.what());
    } catch (...) {
        return fail(GD_ERROR_INTERNAL, "GUI.Drawing ABI caught a non-standard exception");
    }
}

[[nodiscard]] gui_drawing::Color color_from_c(gd_color color) {
    if (color.is_empty > 1U) {
        throw std::invalid_argument("color is_empty must be zero or one");
    }
    return color.is_empty != 0U ? gui_drawing::Color::empty() :
                                 gui_drawing::Color::from_argb(color.argb);
}

[[nodiscard]] gd_color color_to_c(gui_drawing::Color color) noexcept {
    return {color.argb(), color.is_empty() ? 1U : 0U};
}

[[nodiscard]] gui_drawing::PointF point_from_c(gd_point point) {
    return {point.x, point.y};
}

[[nodiscard]] gui_drawing::RectF rect_from_c(gd_rect rect) {
    return {rect.x, rect.y, rect.width, rect.height};
}

[[nodiscard]] gui_drawing::Matrix matrix_from_c(gd_matrix matrix) {
    return {matrix.m11, matrix.m12, matrix.m21, matrix.m22,
            matrix.dx, matrix.dy};
}

[[nodiscard]] std::string string_from_c(gd_string_view value,
                                        std::string_view field) {
    if ((value.size != 0U && value.data == nullptr) ||
        value.size > static_cast<std::uint64_t>(std::numeric_limits<std::size_t>::max())) {
        throw std::invalid_argument(std::string(field) + " is not a bounded string view");
    }
    std::string result;
    if (value.size != 0U) {
        result.assign(value.data, static_cast<std::size_t>(value.size));
    }
    if (result.find('\0') != std::string::npos) {
        throw std::invalid_argument(std::string(field) + " may not contain NUL bytes");
    }
    return result;
}

struct ObjectRecord final {
    std::shared_ptr<gui_drawing::DrawingObject> object;
    std::thread::id owner_thread;
    std::uint32_t kind{};
    std::uint64_t external_references{1};
};

struct Slot final {
    std::uint32_t generation{1};
    std::shared_ptr<ObjectRecord> record;
};

class Registry final {
public:
    template <typename Object, typename... Arguments>
    gd_result create(std::uint32_t kind, gd_handle* output, Arguments&&... arguments) {
        if (output == nullptr) {
            return fail(GD_ERROR_INVALID_ARGUMENT, "drawing creation requires an output handle");
        }
        return translate([&] {
            auto record = std::make_shared<ObjectRecord>();
            record->object = std::make_shared<Object>(std::forward<Arguments>(arguments)...);
            record->owner_thread = std::this_thread::get_id();
            record->kind = kind;
            std::scoped_lock lock(mutex_);
            *output = allocate_locked(std::move(record));
            return GD_OK;
        });
    }

    template <typename Object>
    gd_result adopt(std::uint32_t kind, gd_handle* output,
                    std::unique_ptr<Object> object) {
        if (output == nullptr || !object) {
            return fail(GD_ERROR_INVALID_ARGUMENT,
                        "drawing adoption requires an object and output handle");
        }
        return translate([&] {
            auto record = std::make_shared<ObjectRecord>();
            record->object = std::shared_ptr<Object>(std::move(object));
            record->owner_thread = std::this_thread::get_id();
            record->kind = kind;
            std::scoped_lock lock(mutex_);
            *output = allocate_locked(std::move(record));
            return GD_OK;
        });
    }

    gd_result retain(gd_handle handle) {
        std::scoped_lock lock(mutex_);
        Slot* slot = find_locked(handle);
        if (slot == nullptr) return stale("retain");
        if (slot->record->owner_thread != std::this_thread::get_id()) {
            return wrong_thread("retain");
        }
        if (slot->record->external_references ==
            std::numeric_limits<std::uint64_t>::max()) {
            return fail(GD_ERROR_LIMIT_EXCEEDED, "drawing reference count overflow");
        }
        ++slot->record->external_references;
        return GD_OK;
    }

    gd_result release(gd_handle handle) {
        std::scoped_lock lock(mutex_);
        Slot* slot = find_locked(handle);
        if (slot == nullptr) return stale("release");
        if (slot->record->owner_thread != std::this_thread::get_id()) {
            return wrong_thread("release");
        }
        if (--slot->record->external_references == 0U) {
            if (!slot->record->object->is_disposed()) {
                slot->record->object->dispose();
            }
            recycle_locked(*slot);
        }
        return GD_OK;
    }

    gd_result dispose(gd_handle handle) {
        std::shared_ptr<ObjectRecord> record;
        if (const gd_result result = get(handle, 0U, record, true); result != GD_OK) {
            return result;
        }
        return translate([&] {
            record->object->dispose();
            return GD_OK;
        });
    }

    gd_result object_state(gd_handle handle, std::uint32_t* state) {
        if (state == nullptr) {
            return fail(GD_ERROR_INVALID_ARGUMENT, "object_state requires an output");
        }
        std::shared_ptr<ObjectRecord> record;
        if (const gd_result result = get(handle, 0U, record, true); result != GD_OK) {
            return result;
        }
        *state = static_cast<std::uint32_t>(record->object->state());
        return GD_OK;
    }

    gd_result object_kind(gd_handle handle, std::uint32_t* kind) {
        if (kind == nullptr) {
            return fail(GD_ERROR_INVALID_ARGUMENT, "object_kind requires an output");
        }
        std::shared_ptr<ObjectRecord> record;
        if (const gd_result result = get(handle, 0U, record, true); result != GD_OK) {
            return result;
        }
        *kind = record->kind;
        return GD_OK;
    }

    template <typename Object, typename Operation>
    gd_result with(gd_handle handle, std::uint32_t kind, Operation&& operation) {
        std::shared_ptr<ObjectRecord> record;
        if (const gd_result result = get(handle, kind, record, false); result != GD_OK) {
            return result;
        }
        auto object = kind == 0U ? std::dynamic_pointer_cast<Object>(record->object) :
                                  std::static_pointer_cast<Object>(record->object);
        if (!object) return fail(GD_ERROR_WRONG_HANDLE_KIND, "drawing handle type mismatch");
        return translate([&] { return operation(*object); });
    }

    template <typename Left, typename Right, typename Operation>
    gd_result with_two(gd_handle left_handle, std::uint32_t left_kind,
                       gd_handle right_handle, std::uint32_t right_kind,
                       Operation&& operation) {
        std::shared_ptr<ObjectRecord> left_record;
        if (const gd_result result = get(left_handle, left_kind, left_record, false);
            result != GD_OK) return result;
        std::shared_ptr<ObjectRecord> right_record;
        if (const gd_result result = get(right_handle, right_kind, right_record, false);
            result != GD_OK) return result;
        auto left = left_kind == 0U ? std::dynamic_pointer_cast<Left>(left_record->object) :
                                     std::static_pointer_cast<Left>(left_record->object);
        auto right = right_kind == 0U ? std::dynamic_pointer_cast<Right>(right_record->object) :
                                       std::static_pointer_cast<Right>(right_record->object);
        if (!left || !right) {
            return fail(GD_ERROR_WRONG_HANDLE_KIND, "drawing handle type mismatch");
        }
        return translate([&] { return operation(*left, *right); });
    }

    template <typename Operation>
    gd_result with_text_objects(gd_handle recorder_handle,
                                gd_handle font_handle,
                                gd_handle brush_handle,
                                gd_handle format_handle,
                                Operation&& operation) {
        std::shared_ptr<ObjectRecord> recorder_record;
        std::shared_ptr<ObjectRecord> font_record;
        std::shared_ptr<ObjectRecord> brush_record;
        std::shared_ptr<ObjectRecord> format_record;
        if (const auto result = get(recorder_handle, GD_OBJECT_RECORDER,
                                    recorder_record, false); result != GD_OK) return result;
        if (const auto result = get(font_handle, GD_OBJECT_FONT,
                                    font_record, false); result != GD_OK) return result;
        if (const auto result = get(brush_handle, GD_OBJECT_SOLID_BRUSH,
                                    brush_record, false); result != GD_OK) return result;
        if (const auto result = get(format_handle, GD_OBJECT_STRING_FORMAT,
                                    format_record, false); result != GD_OK) return result;
        auto recorder = std::dynamic_pointer_cast<gui_drawing::GraphicsRecorder>(recorder_record->object);
        auto font = std::dynamic_pointer_cast<gui_drawing::Font>(font_record->object);
        auto brush = std::dynamic_pointer_cast<gui_drawing::SolidBrush>(brush_record->object);
        auto format = std::dynamic_pointer_cast<gui_drawing::StringFormat>(format_record->object);
        if (!recorder || !font || !brush || !format) {
            return fail(GD_ERROR_WRONG_HANDLE_KIND, "drawing text handle type mismatch");
        }
        return translate([&] { return operation(*recorder, *font, *brush, *format); });
    }

private:
    gd_result get(gd_handle handle, std::uint32_t expected_kind,
                  std::shared_ptr<ObjectRecord>& record,
                  bool permit_disposed) {
        {
            std::scoped_lock lock(mutex_);
            Slot* slot = find_locked(handle);
            if (slot == nullptr) return stale("operation");
            record = slot->record;
        }
        if (record->owner_thread != std::this_thread::get_id()) {
            return wrong_thread("operation");
        }
        if (expected_kind != 0U && record->kind != expected_kind) {
            return fail(GD_ERROR_WRONG_HANDLE_KIND,
                        "drawing handle has the wrong object kind");
        }
        if (!permit_disposed && record->object->is_disposed()) {
            return fail(GD_ERROR_DISPOSED, "drawing object is disposed");
        }
        return GD_OK;
    }

    gd_handle allocate_locked(std::shared_ptr<ObjectRecord> record) {
        for (std::size_t index = 0; index < slots_.size(); ++index) {
            if (!slots_[index].record) {
                slots_[index].record = std::move(record);
                return {static_cast<std::uint32_t>(index + 1U), slots_[index].generation};
            }
        }
        if (slots_.size() >= std::numeric_limits<std::uint32_t>::max() - 1U) {
            throw std::length_error("drawing handle table exhausted");
        }
        slots_.push_back({});
        slots_.back().record = std::move(record);
        return {static_cast<std::uint32_t>(slots_.size()), slots_.back().generation};
    }

    Slot* find_locked(gd_handle handle) noexcept {
        if (handle.slot == 0U || handle.slot > slots_.size()) return nullptr;
        Slot& slot = slots_[handle.slot - 1U];
        return slot.record && slot.generation == handle.generation ? &slot : nullptr;
    }

    static void recycle_locked(Slot& slot) noexcept {
        slot.record.reset();
        ++slot.generation;
        if (slot.generation == 0U) ++slot.generation;
    }

    static gd_result stale(std::string_view operation) {
        return fail(GD_ERROR_STALE_HANDLE,
                    std::string(operation) + " received a stale drawing handle");
    }

    static gd_result wrong_thread(std::string_view operation) {
        return fail(GD_ERROR_WRONG_THREAD,
                    std::string(operation) + " must run on the drawing owner thread");
    }

    std::mutex mutex_;
    std::vector<Slot> slots_;
};

Registry& registry() {
    static Registry value;
    return value;
}

std::mutex raster_service_mutex;
gd_raster_service_v0 raster_service{};


[[nodiscard]] bool load_raster_service(gd_raster_service_v0& output) {
    std::scoped_lock lock(raster_service_mutex);
    if (raster_service.abi_version == 0U) return false;
    output = raster_service;
    return true;
}

gd_result api_last_error(gd_error_view* error) {
    if (error == nullptr) return fail(GD_ERROR_INVALID_ARGUMENT, "last_error requires output");
    error->code = static_cast<std::uint32_t>(last_error_code);
    error->message = {last_error_message.data(), last_error_message.size()};
    return GD_OK;
}

gd_result api_retain(gd_handle handle) { return registry().retain(handle); }
gd_result api_release(gd_handle handle) { return registry().release(handle); }
gd_result api_dispose(gd_handle handle) {
    std::uint32_t state{};
    if (const gd_result result = registry().object_state(handle, &state);
        result != GD_OK) return result;
    if (state == GD_OBJECT_DISPOSED) return registry().dispose(handle);
    std::uint32_t kind{};
    if (const gd_result result = registry().object_kind(handle, &kind);
        result != GD_OK) return result;
    if (kind == GD_OBJECT_BITMAP) {
        const gd_result lease_check = registry().with<gui_drawing::Bitmap>(
            handle, GD_OBJECT_BITMAP, [](gui_drawing::Bitmap& bitmap) {
                return gui_drawing::abi::platform::has_hdc_lease(bitmap) ?
                    fail(GD_ERROR_INVALID_ARGUMENT,
                         "bitmap cannot be disposed during an HDC lease") : GD_OK;
            });
        if (lease_check != GD_OK) return lease_check;
    }
    return registry().dispose(handle);
}
gd_result api_object_state(gd_handle handle, std::uint32_t* state) {
    return registry().object_state(handle, state);
}
gd_result api_object_kind(gd_handle handle, std::uint32_t* kind) {
    return registry().object_kind(handle, kind);
}

gd_result api_solid_brush_create(gd_color color, gd_handle* output) {
    return translate([&] {
        const auto native = color_from_c(color);
        return registry().create<gui_drawing::SolidBrush>(
            GD_OBJECT_SOLID_BRUSH, output, native);
    });
}

gd_result api_pen_create(gd_color color, double width, gd_handle* output) {
    return translate([&] {
        const auto native = color_from_c(color);
        return registry().create<gui_drawing::Pen>(GD_OBJECT_PEN, output, native, width);
    });
}

gd_result api_pen_set_width(gd_handle handle, double width) {
    return registry().with<gui_drawing::Pen>(handle, GD_OBJECT_PEN,
        [width](gui_drawing::Pen& pen) { pen.set_width(width); return GD_OK; });
}

gd_result api_pen_set_dash_style(gd_handle handle, std::uint32_t style) {
    if (style > GD_DASH_CUSTOM) {
        return fail(GD_ERROR_INVALID_ARGUMENT, "dash style is outside the declared enum");
    }
    return registry().with<gui_drawing::Pen>(handle, GD_OBJECT_PEN,
        [style](gui_drawing::Pen& pen) {
            pen.set_dash_style(static_cast<gui_drawing::DashStyle>(style));
            return GD_OK;
        });
}

gd_result api_pen_set_dash_pattern(gd_handle handle, const double* entries,
                                   std::uint64_t count) {
    if ((count != 0U && entries == nullptr) ||
        count > static_cast<std::uint64_t>(std::numeric_limits<std::size_t>::max())) {
        return fail(GD_ERROR_INVALID_ARGUMENT, "dash pattern is not a bounded array");
    }
    return registry().with<gui_drawing::Pen>(handle, GD_OBJECT_PEN,
        [entries, count](gui_drawing::Pen& pen) {
            pen.set_dash_pattern({entries, static_cast<std::size_t>(count)});
            return GD_OK;
        });
}

gd_result api_font_create(gd_string_view family, double size,
                          std::uint32_t style, std::uint32_t unit,
                          std::uint32_t charset, gd_handle* output) {
    if ((style & ~UINT32_C(0x0f)) != 0U || unit > 6U || charset > 255U) {
        return fail(GD_ERROR_INVALID_ARGUMENT, "font style, unit, or charset is invalid");
    }
    return translate([&] {
        std::string native_family = string_from_c(family, "font family");
        return registry().create<gui_drawing::Font>(
            GD_OBJECT_FONT, output, std::move(native_family), size,
            static_cast<gui_drawing::FontStyle>(style),
            static_cast<gui_drawing::GraphicsUnit>(unit),
            static_cast<std::uint8_t>(charset));
    });
}

gd_result api_string_format_create(std::uint32_t flags, gd_handle* output) {
    return registry().create<gui_drawing::StringFormat>(
        GD_OBJECT_STRING_FORMAT, output, flags);
}

gd_result api_string_format_set(gd_handle handle, std::uint32_t alignment,
                                std::uint32_t line_alignment,
                                std::uint32_t trimming, std::uint32_t flags) {
    if (alignment > GD_STRING_FAR || line_alignment > GD_STRING_FAR ||
        trimming > GD_TRIM_ELLIPSIS_PATH) {
        return fail(GD_ERROR_INVALID_ARGUMENT, "string format enum is invalid");
    }
    return registry().with<gui_drawing::StringFormat>(
        handle, GD_OBJECT_STRING_FORMAT,
        [=](gui_drawing::StringFormat& format) {
            format.set_alignment(static_cast<gui_drawing::StringAlignment>(alignment));
            format.set_line_alignment(static_cast<gui_drawing::StringAlignment>(line_alignment));
            format.set_trimming(static_cast<gui_drawing::StringTrimming>(trimming));
            format.set_flags(flags);
            return GD_OK;
        });
}

gd_result api_recorder_create(gd_handle* output) {
    return registry().create<gui_drawing::GraphicsRecorder>(GD_OBJECT_RECORDER, output);
}

gd_result api_recorder_save(gd_handle handle, std::uint64_t* token) {
    if (token == nullptr) return fail(GD_ERROR_INVALID_ARGUMENT, "save requires token output");
    return registry().with<gui_drawing::GraphicsRecorder>(
        handle, GD_OBJECT_RECORDER, [token](gui_drawing::GraphicsRecorder& recorder) {
            *token = recorder.save().value;
            return GD_OK;
        });
}

gd_result api_recorder_restore(gd_handle handle, std::uint64_t token) {
    return registry().with<gui_drawing::GraphicsRecorder>(
        handle, GD_OBJECT_RECORDER, [token](gui_drawing::GraphicsRecorder& recorder) {
            recorder.restore({token});
            return GD_OK;
        });
}

gd_result api_recorder_translate(gd_handle handle, double x, double y) {
    return registry().with<gui_drawing::GraphicsRecorder>(
        handle, GD_OBJECT_RECORDER, [=](gui_drawing::GraphicsRecorder& recorder) {
            recorder.translate(x, y);
            return GD_OK;
        });
}

gd_result api_recorder_set_transform(gd_handle handle, gd_matrix transform) {
    return registry().with<gui_drawing::GraphicsRecorder>(
        handle, GD_OBJECT_RECORDER, [=](gui_drawing::GraphicsRecorder& recorder) {
            recorder.set_transform(matrix_from_c(transform));
            return GD_OK;
        });
}

gd_result api_recorder_set_clip(gd_handle handle, gd_rect clip) {
    return registry().with<gui_drawing::GraphicsRecorder>(
        handle, GD_OBJECT_RECORDER, [=](gui_drawing::GraphicsRecorder& recorder) {
            recorder.set_clip(rect_from_c(clip));
            return GD_OK;
        });
}

gd_result api_recorder_reset_clip(gd_handle handle) {
    return registry().with<gui_drawing::GraphicsRecorder>(
        handle, GD_OBJECT_RECORDER, [](gui_drawing::GraphicsRecorder& recorder) {
            recorder.reset_clip();
            return GD_OK;
        });
}

gd_result api_recorder_set_quality(gd_handle handle,
                                   std::uint32_t smoothing,
                                   std::uint32_t interpolation,
                                   std::uint32_t pixel_offset,
                                   std::uint32_t compositing,
                                   std::uint32_t compositing_quality) {
    if (smoothing > 4U || interpolation > 7U || pixel_offset > 4U ||
        compositing > 1U || compositing_quality > 4U) {
        return fail(GD_ERROR_INVALID_ARGUMENT, "graphics quality enum is invalid");
    }
    return registry().with<gui_drawing::GraphicsRecorder>(
        handle, GD_OBJECT_RECORDER, [=](gui_drawing::GraphicsRecorder& recorder) {
            recorder.set_quality(
                static_cast<gui_drawing::SmoothingMode>(smoothing),
                static_cast<gui_drawing::InterpolationMode>(interpolation),
                static_cast<gui_drawing::PixelOffsetMode>(pixel_offset),
                static_cast<gui_drawing::CompositingMode>(compositing),
                static_cast<gui_drawing::CompositingQuality>(compositing_quality));
            return GD_OK;
        });
}

gd_result api_recorder_is_visible(gd_handle handle, gd_point point,
                                  std::uint32_t* visible) {
    if (visible == nullptr) {
        return fail(GD_ERROR_INVALID_ARGUMENT, "visibility query requires output");
    }
    return registry().with<gui_drawing::GraphicsRecorder>(
        handle, GD_OBJECT_RECORDER, [=](gui_drawing::GraphicsRecorder& recorder) {
            *visible = recorder.is_visible(point_from_c(point)) ? 1U : 0U;
            return GD_OK;
        });
}

gd_result api_recorder_clear(gd_handle handle, gd_color color) {
    return translate([&] {
        const auto native = color_from_c(color);
        return registry().with<gui_drawing::GraphicsRecorder>(
            handle, GD_OBJECT_RECORDER, [native](gui_drawing::GraphicsRecorder& recorder) {
                recorder.clear(native);
                return GD_OK;
            });
    });
}

gd_result api_recorder_fill_rectangle(gd_handle recorder, gd_handle brush,
                                      gd_rect rect) {
    return registry().with_two<gui_drawing::GraphicsRecorder, gui_drawing::Brush>(
        recorder, GD_OBJECT_RECORDER, brush, 0U,
        [rect](gui_drawing::GraphicsRecorder& graphics, gui_drawing::Brush& fill) {
            graphics.fill_rectangle(fill, rect_from_c(rect));
            return GD_OK;
        });
}

gd_result api_recorder_draw_rectangle(gd_handle recorder, gd_handle pen,
                                      gd_rect rect) {
    return registry().with_two<gui_drawing::GraphicsRecorder, gui_drawing::Pen>(
        recorder, GD_OBJECT_RECORDER, pen, GD_OBJECT_PEN,
        [rect](gui_drawing::GraphicsRecorder& graphics, gui_drawing::Pen& stroke) {
            graphics.draw_rectangle(stroke, rect_from_c(rect));
            return GD_OK;
        });
}

gd_result api_recorder_draw_line(gd_handle recorder, gd_handle pen,
                                 gd_point from, gd_point to) {
    return registry().with_two<gui_drawing::GraphicsRecorder, gui_drawing::Pen>(
        recorder, GD_OBJECT_RECORDER, pen, GD_OBJECT_PEN,
        [=](gui_drawing::GraphicsRecorder& graphics, gui_drawing::Pen& stroke) {
            graphics.draw_line(stroke, point_from_c(from), point_from_c(to));
            return GD_OK;
        });
}

gd_result api_recorder_draw_string(gd_handle recorder, gd_string_view text,
                                   gd_handle font, gd_handle brush,
                                   gd_point origin, gd_handle format) {
    return translate([&] {
        std::string native_text = string_from_c(text, "draw string text");
        return registry().with_text_objects(
            recorder, font, brush, format,
            [native_text = std::move(native_text), origin](
                gui_drawing::GraphicsRecorder& graphics,
                gui_drawing::Font& native_font,
                gui_drawing::SolidBrush& native_brush,
                gui_drawing::StringFormat& native_format) {
                graphics.draw_string(native_text, native_font, native_brush,
                                     point_from_c(origin), native_format);
                return GD_OK;
            });
    });
}

gd_result api_recorder_close(gd_handle handle) {
    return registry().with<gui_drawing::GraphicsRecorder>(
        handle, GD_OBJECT_RECORDER, [](gui_drawing::GraphicsRecorder& recorder) {
            recorder.close();
            return GD_OK;
        });
}

gd_result api_recorder_command_count(gd_handle handle, std::uint64_t* count) {
    if (count == nullptr) {
        return fail(GD_ERROR_INVALID_ARGUMENT, "command count requires output");
    }
    return registry().with<gui_drawing::GraphicsRecorder>(
        handle, GD_OBJECT_RECORDER, [count](gui_drawing::GraphicsRecorder& recorder) {
            *count = recorder.commands().size();
            return GD_OK;
        });
}

gd_result api_recorder_trace(gd_handle handle, char* buffer,
                             std::uint64_t capacity,
                             std::uint64_t* required_size) {
    if (required_size == nullptr) {
        return fail(GD_ERROR_INVALID_ARGUMENT, "trace read requires a size output");
    }
    return registry().with<gui_drawing::GraphicsRecorder>(
        handle, GD_OBJECT_RECORDER,
        [=](gui_drawing::GraphicsRecorder& recorder) {
            const std::string trace = recorder.deterministic_trace();
            *required_size = trace.size();
            if (capacity < trace.size() || (trace.size() != 0U && buffer == nullptr)) {
                return fail(GD_ERROR_BUFFER_TOO_SMALL,
                            "trace buffer is smaller than the required byte count");
            }
            if (!trace.empty()) std::memcpy(buffer, trace.data(), trace.size());
            return GD_OK;
        });
}

gd_result api_graphics_path_create(std::uint32_t fill_mode, gd_handle* output) {
    if (fill_mode > 1U) {
        return fail(GD_ERROR_INVALID_ARGUMENT, "path fill mode is outside the declared enum");
    }
    return registry().create<gui_drawing::GraphicsPath>(
        GD_OBJECT_GRAPHICS_PATH, output,
        static_cast<gui_drawing::FillMode>(fill_mode));
}

gd_result api_graphics_path_reset(gd_handle handle) {
    return registry().with<gui_drawing::GraphicsPath>(
        handle, GD_OBJECT_GRAPHICS_PATH, [](gui_drawing::GraphicsPath& path) {
            path.reset();
            return GD_OK;
        });
}

gd_result api_graphics_path_start_figure(gd_handle handle) {
    return registry().with<gui_drawing::GraphicsPath>(
        handle, GD_OBJECT_GRAPHICS_PATH, [](gui_drawing::GraphicsPath& path) {
            path.start_figure();
            return GD_OK;
        });
}

gd_result api_graphics_path_close_figure(gd_handle handle) {
    return registry().with<gui_drawing::GraphicsPath>(
        handle, GD_OBJECT_GRAPHICS_PATH, [](gui_drawing::GraphicsPath& path) {
            path.close_figure();
            return GD_OK;
        });
}

gd_result api_graphics_path_add_line(gd_handle handle, gd_point from, gd_point to) {
    return registry().with<gui_drawing::GraphicsPath>(
        handle, GD_OBJECT_GRAPHICS_PATH, [=](gui_drawing::GraphicsPath& path) {
            path.add_line(point_from_c(from), point_from_c(to));
            return GD_OK;
        });
}

gd_result api_graphics_path_add_rectangle(gd_handle handle, gd_rect rectangle) {
    return registry().with<gui_drawing::GraphicsPath>(
        handle, GD_OBJECT_GRAPHICS_PATH, [=](gui_drawing::GraphicsPath& path) {
            path.add_rectangle(rect_from_c(rectangle));
            return GD_OK;
        });
}

gd_result api_graphics_path_add_ellipse(gd_handle handle, gd_rect bounds) {
    return registry().with<gui_drawing::GraphicsPath>(
        handle, GD_OBJECT_GRAPHICS_PATH, [=](gui_drawing::GraphicsPath& path) {
            path.add_ellipse(rect_from_c(bounds));
            return GD_OK;
        });
}

gd_result api_graphics_path_bounds(gd_handle handle, gd_rect* bounds) {
    if (bounds == nullptr) {
        return fail(GD_ERROR_INVALID_ARGUMENT, "path bounds requires output");
    }
    return registry().with<gui_drawing::GraphicsPath>(
        handle, GD_OBJECT_GRAPHICS_PATH, [bounds](gui_drawing::GraphicsPath& path) {
            const auto value = path.bounds();
            *bounds = {value.x, value.y, value.width, value.height};
            return GD_OK;
        });
}

gd_result api_image_reference_create(std::uint64_t stable_id,
                                     std::uint32_t width,
                                     std::uint32_t height,
                                     std::uint32_t pixel_format,
                                     std::uint64_t generation,
                                     gd_handle* output) {
    if (pixel_format > 1U) {
        return fail(GD_ERROR_INVALID_ARGUMENT, "image pixel format is outside the declared enum");
    }
    return registry().create<gui_drawing::ImageReference>(
        GD_OBJECT_IMAGE_REFERENCE, output, stable_id, width, height,
        static_cast<gui_drawing::PixelFormat>(pixel_format), generation);
}

gd_result api_image_attributes_create(gd_handle* output) {
    return registry().create<gui_drawing::ImageAttributes>(
        GD_OBJECT_IMAGE_ATTRIBUTES, output);
}

gd_result api_image_attributes_set_color_matrix(gd_handle handle,
                                                const double* entries,
                                                std::uint64_t count) {
    if (entries == nullptr || count != 25U) {
        return fail(GD_ERROR_INVALID_ARGUMENT,
                    "image color matrix requires exactly 25 entries");
    }
    return registry().with<gui_drawing::ImageAttributes>(
        handle, GD_OBJECT_IMAGE_ATTRIBUTES,
        [entries](gui_drawing::ImageAttributes& attributes) {
            attributes.set_color_matrix(std::span<const double, 25>(entries, 25U));
            return GD_OK;
        });
}

gd_result api_image_attributes_reset(gd_handle handle) {
    return registry().with<gui_drawing::ImageAttributes>(
        handle, GD_OBJECT_IMAGE_ATTRIBUTES,
        [](gui_drawing::ImageAttributes& attributes) {
            attributes.reset_color_matrix();
            return GD_OK;
        });
}

gd_result api_recorder_draw_ellipse(gd_handle recorder, gd_handle pen,
                                    gd_rect bounds) {
    return registry().with_two<gui_drawing::GraphicsRecorder, gui_drawing::Pen>(
        recorder, GD_OBJECT_RECORDER, pen, GD_OBJECT_PEN,
        [bounds](gui_drawing::GraphicsRecorder& graphics, gui_drawing::Pen& stroke) {
            graphics.draw_ellipse(stroke, rect_from_c(bounds));
            return GD_OK;
        });
}

gd_result api_recorder_fill_ellipse(gd_handle recorder, gd_handle brush,
                                    gd_rect bounds) {
    return registry().with_two<gui_drawing::GraphicsRecorder, gui_drawing::Brush>(
        recorder, GD_OBJECT_RECORDER, brush, 0U,
        [bounds](gui_drawing::GraphicsRecorder& graphics,
                 gui_drawing::Brush& fill) {
            graphics.fill_ellipse(fill, rect_from_c(bounds));
            return GD_OK;
        });
}

gd_result api_recorder_fill_polygon(gd_handle recorder, gd_handle brush,
                                    const gd_point* points, std::uint64_t count,
                                    std::uint32_t fill_mode) {
    if (points == nullptr || count < 3U ||
        count > gui_drawing::GraphicsPath::maximum_elements || fill_mode > 1U) {
        return fail(GD_ERROR_INVALID_ARGUMENT,
                    "polygon requires bounded points and a declared fill mode");
    }
    return translate([&] {
        std::vector<gui_drawing::PointF> native_points;
        native_points.reserve(static_cast<std::size_t>(count));
        for (std::uint64_t index = 0; index < count; ++index) {
            native_points.push_back(point_from_c(points[index]));
        }
        return registry().with_two<gui_drawing::GraphicsRecorder,
                                   gui_drawing::Brush>(
            recorder, GD_OBJECT_RECORDER, brush, 0U,
            [&native_points, fill_mode](gui_drawing::GraphicsRecorder& graphics,
                                        gui_drawing::Brush& fill) {
                graphics.fill_polygon(
                    fill, native_points,
                    static_cast<gui_drawing::FillMode>(fill_mode));
                return GD_OK;
            });
    });
}

gd_result api_recorder_draw_path(gd_handle recorder, gd_handle pen,
                                 gd_handle path) {
    return registry().with_two<gui_drawing::GraphicsRecorder, gui_drawing::Pen>(
        recorder, GD_OBJECT_RECORDER, pen, GD_OBJECT_PEN,
        [path](gui_drawing::GraphicsRecorder& graphics, gui_drawing::Pen& stroke) {
            return registry().with<gui_drawing::GraphicsPath>(
                path, GD_OBJECT_GRAPHICS_PATH,
                [&graphics, &stroke](gui_drawing::GraphicsPath& native_path) {
                    graphics.draw_path(stroke, native_path);
                    return GD_OK;
                });
        });
}

gd_result api_recorder_fill_path(gd_handle recorder, gd_handle brush,
                                 gd_handle path) {
    return registry().with_two<gui_drawing::GraphicsRecorder,
                               gui_drawing::Brush>(
        recorder, GD_OBJECT_RECORDER, brush, 0U,
        [path](gui_drawing::GraphicsRecorder& graphics,
               gui_drawing::Brush& fill) {
            return registry().with<gui_drawing::GraphicsPath>(
                path, GD_OBJECT_GRAPHICS_PATH,
                [&graphics, &fill](gui_drawing::GraphicsPath& native_path) {
                    graphics.fill_path(fill, native_path);
                    return GD_OK;
                });
        });
}

gd_result api_recorder_draw_image(gd_handle recorder, gd_handle image,
                                  gd_rect destination, gd_rect source,
                                  gd_handle attributes) {
    return registry().with_two<gui_drawing::GraphicsRecorder,
                               gui_drawing::ImageReference>(
        recorder, GD_OBJECT_RECORDER, image, GD_OBJECT_IMAGE_REFERENCE,
        [=](gui_drawing::GraphicsRecorder& graphics,
            gui_drawing::ImageReference& native_image) {
            return registry().with<gui_drawing::ImageAttributes>(
                attributes, GD_OBJECT_IMAGE_ATTRIBUTES,
                [&](gui_drawing::ImageAttributes& native_attributes) {
                    graphics.draw_image(native_image, rect_from_c(destination),
                                        rect_from_c(source), native_attributes);
                    return GD_OK;
                });
        });
}

gd_result api_bitmap_create(std::uint32_t width, std::uint32_t height,
                            std::uint32_t pixel_format, gd_handle* output) {
    if (pixel_format > GD_PIXEL_RGBA32_PREMULTIPLIED) {
        return fail(GD_ERROR_INVALID_ARGUMENT,
                    "bitmap pixel format is outside the declared enum");
    }
    return registry().create<gui_drawing::Bitmap>(
        GD_OBJECT_BITMAP, output, width, height,
        static_cast<gui_drawing::PixelFormat>(pixel_format));
}

gd_result api_bitmap_dimensions(gd_handle handle, std::uint32_t* width,
                                std::uint32_t* height,
                                std::uint32_t* pixel_format,
                                std::uint64_t* generation) {
    if (width == nullptr || height == nullptr || pixel_format == nullptr ||
        generation == nullptr) {
        return fail(GD_ERROR_INVALID_ARGUMENT,
                    "bitmap dimensions requires all output fields");
    }
    return registry().with<gui_drawing::Bitmap>(
        handle, GD_OBJECT_BITMAP, [=](gui_drawing::Bitmap& bitmap) {
            *width = bitmap.width();
            *height = bitmap.height();
            *pixel_format = static_cast<std::uint32_t>(bitmap.pixel_format());
            *generation = bitmap.generation();
            return GD_OK;
        });
}

gd_result api_bitmap_get_pixel(gd_handle handle, std::uint32_t x,
                               std::uint32_t y, gd_color* color) {
    if (color == nullptr) {
        return fail(GD_ERROR_INVALID_ARGUMENT, "bitmap pixel read requires output");
    }
    return registry().with<gui_drawing::Bitmap>(
        handle, GD_OBJECT_BITMAP, [=](gui_drawing::Bitmap& bitmap) {
            *color = color_to_c(bitmap.get_pixel(x, y));
            return GD_OK;
        });
}

gd_result api_bitmap_set_pixel(gd_handle handle, std::uint32_t x,
                               std::uint32_t y, gd_color color) {
    return translate([&] {
        const auto native = color_from_c(color);
        return registry().with<gui_drawing::Bitmap>(
            handle, GD_OBJECT_BITMAP, [=](gui_drawing::Bitmap& bitmap) {
                bitmap.set_pixel(x, y, native);
                return GD_OK;
            });
    });
}

gd_result api_bitmap_lock(gd_handle handle, std::uint32_t mode,
                          gd_bitmap_lock_view* view) {
    if (view == nullptr || mode > GD_BITMAP_LOCK_READ_WRITE) {
        return fail(GD_ERROR_INVALID_ARGUMENT,
                    "bitmap lock requires output and a declared mode");
    }
    return registry().with<gui_drawing::Bitmap>(
        handle, GD_OBJECT_BITMAP, [=](gui_drawing::Bitmap& bitmap) {
            const auto native = bitmap.lock(
                static_cast<gui_drawing::BitmapLockMode>(mode));
            *view = {native.data, native.writable_data, native.row_bytes,
                     native.width, native.height,
                     static_cast<std::uint32_t>(native.pixel_format), native.token};
            return GD_OK;
        });
}

gd_result api_bitmap_unlock(gd_handle handle, std::uint64_t token) {
    return registry().with<gui_drawing::Bitmap>(
        handle, GD_OBJECT_BITMAP, [token](gui_drawing::Bitmap& bitmap) {
            bitmap.unlock(token);
            return GD_OK;
        });
}

gd_result api_bitmap_clone(gd_handle handle, gd_rect_i source,
                           gd_handle* output) {
    if (output == nullptr) {
        return fail(GD_ERROR_INVALID_ARGUMENT, "bitmap clone requires output");
    }
    return registry().with<gui_drawing::Bitmap>(
        handle, GD_OBJECT_BITMAP, [=](gui_drawing::Bitmap& bitmap) {
            auto clone = bitmap.clone({source.x, source.y,
                                       source.width, source.height});
            return registry().adopt(GD_OBJECT_BITMAP, output, std::move(clone));
        });
}

gd_result api_bitmap_make_transparent(gd_handle handle, gd_color key) {
    return translate([&] {
        const auto native = color_from_c(key);
        return registry().with<gui_drawing::Bitmap>(
            handle, GD_OBJECT_BITMAP, [native](gui_drawing::Bitmap& bitmap) {
                bitmap.make_transparent(native);
                return GD_OK;
            });
    });
}

gd_result api_bitmap_thumbnail(gd_handle handle, std::uint32_t width,
                               std::uint32_t height, gd_handle* output) {
    if (output == nullptr) {
        return fail(GD_ERROR_INVALID_ARGUMENT, "bitmap thumbnail requires output");
    }
    return registry().with<gui_drawing::Bitmap>(
        handle, GD_OBJECT_BITMAP, [=](gui_drawing::Bitmap& bitmap) {
            auto thumbnail = bitmap.thumbnail(width, height);
            return registry().adopt(GD_OBJECT_BITMAP, output, std::move(thumbnail));
        });
}

gd_result api_bitmap_adjusted(gd_handle bitmap_handle,
                              gd_handle attributes_handle,
                              gd_handle* output) {
    if (output == nullptr) {
        return fail(GD_ERROR_INVALID_ARGUMENT, "adjusted bitmap requires output");
    }
    return registry().with_two<gui_drawing::Bitmap,
                               gui_drawing::ImageAttributes>(
        bitmap_handle, GD_OBJECT_BITMAP,
        attributes_handle, GD_OBJECT_IMAGE_ATTRIBUTES,
        [output](gui_drawing::Bitmap& bitmap,
                 gui_drawing::ImageAttributes& attributes) {
            auto adjusted = bitmap.adjusted(attributes);
            return registry().adopt(GD_OBJECT_BITMAP, output, std::move(adjusted));
        });
}

gd_result api_recorder_draw_bitmap(gd_handle recorder, gd_handle bitmap,
                                   gd_rect destination, gd_rect source,
                                   gd_handle attributes) {
    return registry().with_two<gui_drawing::GraphicsRecorder,
                               gui_drawing::Bitmap>(
        recorder, GD_OBJECT_RECORDER, bitmap, GD_OBJECT_BITMAP,
        [=](gui_drawing::GraphicsRecorder& graphics,
            gui_drawing::Bitmap& native_bitmap) {
            return registry().with<gui_drawing::ImageAttributes>(
                attributes, GD_OBJECT_IMAGE_ATTRIBUTES,
                [&](gui_drawing::ImageAttributes& native_attributes) {
                    graphics.draw_image(native_bitmap, rect_from_c(destination),
                                        rect_from_c(source), native_attributes);
                    return GD_OK;
                });
        });
}

gd_result api_graphics_path_add_arc(gd_handle handle, gd_rect bounds,
                                    double start_angle, double sweep_angle) {
    return registry().with<gui_drawing::GraphicsPath>(
        handle, GD_OBJECT_GRAPHICS_PATH, [=](gui_drawing::GraphicsPath& path) {
            path.add_arc(rect_from_c(bounds), start_angle, sweep_angle);
            return GD_OK;
        });
}

gd_result api_graphics_path_add_path(gd_handle handle, gd_handle appended,
                                     std::uint32_t connect) {
    if (connect > 1U) {
        return fail(GD_ERROR_INVALID_ARGUMENT, "path connect must be zero or one");
    }
    return registry().with_two<gui_drawing::GraphicsPath, gui_drawing::GraphicsPath>(
        handle, GD_OBJECT_GRAPHICS_PATH, appended, GD_OBJECT_GRAPHICS_PATH,
        [connect](gui_drawing::GraphicsPath& path,
                  gui_drawing::GraphicsPath& source) {
            path.add_path(source, connect != 0U);
            return GD_OK;
        });
}

gd_result api_graphics_path_transform(gd_handle handle, gd_matrix transform) {
    return registry().with<gui_drawing::GraphicsPath>(
        handle, GD_OBJECT_GRAPHICS_PATH, [=](gui_drawing::GraphicsPath& path) {
            path.transform(matrix_from_c(transform));
            return GD_OK;
        });
}

gd_result api_graphics_path_is_visible(gd_handle handle, gd_point point,
                                       std::uint32_t* visible) {
    if (visible == nullptr) {
        return fail(GD_ERROR_INVALID_ARGUMENT, "path visibility requires output");
    }
    return registry().with<gui_drawing::GraphicsPath>(
        handle, GD_OBJECT_GRAPHICS_PATH, [=](gui_drawing::GraphicsPath& path) {
            *visible = path.is_visible(point_from_c(point)) ? 1U : 0U;
            return GD_OK;
        });
}

gd_result api_graphics_path_points(gd_handle handle, gd_point* points,
                                   std::uint64_t capacity,
                                   std::uint64_t* required_count) {
    if (required_count == nullptr) {
        return fail(GD_ERROR_INVALID_ARGUMENT, "path points requires count output");
    }
    return registry().with<gui_drawing::GraphicsPath>(
        handle, GD_OBJECT_GRAPHICS_PATH, [=](gui_drawing::GraphicsPath& path) {
            const std::vector<gui_drawing::PointF> native = path.path_points();
            *required_count = native.size();
            if (capacity < native.size() || (!native.empty() && points == nullptr)) {
                return fail(GD_ERROR_BUFFER_TOO_SMALL,
                            "path point buffer is smaller than the required count");
            }
            for (std::size_t index = 0; index < native.size(); ++index) {
                points[index] = {native[index].x, native[index].y};
            }
            return GD_OK;
        });
}

gd_result api_graphics_path_clone(gd_handle handle, gd_handle* output) {
    if (output == nullptr) {
        return fail(GD_ERROR_INVALID_ARGUMENT, "path clone requires output");
    }
    return registry().with<gui_drawing::GraphicsPath>(
        handle, GD_OBJECT_GRAPHICS_PATH, [=](gui_drawing::GraphicsPath& path) {
            return registry().adopt(GD_OBJECT_GRAPHICS_PATH, output, path.clone());
        });
}

gd_result api_image_attributes_set_remap_table(gd_handle handle,
                                               const gd_color_remap* entries,
                                               std::uint64_t count) {
    if ((count != 0U && entries == nullptr) || count > 4096U) {
        return fail(GD_ERROR_INVALID_ARGUMENT, "image remap table is not bounded");
    }
    return translate([&] {
        std::vector<gui_drawing::ImageAttributesSnapshot::ColorRemap> native;
        native.reserve(static_cast<std::size_t>(count));
        for (std::uint64_t index = 0; index < count; ++index) {
            native.push_back({color_from_c(entries[index].old_color),
                              color_from_c(entries[index].new_color)});
        }
        return registry().with<gui_drawing::ImageAttributes>(
            handle, GD_OBJECT_IMAGE_ATTRIBUTES,
            [&native](gui_drawing::ImageAttributes& attributes) {
                attributes.set_remap_table(native);
                return GD_OK;
            });
    });
}

gd_result api_image_attributes_reset_remap_table(gd_handle handle) {
    return registry().with<gui_drawing::ImageAttributes>(
        handle, GD_OBJECT_IMAGE_ATTRIBUTES,
        [](gui_drawing::ImageAttributes& attributes) {
            attributes.reset_remap_table();
            return GD_OK;
        });
}

gd_result api_hatch_brush_create(std::uint32_t style, gd_color foreground,
                                 gd_color background, gd_handle* output) {
    if (style > GD_HATCH_DIAGONAL_CROSS) {
        return fail(GD_ERROR_INVALID_ARGUMENT, "hatch style is outside the declared enum");
    }
    return translate([&] {
        return registry().create<gui_drawing::HatchBrush>(
            GD_OBJECT_HATCH_BRUSH, output,
            static_cast<gui_drawing::HatchStyle>(style),
            color_from_c(foreground), color_from_c(background));
    });
}

gd_result api_linear_gradient_brush_create(gd_rect bounds, gd_color first,
                                           gd_color second, double angle,
                                           std::uint32_t wrap_mode,
                                           gd_handle* output) {
    if (wrap_mode > GD_WRAP_CLAMP) {
        return fail(GD_ERROR_INVALID_ARGUMENT, "gradient wrap mode is invalid");
    }
    return translate([&] {
        return registry().create<gui_drawing::LinearGradientBrush>(
            GD_OBJECT_LINEAR_GRADIENT_BRUSH, output, rect_from_c(bounds),
            color_from_c(first), color_from_c(second), angle,
            static_cast<gui_drawing::WrapMode>(wrap_mode));
    });
}

gd_result api_linear_gradient_set_blend(gd_handle handle,
                                        const double* factors,
                                        const double* positions,
                                        std::uint64_t count) {
    if (count < 2U || count > 4096U || factors == nullptr || positions == nullptr) {
        return fail(GD_ERROR_INVALID_ARGUMENT, "gradient blend arrays are not bounded");
    }
    return registry().with<gui_drawing::LinearGradientBrush>(
        handle, GD_OBJECT_LINEAR_GRADIENT_BRUSH,
        [=](gui_drawing::LinearGradientBrush& brush) {
            brush.set_blend({factors, static_cast<std::size_t>(count)},
                            {positions, static_cast<std::size_t>(count)});
            return GD_OK;
        });
}

[[nodiscard]] gd_result set_gradient_interpolation(
    gd_handle handle, std::uint32_t kind, const gd_color* colors,
    const double* positions, std::uint64_t count) {
    if (count < 2U || count > 4096U || colors == nullptr || positions == nullptr) {
        return fail(GD_ERROR_INVALID_ARGUMENT,
                    "gradient interpolation arrays are not bounded");
    }
    return translate([&] {
        gui_drawing::ColorBlend blend;
        blend.colors.reserve(static_cast<std::size_t>(count));
        blend.positions.assign(positions, positions + count);
        for (std::uint64_t index = 0; index < count; ++index) {
            blend.colors.push_back(color_from_c(colors[index]));
        }
        if (kind == GD_OBJECT_LINEAR_GRADIENT_BRUSH) {
            return registry().with<gui_drawing::LinearGradientBrush>(
                handle, kind, [&blend](gui_drawing::LinearGradientBrush& brush) {
                    brush.set_interpolation_colors(blend);
                    return GD_OK;
                });
        }
        return registry().with<gui_drawing::PathGradientBrush>(
            handle, kind, [&blend](gui_drawing::PathGradientBrush& brush) {
                brush.set_interpolation_colors(blend);
                return GD_OK;
            });
    });
}

gd_result api_linear_gradient_set_interpolation(
    gd_handle handle, const gd_color* colors, const double* positions,
    std::uint64_t count) {
    return set_gradient_interpolation(handle, GD_OBJECT_LINEAR_GRADIENT_BRUSH,
                                      colors, positions, count);
}

gd_result api_linear_gradient_set_wrap_mode(gd_handle handle,
                                            std::uint32_t wrap_mode) {
    if (wrap_mode > GD_WRAP_CLAMP) {
        return fail(GD_ERROR_INVALID_ARGUMENT, "gradient wrap mode is invalid");
    }
    return registry().with<gui_drawing::LinearGradientBrush>(
        handle, GD_OBJECT_LINEAR_GRADIENT_BRUSH,
        [wrap_mode](gui_drawing::LinearGradientBrush& brush) {
            brush.set_wrap_mode(static_cast<gui_drawing::WrapMode>(wrap_mode));
            return GD_OK;
        });
}

gd_result api_path_gradient_brush_create(const gd_point* points,
                                         std::uint64_t count,
                                         std::uint32_t wrap_mode,
                                         gd_handle* output) {
    if (points == nullptr || count < 3U ||
        count > gui_drawing::GraphicsPath::maximum_elements ||
        wrap_mode > GD_WRAP_CLAMP) {
        return fail(GD_ERROR_INVALID_ARGUMENT, "path gradient inputs are not bounded");
    }
    return translate([&] {
        std::vector<gui_drawing::PointF> native;
        native.reserve(static_cast<std::size_t>(count));
        for (std::uint64_t index = 0; index < count; ++index) {
            native.push_back(point_from_c(points[index]));
        }
        return registry().create<gui_drawing::PathGradientBrush>(
            GD_OBJECT_PATH_GRADIENT_BRUSH, output,
            std::span<const gui_drawing::PointF>(native),
            static_cast<gui_drawing::WrapMode>(wrap_mode));
    });
}

gd_result api_path_gradient_set_center_color(gd_handle handle, gd_color color) {
    return translate([&] {
        const auto native = color_from_c(color);
        return registry().with<gui_drawing::PathGradientBrush>(
            handle, GD_OBJECT_PATH_GRADIENT_BRUSH,
            [native](gui_drawing::PathGradientBrush& brush) {
                brush.set_center_color(native);
                return GD_OK;
            });
    });
}

gd_result api_path_gradient_set_center_point(gd_handle handle, gd_point point) {
    return registry().with<gui_drawing::PathGradientBrush>(
        handle, GD_OBJECT_PATH_GRADIENT_BRUSH,
        [point](gui_drawing::PathGradientBrush& brush) {
            brush.set_center_point(point_from_c(point));
            return GD_OK;
        });
}

gd_result api_path_gradient_set_surround_colors(gd_handle handle,
                                                const gd_color* colors,
                                                std::uint64_t count) {
    if (count == 0U || count > gui_drawing::GraphicsPath::maximum_elements ||
        colors == nullptr) {
        return fail(GD_ERROR_INVALID_ARGUMENT, "surround color array is not bounded");
    }
    return translate([&] {
        std::vector<gui_drawing::Color> native;
        native.reserve(static_cast<std::size_t>(count));
        for (std::uint64_t index = 0; index < count; ++index) {
            native.push_back(color_from_c(colors[index]));
        }
        return registry().with<gui_drawing::PathGradientBrush>(
            handle, GD_OBJECT_PATH_GRADIENT_BRUSH,
            [&native](gui_drawing::PathGradientBrush& brush) {
                brush.set_surround_colors(native);
                return GD_OK;
            });
    });
}

gd_result api_path_gradient_set_interpolation(
    gd_handle handle, const gd_color* colors, const double* positions,
    std::uint64_t count) {
    return set_gradient_interpolation(handle, GD_OBJECT_PATH_GRADIENT_BRUSH,
                                      colors, positions, count);
}

gd_result api_region_create_rectangle(gd_rect rectangle, gd_handle* output) {
    return registry().create<gui_drawing::Region>(
        GD_OBJECT_REGION, output, rect_from_c(rectangle));
}

gd_result api_region_create_path(gd_handle path, gd_handle* output) {
    if (output == nullptr) {
        return fail(GD_ERROR_INVALID_ARGUMENT, "region path constructor requires output");
    }
    return registry().with<gui_drawing::GraphicsPath>(
        path, GD_OBJECT_GRAPHICS_PATH,
        [output](gui_drawing::GraphicsPath& native_path) {
            return registry().create<gui_drawing::Region>(
                GD_OBJECT_REGION, output, native_path);
        });
}

gd_result api_region_union_rectangle(gd_handle handle, gd_rect rectangle) {
    return registry().with<gui_drawing::Region>(
        handle, GD_OBJECT_REGION, [rectangle](gui_drawing::Region& region) {
            region.unite(rect_from_c(rectangle));
            return GD_OK;
        });
}

gd_result api_region_union_path(gd_handle handle, gd_handle path) {
    return registry().with_two<gui_drawing::Region, gui_drawing::GraphicsPath>(
        handle, GD_OBJECT_REGION, path, GD_OBJECT_GRAPHICS_PATH,
        [](gui_drawing::Region& region, gui_drawing::GraphicsPath& native_path) {
            region.unite(native_path);
            return GD_OK;
        });
}

gd_result api_region_exclude_rectangle(gd_handle handle, gd_rect rectangle) {
    return registry().with<gui_drawing::Region>(
        handle, GD_OBJECT_REGION, [rectangle](gui_drawing::Region& region) {
            region.exclude(rect_from_c(rectangle));
            return GD_OK;
        });
}

gd_result api_region_is_visible(gd_handle handle, gd_point point,
                                std::uint32_t* visible) {
    if (visible == nullptr) {
        return fail(GD_ERROR_INVALID_ARGUMENT, "region visibility requires output");
    }
    return registry().with<gui_drawing::Region>(
        handle, GD_OBJECT_REGION, [=](gui_drawing::Region& region) {
            *visible = region.is_visible(point_from_c(point)) ? 1U : 0U;
            return GD_OK;
        });
}

gd_result api_region_bounds(gd_handle handle, gd_rect* bounds) {
    if (bounds == nullptr) {
        return fail(GD_ERROR_INVALID_ARGUMENT, "region bounds requires output");
    }
    return registry().with<gui_drawing::Region>(
        handle, GD_OBJECT_REGION, [bounds](gui_drawing::Region& region) {
            const auto value = region.bounds();
            *bounds = {value.x, value.y, value.width, value.height};
            return GD_OK;
        });
}

gd_result api_raster_service_install(const gd_raster_service_v0* service) {
    if (service == nullptr || service->struct_size < sizeof(gd_raster_service_v0) ||
        service->abi_version != GD_ABI_VERSION_0_1 || service->execute == nullptr ||
        service->encode_png == nullptr || service->decode_png == nullptr) {
        return fail(GD_ERROR_INVALID_ARGUMENT,
                    "raster service requires a complete ABI 0.1 callback table");
    }
    std::scoped_lock lock(raster_service_mutex);
    if (raster_service.abi_version != 0U &&
        (raster_service.execute != service->execute ||
         raster_service.encode_png != service->encode_png ||
         raster_service.decode_png != service->decode_png)) {
        return fail(GD_ERROR_INVALID_ARGUMENT,
                    "a different GUI.Drawing raster service is already installed");
    }
    raster_service = *service;
    return GD_OK;
}

gd_result api_recorder_execute(gd_handle recorder, gd_handle bitmap,
                               std::uint64_t* commands_executed) {
    if (commands_executed == nullptr) {
        return fail(GD_ERROR_INVALID_ARGUMENT, "raster execution requires count output");
    }
    gd_raster_service_v0 service{};
    if (!load_raster_service(service)) {
        return fail(GD_ERROR_INTERNAL, "GUI.Drawing raster service is not installed");
    }
    return registry().with_two<gui_drawing::GraphicsRecorder, gui_drawing::Bitmap>(
        recorder, GD_OBJECT_RECORDER, bitmap, GD_OBJECT_BITMAP,
        [&](gui_drawing::GraphicsRecorder& native_recorder,
            gui_drawing::Bitmap& native_bitmap) {
            const gd_result result = service.execute(
                &native_recorder, &native_bitmap, commands_executed);
            return result == GD_OK ? GD_OK :
                fail(result, "GUI.Drawing retained raster execution failed");
        });
}

gd_result api_bitmap_encode_png(gd_handle bitmap, void* buffer,
                                std::uint64_t capacity,
                                std::uint64_t* required_size) {
    if (required_size == nullptr) {
        return fail(GD_ERROR_INVALID_ARGUMENT, "PNG encode requires size output");
    }
    gd_raster_service_v0 service{};
    if (!load_raster_service(service)) {
        return fail(GD_ERROR_INTERNAL, "GUI.Drawing raster service is not installed");
    }
    return registry().with<gui_drawing::Bitmap>(
        bitmap, GD_OBJECT_BITMAP, [&](gui_drawing::Bitmap& native_bitmap) {
            const gd_result result = service.encode_png(
                &native_bitmap, buffer, capacity, required_size);
            return result == GD_OK || result == GD_ERROR_BUFFER_TOO_SMALL ? result :
                fail(result, "GUI.Drawing PNG encode failed");
        });
}

gd_result api_bitmap_decode_png(const void* data, std::uint64_t size,
                                gd_handle* output) {
    if (data == nullptr || size == 0U || output == nullptr) {
        return fail(GD_ERROR_INVALID_ARGUMENT,
                    "PNG decode requires nonempty bytes and output handle");
    }
    gd_raster_service_v0 service{};
    if (!load_raster_service(service)) {
        return fail(GD_ERROR_INTERNAL, "GUI.Drawing raster service is not installed");
    }
    void* decoded = nullptr;
    const gd_result result = service.decode_png(data, size, &decoded);
    if (result != GD_OK || decoded == nullptr) {
        delete static_cast<gui_drawing::Bitmap*>(decoded);
        return fail(result == GD_OK ? GD_ERROR_INTERNAL : result,
                    "GUI.Drawing PNG decode failed");
    }
    std::unique_ptr<gui_drawing::Bitmap> bitmap(
        static_cast<gui_drawing::Bitmap*>(decoded));
    return registry().adopt(GD_OBJECT_BITMAP, output, std::move(bitmap));
}

gd_result api_image_attributes_clone(gd_handle handle, gd_handle* output) {
    if (output == nullptr) {
        return fail(GD_ERROR_INVALID_ARGUMENT,
                    "image attributes clone requires output");
    }
    return registry().with<gui_drawing::ImageAttributes>(
        handle, GD_OBJECT_IMAGE_ATTRIBUTES,
        [output](gui_drawing::ImageAttributes& attributes) {
            return registry().adopt(GD_OBJECT_IMAGE_ATTRIBUTES, output,
                                    attributes.clone());
        });
}

gd_result api_bitmap_export_hbitmap(gd_handle handle, gd_color background,
                                    std::uintptr_t* output) {
    if (output == nullptr) {
        return fail(GD_ERROR_INVALID_ARGUMENT, "HBITMAP export requires output");
    }
    *output = 0U;
    return registry().with<gui_drawing::Bitmap>(
        handle, GD_OBJECT_BITMAP, [&](gui_drawing::Bitmap& bitmap) {
            const gd_result result = gui_drawing::abi::platform::export_hbitmap(
                bitmap, color_from_c(background), *output);
            return result == GD_OK ? GD_OK :
                fail(result, "native bitmap export is unavailable");
        });
}

gd_result api_bitmap_import_hbitmap(std::uintptr_t source,
                                    gd_handle* output) {
    if (source == 0U || output == nullptr) {
        return fail(GD_ERROR_INVALID_ARGUMENT,
                    "HBITMAP import requires a native handle and output");
    }
    *output = {};
    return translate([&] {
        std::unique_ptr<gui_drawing::Bitmap> bitmap;
        const gd_result result = gui_drawing::abi::platform::import_hbitmap(
            source, bitmap);
        if (result != GD_OK) return fail(result, "native bitmap import is unavailable");
        return registry().adopt(GD_OBJECT_BITMAP, output, std::move(bitmap));
    });
}

gd_result api_native_surface_capture(std::uintptr_t source, std::uint32_t kind,
                                     gd_handle* output, gd_rect* captured_bounds) {
    if (source == 0U || output == nullptr || captured_bounds == nullptr ||
        kind > GD_NATIVE_SURFACE_HWND) {
        return fail(GD_ERROR_INVALID_ARGUMENT,
                    "native surface capture requires a handle, kind, and output");
    }
    *output = {};
    *captured_bounds = {};
    return translate([&] {
        gui_drawing::abi::platform::CapturedSurface captured;
        const gd_result result = gui_drawing::abi::platform::capture_surface(
            source, kind, captured);
        if (result != GD_OK) return fail(result, "native surface capture is unavailable");
        const gd_result adopted = registry().adopt(
            GD_OBJECT_BITMAP, output, std::move(captured.bitmap));
        if (adopted == GD_OK) {
            *captured_bounds = {captured.bounds.x, captured.bounds.y,
                                captured.bounds.width, captured.bounds.height};
        }
        return adopted;
    });
}

gd_result api_native_surface_present(std::uintptr_t destination,
                                     std::uint32_t kind, gd_handle handle) {
    if (destination == 0U || kind > GD_NATIVE_SURFACE_HWND) {
        return fail(GD_ERROR_INVALID_ARGUMENT,
                    "native surface presentation requires a handle and kind");
    }
    return registry().with<gui_drawing::Bitmap>(
        handle, GD_OBJECT_BITMAP, [&](gui_drawing::Bitmap& bitmap) {
            const gd_result result = gui_drawing::abi::platform::present_surface(
                destination, kind, bitmap);
            return result == GD_OK ? GD_OK :
                fail(result, "native surface presentation is unavailable");
        });
}

gd_result api_bitmap_acquire_hdc(gd_handle handle, std::uintptr_t* output,
                                 std::uint64_t* lease_token) {
    if (output == nullptr || lease_token == nullptr) {
        return fail(GD_ERROR_INVALID_ARGUMENT,
                    "HDC acquisition requires handle and token outputs");
    }
    *output = 0U;
    *lease_token = 0U;
    const gd_result acquired = registry().with<gui_drawing::Bitmap>(
        handle, GD_OBJECT_BITMAP, [&](gui_drawing::Bitmap& bitmap) {
            const gd_result result = gui_drawing::abi::platform::acquire_hdc(
                bitmap, *output, *lease_token);
            return result == GD_OK ? GD_OK :
                fail(result, "bitmap-backed device-context lease is unavailable");
        });
    if (acquired != GD_OK) return acquired;
    const gd_result retained = registry().retain(handle);
    if (retained == GD_OK) return GD_OK;
    registry().with<gui_drawing::Bitmap>(
        handle, GD_OBJECT_BITMAP, [&](gui_drawing::Bitmap& bitmap) {
            return gui_drawing::abi::platform::release_hdc(bitmap, *lease_token);
        });
    *output = 0U;
    *lease_token = 0U;
    return retained;
}

gd_result api_bitmap_release_hdc(gd_handle handle, std::uint64_t lease_token) {
    if (lease_token == 0U) {
        return fail(GD_ERROR_INVALID_ARGUMENT, "HDC release requires a lease token");
    }
    const gd_result released = registry().with<gui_drawing::Bitmap>(
        handle, GD_OBJECT_BITMAP, [&](gui_drawing::Bitmap& bitmap) {
            const gd_result result = gui_drawing::abi::platform::release_hdc(
                bitmap, lease_token);
            return result == GD_OK ? GD_OK :
                fail(result, "bitmap-backed device-context release failed");
        });
    if (released != GD_OK) return released;
    return registry().release(handle);
}

const gd_api_v0 api_table{
    sizeof(gd_api_v0), GD_ABI_VERSION_0_1,
    &api_last_error, &api_retain, &api_release, &api_dispose,
    &api_object_state, &api_object_kind,
    &api_solid_brush_create, &api_pen_create, &api_pen_set_width,
    &api_pen_set_dash_style, &api_pen_set_dash_pattern,
    &api_font_create, &api_string_format_create, &api_string_format_set,
    &api_recorder_create, &api_recorder_save, &api_recorder_restore,
    &api_recorder_translate, &api_recorder_set_transform,
    &api_recorder_set_clip, &api_recorder_reset_clip,
    &api_recorder_set_quality, &api_recorder_is_visible,
    &api_recorder_clear, &api_recorder_fill_rectangle,
    &api_recorder_draw_rectangle, &api_recorder_draw_line,
    &api_recorder_draw_string, &api_recorder_close,
    &api_recorder_command_count, &api_recorder_trace,
    &api_graphics_path_create, &api_graphics_path_reset,
    &api_graphics_path_start_figure, &api_graphics_path_close_figure,
    &api_graphics_path_add_line, &api_graphics_path_add_rectangle,
    &api_graphics_path_add_ellipse, &api_graphics_path_bounds,
    &api_image_reference_create, &api_image_attributes_create,
    &api_image_attributes_set_color_matrix, &api_image_attributes_reset,
    &api_recorder_draw_ellipse, &api_recorder_fill_ellipse,
    &api_recorder_fill_polygon, &api_recorder_draw_path,
    &api_recorder_fill_path, &api_recorder_draw_image,
    &api_bitmap_create, &api_bitmap_dimensions, &api_bitmap_get_pixel,
    &api_bitmap_set_pixel, &api_bitmap_lock, &api_bitmap_unlock,
    &api_bitmap_clone, &api_bitmap_make_transparent,
    &api_bitmap_thumbnail, &api_bitmap_adjusted,
    &api_recorder_draw_bitmap,
    &api_graphics_path_add_arc, &api_graphics_path_add_path,
    &api_graphics_path_transform, &api_graphics_path_is_visible,
    &api_graphics_path_points, &api_graphics_path_clone,
    &api_image_attributes_set_remap_table,
    &api_image_attributes_reset_remap_table,
    &api_hatch_brush_create, &api_linear_gradient_brush_create,
    &api_linear_gradient_set_blend,
    &api_linear_gradient_set_interpolation,
    &api_linear_gradient_set_wrap_mode,
    &api_path_gradient_brush_create,
    &api_path_gradient_set_center_color,
    &api_path_gradient_set_center_point,
    &api_path_gradient_set_surround_colors,
    &api_path_gradient_set_interpolation,
    &api_region_create_rectangle, &api_region_create_path,
    &api_region_union_rectangle, &api_region_union_path,
    &api_region_exclude_rectangle, &api_region_is_visible,
    &api_region_bounds,
    &api_raster_service_install, &api_recorder_execute,
    &api_bitmap_encode_png, &api_bitmap_decode_png,
    &api_image_attributes_clone,
    &api_bitmap_export_hbitmap, &api_bitmap_import_hbitmap,
    &api_native_surface_capture, &api_native_surface_present,
    &api_bitmap_acquire_hdc, &api_bitmap_release_hdc,
};

} // namespace

extern "C" GD_C_API_EXPORT gd_result gd_get_api_v0(
    std::uint32_t requested_version, gd_api_v0* table) {
    if (table == nullptr || table->struct_size < sizeof(std::uint32_t) * 2U) {
        return fail(GD_ERROR_INVALID_ARGUMENT,
                    "drawing ABI negotiation requires a size-prefixed table");
    }
    if (requested_version != GD_ABI_VERSION_0_1) {
        return fail(GD_ERROR_UNSUPPORTED_VERSION,
                    "requested GUI.Drawing ABI version is unsupported");
    }
    const std::uint32_t caller_size = table->struct_size;
    const std::size_t copied = std::min<std::size_t>(caller_size, sizeof(api_table));
    std::memcpy(table, &api_table, copied);
    table->struct_size = sizeof(api_table);
    table->abi_version = GD_ABI_VERSION_0_1;
    return GD_OK;
}
