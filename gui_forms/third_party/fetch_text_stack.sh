#!/bin/sh
set -eu

script_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
harfbuzz_revision=56feae4035bdd48f62ba2b8d8c16232d4d89b3a4
freetype_revision=f4205da14867c5387cd6a329b90ee10a6df6eeff

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

fetch_revision "$script_dir/harfbuzz" \
  https://github.com/harfbuzz/harfbuzz.git "$harfbuzz_revision"
fetch_revision "$script_dir/freetype" \
  https://gitlab.freedesktop.org/freetype/freetype.git "$freetype_revision"

test "$(git -C "$script_dir/harfbuzz" rev-parse HEAD)" = "$harfbuzz_revision"
test "$(git -C "$script_dir/freetype" rev-parse HEAD)" = "$freetype_revision"
