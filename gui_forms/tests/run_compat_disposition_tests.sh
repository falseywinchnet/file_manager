#!/bin/sh
set -eu

script_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
project_dir=$(CDPATH= cd -- "$script_dir/.." && pwd)
capture="$project_dir/compatibility/retired compatibility specimen/retired compatibility specimen-next-x64-6-1922.capture-v0.json"
policy="$project_dir/compatibility/retired compatibility specimen/retired compatibility specimen-next-x64-6-1922.disposition-policy-v1.json"
checked_in="$project_dir/compatibility/retired compatibility specimen/retired compatibility specimen-next-x64-6-1922.facade-catalogue-v1.json"
work_dir=$(mktemp -d "${TMPDIR:-/tmp}/gui-forms-disposition.XXXXXX")
trap 'rm -rf "$work_dir"' EXIT HUP INT TERM

python3 "$project_dir/tools/compat_capture/apply_dispositions.py" \
  --capture "$capture" --policy "$policy" --output "$work_dir/catalogue-a.json"
python3 "$project_dir/tools/compat_capture/apply_dispositions.py" \
  --capture "$capture" --policy "$policy" --output "$work_dir/catalogue-b.json"

cmp "$work_dir/catalogue-a.json" "$work_dir/catalogue-b.json"
cmp "$work_dir/catalogue-a.json" "$checked_in"

python3 - "$work_dir/catalogue-a.json" <<'PY'
import json
import sys

with open(sys.argv[1], encoding="utf-8") as source:
    catalogue = json.load(source)

assert catalogue["schema"] == "gui.forms.compat.facade-catalogue/v1"
summary = catalogue["summary"]
assert summary["apiRowCount"] == 1436
assert summary["customControlRowCount"] == 107
assert summary["nativeImportRowCount"] == 409
assert summary["totalRowCount"] == 1952
assert summary["unclassifiedCount"] == 0
assert sum(summary["apiByDisposition"].values()) == 1436
assert summary["customControlByDisposition"] == {"application_side_port": 107}
assert summary["nativeImportByDisposition"] == {"application_side_port": 409}
assert len(catalogue["facade"]["requiredTypes"]) == summary["requiredFacadeTypeCount"]

api_ids = [row["id"] for row in catalogue["apiRows"]]
all_ids = api_ids + [row["id"] for row in catalogue["customControlRows"]]
all_ids += [row["id"] for row in catalogue["nativeImportRows"]]
assert len(all_ids) == len(set(all_ids))
assert not any(row["disposition"] == "unclassified" for row in catalogue["apiRows"])
assert any(row["type"] == "System.Windows.Forms.Control" and
           row["disposition"] == "required" for row in catalogue["apiRows"])
assert all(row["disposition"] == "deferred" for row in catalogue["apiRows"]
           if row["type"].startswith("System.Windows.Forms.Design."))
assert all(row["implementationOwner"] == "specimen_native_dependency"
           for row in catalogue["nativeImportRows"])
PY

cp "$policy" "$work_dir/tampered-policy.json"
python3 - "$work_dir/tampered-policy.json" <<'PY'
import json
import sys

path = sys.argv[1]
with open(path, encoding="utf-8") as source:
    policy = json.load(source)
policy["sourceCaptureSha256"] = "0" * 64
with open(path, "w", encoding="utf-8") as output:
    json.dump(policy, output)
PY
if python3 "$project_dir/tools/compat_capture/apply_dispositions.py" \
  --capture "$capture" --policy "$work_dir/tampered-policy.json" \
  --output "$work_dir/rejected.json"; then
  echo "disposition tool unexpectedly accepted a mismatched capture pin" >&2
  exit 1
fi
test ! -e "$work_dir/rejected.json"

echo "compat-disposition: PASS"
