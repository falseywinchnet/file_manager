#!/bin/sh
set -eu

if [ "$(/usr/bin/uname -s)" != Darwin ]; then
  printf '%s\n' "File Manager dogfood staging requires macOS" >&2
  exit 2
fi

source_bundle=${1:-"frontend/build/File Manager.app"}
codex_runs=${FILE_MANAGER_CODEX_RUNS:-"$HOME/Developer/CodexRuns"}
tool_directory=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)

fail() {
  printf 'File Manager dogfood staging: %s\n' "$1" >&2
  exit 1
}

[ -d "$source_bundle" ] || fail "source bundle is missing: $source_bundle"
[ -x "$source_bundle/Contents/MacOS/File Manager" ] ||
  fail "source bundle has no File Manager executable: $source_bundle"
/bin/mkdir -p "$codex_runs"

staging_bundle="$codex_runs/.file-manager-dogfood-staging-$$.app"
staging_manifest="$codex_runs/.file-manager-dogfood-current-$$.manifest"
cleanup() {
  if [ -e "$staging_bundle" ]; then
    /bin/rm -rf -- "$staging_bundle"
  fi
  if [ -e "$staging_manifest" ]; then
    /bin/rm -f -- "$staging_manifest"
  fi
}
trap cleanup EXIT HUP INT TERM

/usr/bin/ditto "$source_bundle" "$staging_bundle"
if [ -d "$staging_bundle/Contents/Frameworks" ]; then
  /usr/bin/find "$staging_bundle/Contents/Frameworks" -type f -name '*.dylib' \
    -exec /usr/bin/codesign --force --sign - --timestamp=none '{}' ';'
fi
/usr/bin/codesign --force --deep --sign - --timestamp=none "$staging_bundle"
/usr/bin/codesign --verify --deep --strict "$staging_bundle"

executable="$staging_bundle/Contents/MacOS/File Manager"
executable_hash=$(/usr/bin/shasum -a 256 "$executable" | /usr/bin/awk '{ print $1 }')
timestamp=$(/bin/date -u '+%Y%m%dT%H%M%SZ')
bundle_name="file-manager-dogfood-${timestamp}-$(printf '%.12s' "$executable_hash").app"
destination="$codex_runs/$bundle_name"
[ ! -e "$destination" ] || fail "refusing to overwrite existing candidate: $destination"
/bin/mv "$staging_bundle" "$destination"

printf 'format=1\nbundle=%s\nexecutable_sha256=%s\nstaged_utc=%s\n' \
  "$bundle_name" "$executable_hash" "$timestamp" >"$staging_manifest"
/bin/mv "$staging_manifest" "$codex_runs/file-manager-dogfood-current.manifest"

install_launcher() {
  source=$1
  destination_name=$2
  temporary="$codex_runs/.${destination_name}.$$"
  /bin/cp "$source" "$temporary"
  /bin/chmod 755 "$temporary"
  /bin/mv "$temporary" "$codex_runs/$destination_name"
}

install_launcher "$tool_directory/launch_m4_dogfood.sh" \
  ".file-manager-launch-m4.sh"
install_launcher "$tool_directory/launch_m4_daily.command" \
  "run-file-manager-daily.command"
install_launcher "$tool_directory/launch_m4_protected.command" \
  "run-file-manager-protected-read-only.command"
install_launcher "$tool_directory/launch_m4_mutation_sandbox.command" \
  "run-file-manager-mutation-sandbox.command"

# The copied .command files expect their sibling helper under the source name.
/bin/cp "$codex_runs/.file-manager-launch-m4.sh" \
  "$codex_runs/launch_m4_dogfood.sh"
/bin/chmod 755 "$codex_runs/launch_m4_dogfood.sh"

printf 'Staged File Manager dogfood candidate\n'
printf 'bundle=%s\n' "$destination"
printf 'executable_sha256=%s\n' "$executable_hash"
printf 'manifest=%s\n' "$codex_runs/file-manager-dogfood-current.manifest"
printf 'daily_launcher=%s\n' "$codex_runs/run-file-manager-daily.command"
printf 'protected_launcher=%s\n' \
  "$codex_runs/run-file-manager-protected-read-only.command"
printf 'mutation_launcher=%s\n' \
  "$codex_runs/run-file-manager-mutation-sandbox.command"
