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

struct worker_context {
    gf_handle control;
    gf_result result;
};

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
    require(gf_get_api_v0(GF_ABI_VERSION_0_1, &api) == GF_OK,
            "full ABI table negotiation failed");
    require(api.struct_size == sizeof(api) && api.control_create != NULL &&
                api.disconnect != NULL,
            "negotiated ABI table is incomplete");

    gf_api_v0 unsupported;
    memset(&unsupported, 0, sizeof(unsupported));
    unsupported.struct_size = (uint32_t)sizeof(unsupported);
    require(gf_get_api_v0(UINT32_C(0x00000002), &unsupported) ==
                GF_ERROR_UNSUPPORTED_VERSION,
            "unsupported ABI version was accepted");
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
    test_properties_tree_and_errors();
    test_retain_release_and_thread_affinity();
    test_event_tokens_and_callback_disposal();
    puts("gui_forms_c_api_c11_tests: all tests passed");
    return EXIT_SUCCESS;
}
