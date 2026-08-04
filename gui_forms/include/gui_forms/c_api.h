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
    GF_EVENT_STATE_CHANGED = 1
} gf_event_kind;

typedef void (*gf_event_callback)(gf_handle sender, uint32_t event_kind, void* context);

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
