#!/bin/sh
set -eu
script_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
skia_dir="$script_dir/skia"
output_dir=${1:-"$skia_dir/out/gui_forms-linux-cpu"}
case $(uname -m) in
  aarch64) target_cpu=arm64 ;;
  x86_64) target_cpu=x64 ;;
  *) echo "Unsupported Linux build architecture" >&2; exit 1 ;;
esac
command -v gn >/dev/null
command -v ninja >/dev/null
mkdir -p "$output_dir"
sed -e "s/target_cpu = .*/target_cpu = \"$target_cpu\"/" \
    -e 's/skia_use_fonthost_mac = true/skia_use_fonthost_mac = false/' \
    -e 's/skia_use_freetype = false/skia_use_freetype = true/' \
    "$script_dir/skia_cpu_args.gn" > "$output_dir/args.gn"
cat >> "$output_dir/args.gn" <<'ARGS'
target_os = "linux"
cc = "clang"
cxx = "clang++"
skia_use_system_freetype2 = false
skia_enable_fontmgr_custom_embedded = true
skia_enable_fontmgr_custom_empty = true
extra_cflags = [ "-fPIC" ]
ARGS
gn gen "$output_dir" --root="$skia_dir"
ninja -C "$output_dir" -j "${BUILD_JOBS:-2}" skia
