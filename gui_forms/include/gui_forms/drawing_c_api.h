#ifndef GUI_FORMS_DRAWING_C_API_H
#define GUI_FORMS_DRAWING_C_API_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_WIN32)
#if defined(GD_C_API_BUILD)
#define GD_C_API_EXPORT __declspec(dllexport)
#else
#define GD_C_API_EXPORT __declspec(dllimport)
#endif
#elif defined(__GNUC__) || defined(__clang__)
#define GD_C_API_EXPORT __attribute__((visibility("default")))
#else
#define GD_C_API_EXPORT
#endif

/* Experimental GUI.Drawing ABI. Nothing in this header is a 1.0 promise. */
#define GD_ABI_VERSION_0_1 UINT32_C(0x00000001)
#define GD_ABI_VERSION_0_2 UINT32_C(0x00000002)

typedef struct gd_handle {
    uint32_t slot;
    uint32_t generation;
} gd_handle;

typedef struct gd_string_view {
    const char* data;
    uint64_t size;
} gd_string_view;

typedef struct gd_color {
    uint32_t argb;
    uint32_t is_empty;
} gd_color;

typedef struct gd_point {
    double x;
    double y;
} gd_point;

typedef struct gd_size {
    double width;
    double height;
} gd_size;

typedef struct gd_rect {
    double x;
    double y;
    double width;
    double height;
} gd_rect;

typedef struct gd_rect_i {
    int32_t x;
    int32_t y;
    int32_t width;
    int32_t height;
} gd_rect_i;

typedef struct gd_color_remap {
    gd_color old_color;
    gd_color new_color;
} gd_color_remap;

typedef struct gd_matrix {
    double m11;
    double m12;
    double m21;
    double m22;
    double dx;
    double dy;
} gd_matrix;

typedef enum gd_result {
    GD_OK = 0,
    GD_ERROR_INVALID_ARGUMENT = 1,
    GD_ERROR_STALE_HANDLE = 2,
    GD_ERROR_WRONG_HANDLE_KIND = 3,
    GD_ERROR_WRONG_THREAD = 4,
    GD_ERROR_DISPOSED = 5,
    GD_ERROR_BUFFER_TOO_SMALL = 6,
    GD_ERROR_UNSUPPORTED_VERSION = 7,
    GD_ERROR_LIMIT_EXCEEDED = 8,
    GD_ERROR_INTERNAL = 9
} gd_result;

typedef enum gd_object_state {
    GD_OBJECT_ALIVE = 0,
    GD_OBJECT_DISPOSING = 1,
    GD_OBJECT_DISPOSED = 2
} gd_object_state;

typedef enum gd_object_kind {
    GD_OBJECT_SOLID_BRUSH = 1,
    GD_OBJECT_PEN = 2,
    GD_OBJECT_FONT = 3,
    GD_OBJECT_STRING_FORMAT = 4,
    GD_OBJECT_RECORDER = 5,
    GD_OBJECT_GRAPHICS_PATH = 6,
    GD_OBJECT_IMAGE_REFERENCE = 7,
    GD_OBJECT_IMAGE_ATTRIBUTES = 8,
    GD_OBJECT_BITMAP = 9,
    GD_OBJECT_HATCH_BRUSH = 10,
    GD_OBJECT_LINEAR_GRADIENT_BRUSH = 11,
    GD_OBJECT_PATH_GRADIENT_BRUSH = 12,
    GD_OBJECT_REGION = 13
} gd_object_kind;

typedef enum gd_wrap_mode {
    GD_WRAP_TILE = 0,
    GD_WRAP_TILE_FLIP_X = 1,
    GD_WRAP_TILE_FLIP_Y = 2,
    GD_WRAP_TILE_FLIP_XY = 3,
    GD_WRAP_CLAMP = 4
} gd_wrap_mode;

typedef enum gd_hatch_style {
    GD_HATCH_HORIZONTAL = 0,
    GD_HATCH_VERTICAL = 1,
    GD_HATCH_FORWARD_DIAGONAL = 2,
    GD_HATCH_BACKWARD_DIAGONAL = 3,
    GD_HATCH_CROSS = 4,
    GD_HATCH_DIAGONAL_CROSS = 5
} gd_hatch_style;

typedef enum gd_dash_style {
    GD_DASH_SOLID = 0,
    GD_DASH_DASH = 1,
    GD_DASH_DOT = 2,
    GD_DASH_DASH_DOT = 3,
    GD_DASH_DASH_DOT_DOT = 4,
    GD_DASH_CUSTOM = 5
} gd_dash_style;

typedef enum gd_string_alignment {
    GD_STRING_NEAR = 0,
    GD_STRING_CENTER = 1,
    GD_STRING_FAR = 2
} gd_string_alignment;

typedef enum gd_string_trimming {
    GD_TRIM_NONE = 0,
    GD_TRIM_CHARACTER = 1,
    GD_TRIM_WORD = 2,
    GD_TRIM_ELLIPSIS_CHARACTER = 3,
    GD_TRIM_ELLIPSIS_WORD = 4,
    GD_TRIM_ELLIPSIS_PATH = 5
} gd_string_trimming;

typedef enum gd_fill_mode {
    GD_FILL_ALTERNATE = 0,
    GD_FILL_WINDING = 1
} gd_fill_mode;

typedef enum gd_pixel_format {
    GD_PIXEL_BGRA32_PREMULTIPLIED = 0,
    GD_PIXEL_RGBA32_PREMULTIPLIED = 1
} gd_pixel_format;

typedef enum gd_bitmap_lock_mode {
    GD_BITMAP_LOCK_READ = 0,
    GD_BITMAP_LOCK_WRITE = 1,
    GD_BITMAP_LOCK_READ_WRITE = 2
} gd_bitmap_lock_mode;

typedef enum gd_native_surface_kind {
    GD_NATIVE_SURFACE_HDC = 0,
    GD_NATIVE_SURFACE_HWND = 1
} gd_native_surface_kind;

typedef struct gd_bitmap_lock_view {
    const void* data;
    void* writable_data;
    uint64_t row_bytes;
    uint32_t width;
    uint32_t height;
    uint32_t pixel_format;
    uint64_t token;
} gd_bitmap_lock_view;

typedef struct gd_bitmap_edit_view {
    const void* data;
    void* writable_data;
    uint64_t row_bytes;
    gd_rect_i bounds;
    uint32_t pixel_format;
    uint64_t token;
} gd_bitmap_edit_view;

typedef struct gd_bitmap_damage_summary {
    uint64_t from_generation;
    uint64_t to_generation;
    uint32_t history_complete;
} gd_bitmap_damage_summary;

typedef struct gd_error_view {
    uint32_t code;
    gd_string_view message;
} gd_error_view;

/*
 * Private same-build renderer attachment. The callbacks receive GUI.Drawing
 * C++ object addresses only after the handle registry has validated kind,
 * lifetime, owner thread, and disposal. This seam keeps gd_get_api_v0 and the
 * semantic ABI renderer-free while allowing a separately packaged raster
 * module to execute retained commands.
 */
typedef struct gd_raster_service_v0 {
    uint32_t struct_size;
    uint32_t abi_version;
    gd_result (*execute)(const void* recorder, void* bitmap,
                         uint64_t first_command,
                         uint64_t* commands_executed);
    gd_result (*encode_png)(const void* bitmap, void* buffer,
                            uint64_t capacity, uint64_t* required_size);
    gd_result (*decode_png)(const void* data, uint64_t size,
                            void** bitmap);
    gd_result (*measure_string)(const void* font, const void* format,
                                gd_string_view text, double layout_width,
                                gd_size* measured);
} gd_raster_service_v0;

typedef struct gd_api_v0 {
    uint32_t struct_size;
    uint32_t abi_version;

    gd_result (*last_error)(gd_error_view* error);
    gd_result (*retain)(gd_handle handle);
    gd_result (*release)(gd_handle handle);
    gd_result (*dispose)(gd_handle handle);
    gd_result (*object_state)(gd_handle handle, uint32_t* state);
    gd_result (*object_kind)(gd_handle handle, uint32_t* kind);

    gd_result (*solid_brush_create)(gd_color color, gd_handle* brush);
    gd_result (*pen_create)(gd_color color, double width, gd_handle* pen);
    gd_result (*pen_set_width)(gd_handle pen, double width);
    gd_result (*pen_set_dash_style)(gd_handle pen, uint32_t style);
    gd_result (*pen_set_dash_pattern)(gd_handle pen,
                                      const double* entries,
                                      uint64_t count);
    gd_result (*font_create)(gd_string_view family, double size,
                             uint32_t style, uint32_t unit,
                             uint32_t charset, gd_handle* font);
    gd_result (*string_format_create)(uint32_t flags, gd_handle* format);
    gd_result (*string_format_set)(gd_handle format,
                                   uint32_t alignment,
                                   uint32_t line_alignment,
                                   uint32_t trimming,
                                   uint32_t flags);

    gd_result (*recorder_create)(gd_handle* recorder);
    gd_result (*recorder_save)(gd_handle recorder, uint64_t* token);
    gd_result (*recorder_restore)(gd_handle recorder, uint64_t token);
    gd_result (*recorder_translate)(gd_handle recorder, double x, double y);
    gd_result (*recorder_set_transform)(gd_handle recorder, gd_matrix transform);
    gd_result (*recorder_set_clip)(gd_handle recorder, gd_rect clip);
    gd_result (*recorder_reset_clip)(gd_handle recorder);
    gd_result (*recorder_set_quality)(gd_handle recorder,
                                      uint32_t smoothing,
                                      uint32_t interpolation,
                                      uint32_t pixel_offset,
                                      uint32_t compositing,
                                      uint32_t compositing_quality);
    gd_result (*recorder_is_visible)(gd_handle recorder, gd_point point,
                                     uint32_t* visible);
    gd_result (*recorder_clear)(gd_handle recorder, gd_color color);
    gd_result (*recorder_fill_rectangle)(gd_handle recorder, gd_handle brush,
                                         gd_rect rect);
    gd_result (*recorder_draw_rectangle)(gd_handle recorder, gd_handle pen,
                                         gd_rect rect);
    gd_result (*recorder_draw_line)(gd_handle recorder, gd_handle pen,
                                    gd_point from, gd_point to);
    gd_result (*recorder_draw_string)(gd_handle recorder, gd_string_view text,
                                      gd_handle font, gd_handle brush,
                                      gd_point origin, gd_handle format);
    gd_result (*recorder_close)(gd_handle recorder);
    gd_result (*recorder_command_count)(gd_handle recorder, uint64_t* count);
    gd_result (*recorder_trace)(gd_handle recorder,
                                char* buffer,
                                uint64_t capacity,
                                uint64_t* required_size);

    /* Appended M11e vocabulary; preserves the original ABI 0.1 prefix. */
    gd_result (*graphics_path_create)(uint32_t fill_mode, gd_handle* path);
    gd_result (*graphics_path_reset)(gd_handle path);
    gd_result (*graphics_path_start_figure)(gd_handle path);
    gd_result (*graphics_path_close_figure)(gd_handle path);
    gd_result (*graphics_path_add_line)(gd_handle path, gd_point from, gd_point to);
    gd_result (*graphics_path_add_rectangle)(gd_handle path, gd_rect rectangle);
    gd_result (*graphics_path_add_ellipse)(gd_handle path, gd_rect bounds);
    gd_result (*graphics_path_bounds)(gd_handle path, gd_rect* bounds);
    gd_result (*image_reference_create)(uint64_t stable_id,
                                        uint32_t width,
                                        uint32_t height,
                                        uint32_t pixel_format,
                                        uint64_t generation,
                                        gd_handle* image);
    gd_result (*image_attributes_create)(gd_handle* attributes);
    gd_result (*image_attributes_set_color_matrix)(gd_handle attributes,
                                                   const double* entries,
                                                   uint64_t count);
    gd_result (*image_attributes_reset)(gd_handle attributes);
    gd_result (*recorder_draw_ellipse)(gd_handle recorder, gd_handle pen,
                                       gd_rect bounds);
    gd_result (*recorder_fill_ellipse)(gd_handle recorder, gd_handle brush,
                                       gd_rect bounds);
    gd_result (*recorder_fill_polygon)(gd_handle recorder, gd_handle brush,
                                       const gd_point* points, uint64_t count,
                                       uint32_t fill_mode);
    gd_result (*recorder_draw_path)(gd_handle recorder, gd_handle pen,
                                    gd_handle path);
    gd_result (*recorder_fill_path)(gd_handle recorder, gd_handle brush,
                                    gd_handle path);
    gd_result (*recorder_draw_image)(gd_handle recorder, gd_handle image,
                                     gd_rect destination, gd_rect source,
                                     gd_handle attributes);

    /* Owned CPU bitmap and pixel-lease operations appended by M11f. */
    gd_result (*bitmap_create)(uint32_t width, uint32_t height,
                               uint32_t pixel_format, gd_handle* bitmap);
    gd_result (*bitmap_dimensions)(gd_handle bitmap, uint32_t* width,
                                   uint32_t* height, uint32_t* pixel_format,
                                   uint64_t* generation);
    gd_result (*bitmap_get_pixel)(gd_handle bitmap, uint32_t x, uint32_t y,
                                  gd_color* color);
    gd_result (*bitmap_set_pixel)(gd_handle bitmap, uint32_t x, uint32_t y,
                                  gd_color color);
    gd_result (*bitmap_lock)(gd_handle bitmap, uint32_t mode,
                             gd_bitmap_lock_view* view);
    gd_result (*bitmap_unlock)(gd_handle bitmap, uint64_t token);
    gd_result (*bitmap_clone)(gd_handle bitmap, gd_rect_i source,
                              gd_handle* clone);
    gd_result (*bitmap_make_transparent)(gd_handle bitmap, gd_color key);
    gd_result (*bitmap_thumbnail)(gd_handle bitmap, uint32_t width,
                                  uint32_t height, gd_handle* thumbnail);
    gd_result (*bitmap_adjusted)(gd_handle bitmap, gd_handle attributes,
                                 gd_handle* adjusted);
    gd_result (*recorder_draw_bitmap)(gd_handle recorder, gd_handle bitmap,
                                      gd_rect destination, gd_rect source,
                                      gd_handle attributes);

    /* M11f retained path, image-adjustment, and brush vocabulary. */
    gd_result (*graphics_path_add_arc)(gd_handle path, gd_rect bounds,
                                       double start_angle, double sweep_angle);
    gd_result (*graphics_path_add_path)(gd_handle path, gd_handle appended,
                                        uint32_t connect);
    gd_result (*graphics_path_transform)(gd_handle path, gd_matrix transform);
    gd_result (*graphics_path_is_visible)(gd_handle path, gd_point point,
                                          uint32_t* visible);
    gd_result (*graphics_path_points)(gd_handle path, gd_point* points,
                                      uint64_t capacity, uint64_t* required_count);
    gd_result (*graphics_path_clone)(gd_handle path, gd_handle* clone);
    gd_result (*image_attributes_set_remap_table)(gd_handle attributes,
                                                  const gd_color_remap* entries,
                                                  uint64_t count);
    gd_result (*image_attributes_reset_remap_table)(gd_handle attributes);
    gd_result (*hatch_brush_create)(uint32_t style, gd_color foreground,
                                    gd_color background, gd_handle* brush);
    gd_result (*linear_gradient_brush_create)(gd_rect bounds, gd_color first,
                                              gd_color second, double angle,
                                              uint32_t wrap_mode,
                                              gd_handle* brush);
    gd_result (*linear_gradient_set_blend)(gd_handle brush,
                                           const double* factors,
                                           const double* positions,
                                           uint64_t count);
    gd_result (*linear_gradient_set_interpolation)(gd_handle brush,
                                                   const gd_color* colors,
                                                   const double* positions,
                                                   uint64_t count);
    gd_result (*linear_gradient_set_wrap_mode)(gd_handle brush,
                                               uint32_t wrap_mode);
    gd_result (*path_gradient_brush_create)(const gd_point* points,
                                            uint64_t count,
                                            uint32_t wrap_mode,
                                            gd_handle* brush);
    gd_result (*path_gradient_set_center_color)(gd_handle brush, gd_color color);
    gd_result (*path_gradient_set_center_point)(gd_handle brush, gd_point point);
    gd_result (*path_gradient_set_surround_colors)(gd_handle brush,
                                                   const gd_color* colors,
                                                   uint64_t count);
    gd_result (*path_gradient_set_interpolation)(gd_handle brush,
                                                 const gd_color* colors,
                                                 const double* positions,
                                                 uint64_t count);
    gd_result (*region_create_rectangle)(gd_rect rectangle, gd_handle* region);
    gd_result (*region_create_path)(gd_handle path, gd_handle* region);
    gd_result (*region_union_rectangle)(gd_handle region, gd_rect rectangle);
    gd_result (*region_union_path)(gd_handle region, gd_handle path);
    gd_result (*region_exclude_rectangle)(gd_handle region, gd_rect rectangle);
    gd_result (*region_is_visible)(gd_handle region, gd_point point,
                                   uint32_t* visible);
    gd_result (*region_bounds)(gd_handle region, gd_rect* bounds);
    gd_result (*raster_service_install)(const gd_raster_service_v0* service);
    gd_result (*recorder_execute)(gd_handle recorder, gd_handle bitmap,
                                  uint64_t* commands_executed);
    gd_result (*bitmap_encode_png)(gd_handle bitmap, void* buffer,
                                   uint64_t capacity, uint64_t* required_size);
    gd_result (*bitmap_decode_png)(const void* data, uint64_t size,
                                   gd_handle* bitmap);
    gd_result (*image_attributes_clone)(gd_handle attributes,
                                        gd_handle* clone);
    /* D8 platform extension. Returned HBITMAP ownership transfers to caller. */
    gd_result (*bitmap_export_hbitmap)(gd_handle bitmap, gd_color background,
                                       uintptr_t* hbitmap);
    /* D8 platform extension. The source HBITMAP remains caller-owned. */
    gd_result (*bitmap_import_hbitmap)(uintptr_t hbitmap, gd_handle* bitmap);
    /* D8 captures an opaque Windows surface into owned GUI.Drawing storage. */
    gd_result (*native_surface_capture)(uintptr_t surface, uint32_t kind,
                                        gd_handle* bitmap, gd_rect* bounds);
    /* D8 presents owned GUI.Drawing storage back to the borrowed surface. */
    gd_result (*native_surface_present)(uintptr_t surface, uint32_t kind,
                                        gd_handle bitmap);
    /* D8 tokenized bitmap-backed HDC lease. */
    gd_result (*bitmap_acquire_hdc)(gd_handle bitmap, uintptr_t* hdc,
                                    uint64_t* lease_token);
    gd_result (*bitmap_release_hdc)(gd_handle bitmap, uint64_t lease_token);
    /* M11g renderer-owned glyph metrics; a zero format handle means defaults. */
    gd_result (*measure_string)(gd_string_view text, gd_handle font,
                                gd_handle format, double layout_width,
                                gd_size* measured);
    /* M11g incremental retained commit; returns commands run from first_command. */
    gd_result (*recorder_execute_from)(gd_handle recorder, gd_handle bitmap,
                                       uint64_t first_command,
                                       uint64_t* commands_executed);
    /* M12-P26 transactional rectangular edit and multi-consumer damage query. */
    gd_result (*bitmap_edit_begin)(gd_handle bitmap, gd_rect_i bounds,
                                   gd_bitmap_edit_view* view);
    gd_result (*bitmap_edit_commit)(gd_handle bitmap, uint64_t token,
                                    uint64_t* generation);
    gd_result (*bitmap_edit_cancel)(gd_handle bitmap, uint64_t token);
    gd_result (*bitmap_changes_since)(gd_handle bitmap, uint64_t generation,
                                      gd_rect_i* rectangles, uint64_t capacity,
                                      uint64_t* required_count,
                                      gd_bitmap_damage_summary* summary);
    /* M12-P27 refreshes an existing bitmap from a borrowed native surface. */
    gd_result (*native_surface_refresh)(uintptr_t surface, uint32_t kind,
                                        gd_handle bitmap, gd_rect* bounds);
    /* M12-P27 submits a coherent frame directly to a retained host endpoint. */
    gd_result (*native_surface_publish_retained)(uintptr_t surface, uint32_t kind,
                                                 gd_handle bitmap);
} gd_api_v0;

/*
 * The caller sets table->struct_size. The implementation copies only the
 * common prefix and reports its complete table size on success.
 */
GD_C_API_EXPORT gd_result gd_get_api_v0(uint32_t requested_version,
                                        gd_api_v0* table);

#ifdef __cplusplus
}
#endif

#endif
