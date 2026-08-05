#!/bin/sh
set -eu

build_directory=${1:-"$(pwd)/build-windows-x64"}
wine_binary=${WINE_BINARY:-/opt/homebrew/bin/wine}
gallery="$build_directory/GUI.Forms Gallery.exe"
probe="$build_directory/gui_forms_windows_probe.exe"
log="$build_directory/wine-gallery-smoke.jsonl"

test -x "$wine_binary"
test -f "$gallery"
test -f "$probe"

WINEDEBUG=-all "$wine_binary" "$gallery" --automation >"$log" 2>&1 &
gallery_pid=$!
cleanup() {
    kill "$gallery_pid" 2>/dev/null || true
}
trap cleanup EXIT INT TERM

attempt=0
until WINEDEBUG=-all "$wine_binary" "$probe" snapshot >/dev/null 2>&1; do
    attempt=$((attempt + 1))
    if test "$attempt" -ge 100; then
        echo "Gallery automation window did not appear" >&2
        exit 1
    fi
    sleep 0.1
done

for stable_id in \
    gallery.checkbox \
    gallery.radio.quiet \
    gallery.category.values \
    gallery.collection.gamma
do
    WINEDEBUG=-all "$wine_binary" "$probe" click "$stable_id" >/dev/null
done
WINEDEBUG=-all "$wine_binary" "$probe" capture >/dev/null
WINEDEBUG=-all "$wine_binary" "$probe" snapshot >/dev/null
WINEDEBUG=-all "$wine_binary" "$probe" close >/dev/null
wait "$gallery_pid"
trap - EXIT INT TERM

test "$(rg -c '"automation":"click".*"handled":true' "$log")" -eq 4
rg -q '"automation":"capture","saved":true' "$log"
rg -q '"renderer_name":"Win32 DIB CPU · GDI text · WIC PNG · Rapids UI"' "$log"
rg -q '"events_rejected":0' "$log"
rg -q '"close_requests":1' "$log"
rg -q '"closed":true' "$log"
rg -q '"shutdown":true' "$log"
test -s "$build_directory/gallery-automation.bmp"

echo "Wine Gallery smoke passed: $log"
