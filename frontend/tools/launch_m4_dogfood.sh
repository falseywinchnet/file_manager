#!/bin/sh
set -eu

mode=${1:-}
case "$mode" in
  daily|protected|mutation-sandbox) ;;
  *)
    printf '%s\n' \
      "File Manager dogfood: expected daily, protected, or mutation-sandbox mode" >&2
    exit 2
    ;;
esac

codex_runs=${FILE_MANAGER_CODEX_RUNS:-"$HOME/Developer/CodexRuns"}
manifest=${FILE_MANAGER_DOGFOOD_MANIFEST:-"$codex_runs/file-manager-dogfood-current.manifest"}

fail() {
  printf 'File Manager dogfood: %s\n' "$1" >&2
  exit 1
}

[ -f "$manifest" ] || fail "current-candidate manifest is missing: $manifest"

manifest_value() {
  key=$1
  values=$(/usr/bin/sed -n "s/^${key}=//p" "$manifest")
  count=$(printf '%s\n' "$values" | /usr/bin/awk 'NF { count += 1 } END { print count + 0 }')
  [ "$count" -eq 1 ] || fail "manifest must contain exactly one nonempty ${key} entry"
  printf '%s\n' "$values"
}

format=$(manifest_value format)
bundle_name=$(manifest_value bundle)
expected_hash=$(manifest_value executable_sha256)

[ "$format" = 1 ] || fail "unsupported manifest format: $format"
case "$bundle_name" in
  ''|*/*|.|..|.*|*[!A-Za-z0-9._+-]*)
    fail "manifest bundle must be one safe .app basename"
    ;;
  *.app) ;;
  *) fail "manifest bundle must end in .app" ;;
esac
case "$expected_hash" in
  *[!0-9a-f]*|'') fail "manifest executable_sha256 must be lowercase hexadecimal" ;;
esac
[ "${#expected_hash}" -eq 64 ] ||
  fail "manifest executable_sha256 must contain 64 characters"

bundle="$codex_runs/$bundle_name"
executable="$bundle/Contents/MacOS/File Manager"
[ -d "$bundle" ] || fail "candidate bundle is missing: $bundle"
[ -x "$executable" ] || fail "candidate executable is missing: $executable"

actual_hash=$(/usr/bin/shasum -a 256 "$executable" | /usr/bin/awk '{ print $1 }')
[ "$actual_hash" = "$expected_hash" ] ||
  fail "candidate executable hash mismatch (expected $expected_hash, observed $actual_hash)"
/usr/bin/codesign --verify --deep --strict "$bundle" 2>/dev/null ||
  fail "candidate bundle failed deep strict signature verification: $bundle"

set --
case "$mode" in
  daily)
    ;;
  protected)
    protected_root="$codex_runs/fmsandbox"
    [ -d "$protected_root" ] ||
      fail "protected dogfood root is missing: $protected_root"
    set -- --root "$protected_root" --engine-root-id fm1-contained --read-only
    ;;
  mutation-sandbox)
    protected_root="$codex_runs/fmsandbox"
    quarantine_root="$codex_runs/fmquarantine"
    [ -d "$protected_root" ] ||
      fail "mutation sandbox root is missing: $protected_root"
    [ -d "$quarantine_root" ] ||
      fail "mutation quarantine is missing: $quarantine_root"
    set -- --root "$protected_root" --engine-root-id fm1-contained \
      --allow-mutations --quarantine "$quarantine_root"
    ;;
esac

printf 'File Manager dogfood mode: %s\n' "$mode"
printf 'Verified bundle: %s\n' "$bundle"
printf 'Executable SHA-256: %s\n' "$actual_hash"

if [ "${FILE_MANAGER_LAUNCH_DRY_RUN:-0}" = 1 ]; then
  printf 'argument_count=%s\n' "$#"
  argument_index=0
  for argument in "$@"; do
    argument_index=$((argument_index + 1))
    printf 'argument.%s=%s\n' "$argument_index" "$argument"
  done
  exit 0
fi

exec /usr/bin/open -n "$bundle" ${1+"--args"} "$@"
