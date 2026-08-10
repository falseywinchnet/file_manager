#!/bin/sh
set -eu

repo=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
build_dir=${1:-"$repo/.build/windows-x64-skia-make"}
skia_out=${2:-"$repo/.build/skia-windows-mingw"}

case $build_dir in
  /*) ;;
  *) build_dir="$(CDPATH= cd -- "$repo/.." && pwd)/$build_dir" ;;
esac
case $skia_out in
  /*) ;;
  *) skia_out="$(CDPATH= cd -- "$repo/.." && pwd)/$skia_out" ;;
esac

/usr/bin/env PATH=/opt/homebrew/bin:/usr/bin:/bin \
  /opt/homebrew/bin/cmake -S "$repo" -B "$build_dir" -G "Unix Makefiles" \
  -DCMAKE_TOOLCHAIN_FILE="$repo/cmake/toolchains/x86_64-w64-mingw32.cmake" \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
  -DGUI_FORMS_ENABLE_SKIA=ON \
  -DGUI_FORMS_SKIA_PREBUILT=ON \
  -DGUI_FORMS_SKIA_OUT="$skia_out" \
  -DGUI_FORMS_ENABLE_WINDOWS_HOST=ON
