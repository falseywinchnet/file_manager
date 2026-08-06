#!/bin/sh
set -eu

build_directory=${1:-"$(pwd)/build-windows-x64"}
wine_binary=${WINE_BINARY:-/opt/homebrew/bin/wine}
showcase="$build_directory/GUI.Forms Complete Showcase.exe"
probe="$build_directory/gui_forms_windows_probe.exe"
log="$build_directory/wine-showcase-text-smoke.jsonl"
capture="$build_directory/gallery-automation.bmp"

test -x "$wine_binary"
test -f "$showcase"
test -f "$probe"

GUI_FORMS_TRACE_WIN32_TEXT=1 WINEDEBUG=-all \
    "$wine_binary" "$showcase" --automation >"$log" 2>&1 &
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

WINEDEBUG=-all "$wine_binary" "$probe" activate showcase.navigation.5 >/dev/null
WINEDEBUG=-all "$wine_binary" "$probe" capture >/dev/null
WINEDEBUG=-all "$wine_binary" "$probe" close >/dev/null
wait "$showcase_pid"
trap - EXIT INT TERM

test -s "$capture"
rg -q 'win32-text selected=Noto Sans CJK JP' "$log"
rg -q 'win32-text selected=Noto Emoji' "$log"
rg -q 'win32-text shaped=1 glyph-count=1 default=0000 glyphs=02c5,' "$log"
rg -q 'win32-text draw family=Noto Emoji units=2 rendered=1' "$log"
rg -q '"renderer_name":"Win32 DIB CPU · Uniscribe/GDI text · WIC PNG · bundled fonts"' "$log"
rg -q '"events_rejected":0' "$log"
rg -q '"close_requests":1' "$log"
rg -q '"closed":true' "$log"
rg -q '"shutdown":true' "$log"

echo "Wine Complete Showcase text fallback smoke passed: $log"
