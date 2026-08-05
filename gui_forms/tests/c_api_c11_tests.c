#include "gui_forms/c_api.h"

#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static gf_api_v0 api;

static void require(int condition, const char* message) {
    if (!condition) {
        fprintf(stderr, "gui_forms_c_api_c11_tests: %s\n", message);
        exit(EXIT_FAILURE);
    }
}

static gf_string_view text(const char* value) {
    gf_string_view result;
    result.data = value;
    result.size = (uint64_t)strlen(value);
    return result;
}

struct callback_context {
    unsigned calls;
    gf_result dispose_result;
};

static void dispose_sender(gf_handle sender, uint32_t event_kind, void* opaque) {
    struct callback_context* context = (struct callback_context*)opaque;
    require(event_kind == GF_EVENT_STATE_CHANGED, "callback event kind changed");
    ++context->calls;
    context->dispose_result = api.dispose(sender);
}

static void count_callback(gf_handle sender, uint32_t event_kind, void* opaque) {
    unsigned* calls = (unsigned*)opaque;
    (void)sender;
    require(event_kind == GF_EVENT_STATE_CHANGED, "count callback event kind changed");
    ++*calls;
}

static uint32_t count_click_callback(gf_handle sender, uint32_t event_kind,
                                     void* opaque) {
    unsigned* calls = (unsigned*)opaque;
    (void)sender;
    require(event_kind == GF_EVENT_CLICKED, "click callback event kind changed");
    ++*calls;
    return GF_EVENT_CALLBACK_CONTINUE;
}

struct worker_context {
    gf_handle control;
    gf_result result;
};

struct m11c_context {
    gf_handle form;
    gf_handle button;
    unsigned clicks;
    unsigned dispatches;
    unsigned closing;
    unsigned closed;
};

struct close_cancel_context {
    unsigned closing;
    unsigned closed;
};

struct prehost_dispatch_context {
    gf_handle form;
    unsigned dispatched;
    unsigned cancelled;
    unsigned close_when_dispatched;
};

struct pointer_context {
    unsigned moves;
    unsigned downs;
    unsigned ups;
    double x;
    double y;
};

static uint32_t pointer_event(gf_handle sender, uint32_t event_kind,
                              double x, double y, double wheel_delta,
                              uint32_t button, void* opaque) {
    struct pointer_context* context = (struct pointer_context*)opaque;
    (void)sender;
    (void)wheel_delta;
    (void)button;
    context->x = x;
    context->y = y;
    if (event_kind == GF_EVENT_MOUSE_MOVE) {
        ++context->moves;
    } else if (event_kind == GF_EVENT_MOUSE_DOWN) {
        ++context->downs;
    } else if (event_kind == GF_EVENT_MOUSE_UP) {
        ++context->ups;
    }
    return GF_EVENT_CALLBACK_CONTINUE;
}

static uint32_t prehost_dispatch(void* opaque, uint32_t cancelled) {
    struct prehost_dispatch_context* context =
        (struct prehost_dispatch_context*)opaque;
    if (cancelled != 0U) {
        ++context->cancelled;
        return GF_EVENT_CALLBACK_CONTINUE;
    }
    ++context->dispatched;
    if (context->close_when_dispatched != 0U) {
        require(api.request_close(context->form) == GF_OK,
                "pre-host dispatch could not request close");
    }
    return GF_EVENT_CALLBACK_CONTINUE;
}

static uint32_t m11c_dispatch(void* opaque, uint32_t cancelled) {
    struct m11c_context* context = (struct m11c_context*)opaque;
    require(cancelled == 0U, "M11c dispatch was unexpectedly cancelled");
    ++context->dispatches;
    require(api.set_text(context->button, text("Dispatched")) == GF_OK,
            "queued UI-thread mutation failed");
    require(api.request_close(context->form) == GF_OK,
            "queued programmatic close failed");
    return GF_EVENT_CALLBACK_CONTINUE;
}

static uint32_t m11c_event(gf_handle sender, uint32_t event_kind, void* opaque) {
    struct m11c_context* context = (struct m11c_context*)opaque;
    if (event_kind == GF_EVENT_CLICKED) {
        require(sender.slot == context->button.slot,
                "clicked callback sender changed");
        ++context->clicks;
        require(api.begin_invoke(sender, m11c_dispatch, context) == GF_OK,
                "clicked callback could not queue UI work");
        return GF_EVENT_CALLBACK_FAULTED;
    }
    if (event_kind == GF_EVENT_FORM_CLOSING) {
        ++context->closing;
        return GF_EVENT_CALLBACK_CONTINUE;
    }
    if (event_kind == GF_EVENT_FORM_CLOSED) {
        ++context->closed;
        return GF_EVENT_CALLBACK_CONTINUE;
    }
    require(0, "unexpected M11c event kind");
    return GF_EVENT_CALLBACK_FAULTED;
}

static uint32_t cancel_close_event(gf_handle sender, uint32_t event_kind,
                                   void* opaque) {
    struct close_cancel_context* context =
        (struct close_cancel_context*)opaque;
    (void)sender;
    if (event_kind == GF_EVENT_FORM_CLOSING) {
        ++context->closing;
        return GF_EVENT_CALLBACK_CANCEL;
    }
    require(event_kind == GF_EVENT_FORM_CLOSED,
            "unexpected close-cancellation event kind");
    ++context->closed;
    return GF_EVENT_CALLBACK_CONTINUE;
}

static void* wrong_thread_worker(void* opaque) {
    struct worker_context* context = (struct worker_context*)opaque;
    uint32_t visible = 99U;
    context->result = api.get_visible(context->control, &visible);
    return NULL;
}

static void test_version_negotiation(void) {
    struct api_header {
        uint32_t struct_size;
        uint32_t abi_version;
    } header;
    memset(&header, 0, sizeof(header));
    header.struct_size = (uint32_t)sizeof(header);
    require(gf_get_api_v0(GF_ABI_VERSION_0_1, (gf_api_v0*)&header) == GF_OK,
            "size-prefixed header negotiation failed");
    require(header.struct_size == sizeof(gf_api_v0) &&
                header.abi_version == GF_ABI_VERSION_0_1,
            "negotiation did not report implementation size/version");

    memset(&api, 0, sizeof(api));
    api.struct_size = (uint32_t)sizeof(api);
    require(gf_get_api_v0(GF_ABI_VERSION_0_15, &api) == GF_OK,
            "full ABI table negotiation failed");
    require(api.struct_size == sizeof(api) && api.control_create != NULL &&
                api.disconnect != NULL && api.control_create_kind != NULL &&
                api.get_enabled != NULL && api.run_window != NULL &&
                api.last_host_trace != NULL && api.subscribe_v2 != NULL &&
                api.begin_invoke != NULL && api.request_close != NULL &&
                api.callback_fault_count != NULL &&
                api.set_control_png != NULL && api.set_child_index != NULL &&
                api.set_control_colors != NULL && api.subscribe_pointer != NULL &&
                api.set_check_state != NULL && api.get_check_state != NULL &&
                api.subscribe_key != NULL && api.subscribe_text != NULL &&
                api.set_range != NULL && api.get_range != NULL &&
                api.set_range_value != NULL && api.get_range_value != NULL &&
                api.set_pointer_capture != NULL &&
                api.get_pointer_capture != NULL &&
                api.show_path_dialog != NULL &&
                api.last_dialog_path != NULL &&
                api.show_tooltip != NULL && api.hide_tooltip != NULL &&
                api.set_field_selection != NULL &&
                api.set_field_edit_state != NULL &&
                api.field_position_from_point != NULL &&
                api.write_clipboard_text != NULL &&
                api.read_clipboard_text != NULL &&
                api.field_navigate != NULL &&
                api.field_replace != NULL &&
                api.field_history != NULL &&
                api.field_clear_history != NULL &&
                api.abi_version == GF_ABI_VERSION_0_15,
            "negotiated ABI table is incomplete");

    gf_api_v0 unsupported;
    memset(&unsupported, 0, sizeof(unsupported));
    unsupported.struct_size = (uint32_t)sizeof(unsupported);
    require(gf_get_api_v0(UINT32_C(0x00000010), &unsupported) ==
                GF_ERROR_UNSUPPORTED_VERSION,
            "unsupported ABI version was accepted");
}

static void test_abi_0_11_field_selection_contract(void) {
    gf_handle field = {0U, 0U};
    gf_handle panel = {0U, 0U};
    require(api.control_create_kind(GF_CONTROL_TEXT_BOX,
                                    text("abi.field.selection"), &field) == GF_OK &&
                api.control_create_kind(GF_CONTROL_PANEL,
                                        text("abi.field.panel"), &panel) == GF_OK,
            "0.11 field-selection fixtures failed");
    require(api.set_text(field, text("field")) == GF_OK &&
                api.set_field_selection(field, 1U, 3U, 1U) == GF_OK &&
                api.set_field_selection(field, 5U, 0U, 1U) == GF_OK,
            "0.11 valid field selection was rejected");
    require(api.set_field_selection(field, 4U, 2U, 1U) ==
                GF_ERROR_INVALID_ARGUMENT &&
                api.set_field_selection(field, 0U, 0U, 2U) ==
                GF_ERROR_INVALID_ARGUMENT &&
                api.set_field_selection(panel, 0U, 0U, 1U) ==
                GF_ERROR_WRONG_HANDLE_KIND,
            "0.11 invalid field selection was accepted");
    require(api.dispose(field) == GF_OK && api.dispose(panel) == GF_OK,
            "0.11 field-selection fixture disposal failed");
}

static void test_abi_0_12_field_edit_geometry_contract(void) {
    gf_handle field = {0U, 0U};
    gf_handle panel = {0U, 0U};
    uint64_t position = UINT64_MAX;
    require(api.control_create_kind(GF_CONTROL_TEXT_BOX,
                                    text("abi.field.edit-state"), &field) == GF_OK &&
                api.control_create_kind(GF_CONTROL_PANEL,
                                        text("abi.field.edit-panel"), &panel) == GF_OK,
            "0.12 field-edit fixtures failed");
    require(api.set_text(field, text("a\xCC\x81" "bc")) == GF_OK &&
                api.set_field_edit_state(field, 4U, 0U, 1U) == GF_OK &&
                api.set_field_edit_state(field, 0U, 4U, 1U) == GF_OK,
            "0.12 directional grapheme selection was rejected");
    require(api.set_field_edit_state(field, 1U, 4U, 1U) ==
                GF_ERROR_INVALID_ARGUMENT &&
                api.set_field_edit_state(field, 0U, 4U, 2U) ==
                GF_ERROR_INVALID_ARGUMENT &&
                api.set_field_edit_state(panel, 0U, 0U, 1U) ==
                GF_ERROR_WRONG_HANDLE_KIND,
            "0.12 invalid field edit state was accepted");
    require(api.field_position_from_point(field, 0.0, &position) == GF_OK &&
                position == 0U &&
                api.field_position_from_point(field, 10.0, NULL) ==
                    GF_ERROR_INVALID_ARGUMENT &&
                api.field_position_from_point(panel, 10.0, &position) ==
                    GF_ERROR_WRONG_HANDLE_KIND,
            "0.12 retained field hit testing contract failed");
    require(api.dispose(field) == GF_OK && api.dispose(panel) == GF_OK,
            "0.12 field-edit fixture disposal failed");
}

static void test_abi_0_13_clipboard_contract(void) {
    gf_handle form = {0U, 0U};
    gf_handle field = {0U, 0U};
    uint64_t required = 99U;
    uint32_t has_text = 99U;
    char buffer[32] = {0};
    require(api.control_create_kind(GF_CONTROL_FORM,
                                    text("abi.clipboard.form"), &form) == GF_OK &&
                api.control_create_kind(GF_CONTROL_TEXT_BOX,
                                        text("abi.clipboard.field"), &field) == GF_OK &&
                api.add_child(form, field) == GF_OK,
            "0.13 clipboard fixtures failed");
    require(api.write_clipboard_text(field, text("host \xCE\xA9")) == GF_OK,
            "0.13 clipboard write failed");
    require(api.read_clipboard_text(field, NULL, 0U, &required, &has_text) ==
                GF_ERROR_BUFFER_TOO_SMALL &&
                required == 7U && has_text == 1U,
            "0.13 clipboard sizing contract failed");
    require(api.read_clipboard_text(field, buffer, sizeof(buffer), &required,
                                    &has_text) == GF_OK &&
                required == 7U && has_text == 1U &&
                memcmp(buffer, "host \xCE\xA9", 7U) == 0,
            "0.13 clipboard round trip failed");
    require(api.read_clipboard_text(field, buffer, sizeof(buffer), NULL,
                                    &has_text) == GF_ERROR_INVALID_ARGUMENT &&
                api.read_clipboard_text(field, buffer, sizeof(buffer), &required,
                                        NULL) == GF_ERROR_INVALID_ARGUMENT,
            "0.13 clipboard accepted missing outputs");
    require(api.dispose(form) == GF_OK,
            "0.13 clipboard fixture disposal failed");
}

static void test_abi_0_14_grapheme_navigation_contract(void) {
    gf_handle field = {0U, 0U};
    gf_handle panel = {0U, 0U};
    uint64_t position = UINT64_MAX;
    require(api.control_create_kind(GF_CONTROL_TEXT_BOX,
                                    text("abi.field.navigation"), &field) == GF_OK &&
                api.control_create_kind(GF_CONTROL_PANEL,
                                        text("abi.field.navigation-panel"), &panel) == GF_OK &&
                api.set_text(field, text("a\xCC\x81" "bc")) == GF_OK,
            "0.14 navigation fixtures failed");
    require(api.field_navigate(field, 3U, -1, &position) == GF_OK &&
                position == 0U &&
                api.field_navigate(field, 3U, 1, &position) == GF_OK &&
                position == 4U &&
                api.field_navigate(field, 0U, -1, &position) == GF_OK &&
                position == 0U &&
                api.field_navigate(field, 5U, 1, &position) == GF_OK &&
                position == 5U,
            "0.14 grapheme navigation returned the wrong boundary");
    require(api.field_navigate(field, 1U, 1, &position) ==
                GF_ERROR_INVALID_ARGUMENT &&
                api.field_navigate(field, 3U, 0, &position) ==
                GF_ERROR_INVALID_ARGUMENT &&
                api.field_navigate(panel, 0U, 1, &position) ==
                GF_ERROR_WRONG_HANDLE_KIND &&
                api.field_navigate(field, 0U, 1, NULL) ==
                GF_ERROR_INVALID_ARGUMENT,
            "0.14 invalid navigation was accepted");
    require(api.dispose(field) == GF_OK && api.dispose(panel) == GF_OK,
            "0.14 navigation fixture disposal failed");
}

static void test_abi_0_15_field_mutation_history_contract(void) {
    gf_handle field = {0U, 0U};
    gf_handle panel = {0U, 0U};
    gf_field_edit_result edit = {0};
    char buffer[32] = {0};
    uint64_t required = 0U;
    require(api.control_create_kind(GF_CONTROL_TEXT_BOX,
                                    text("abi.field.history"), &field) == GF_OK &&
                api.control_create_kind(GF_CONTROL_PANEL,
                                        text("abi.field.history-panel"), &panel) == GF_OK &&
                api.set_text(field, text("a\xCC\x81" "bc")) == GF_OK &&
                api.set_field_edit_state(field, 3U, 4U, 1U) == GF_OK,
            "0.15 field-history fixtures failed");
    require(api.field_replace(field, 3U, 1U, text("\xCE\xA9"), &edit) == GF_OK &&
                edit.changed == 1U && edit.anchor_utf8 == 5U &&
                edit.caret_utf8 == 5U && edit.can_undo == 1U &&
                edit.can_redo == 0U,
            "0.15 selection-aware field replacement failed");
    require(api.get_text(field, buffer, sizeof(buffer), &required) == GF_OK &&
                required == 6U && memcmp(buffer, "a\xCC\x81\xCE\xA9" "c", 6U) == 0,
            "0.15 native field text did not become authoritative");
    require(api.field_history(field, -1, &edit) == GF_OK &&
                edit.changed == 1U && edit.anchor_utf8 == 3U &&
                edit.caret_utf8 == 4U && edit.can_undo == 0U &&
                edit.can_redo == 1U,
            "0.15 undo did not restore text and directional selection");
    memset(buffer, 0, sizeof(buffer));
    require(api.get_text(field, buffer, sizeof(buffer), &required) == GF_OK &&
                required == 5U && memcmp(buffer, "a\xCC\x81" "bc", 5U) == 0,
            "0.15 undo did not restore native text");
    require(api.field_history(field, 1, &edit) == GF_OK &&
                edit.changed == 1U && edit.anchor_utf8 == 5U &&
                edit.caret_utf8 == 5U && edit.can_undo == 1U &&
                edit.can_redo == 0U,
            "0.15 redo did not restore the replacement");
    require(api.field_clear_history(field) == GF_OK &&
                api.field_history(field, -1, &edit) == GF_OK &&
                edit.changed == 0U && edit.can_undo == 0U &&
                edit.can_redo == 0U,
            "0.15 history reset was not deterministic");
    require(api.field_replace(field, 1U, 1U, text("x"), &edit) ==
                GF_ERROR_INVALID_ARGUMENT &&
                api.field_replace(panel, 0U, 0U, text("x"), &edit) ==
                GF_ERROR_WRONG_HANDLE_KIND &&
                api.field_history(field, 0, &edit) == GF_ERROR_INVALID_ARGUMENT &&
                api.field_history(field, -1, NULL) == GF_ERROR_INVALID_ARGUMENT &&
                api.field_clear_history(panel) == GF_ERROR_WRONG_HANDLE_KIND,
            "0.15 invalid mutation or history operation was accepted");
    require(api.dispose(field) == GF_OK && api.dispose(panel) == GF_OK,
            "0.15 field-history fixture disposal failed");
}

static void test_abi_0_10_dialog_and_tooltip_contract(void) {
    gf_handle form = {0U, 0U};
    uint32_t accepted = 99U;
    uint64_t required = 99U;
    require(api.control_create_kind(GF_CONTROL_FORM,
                                    text("abi.host.adapters"), &form) == GF_OK,
            "0.10 host-adapter fixture creation failed");
    require(api.show_path_dialog(form, GF_PATH_DIALOG_OPEN_FILE,
                                 text("Open fixture"), text(""), text(""),
                                 text(""), text("Images|*.png"),
                                 GF_PATH_DIALOG_DEFAULT, &accepted) == GF_OK &&
                accepted == 0U,
            "provider-free path dialog did not deterministically cancel");
    require(api.last_dialog_path(form, NULL, 0U, &required) == GF_OK &&
                required == 0U,
            "cancelled path dialog retained a stale result");
    require(api.show_tooltip(form, text("Retained tooltip"), 8.0, 12.0,
                             250U) == GF_OK &&
                api.hide_tooltip(form) == GF_OK,
            "provider-free tooltip contract was not safely stateful");
    require(api.show_path_dialog(form, 99U, text(""), text(""), text(""),
                                 text(""), text(""), 0U, &accepted) ==
                GF_ERROR_INVALID_ARGUMENT,
            "invalid path-dialog kind was accepted");
    require(api.dispose(form) == GF_OK,
            "0.10 host-adapter fixture disposal failed");
}

struct range_context {
    gf_handle range;
    unsigned sequence[4];
    unsigned count;
    uint32_t captured_during_scroll;
};

static uint32_t range_event(gf_handle sender, uint32_t event_kind,
                            void* opaque) {
    struct range_context* context = (struct range_context*)opaque;
    require(sender.slot == context->range.slot,
            "range callback sender changed");
    require(context->count < 4U, "range callback sequence overflowed");
    context->sequence[context->count++] = event_kind;
    if (event_kind == GF_EVENT_RANGE_SCROLL) {
        require(api.set_pointer_capture(sender, 1U) == GF_OK,
                "attached range could not acquire explicit capture");
        require(api.get_pointer_capture(sender,
                                        &context->captured_during_scroll) == GF_OK,
                "range capture could not be queried");
    }
    return GF_EVENT_CALLBACK_CONTINUE;
}

static void test_abi_0_9_range_and_capture(void) {
    gf_handle form = {0U, 0U};
    gf_handle track = {0U, 0U};
    gf_handle progress = {0U, 0U};
    gf_event_token scroll = {0U, 0U};
    gf_event_token changed = {0U, 0U};
    struct range_context context;
    double minimum = 0.0;
    double maximum = 0.0;
    double value = 0.0;
    uint32_t captured = 99U;
    memset(&context, 0, sizeof(context));

    require(api.control_create_kind(GF_CONTROL_FORM,
                                    text("abi.range.form"), &form) == GF_OK &&
                api.control_create_kind(GF_CONTROL_TRACK_BAR,
                                        text("abi.range.track"), &track) == GF_OK &&
                api.control_create_kind(GF_CONTROL_PROGRESS_BAR,
                                        text("abi.range.progress"), &progress) == GF_OK,
            "0.9 range fixtures failed");
    context.range = track;
    require(api.set_bounds(form, (gf_rect){0.0, 0.0, 320.0, 200.0}) == GF_OK &&
                api.set_bounds(track,
                               (gf_rect){20.0, 30.0, 200.0, 28.0}) == GF_OK &&
                api.add_child(form, track) == GF_OK,
            "0.9 range fixture tree failed");
    require(api.set_range(track, -10.0, 10.0) == GF_OK &&
                api.set_range_value(track, -5.0) == GF_OK &&
                api.get_range(track, &minimum, &maximum) == GF_OK &&
                api.get_range_value(track, &value) == GF_OK &&
                minimum == -10.0 && maximum == 10.0 && value == -5.0,
            "0.9 track range did not round-trip");
    require(api.set_range(progress, 0.0, 100.0) == GF_OK &&
                api.set_range_value(progress, 63.0) == GF_OK &&
                api.get_range_value(progress, &value) == GF_OK && value == 63.0,
            "0.9 progress value did not project");
    require(api.set_range_value(track, 11.0) == GF_ERROR_INVALID_ARGUMENT &&
                api.set_range(progress, 5.0, 5.0) == GF_ERROR_INVALID_ARGUMENT &&
                api.get_range(form, &minimum, &maximum) ==
                    GF_ERROR_WRONG_HANDLE_KIND,
            "0.9 invalid range access was accepted");
    require(api.get_pointer_capture(track, &captured) == GF_OK && captured == 0U &&
                api.set_pointer_capture(track, 1U) == GF_ERROR_INVALID_ARGUMENT &&
                api.set_pointer_capture(track, 2U) == GF_ERROR_INVALID_ARGUMENT,
            "0.9 unattached/invalid capture semantics changed");
    require(api.subscribe_v2(track, GF_EVENT_RANGE_SCROLL,
                             range_event, &context, &scroll) == GF_OK &&
                api.subscribe_v2(track, GF_EVENT_RANGE_VALUE_CHANGED,
                                 range_event, &context, &changed) == GF_OK,
            "0.9 range subscriptions failed");
    context.count = 0U;
    require(api.run_window(form,
                           GF_WINDOW_RUN_FORCE_HEADLESS |
                               GF_WINDOW_RUN_AUTOMATION_ACTIVATE) == GF_OK,
            "0.9 headless range run failed");
    require(context.count == 2U &&
                context.sequence[0] == GF_EVENT_RANGE_SCROLL &&
                context.sequence[1] == GF_EVENT_RANGE_VALUE_CHANGED &&
                context.captured_during_scroll == 1U,
            "0.9 input range ordering/capture changed");
    require(api.get_pointer_capture(track, &captured) == GF_OK && captured == 0U,
            "0.9 capture survived host shutdown");
    require(api.disconnect(scroll) == GF_OK && api.disconnect(changed) == GF_OK &&
                api.dispose(form) == GF_OK && api.dispose(progress) == GF_OK,
            "0.9 range fixture cleanup failed");
}

static uint32_t ignored_key(gf_handle sender, uint32_t event_kind,
                            uint32_t physical_key, uint32_t modifiers,
                            uint32_t repeat, void* opaque) {
    (void)sender; (void)event_kind; (void)physical_key;
    (void)modifiers; (void)repeat; (void)opaque;
    return GF_EVENT_CALLBACK_CONTINUE;
}

static uint32_t ignored_text(gf_handle sender, gf_string_view input,
                             uint32_t composing, int32_t replacement_start,
                             int32_t replacement_length, void* opaque) {
    (void)sender; (void)input; (void)composing;
    (void)replacement_start; (void)replacement_length; (void)opaque;
    return GF_EVENT_CALLBACK_CONTINUE;
}

static void test_abi_0_8_field_keyboard_subscriptions(void) {
    gf_handle field = {0U, 0U};
    gf_handle panel = {0U, 0U};
    gf_handle custom = {0U, 0U};
    gf_event_token key = {0U, 0U};
    gf_event_token custom_key = {0U, 0U};
    gf_event_token input = {0U, 0U};
    require(api.control_create_kind(GF_CONTROL_TEXT_BOX,
                                    text("abi.keyboard.field"), &field) == GF_OK &&
                api.control_create_kind(GF_CONTROL_PANEL,
                                        text("abi.keyboard.panel"), &panel) == GF_OK &&
                api.control_create_kind(GF_CONTROL_CUSTOM,
                                        text("abi.keyboard.custom"), &custom) == GF_OK,
            "0.8 keyboard fixtures failed");
    require(api.subscribe_key(field, ignored_key, NULL, &key) == GF_OK &&
                api.subscribe_text(field, ignored_text, NULL, &input) == GF_OK,
            "0.8 field keyboard subscriptions failed");
    require(api.subscribe_key(custom, ignored_key, NULL, &custom_key) == GF_OK,
            "interactive custom control rejected key subscription");
    require(api.subscribe_key(panel, ignored_key, NULL, &key) ==
                GF_ERROR_WRONG_HANDLE_KIND &&
                api.subscribe_text(panel, ignored_text, NULL, &input) ==
                GF_ERROR_WRONG_HANDLE_KIND,
            "0.8 non-field accepted keyboard subscriptions");
    require(api.disconnect(key) == GF_OK && api.disconnect(custom_key) == GF_OK &&
                api.disconnect(input) == GF_OK,
            "0.8 keyboard subscription disconnect failed");
    require(api.dispose(field) == GF_OK && api.dispose(panel) == GF_OK &&
                api.dispose(custom) == GF_OK,
            "0.8 keyboard fixture disposal failed");
}

static void test_abi_0_7_checked_state_and_transparent_input(void) {
    gf_handle form = {0U, 0U};
    gf_handle check = {0U, 0U};
    gf_handle overlay = {0U, 0U};
    gf_handle painted_overlay = {0U, 0U};
    gf_event_token clicked = {0U, 0U};
    unsigned click_count = 0U;
    uint32_t state = 99U;
    require(api.control_create_kind(GF_CONTROL_FORM,
                                    text("abi.checked.form"), &form) == GF_OK &&
                api.control_create_kind(GF_CONTROL_CHECK_BOX,
                                        text("abi.checked.check"), &check) == GF_OK &&
                api.control_create_kind(GF_CONTROL_INPUT_TRANSPARENT,
                                        text("abi.checked.overlay"), &overlay) == GF_OK &&
                api.control_create_kind(GF_CONTROL_INPUT_TRANSPARENT_CUSTOM,
                                        text("abi.checked.painted-overlay"),
                                        &painted_overlay) == GF_OK,
            "0.7 checked-state fixtures failed");
    require(api.set_bounds(form, (gf_rect){0.0, 0.0, 320.0, 200.0}) == GF_OK,
            "0.7 form bounds failed");
    require(api.set_bounds(check, (gf_rect){24.0, 30.0, 140.0, 24.0}) == GF_OK,
            "0.7 checkbox bounds failed");
    require(api.set_bounds(overlay, (gf_rect){0.0, 0.0, 320.0, 200.0}) == GF_OK,
            "0.7 overlay bounds failed");
    require(api.set_bounds(painted_overlay,
                           (gf_rect){0.0, 0.0, 320.0, 200.0}) == GF_OK,
            "0.7 painted overlay bounds failed");
    require(api.add_child(form, check) == GF_OK,
            "0.7 checkbox parenting failed");
    require(api.add_child(form, overlay) == GF_OK,
            "0.7 overlay parenting failed");
    require(api.add_child(form, painted_overlay) == GF_OK,
            "0.7 painted overlay parenting failed");
    require(api.subscribe_v2(check, GF_EVENT_CLICKED, count_click_callback,
                             &click_count, &clicked) == GF_OK,
            "0.7 checkbox subscription failed");
    require(api.run_window(form,
                           GF_WINDOW_RUN_FORCE_HEADLESS |
                               GF_WINDOW_RUN_AUTOMATION_ACTIVATE) == GF_OK &&
                click_count == 1U &&
                api.get_check_state(check, &state) == GF_OK && state == 1U,
            "input-transparent overlay intercepted checkbox activation");
    require(api.set_check_state(check, 0U) == GF_OK &&
                api.get_check_state(check, &state) == GF_OK && state == 0U,
            "checkbox state did not round-trip");
    require(api.set_check_state(check, 3U) == GF_ERROR_INVALID_ARGUMENT,
            "invalid checkbox state was accepted");
    require(api.get_check_state(form, &state) == GF_ERROR_WRONG_HANDLE_KIND,
            "non-check control accepted checked-state access");
    require(api.dispose(form) == GF_OK, "0.7 checked form disposal failed");

    gf_handle radio = {0U, 0U};
    require(api.control_create_kind(GF_CONTROL_RADIO_BUTTON,
                                    text("abi.checked.radio"), &radio) == GF_OK &&
                api.set_check_state(radio, 1U) == GF_OK &&
                api.get_check_state(radio, &state) == GF_OK && state == 1U,
            "radio state did not round-trip");
    require(api.set_check_state(radio, 2U) == GF_ERROR_INVALID_ARGUMENT,
            "radio accepted indeterminate state");
    require(api.dispose(radio) == GF_OK, "0.7 radio disposal failed");
}

static void test_abi_0_6_pointer_delivery(void) {
    gf_handle form = {0U, 0U};
    gf_handle raster = {0U, 0U};
    gf_event_token pointer = {0U, 0U};
    struct pointer_context context;
    memset(&context, 0, sizeof(context));
    require(api.control_create_kind(GF_CONTROL_FORM, text("abi.pointer.form"),
                                    &form) == GF_OK &&
                api.control_create_kind(GF_CONTROL_CUSTOM,
                                        text("abi.pointer.raster"),
                                        &raster) == GF_OK,
            "0.6 pointer fixtures failed");
    require(api.set_bounds(form, (gf_rect){0.0, 0.0, 400.0, 240.0}) == GF_OK &&
                api.set_bounds(raster,
                               (gf_rect){20.0, 30.0, 180.0, 90.0}) == GF_OK &&
                api.add_child(form, raster) == GF_OK &&
                api.subscribe_pointer(raster, pointer_event, &context,
                                      &pointer) == GF_OK,
            "0.6 pointer tree/subscription failed");
    require(api.run_window(form,
                           GF_WINDOW_RUN_FORCE_HEADLESS |
                               GF_WINDOW_RUN_AUTOMATION_ACTIVATE) == GF_OK,
            "0.6 headless pointer run failed");
    require(context.downs == 1U && context.ups == 1U &&
                context.x >= 0.0 && context.x <= 180.0 &&
                context.y >= 0.0 && context.y <= 90.0,
            "0.6 pointer callback sequence/coordinates changed");
    uint64_t faults = 1U;
    require(api.callback_fault_count(form, &faults) == GF_OK && faults == 0U,
            "0.6 pointer callback reported a fault");
    require(api.disconnect(pointer) == GF_OK,
            "0.6 pointer disconnect failed");
    require(api.dispose(form) == GF_OK,
            "0.6 pointer form disposal failed");

    memset(&context, 0, sizeof(context));
    form = (gf_handle){0U, 0U};
    gf_handle numeric = {0U, 0U};
    pointer = (gf_event_token){0U, 0U};
    require(api.control_create_kind(GF_CONTROL_FORM, text("abi.pointer.field-form"),
                                    &form) == GF_OK &&
                api.control_create_kind(GF_CONTROL_NUMERIC_UP_DOWN,
                                        text("abi.pointer.numeric"),
                                        &numeric) == GF_OK &&
                api.set_bounds(form, (gf_rect){0.0, 0.0, 240.0, 120.0}) == GF_OK &&
                api.set_bounds(numeric, (gf_rect){20.0, 24.0, 120.0, 24.0}) == GF_OK &&
                api.add_child(form, numeric) == GF_OK &&
                api.subscribe_pointer(numeric, pointer_event, &context,
                                      &pointer) == GF_OK,
            "field pointer fixtures failed");
    require(api.run_window(form,
                           GF_WINDOW_RUN_FORCE_HEADLESS |
                               GF_WINDOW_RUN_AUTOMATION_ACTIVATE) == GF_OK &&
                context.downs == 1U && context.ups == 1U,
            "numeric field did not publish retained pointer input");
    require(api.dispose(form) == GF_OK,
            "numeric field form disposal failed");
}

static void test_properties_tree_and_errors(void) {
    gf_handle parent = {0U, 0U};
    gf_handle child = {0U, 0U};
    require(api.control_create(text("abi.parent"), &parent) == GF_OK,
            "parent creation failed");
    require(api.control_create(text("abi.child"), &child) == GF_OK,
            "child creation failed");

    uint64_t required = 0U;
    require(api.stable_id(child, NULL, 0U, &required) == GF_ERROR_BUFFER_TOO_SMALL &&
                required == strlen("abi.child"),
            "length-delimited stable ID sizing failed");
    char id[16] = {0};
    require(api.stable_id(child, id, sizeof(id), &required) == GF_OK &&
                memcmp(id, "abi.child", required) == 0,
            "stable ID copy failed");

    const gf_rect expected = {4.0, 7.0, 80.0, 25.0};
    require(api.set_bounds(child, expected) == GF_OK, "bounds mutation failed");
    gf_rect observed = {0.0, 0.0, 0.0, 0.0};
    require(api.get_bounds(child, &observed) == GF_OK && observed.x == expected.x &&
                observed.y == expected.y && observed.width == expected.width &&
                observed.height == expected.height,
            "bounds round trip failed");

    require(api.add_child(parent, child) == GF_OK, "visual parenting failed");
    require(api.remove_child(parent, child) == GF_OK, "visual detach failed");
    require(api.remove_child(parent, child) == GF_ERROR_INVALID_ARGUMENT,
            "invalid visual detach did not produce a structured error");
    gf_error_view error = {0U, {NULL, 0U}};
    require(api.last_error(&error) == GF_OK && error.code == GF_ERROR_INVALID_ARGUMENT &&
                error.message.size > 0U,
            "structured last error was not available");

    require(api.add_child(parent, child) == GF_OK, "visual reattachment failed");
    require(api.dispose(parent) == GF_OK, "parent disposal failed");
    uint32_t visible = 0U;
    require(api.get_visible(parent, &visible) == GF_ERROR_STALE_HANDLE &&
                api.get_visible(child, &visible) == GF_ERROR_STALE_HANDLE,
            "parent disposal did not stale every subtree handle");
}

static void test_abi_0_2_control_surface(void) {
    gf_handle control = {0U, 0U};
    require(api.control_create_kind(GF_CONTROL_BUTTON, text("abi.button"), &control) == GF_OK,
            "kinded control creation failed");
    require(api.set_name(control, text("startButton")) == GF_OK &&
                api.set_text(control, text("Start radio")) == GF_OK &&
                api.set_enabled(control, 0U) == GF_OK,
            "0.2 property mutation failed");

    uint64_t required = 0U;
    require(api.get_name(control, NULL, 0U, &required) == GF_ERROR_BUFFER_TOO_SMALL &&
                required == strlen("startButton"),
            "name sizing failed");
    char name[32] = {0};
    require(api.get_name(control, name, sizeof(name), &required) == GF_OK &&
                memcmp(name, "startButton", required) == 0,
            "name round trip failed");
    char label[32] = {0};
    require(api.get_text(control, label, sizeof(label), &required) == GF_OK &&
                memcmp(label, "Start radio", required) == 0,
            "text round trip failed");
    uint32_t enabled = 1U;
    require(api.get_enabled(control, &enabled) == GF_OK && enabled == 0U,
            "enabled round trip failed");
    require(api.dispose(control) == GF_OK, "0.2 control disposal failed");
}

static void test_abi_0_3_headless_window(void) {
    gf_handle form = {0U, 0U};
    gf_handle button = {0U, 0U};
    require(api.control_create_kind(GF_CONTROL_FORM, text("abi.form"), &form) == GF_OK &&
                api.control_create_kind(GF_CONTROL_BUTTON, text("abi.form.start"), &button) == GF_OK,
            "0.3 window fixtures failed");
    require(api.set_text(form, text("ABI managed surface")) == GF_OK &&
                api.set_bounds(form, (gf_rect){0.0, 0.0, 640.0, 420.0}) == GF_OK &&
                api.set_text(button, text("Start")) == GF_OK &&
                api.set_bounds(button, (gf_rect){20.0, 24.0, 100.0, 30.0}) == GF_OK &&
                api.add_child(form, button) == GF_OK,
            "0.3 retained window construction failed");
    require(api.run_window(form, GF_WINDOW_RUN_FORCE_HEADLESS) == GF_OK,
            "0.3 headless window run failed");
    uint64_t required = 0U;
    require(api.last_host_trace(form, NULL, 0U, &required) == GF_ERROR_BUFFER_TOO_SMALL &&
                required > 0U,
            "0.3 host trace sizing failed");
    char trace[4096] = {0};
    require(api.last_host_trace(form, trace, sizeof(trace), &required) == GF_OK &&
                strstr(trace, "event=attach") != NULL &&
                strstr(trace, "closed=1") != NULL && strstr(trace, "shutdown=1") != NULL,
            "0.3 host trace did not close deterministically");
    require(api.dispose(form) == GF_OK, "0.3 form disposal failed");
}

static void test_abi_0_4_callbacks_dispatch_and_close(void) {
    struct m11c_context context;
    memset(&context, 0, sizeof(context));
    require(api.control_create_kind(GF_CONTROL_FORM, text("abi.m11c.form"),
                                    &context.form) == GF_OK &&
                api.control_create_kind(GF_CONTROL_BUTTON, text("abi.m11c.button"),
                                        &context.button) == GF_OK,
            "0.4 callback fixtures failed");
    require(api.set_bounds(context.form,
                           (gf_rect){0.0, 0.0, 640.0, 420.0}) == GF_OK &&
                api.set_bounds(context.button,
                               (gf_rect){20.0, 24.0, 120.0, 30.0}) == GF_OK &&
                api.set_text(context.button, text("Activate")) == GF_OK &&
                api.add_child(context.form, context.button) == GF_OK,
            "0.4 retained callback tree failed");
    gf_event_token click = {0U, 0U};
    gf_event_token closing = {0U, 0U};
    gf_event_token closed = {0U, 0U};
    require(api.subscribe_v2(context.button, GF_EVENT_CLICKED, m11c_event,
                             &context, &click) == GF_OK &&
                api.subscribe_v2(context.form, GF_EVENT_FORM_CLOSING, m11c_event,
                                 &context, &closing) == GF_OK &&
                api.subscribe_v2(context.form, GF_EVENT_FORM_CLOSED, m11c_event,
                                 &context, &closed) == GF_OK,
            "0.4 typed subscriptions failed");
    require(api.run_window(context.form,
                           GF_WINDOW_RUN_FORCE_HEADLESS |
                               GF_WINDOW_RUN_AUTOMATION_ACTIVATE) == GF_OK,
            "0.4 headless callback run failed");
    uint64_t faults = 0U;
    require(context.clicks == 1U && context.dispatches == 1U &&
                context.closing == 1U && context.closed == 1U &&
                api.callback_fault_count(context.form, &faults) == GF_OK &&
                faults == 1U,
            "0.4 callback/dispatch/lifecycle counts changed");
    uint64_t required = 0U;
    char label[32] = {0};
    require(api.get_text(context.button, label, sizeof(label), &required) == GF_OK &&
                memcmp(label, "Dispatched", required) == 0,
            "0.4 queued mutation was not retained");
    require(api.dispose(context.form) == GF_OK,
            "0.4 form disposal failed");
}

static void test_abi_0_4_prehost_dispatch_and_disposal_cancellation(void) {
    struct prehost_dispatch_context running;
    memset(&running, 0, sizeof(running));
    running.close_when_dispatched = 1U;
    require(api.control_create_kind(GF_CONTROL_FORM, text("abi.prehost.form"),
                                    &running.form) == GF_OK &&
                api.begin_invoke(running.form, prehost_dispatch, &running) == GF_OK,
            "pre-host dispatch fixture failed");
    require(api.run_window(running.form, GF_WINDOW_RUN_FORCE_HEADLESS) == GF_OK &&
                running.dispatched == 1U && running.cancelled == 0U,
            "pre-host dispatch did not drain exactly once");
    require(api.dispose(running.form) == GF_OK,
            "pre-host dispatch form disposal failed");

    struct prehost_dispatch_context disposed;
    memset(&disposed, 0, sizeof(disposed));
    require(api.control_create_kind(GF_CONTROL_FORM,
                                    text("abi.prehost.dispose-form"),
                                    &disposed.form) == GF_OK &&
                api.begin_invoke(disposed.form, prehost_dispatch, &disposed) == GF_OK &&
                api.dispose(disposed.form) == GF_OK,
            "pre-host cancellation fixture failed");
    require(disposed.dispatched == 0U && disposed.cancelled == 1U,
            "disposing a pre-host queue did not cancel it exactly once");
}

static void test_abi_0_4_close_cancellation(void) {
    gf_handle form = {0U, 0U};
    gf_event_token closing = {0U, 0U};
    gf_event_token closed = {0U, 0U};
    struct close_cancel_context context = {0U, 0U};
    require(api.control_create_kind(GF_CONTROL_FORM,
                                    text("abi.m11c.cancel-form"),
                                    &form) == GF_OK &&
                api.set_bounds(form,
                               (gf_rect){0.0, 0.0, 320.0, 200.0}) == GF_OK,
            "0.4 cancellation fixture failed");
    require(api.subscribe_v2(form, GF_EVENT_FORM_CLOSING,
                             cancel_close_event, &context, &closing) == GF_OK &&
                api.subscribe_v2(form, GF_EVENT_FORM_CLOSED,
                                 cancel_close_event, &context, &closed) == GF_OK,
            "0.4 cancellation subscriptions failed");
    require(api.run_window(form, GF_WINDOW_RUN_FORCE_HEADLESS) == GF_OK &&
                context.closing == 1U && context.closed == 0U,
            "0.4 cancelled close reached FormClosed");
    require(api.dispose(form) == GF_OK,
            "0.4 cancelled form disposal failed");
}

static void test_retain_release_and_thread_affinity(void) {
    gf_handle control = {0U, 0U};
    require(api.control_create(text("abi.retain"), &control) == GF_OK,
            "retain fixture creation failed");
    require(api.retain(control) == GF_OK && api.release(control) == GF_OK,
            "retain/release balance failed");
    uint32_t visible = 0U;
    require(api.get_visible(control, &visible) == GF_OK && visible == 1U,
            "balanced release invalidated a live handle");

    struct worker_context worker = {control, GF_OK};
    pthread_t thread;
    require(pthread_create(&thread, NULL, wrong_thread_worker, &worker) == 0,
            "failed to start wrong-thread fixture");
    require(pthread_join(thread, NULL) == 0, "failed to join wrong-thread fixture");
    require(worker.result == GF_ERROR_WRONG_THREAD,
            "wrong-thread property access was not rejected");

    require(api.release(control) == GF_OK, "final release failed");
    require(api.get_visible(control, &visible) == GF_ERROR_STALE_HANDLE,
            "final release did not stale the handle");
}

static void test_event_tokens_and_callback_disposal(void) {
    gf_handle control = {0U, 0U};
    require(api.control_create(text("abi.events"), &control) == GF_OK,
            "event fixture creation failed");
    unsigned disconnected_calls = 0U;
    gf_event_token disconnected = {0U, 0U};
    require(api.subscribe(control, GF_EVENT_STATE_CHANGED, count_callback,
                          &disconnected_calls, &disconnected) == GF_OK,
            "event subscription failed");
    require(api.disconnect(disconnected) == GF_OK, "event disconnect failed");
    require(api.set_visible(control, 0U) == GF_OK && disconnected_calls == 0U,
            "disconnected callback was emitted");

    struct callback_context callback = {0U, GF_ERROR_INTERNAL};
    gf_event_token disposal_token = {0U, 0U};
    require(api.subscribe(control, GF_EVENT_STATE_CHANGED, dispose_sender, &callback,
                          &disposal_token) == GF_OK,
            "callback-disposal subscription failed");
    require(api.set_visible(control, 1U) == GF_OK && callback.calls == 1U &&
                callback.dispose_result == GF_OK,
            "handler-driven disposal did not complete exactly once");
    uint32_t visible = 0U;
    require(api.get_visible(control, &visible) == GF_ERROR_STALE_HANDLE,
            "handler-driven disposal left the sender handle live");
    require(api.disconnect(disposal_token) == GF_ERROR_STALE_HANDLE,
            "sender disposal did not revoke and stale its event token");
}

int main(void) {
    test_version_negotiation();
    test_abi_0_11_field_selection_contract();
    test_abi_0_12_field_edit_geometry_contract();
    test_abi_0_13_clipboard_contract();
    test_abi_0_14_grapheme_navigation_contract();
    test_abi_0_15_field_mutation_history_contract();
    test_properties_tree_and_errors();
    test_abi_0_2_control_surface();
    test_abi_0_3_headless_window();
    test_abi_0_4_callbacks_dispatch_and_close();
    test_abi_0_4_prehost_dispatch_and_disposal_cancellation();
    test_abi_0_4_close_cancellation();
    test_abi_0_6_pointer_delivery();
    test_abi_0_7_checked_state_and_transparent_input();
    test_abi_0_8_field_keyboard_subscriptions();
    test_abi_0_9_range_and_capture();
    test_abi_0_10_dialog_and_tooltip_contract();
    test_retain_release_and_thread_affinity();
    test_event_tokens_and_callback_disposal();
    puts("gui_forms_c_api_c11_tests: all tests passed");
    return EXIT_SUCCESS;
}
