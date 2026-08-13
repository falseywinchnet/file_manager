#!/bin/sh
set -eu

source_root=$1
temporary=$(/usr/bin/mktemp -d "${TMPDIR:-/tmp}/file-manager-launcher-tests.XXXXXX")
cleanup() {
  /bin/rm -rf -- "$temporary"
}
trap cleanup EXIT HUP INT TERM

test_home="$temporary/home"
codex_runs="$test_home/Developer/CodexRuns"
bundle="$codex_runs/file-manager-test.app"
/bin/mkdir -p "$bundle/Contents/MacOS" "$codex_runs/fmsandbox" \
  "$codex_runs/fmquarantine"
/bin/cp /usr/bin/true "$bundle/Contents/MacOS/File Manager"
printf '%s\n' \
  '<?xml version="1.0" encoding="UTF-8"?>' \
  '<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">' \
  '<plist version="1.0"><dict>' \
  '<key>CFBundleExecutable</key><string>File Manager</string>' \
  '<key>CFBundleIdentifier</key><string>com.filemanager.launcher-test</string>' \
  '<key>CFBundleName</key><string>File Manager Launcher Test</string>' \
  '<key>CFBundlePackageType</key><string>APPL</string>' \
  '</dict></plist>' >"$bundle/Contents/Info.plist"
/usr/bin/codesign --force --sign - --timestamp=none "$bundle" >/dev/null
hash=$(/usr/bin/shasum -a 256 "$bundle/Contents/MacOS/File Manager" |
  /usr/bin/awk '{ print $1 }')
manifest="$codex_runs/file-manager-dogfood-current.manifest"
printf 'format=1\nbundle=file-manager-test.app\nexecutable_sha256=%s\n' \
  "$hash" >"$manifest"

launcher="$source_root/tools/launch_m4_dogfood.sh"
daily=$(HOME="$test_home" FILE_MANAGER_LAUNCH_DRY_RUN=1 \
  /bin/sh "$launcher" daily)
printf '%s\n' "$daily" | /usr/bin/grep -q '^argument_count=0$'

protected=$(HOME="$test_home" FILE_MANAGER_LAUNCH_DRY_RUN=1 \
  /bin/sh "$launcher" protected)
printf '%s\n' "$protected" | /usr/bin/grep -q '^argument_count=4$'
printf '%s\n' "$protected" | /usr/bin/grep -q '^argument.1=--root$'
printf '%s\n' "$protected" | /usr/bin/grep -q '^argument.3=--engine-root-id$'
if printf '%s\n' "$protected" | /usr/bin/grep -q -- '--allow-mutations'; then
  printf '%s\n' "protected launcher unexpectedly enabled mutations" >&2
  exit 1
fi

mutation=$(HOME="$test_home" FILE_MANAGER_LAUNCH_DRY_RUN=1 \
  /bin/sh "$launcher" mutation-sandbox)
printf '%s\n' "$mutation" | /usr/bin/grep -q '^argument_count=7$'
printf '%s\n' "$mutation" | /usr/bin/grep -q '^argument.5=--allow-mutations$'
printf '%s\n' "$mutation" | /usr/bin/grep -q '^argument.6=--quarantine$'

printf 'format=1\nbundle=file-manager-test.app\nexecutable_sha256=%064d\n' \
  0 >"$manifest"
if HOME="$test_home" FILE_MANAGER_LAUNCH_DRY_RUN=1 \
    /bin/sh "$launcher" daily >"$temporary/out" 2>"$temporary/error"; then
  printf '%s\n' "launcher accepted a mismatched executable hash" >&2
  exit 1
fi
/usr/bin/grep -q 'hash mismatch' "$temporary/error"

/bin/rm -f "$manifest"
if HOME="$test_home" FILE_MANAGER_LAUNCH_DRY_RUN=1 \
    /bin/sh "$launcher" daily >"$temporary/out" 2>"$temporary/error"; then
  printf '%s\n' "launcher accepted a missing manifest" >&2
  exit 1
fi
/usr/bin/grep -q 'manifest is missing' "$temporary/error"

printf '%s\n' "file manager dogfood launcher tests passed"
