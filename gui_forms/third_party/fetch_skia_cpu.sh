#!/bin/sh
set -eu

script_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
skia_dir="$script_dir/skia"
skia_revision=2a9b593bab4b2fd019fa494c8d401ff1fab0b883
libpng_revision=d5515b5b8be3901aac04e5bd8bd5c89f287bcd33
zlib_revision=646b7f569718921d7d4b5b8e22572ff6c76f2596

fetch_revision() {
  destination=$1
  remote=$2
  revision=$3
  if [ ! -d "$destination/.git" ]; then
    git init "$destination"
    git -C "$destination" remote add origin "$remote"
  fi
  if ! git -C "$destination" cat-file -e "$revision^{commit}" 2>/dev/null; then
    git -C "$destination" fetch --depth=1 origin "$revision"
  fi
  git -C "$destination" checkout --detach "$revision"
}

fetch_revision "$skia_dir" https://skia.googlesource.com/skia.git "$skia_revision"
if git -C "$skia_dir" apply --check "$script_dir/skia_png_only.patch" 2>/dev/null; then
  git -C "$skia_dir" apply "$script_dir/skia_png_only.patch"
elif ! git -C "$skia_dir" apply --reverse --check "$script_dir/skia_png_only.patch" 2>/dev/null; then
  echo "Skia PNG-only source patch does not apply cleanly." >&2
  exit 1
fi
"$skia_dir/bin/fetch-gn"
"$skia_dir/bin/fetch-ninja"
mkdir -p "$skia_dir/third_party/externals"
fetch_revision "$skia_dir/third_party/externals/libpng" \
  https://skia.googlesource.com/third_party/libpng.git "$libpng_revision"
fetch_revision "$skia_dir/third_party/externals/zlib" \
  https://chromium.googlesource.com/chromium/src/third_party/zlib "$zlib_revision"

test "$(git -C "$skia_dir" rev-parse HEAD)" = "$skia_revision"
test "$(git -C "$skia_dir/third_party/externals/libpng" rev-parse HEAD)" = "$libpng_revision"
test "$(git -C "$skia_dir/third_party/externals/zlib" rev-parse HEAD)" = "$zlib_revision"
