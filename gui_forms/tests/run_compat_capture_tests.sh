#!/bin/sh
set -eu

script_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
project_dir=$(CDPATH= cd -- "$script_dir/.." && pwd)
fixture="$script_dir/fixtures/compat_capture/Consumer/Consumer.csproj"
work_dir=$(mktemp -d "${TMPDIR:-/tmp}/gui-forms-capture.XXXXXX")
trap 'rm -rf "$work_dir"' EXIT HUP INT TERM

dotnet build "$project_dir/tools/compat_capture/GuiForms.CompatCapture.csproj" \
  -c Release --nologo --verbosity quiet
dotnet build "$fixture" -c Release --nologo --verbosity quiet -o "$work_dir/fixture"

consumer="$work_dir/fixture/CaptureFixture.Consumer.dll"
expected_hash=$(shasum -a 256 "$consumer" | awk '{print $1}')
sentinel="$work_dir/target-executed"
export GUI_FORMS_CAPTURE_SENTINEL="$sentinel"

dotnet run --project "$project_dir/tools/compat_capture/GuiForms.CompatCapture.csproj" \
  -c Release --no-build -- \
  --input "$consumer" \
  --output "$work_dir/manifest-a.json" \
  --label synthetic-capture-fixture \
  --expected-sha256 "$expected_hash"

dotnet run --project "$project_dir/tools/compat_capture/GuiForms.CompatCapture.csproj" \
  -c Release --no-build -- \
  --input "$consumer" \
  --output "$work_dir/manifest-b.json" \
  --label synthetic-capture-fixture \
  --expected-sha256 "$expected_hash"

cmp "$work_dir/manifest-a.json" "$work_dir/manifest-b.json"
test ! -e "$sentinel"
test "$(grep -c 'FixtureWindow' "$work_dir/manifest-a.json" || true)" -eq 0
test "$(grep -F -c '\u003Cprivate:type-' "$work_dir/manifest-a.json" || true)" -gt 0

python3 - "$work_dir/manifest-a.json" <<'PY'
import json
import sys

with open(sys.argv[1], encoding="utf-8") as source:
    manifest = json.load(source)

assert manifest["schema"] == "gui.forms.compat.capture/v0"
assert manifest["policy"]["executeTargetCode"] is False
assert manifest["policy"]["includeMethodBodyBytes"] is False
assert manifest["summary"]["managedAssemblyCount"] == 1
assert manifest["summary"]["customControlCount"] == 2
assert manifest["summary"]["nativeImportCount"] == 1
assert manifest["summary"]["errorCount"] == 0
assert any(use["type"] == "System.Windows.Forms.Button" and
           "newobj" in use["operations"] for use in manifest["apiUses"])
assert any(use.get("member") == "add_Click" and use["ilOccurrenceCount"] == 1
           for use in manifest["apiUses"])
assert manifest["nativeImports"][0]["module"] == "user32.dll"
assert manifest["nativeImports"][0]["entryPoint"] == "SendMessageW"
PY

if dotnet run --project "$project_dir/tools/compat_capture/GuiForms.CompatCapture.csproj" \
  -c Release --no-build -- \
  --input "$consumer" --output "$work_dir/rejected.json" --label rejected \
  --expected-sha256 0000000000000000000000000000000000000000000000000000000000000000 \
  >"$work_dir/rejected.out" 2>"$work_dir/rejected.err"; then
  echo "capture unexpectedly accepted an incorrect specimen hash" >&2
  exit 1
fi
test ! -e "$work_dir/rejected.json"

echo "compat-capture: PASS"
