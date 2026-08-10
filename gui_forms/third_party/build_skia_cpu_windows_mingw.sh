#!/bin/sh
set -eu

script_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
skia_dir="$script_dir/skia"
output_dir=${1:-"$skia_dir/out/gui_forms-cpu-windows-mingw"}

if [ ! -x "$skia_dir/bin/gn" ] || [ ! -x "$skia_dir/third_party/ninja/ninja" ]; then
  echo "Skia is absent. Run $script_dir/fetch_skia_cpu.sh first." >&2
  exit 1
fi

mkdir -p "$output_dir"
cp "$script_dir/skia_cpu_windows_mingw_args.gn" "$output_dir/args.gn"
"$skia_dir/bin/gn" gen "$output_dir" --root="$skia_dir"
"$skia_dir/third_party/ninja/ninja" -C "$output_dir" skia
