#!/bin/sh
set -eu

repo=${1:-"$(CDPATH= cd -- "$(dirname "$0")/.." && pwd)"}
windows_ref=${2:-"$HOME/.wine/drive_c/Program Files/dotnet/packs/Microsoft.WindowsDesktop.App.Ref/10.0.5/ref/net10.0"}
windows_runtime=${3:-"$HOME/.wine/drive_c/Program Files/dotnet/shared/Microsoft.WindowsDesktop.App/10.0.5"}
wine_binary=${WINE_BINARY:-/opt/homebrew/bin/wine}
catalogue=
for candidate in "$repo"/compatibility/*/*.facade-catalogue-v1.json; do
  [ -f "$candidate" ] || continue
  [ -z "$catalogue" ] || {
    echo "multiple private compatibility catalogues found" >&2
    exit 1
  }
  catalogue=$candidate
done
[ -n "$catalogue" ] || {
  echo "private compatibility catalogue not found" >&2
  exit 1
}
generated="$repo/generated/facade-v1"
mac_build="$repo/build-abi-surface"
windows_build="$repo/build-abi-windows-skia-smoke"
smoke_output="$repo/tools/facade_smoke/bin/Release/net10.0"
behavior_output="$repo/tools/facade_behavior_smoke/bin/Release/net10.0"

dotnet run --project "$repo/tools/facade_generator/GuiForms.FacadeGenerator.csproj" -c Release -- \
  "$catalogue" "$windows_ref" "$generated"
dotnet build "$generated/System.Windows.Forms/System.Windows.Forms.csproj" -c Release
dotnet run --project "$repo/tools/facade_verifier/GuiForms.FacadeVerifier.csproj" -c Release -- \
  "$catalogue" "$windows_ref" \
  "$generated/System.Windows.Forms/bin/Release/net10.0/System.Windows.Forms.dll" \
  "$generated/System.Windows.Forms.Primitives/bin/Release/net10.0/System.Windows.Forms.Primitives.dll" \
  "$generated/System.Drawing.Common/bin/Release/net10.0/System.Drawing.Common.dll"

cmake -S "$repo" -B "$mac_build" -DGUI_FORMS_BUILD_GALLERY=OFF \
  -DGUI_FORMS_ENABLE_SKIA=OFF -DGUI_FORMS_ENABLE_MACOS_HOST=OFF \
  -DGUI_FORMS_BUILD_TESTS=ON
cmake --build "$mac_build" --target gui_forms_c_api_c11_tests gui_forms_c_api_cpp_tests --parallel
ctest --test-dir "$mac_build" -R 'gui_forms_c_api_(c11|cpp)_tests' --output-on-failure
dotnet build "$repo/tools/facade_smoke/GuiForms.FacadeSmoke.csproj" -c Release
dotnet build "$repo/tools/facade_behavior_smoke/GuiForms.FacadeBehaviorSmoke.csproj" -c Release
GUI_FORMS_FORCE_HEADLESS=1 GUI_FORMS_AUTOMATION_ACTIVATE=1 \
  DYLD_LIBRARY_PATH="$mac_build" \
  dotnet "$smoke_output/GuiForms.FacadeSmoke.dll"
behavior_host_log=$(mktemp "${TMPDIR:-/tmp}/gui-forms-behavior-host.XXXXXX")
for behavior_mode in form-semantics secondary-form cursor dock-padding split-container lifecycle-order; do
  GUI_FORMS_FORCE_HEADLESS=1 DYLD_LIBRARY_PATH="$mac_build" \
    dotnet "$behavior_output/GuiForms.FacadeBehaviorSmoke.dll" "$behavior_mode" \
    >>"$behavior_host_log"
done
cat "$behavior_host_log"
rg -q 'form-semantics=tab:nested\|accept:ordered\|cancel:ordered\|active-control:scoped\|owner:acyclic\|modal:focus-restored\|label-focus:rejected' "$behavior_host_log"
rg -q 'secondary-form=attached:true\|owned:true\|clamped:true\|reopened:true\|cancelled:true\|detached:true\|focus-restored:true\|loads:1\|closed:1' "$behavior_host_log"
rg -q 'cursor=identity:stable\|projection:roundtrip\|inherit:restored' "$behavior_host_log"
rg -q 'dock-padding=projection:owned\|fill:inset\|relayout:updated' "$behavior_host_log"
rg -q 'split-container=geometry:constrained\|drag:live\|collapse:focus-transferred\|orientation:horizontal\|fixed:enforced' "$behavior_host_log"
rg -q 'lifecycle-order=load>input>closing>closed' "$behavior_host_log"
unlink "$behavior_host_log"

cmake -S "$repo" -B "$windows_build" \
  -DCMAKE_TOOLCHAIN_FILE="$repo/cmake/toolchains/x86_64-w64-mingw32.cmake" \
  -DGUI_FORMS_BUILD_GALLERY=OFF -DGUI_FORMS_ENABLE_SKIA=ON \
  -DGUI_FORMS_SKIA_PREBUILT=ON \
  -DGUI_FORMS_SKIA_OUT="$repo/third_party/skia/out/gui_forms-cpu-windows-mingw" \
  -DGUI_FORMS_ENABLE_WINDOWS_HOST=ON -DGUI_FORMS_BUILD_TESTS=OFF
cmake --build "$windows_build" --target gui_forms_c_api gui_drawing_c_api gui_drawing_raster_c_api gui_forms_windows_probe --parallel
cp "$windows_build/gui_forms_abi0.dll" "$smoke_output/"
cp "$windows_build/gui_forms_abi0.dll" "$behavior_output/"
cp "$windows_build/gui_drawing_abi0.dll" "$smoke_output/"
cp "$windows_build/gui_drawing_abi0.dll" "$behavior_output/"
cp "$windows_build/gui_drawing_raster0.dll" "$smoke_output/"
cp "$windows_build/gui_drawing_raster0.dll" "$behavior_output/"
mkdir -p "$smoke_output/fonts" "$behavior_output/fonts"
cp "$windows_build/fonts/PortsmouthRapids.ttf" "$smoke_output/fonts/"
cp "$windows_build/fonts/PortsmouthRapids-Bold.ttf" "$smoke_output/fonts/"
cp "$windows_build/fonts/PortsmouthRapids.ttf" "$behavior_output/fonts/"
cp "$windows_build/fonts/PortsmouthRapids-Bold.ttf" "$behavior_output/fonts/"
cp /opt/homebrew/Cellar/mingw-w64/13.0.0_2/toolchain-x86_64/x86_64-w64-mingw32/lib/libgcc_s_seh-1.dll "$smoke_output/"
cp /opt/homebrew/Cellar/mingw-w64/13.0.0_2/toolchain-x86_64/x86_64-w64-mingw32/lib/libstdc++-6.dll "$smoke_output/"
cp /opt/homebrew/Cellar/mingw-w64/13.0.0_2/toolchain-x86_64/x86_64-w64-mingw32/bin/libwinpthread-1.dll "$smoke_output/"
cp /opt/homebrew/Cellar/mingw-w64/13.0.0_2/toolchain-x86_64/x86_64-w64-mingw32/lib/libgcc_s_seh-1.dll "$behavior_output/"
cp /opt/homebrew/Cellar/mingw-w64/13.0.0_2/toolchain-x86_64/x86_64-w64-mingw32/lib/libstdc++-6.dll "$behavior_output/"
cp /opt/homebrew/Cellar/mingw-w64/13.0.0_2/toolchain-x86_64/x86_64-w64-mingw32/bin/libwinpthread-1.dll "$behavior_output/"
for dependency in Microsoft.Win32.SystemEvents.dll System.Formats.Nrbf.dll System.Private.Windows.Core.dll System.Private.Windows.GdiPlus.dll; do
  cp "$windows_runtime/$dependency" "$smoke_output/"
  cp "$windows_runtime/$dependency" "$behavior_output/"
done
wine_log=$(mktemp "${TMPDIR:-/tmp}/gui-forms-facade-wine.XXXXXX")
(cd "$smoke_output" && WINEDEBUG=-all \
  "$wine_binary" 'C:\Program Files\dotnet\dotnet.exe' GuiForms.FacadeSmoke.dll \
  >"$wine_log" 2>&1) &
wine_pid=$!
attempt=0
until GUI_FORMS_AUTOMATION_TITLE='GUI.Forms Radio Console' WINEDEBUG=-all \
  "$wine_binary" "$windows_build/gui_forms_windows_probe.exe" snapshot \
    >/dev/null 2>&1; do
  attempt=$((attempt + 1))
  if [ "$attempt" -ge 80 ]; then
    kill "$wine_pid" 2>/dev/null || true
    wait "$wine_pid" 2>/dev/null || true
    echo "managed Wine surface did not publish its automation window" >&2
    exit 1
  fi
  sleep 0.25
done
GUI_FORMS_AUTOMATION_TITLE='GUI.Forms Radio Console' WINEDEBUG=-all \
  "$wine_binary" "$windows_build/gui_forms_windows_probe.exe" \
  click startButton >/dev/null
wait "$wine_pid"
cat "$wine_log"
rg -q 'callbacks=click:1\|dispatch:1\|closing:1\|closed:1\|faults:1' "$wine_log"
unlink "$wine_log"

behavior_wine_log=$(mktemp "${TMPDIR:-/tmp}/gui-forms-behavior-wine.XXXXXX")
for behavior_mode in form-semantics secondary-form cursor dock-padding split-container lifecycle-order; do
  (cd "$behavior_output" && GUI_FORMS_FORCE_HEADLESS=1 WINEDEBUG=-all \
    "$wine_binary" 'C:\Program Files\dotnet\dotnet.exe' \
    GuiForms.FacadeBehaviorSmoke.dll "$behavior_mode") >>"$behavior_wine_log" 2>&1
done
cat "$behavior_wine_log"
rg -q 'form-semantics=tab:nested\|accept:ordered\|cancel:ordered\|active-control:scoped\|owner:acyclic\|modal:focus-restored\|label-focus:rejected' "$behavior_wine_log"
rg -q 'secondary-form=attached:true\|owned:true\|clamped:true\|reopened:true\|cancelled:true\|detached:true\|focus-restored:true\|loads:1\|closed:1' "$behavior_wine_log"
rg -q 'cursor=identity:stable\|projection:roundtrip\|inherit:restored' "$behavior_wine_log"
rg -q 'dock-padding=projection:owned\|fill:inset\|relayout:updated' "$behavior_wine_log"
rg -q 'split-container=geometry:constrained\|drag:live\|collapse:focus-transferred\|orientation:horizontal\|fixed:enforced' "$behavior_wine_log"
rg -q 'lifecycle-order=load>input>closing>closed\|handle-created:1\|handle-destroyed:1\|early-close:suppressed' "$behavior_wine_log"
unlink "$behavior_wine_log"

echo "Generated facade surface and current experimental-ABI managed-loop smoke passed on host .NET and Wine."
