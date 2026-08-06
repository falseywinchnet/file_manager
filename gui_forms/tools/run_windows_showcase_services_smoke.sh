#!/bin/sh
set -eu

build_directory=${1:-"$(pwd)/build-windows-x64"}
wine_binary=${WINE_BINARY:-/opt/homebrew/bin/wine}
showcase="$build_directory/GUI.Forms Complete Showcase.exe"
probe="$build_directory/gui_forms_windows_probe.exe"
log="$build_directory/wine-showcase-services-smoke.jsonl"

test -x "$wine_binary"
test -f "$showcase"
test -f "$probe"

WINEDEBUG=-all "$wine_binary" "$showcase" --automation >"$log" 2>&1 &
showcase_pid=$!
cleanup() {
    kill "$showcase_pid" 2>/dev/null || true
}
trap cleanup EXIT INT TERM

attempt=0
until WINEDEBUG=-all "$wine_binary" "$probe" snapshot >/dev/null 2>&1; do
    attempt=$((attempt + 1))
    if test "$attempt" -ge 120; then
        echo "Complete Showcase automation window did not appear" >&2
        exit 1
    fi
    sleep 0.1
done

for stable_id in \
    showcase.navigation.3 \
    showcase.animation.reduced
do
    WINEDEBUG=-all "$wine_binary" "$probe" activate "$stable_id" >/dev/null
done
WINEDEBUG=-all "$wine_binary" "$probe" snapshot >/dev/null
WINEDEBUG=-all "$wine_binary" "$probe" activate showcase.animation.pause >/dev/null
WINEDEBUG=-all "$wine_binary" "$probe" snapshot >/dev/null
WINEDEBUG=-all "$wine_binary" "$probe" activate showcase.animation.pause >/dev/null
WINEDEBUG=-all "$wine_binary" "$probe" snapshot >/dev/null
WINEDEBUG=-all "$wine_binary" "$probe" activate showcase.animation.reduced >/dev/null
WINEDEBUG=-all "$wine_binary" "$probe" snapshot >/dev/null

WINEDEBUG=-all "$wine_binary" "$probe" activate showcase.navigation.4 >/dev/null
WINEDEBUG=-all "$wine_binary" "$probe" activate showcase.dispatcher.post >/dev/null
sleep 0.2
WINEDEBUG=-all "$wine_binary" "$probe" snapshot >/dev/null
WINEDEBUG=-all "$wine_binary" "$probe" activate showcase.dispatcher.cancel >/dev/null
sleep 0.1
WINEDEBUG=-all "$wine_binary" "$probe" snapshot >/dev/null
WINEDEBUG=-all "$wine_binary" "$probe" activate showcase.dispatcher.invoke >/dev/null
sleep 0.2
WINEDEBUG=-all "$wine_binary" "$probe" snapshot >/dev/null

for stable_id in \
    showcase.navigation.13 \
    showcase.host.service.2 \
    showcase.host.service.3
do
    WINEDEBUG=-all "$wine_binary" "$probe" activate "$stable_id" >/dev/null
done

dismiss_native_dialog() {
    stable_id=$1
    WINEDEBUG=-all "$wine_binary" "$probe" activate "$stable_id" >/dev/null 2>&1 &
    activation_pid=$!
    attempt=0
    until WINEDEBUG=-all "$wine_binary" "$probe" dismiss-dialog >/dev/null 2>&1; do
        attempt=$((attempt + 1))
        if test "$attempt" -ge 50; then
            kill "$activation_pid" 2>/dev/null || true
            echo "Owned native dialog did not appear for $stable_id" >&2
            return 1
        fi
        sleep 0.1
    done
    wait "$activation_pid"
}

dismiss_native_dialog showcase.host.message.1
dismiss_native_dialog showcase.host.dialog.4

WINEDEBUG=-all "$wine_binary" "$probe" snapshot >/dev/null
WINEDEBUG=-all "$wine_binary" "$probe" close >/dev/null
wait "$showcase_pid"
trap - EXIT INT TERM

rg -q '"automation":"activate","id":"showcase.navigation.13"' "$log"
rg -q '"automation":"activate","id":"showcase.navigation.3"' "$log"
rg -q '"automation":"activate","id":"showcase.animation.reduced"' "$log"
rg -q '"automation":"activate","id":"showcase.animation.pause"' "$log"
zero_motion_snapshots=$(rg -c '"automation":"snapshot".*"active_surface_count":0' "$log")
live_motion_snapshots=$(rg -c '"automation":"snapshot".*"active_surface_count":4' "$log")
test "$zero_motion_snapshots" -ge 1
test "$live_motion_snapshots" -ge 3
rg -q '"automation":"activate","id":"showcase.dispatcher.post"' "$log"
rg -q '"automation":"activate","id":"showcase.dispatcher.cancel"' "$log"
rg -q '"automation":"activate","id":"showcase.dispatcher.invoke"' "$log"
rg -Fq '"dispatcher":{"posted":4,"invoked":4,"cancelled":0,"faulted":0,"synchronous_invocations":0,"inline_invocations":0,"marshalled_invocations":0,"pending":0,"maximum_pending":3,"accepting":true}' "$log"
rg -Fq '"dispatcher":{"posted":5,"invoked":4,"cancelled":1,"faulted":0,"synchronous_invocations":0,"inline_invocations":0,"marshalled_invocations":0,"pending":0,"maximum_pending":3,"accepting":true}' "$log"
rg -Fq '"dispatcher":{"posted":6,"invoked":5,"cancelled":1,"faulted":0,"synchronous_invocations":1,"inline_invocations":0,"marshalled_invocations":1,"pending":0,"maximum_pending":3,"accepting":true}' "$log"
rg -q '"automation":"activate","id":"showcase.host.service.2"' "$log"
rg -q '"automation":"activate","id":"showcase.host.service.3"' "$log"
rg -q '"platform":"win32-dib"' "$log"
rg -q '"monitor_geometry"' "$log"
rg -q '"clipboard"' "$log"
rg -q '"dialogs"' "$log"
rg -q '"sound_cues"' "$log"
rg -q '"monitor_queries":1' "$log"
rg -q '"sound_requests":1' "$log"
rg -q '"sound_playbacks":1' "$log"
rg -q '"dialog_requests":2' "$log"
rg -q '"dialog_completions":2' "$log"
rg -q '"dialog_cancellations":2' "$log"
rg -q '"maximum_modal_depth":1' "$log"
rg -q '"modal_depth":0' "$log"
rg -q '"events_rejected":0' "$log"
rg -q '"close_requests":1' "$log"
rg -q '"closed":true' "$log"
rg -q '"shutdown":true' "$log"

echo "Wine Complete Showcase host-services smoke passed: $log"
