#!/bin/sh
set -eu

script_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
project_dir=$(CDPATH= cd -- "$script_dir/.." && pwd)
lab_dir="$project_dir/tools/compat_loader_lab"
work_dir=$(mktemp -d "${TMPDIR:-/tmp}/gui-forms-loader-lab.XXXXXX")
trap 'rm -rf "$work_dir"' EXIT HUP INT TERM

forms_reference=${GUI_FORMS_WINDOWS_FORMS_REFERENCE:-"$HOME/.wine/drive_c/Program Files/dotnet/packs/Microsoft.WindowsDesktop.App.Ref/10.0.5/ref/net10.0/System.Windows.Forms.dll"}
wine_dotnet=${GUI_FORMS_WINE_DOTNET:-"$HOME/.wine/drive_c/Program Files/dotnet/dotnet.exe"}

test -f "$forms_reference"
test -f "$wine_dotnet"

dotnet build "$lab_dir/Facade/GuiForms.FacadeLab.csproj" -c Release \
  --nologo --verbosity quiet -o "$work_dir/facade" \
  -p:BaseIntermediateOutputPath="$work_dir/obj-a/facade/"
dotnet build "$lab_dir/Consumer/GuiForms.LoaderConsumer.csproj" -c Release \
  --nologo --verbosity quiet -o "$work_dir/consumer" \
  -p:FormsReferencePath="$forms_reference" \
  -p:BaseIntermediateOutputPath="$work_dir/obj-a/consumer/"
dotnet build "$lab_dir/Loader/GuiForms.LoaderLab.csproj" -c Release \
  --nologo --verbosity quiet -o "$work_dir/loader" \
  -p:BaseIntermediateOutputPath="$work_dir/obj-a/loader/"

dotnet build "$lab_dir/Facade/GuiForms.FacadeLab.csproj" -c Release \
  --nologo --verbosity quiet -o "$work_dir/facade-repeat" \
  -p:BaseIntermediateOutputPath="$work_dir/obj-b/facade/"
dotnet build "$lab_dir/Consumer/GuiForms.LoaderConsumer.csproj" -c Release \
  --nologo --verbosity quiet -o "$work_dir/consumer-repeat" \
  -p:FormsReferencePath="$forms_reference" \
  -p:BaseIntermediateOutputPath="$work_dir/obj-b/consumer/"
dotnet build "$lab_dir/Loader/GuiForms.LoaderLab.csproj" -c Release \
  --nologo --verbosity quiet -o "$work_dir/loader-repeat" \
  -p:BaseIntermediateOutputPath="$work_dir/obj-b/loader/"

consumer="$work_dir/consumer/GuiForms.LoaderConsumer.dll"
facade="$work_dir/facade/System.Windows.Forms.dll"
loader="$work_dir/loader/gui-forms-loader-lab.dll"

test ! -e "$work_dir/consumer/System.Windows.Forms.dll"
cmp "$facade" "$work_dir/facade-repeat/System.Windows.Forms.dll"
cmp "$consumer" "$work_dir/consumer-repeat/GuiForms.LoaderConsumer.dll"
cmp "$loader" "$work_dir/loader-repeat/gui-forms-loader-lab.dll"

dotnet "$loader" "$consumer" "$facade" > "$work_dir/native.json"
wine "$wine_dotnet" "$(winepath -w "$loader")" \
  "$(winepath -w "$consumer")" "$(winepath -w "$facade")" \
  > "$work_dir/wine.json"

python3 - "$work_dir/native.json" "$work_dir/wine.json" <<'PY'
import json
import sys

documents = []
for path in sys.argv[1:]:
    with open(path, encoding="utf-8") as source:
        documents.append(json.load(source))
for document in documents:
    assert document["schema"] == "gui.forms.compat.loader-lab/v1"
    assert document["status"] == "pass"
    assert document["requestedAssembly"].startswith("System.Windows.Forms, Version=10.0.0.0")
    assert document["requestedPublicKeyToken"] == "b77a5c561934e089"
    assert document["loadedAssembly"].startswith("System.Windows.Forms, Version=10.0.0.0")
    assert document["loadedPublicKeyToken"] == "null"
    assert document["identityMismatchAccepted"] is True
    assert document["probeResult"] == "clicks=1;controls=1;text=Activate"
assert documents[0] == documents[1]
PY

echo "compat-loader-lab: PASS"
