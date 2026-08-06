#ifndef GUI_FORMS_C_API_H
#define GUI_FORMS_C_API_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_WIN32)
#if defined(GF_C_API_BUILD)
#define GF_C_API_EXPORT __declspec(dllexport)
#else
#define GF_C_API_EXPORT __declspec(dllimport)
#endif
#elif defined(__GNUC__) || defined(__clang__)
#define GF_C_API_EXPORT __attribute__((visibility("default")))
#else
#define GF_C_API_EXPORT
#endif

/* Experimental 0.x ABI. Nothing in this header is a GUI.Forms 1.0 promise. */
#define GF_ABI_VERSION_0_1 UINT32_C(0x00000001)
#define GF_ABI_VERSION_0_2 UINT32_C(0x00000002)
#define GF_ABI_VERSION_0_3 UINT32_C(0x00000003)
#define GF_ABI_VERSION_0_4 UINT32_C(0x00000004)
#define GF_ABI_VERSION_0_5 UINT32_C(0x00000005)
#define GF_ABI_VERSION_0_6 UINT32_C(0x00000006)
#define GF_ABI_VERSION_0_7 UINT32_C(0x00000007)
#define GF_ABI_VERSION_0_8 UINT32_C(0x00000008)
#define GF_ABI_VERSION_0_9 UINT32_C(0x00000009)
#define GF_ABI_VERSION_0_10 UINT32_C(0x0000000a)
#define GF_ABI_VERSION_0_11 UINT32_C(0x0000000b)
#define GF_ABI_VERSION_0_12 UINT32_C(0x0000000c)
#define GF_ABI_VERSION_0_13 UINT32_C(0x0000000d)
#define GF_ABI_VERSION_0_14 UINT32_C(0x0000000e)
#define GF_ABI_VERSION_0_15 UINT32_C(0x0000000f)
#define GF_ABI_VERSION_0_16 UINT32_C(0x00000010)
#define GF_ABI_VERSION_0_17 UINT32_C(0x00000011)
#define GF_ABI_VERSION_0_18 UINT32_C(0x00000012)
#define GF_ABI_VERSION_0_19 UINT32_C(0x00000013)

#define GF_PIXEL_FORMAT_BGRA32_PREMULTIPLIED UINT32_C(1)

typedef struct gf_handle {
    uint32_t slot;
    uint32_t generation;
} gf_handle;

typedef gf_handle gf_event_token;

typedef struct gf_string_view {
    const char* data;
    uint64_t size;
} gf_string_view;

typedef struct gf_rect {
    double x;
    double y;
    double width;
    double height;
} gf_rect;

typedef struct gf_field_edit_result {
    uint64_t anchor_utf8;
    uint64_t caret_utf8;
    uint64_t revision;
    uint32_t changed;
    uint32_t can_undo;
    uint32_t can_redo;
} gf_field_edit_result;

typedef enum gf_result {
    GF_OK = 0,
    GF_ERROR_INVALID_ARGUMENT = 1,
    GF_ERROR_STALE_HANDLE = 2,
    GF_ERROR_WRONG_HANDLE_KIND = 3,
    GF_ERROR_WRONG_THREAD = 4,
    GF_ERROR_DISPOSED = 5,
    GF_ERROR_BUFFER_TOO_SMALL = 6,
    GF_ERROR_UNSUPPORTED_VERSION = 7,
    GF_ERROR_INTERNAL = 8
} gf_result;

typedef enum gf_component_state {
    GF_COMPONENT_ALIVE = 0,
    GF_COMPONENT_DISPOSING = 1,
    GF_COMPONENT_DISPOSED = 2
} gf_component_state;

typedef enum gf_event_kind {
    GF_EVENT_STATE_CHANGED = 1,
    GF_EVENT_CLICKED = 2,
    GF_EVENT_FORM_CLOSING = 3,
    GF_EVENT_FORM_CLOSED = 4,
    GF_EVENT_MOUSE_MOVE = 5,
    GF_EVENT_MOUSE_DOWN = 6,
    GF_EVENT_MOUSE_UP = 7,
    GF_EVENT_MOUSE_WHEEL = 8,
    GF_EVENT_MOUSE_ENTER = 9,
    GF_EVENT_MOUSE_LEAVE = 10,
    GF_EVENT_KEY_DOWN = 11,
    GF_EVENT_KEY_UP = 12,
    GF_EVENT_TEXT_INPUT = 13,
    GF_EVENT_RANGE_VALUE_CHANGED = 14,
    GF_EVENT_RANGE_SCROLL = 15,
    GF_EVENT_BOUNDS_CHANGED = 16
} gf_event_kind;

typedef enum gf_event_callback_result {
    GF_EVENT_CALLBACK_CONTINUE = 0,
    GF_EVENT_CALLBACK_CANCEL = 1,
    GF_EVENT_CALLBACK_FAULTED = 2
} gf_event_callback_result;

typedef enum gf_control_kind {
    GF_CONTROL_GENERIC = 0,
    GF_CONTROL_FORM = 1,
    GF_CONTROL_USER_CONTROL = 2,
    GF_CONTROL_PANEL = 3,
    GF_CONTROL_BUTTON = 4,
    GF_CONTROL_CHECK_BOX = 5,
    GF_CONTROL_COMBO_BOX = 6,
    GF_CONTROL_LABEL = 7,
    GF_CONTROL_LIST_BOX = 8,
    GF_CONTROL_TEXT_BOX = 9,
    GF_CONTROL_TRACK_BAR = 10,
    GF_CONTROL_RADIO_BUTTON = 11,
    GF_CONTROL_GROUP_BOX = 12,
    GF_CONTROL_PROGRESS_BAR = 13,
    GF_CONTROL_LINK_LABEL = 14,
    GF_CONTROL_PICTURE_BOX = 15,
    GF_CONTROL_DATA_GRID_VIEW = 16,
    GF_CONTROL_TOOL_STRIP = 17,
    GF_CONTROL_NUMERIC_UP_DOWN = 18,
    /* Retained layout participant which never becomes a pointer target. */
    GF_CONTROL_INPUT_TRANSPARENT = 19,
    /* Owner-painted retained surface which remains pointer transparent. */
    GF_CONTROL_INPUT_TRANSPARENT_CUSTOM = 20,
    GF_CONTROL_CUSTOM = 0x7fffffff
} gf_control_kind;

typedef enum gf_cursor_kind {
    GF_CURSOR_INHERIT = 0,
    GF_CURSOR_ARROW = 1,
    GF_CURSOR_TEXT = 2,
    GF_CURSOR_HAND = 3,
    GF_CURSOR_CROSSHAIR = 4,
    GF_CURSOR_RESIZE_HORIZONTAL = 5,
    GF_CURSOR_RESIZE_VERTICAL = 6,
    GF_CURSOR_WAIT = 7,
    GF_CURSOR_FORBIDDEN = 8
} gf_cursor_kind;

typedef enum gf_window_run_flag {
    GF_WINDOW_RUN_DEFAULT = 0,
    GF_WINDOW_RUN_AUTOMATION_CLOSE = 1 << 0,
    GF_WINDOW_RUN_FORCE_HEADLESS = 1 << 1,
    GF_WINDOW_RUN_AUTOMATION_ACTIVATE = 1 << 2,
    /* Run an unattached custom surface as a transient retained popup. */
    GF_WINDOW_RUN_POPUP = 1 << 3
} gf_window_run_flag;

typedef enum gf_path_dialog_kind {
    GF_PATH_DIALOG_OPEN_FILE = 1,
    GF_PATH_DIALOG_SAVE_FILE = 2,
    GF_PATH_DIALOG_SELECT_FOLDER = 3
} gf_path_dialog_kind;

typedef enum gf_path_dialog_flag {
    GF_PATH_DIALOG_DEFAULT = 0,
    GF_PATH_DIALOG_ALLOW_MULTIPLE = 1 << 0,
    GF_PATH_DIALOG_CONFIRM_OVERWRITE = 1 << 1
} gf_path_dialog_flag;

typedef void (*gf_event_callback)(gf_handle sender, uint32_t event_kind, void* context);
typedef uint32_t (*gf_event_callback_v2)(gf_handle sender,
                                         uint32_t event_kind,
                                         void* context);
typedef uint32_t (*gf_dispatch_callback)(void* context, uint32_t cancelled);
typedef uint32_t (*gf_pointer_callback)(gf_handle sender,
                                        uint32_t event_kind,
                                        double x,
                                        double y,
                                        double wheel_delta,
                                        uint32_t button,
                                        void* context);
typedef uint32_t (*gf_key_callback)(gf_handle sender,
                                    uint32_t event_kind,
                                    uint32_t physical_key,
                                    uint32_t modifiers,
                                    uint32_t repeat,
                                    void* context);
typedef uint32_t (*gf_text_callback)(gf_handle sender,
                                     gf_string_view text,
                                     uint32_t composing,
                                     int32_t replacement_start,
                                     int32_t replacement_length,
                                     void* context);

typedef struct gf_error_view {
    uint32_t code;
    gf_string_view message;
} gf_error_view;

typedef struct gf_api_v0 {
    uint32_t struct_size;
    uint32_t abi_version;

    gf_result (*last_error)(gf_error_view* error);
    gf_result (*control_create)(gf_string_view stable_id, gf_handle* control);
    gf_result (*retain)(gf_handle handle);
    gf_result (*release)(gf_handle handle);
    gf_result (*dispose)(gf_handle handle);
    gf_result (*component_state)(gf_handle handle, uint32_t* state);
    gf_result (*stable_id)(gf_handle handle,
                           char* buffer,
                           uint64_t capacity,
                           uint64_t* required_size);
    gf_result (*set_visible)(gf_handle handle, uint32_t visible);
    gf_result (*get_visible)(gf_handle handle, uint32_t* visible);
    gf_result (*set_bounds)(gf_handle handle, gf_rect bounds);
    gf_result (*get_bounds)(gf_handle handle, gf_rect* bounds);
    gf_result (*add_child)(gf_handle parent, gf_handle child);
    gf_result (*remove_child)(gf_handle parent, gf_handle child);
    gf_result (*subscribe)(gf_handle sender,
                           uint32_t event_kind,
                           gf_event_callback callback,
                           void* context,
                           gf_event_token* token);
    gf_result (*disconnect)(gf_event_token token);

    /* ABI 0.2 additions. The 0.1 table is the prefix ending at disconnect. */
    gf_result (*control_create_kind)(uint32_t kind,
                                     gf_string_view stable_id,
                                     gf_handle* control);
    gf_result (*set_name)(gf_handle handle, gf_string_view name);
    gf_result (*get_name)(gf_handle handle,
                          char* buffer,
                          uint64_t capacity,
                          uint64_t* required_size);
    gf_result (*set_text)(gf_handle handle, gf_string_view text);
    gf_result (*get_text)(gf_handle handle,
                          char* buffer,
                          uint64_t capacity,
                          uint64_t* required_size);
    gf_result (*set_enabled)(gf_handle handle, uint32_t enabled);
    gf_result (*get_enabled)(gf_handle handle, uint32_t* enabled);

    /* ABI 0.3 additions. */
    gf_result (*run_window)(gf_handle form, uint32_t flags);
    gf_result (*last_host_trace)(gf_handle form,
                                 char* buffer,
                                 uint64_t capacity,
                                 uint64_t* required_size);

    /* ABI 0.4 additions. */
    gf_result (*subscribe_v2)(gf_handle sender,
                              uint32_t event_kind,
                              gf_event_callback_v2 callback,
                              void* context,
                              gf_event_token* token);
    gf_result (*begin_invoke)(gf_handle control,
                              gf_dispatch_callback callback,
                              void* context);
    gf_result (*request_close)(gf_handle form);
    gf_result (*callback_fault_count)(gf_handle control, uint64_t* count);

    /* ABI 0.5 addition: retained owner-draw compatibility raster. */
    gf_result (*set_control_png)(gf_handle control,
                                 const uint8_t* encoded_png,
                                 uint64_t encoded_size);
    gf_result (*set_child_index)(gf_handle parent,
                                 gf_handle child,
                                 uint64_t index);
    gf_result (*set_control_colors)(gf_handle control,
                                    uint32_t foreground_argb,
                                    uint32_t background_argb);

    /* ABI 0.6 addition: pointer delivery for retained owner-draw controls. */
    gf_result (*subscribe_pointer)(gf_handle sender,
                                   gf_pointer_callback callback,
                                   void* context,
                                   gf_event_token* token);

    /* ABI 0.7 additions: native/managed checked-state projection. */
    gf_result (*set_check_state)(gf_handle control, uint32_t check_state);
    gf_result (*get_check_state)(gf_handle control, uint32_t* check_state);

    /* ABI 0.8 additions: focused key and composed text delivery. */
    gf_result (*subscribe_key)(gf_handle sender,
                               gf_key_callback callback,
                               void* context,
                               gf_event_token* token);
    gf_result (*subscribe_text)(gf_handle sender,
                                gf_text_callback callback,
                                void* context,
                                gf_event_token* token);

    /* ABI 0.9 additions: retained range state and explicit pointer capture. */
    gf_result (*set_range)(gf_handle control, double minimum, double maximum);
    gf_result (*get_range)(gf_handle control, double* minimum, double* maximum);
    gf_result (*set_range_value)(gf_handle control, double value);
    gf_result (*get_range_value)(gf_handle control, double* value);
    gf_result (*set_pointer_capture)(gf_handle control, uint32_t captured);
    gf_result (*get_pointer_capture)(gf_handle control, uint32_t* captured);

    /* ABI 0.10 additions: portable common-dialog and tooltip host adapters. */
    gf_result (*show_path_dialog)(gf_handle owner,
                                  uint32_t kind,
                                  gf_string_view title,
                                  gf_string_view initial_directory,
                                  gf_string_view suggested_name,
                                  gf_string_view default_extension,
                                  gf_string_view filter,
                                  uint32_t flags,
                                  uint32_t* accepted);
    gf_result (*last_dialog_path)(gf_handle owner,
                                  char* buffer,
                                  uint64_t capacity,
                                  uint64_t* required_size);
    gf_result (*show_tooltip)(gf_handle owner,
                              gf_string_view text,
                              double x,
                              double y,
                              uint32_t duration_milliseconds);
    gf_result (*hide_tooltip)(gf_handle owner);

    /* ABI 0.11 addition: retained field selection and caret projection. */
    gf_result (*set_field_selection)(gf_handle control,
                                     uint64_t selection_start_utf8,
                                     uint64_t selection_length_utf8,
                                     uint32_t caret_visible);

    /* ABI 0.12 additions: directional edit state and renderer-owned hit test. */
    gf_result (*set_field_edit_state)(gf_handle control,
                                      uint64_t anchor_utf8,
                                      uint64_t caret_utf8,
                                      uint32_t caret_visible);
    gf_result (*field_position_from_point)(gf_handle control,
                                           double local_x,
                                           uint64_t* position_utf8);

    /* ABI 0.13 additions: bounded host clipboard text transport. */
    gf_result (*write_clipboard_text)(gf_handle owner, gf_string_view text);
    gf_result (*read_clipboard_text)(gf_handle owner,
                                     char* buffer,
                                     uint64_t capacity,
                                     uint64_t* required_size,
                                     uint32_t* has_text);

    /* ABI 0.14 addition: grapheme-safe field navigation. Direction is -1/+1. */
    gf_result (*field_navigate)(gf_handle control,
                                uint64_t position_utf8,
                                int32_t direction,
                                uint64_t* result_utf8);

    /* ABI 0.15 additions: native field mutation and deterministic history. */
    gf_result (*field_replace)(gf_handle control,
                               uint64_t start_utf8,
                               uint64_t length_utf8,
                               gf_string_view replacement,
                               gf_field_edit_result* result);
    gf_result (*field_history)(gf_handle control,
                               int32_t direction,
                               gf_field_edit_result* result);
    gf_result (*field_clear_history)(gf_handle control);

    /* ABI 0.16 addition: owned, unencoded renderer-neutral paint surface. */
    gf_result (*set_control_pixels)(gf_handle control,
                                    const uint8_t* pixels,
                                    uint32_t width,
                                    uint32_t height,
                                    uint64_t row_bytes,
                                    uint32_t pixel_format);

    /* ABI 0.17 addition: renderer-authoritative bounds in root client space. */
    gf_result (*get_control_absolute_bounds)(gf_handle control,
                                             gf_rect* bounds);

    /*
     * ABI 0.18 addition: form-level preview for dialog/focus-scope keys.
     * Returning GF_EVENT_CALLBACK_CANCEL marks the key handled before it
     * reaches the focused descendant; CONTINUE preserves ordinary routing.
     */
    gf_result (*subscribe_key_preview)(gf_handle form,
                                       gf_key_callback callback,
                                       void* context,
                                       gf_event_token* token);

    /* ABI 0.19 additions: inherited retained cursor projection. */
    gf_result (*set_cursor)(gf_handle control, uint32_t cursor_kind);
    gf_result (*get_cursor)(gf_handle control, uint32_t* cursor_kind);
} gf_api_v0;

/*
 * The caller sets table->struct_size before entry. The implementation copies
 * only the common prefix and reports its complete table size on success.
 */
GF_C_API_EXPORT gf_result gf_get_api_v0(uint32_t requested_version,
                                        gf_api_v0* table);


#ifdef __cplusplus
}
#endif

#endif
