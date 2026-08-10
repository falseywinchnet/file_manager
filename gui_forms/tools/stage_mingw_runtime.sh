#!/bin/sh
set -eu

destination=${1:?usage: stage_mingw_runtime.sh DESTINATION}
toolchain=/opt/homebrew/opt/mingw-w64/toolchain-x86_64/x86_64-w64-mingw32

mkdir -p "$destination"
cp "$toolchain/lib/libgcc_s_seh-1.dll" "$destination/"
cp "$toolchain/lib/libstdc++-6.dll" "$destination/"
cp "$toolchain/bin/libwinpthread-1.dll" "$destination/"
