#!/bin/sh
set -eu

if [ "$#" -lt 2 ] || [ "$#" -gt 6 ]; then
    echo "usage: run_inventory.sh <main-build-directory> <json-output> [pointer-rewrite-output] [auto-rewrite-output] [native|mingw] [production|first-party]" >&2
    exit 2
fi

tool_directory=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
gui_forms_root=$(CDPATH= cd -- "$tool_directory/../.." && pwd)
build_directory=$1
json_path=$2
pointer_rewrite_path=${3-}
auto_rewrite_path=
platform=native
source_scope=${6-production}
checker_mode=${GUI_FORMS_HOUSE_POLICY_MODE:-inventory}
case ${4-} in
    native|mingw) platform=$4 ;;
    *)
        auto_rewrite_path=${4-}
        platform=${5-native}
        ;;
esac
checker_build="$gui_forms_root/.build/house-policy-check"
checker="$checker_build/gui_forms_house_policy_check"
database="$build_directory/compile_commands.json"
checker_database_directory=$build_directory
llvm_clang=/opt/homebrew/opt/llvm/bin/clang
resource_directory=$($llvm_clang -print-resource-dir)
sdk_root=$(/usr/bin/xcrun --show-sdk-path)

case "$platform" in
    native|mingw) ;;
    *)
        echo "unsupported inventory platform: $platform" >&2
        exit 2
        ;;
esac

case "$source_scope" in
    production|first-party) ;;
    *)
        echo "unsupported inventory source scope: $source_scope" >&2
        exit 2
        ;;
esac

case "$checker_mode" in
    inventory|closure) ;;
    *)
        echo "unsupported house-policy mode: $checker_mode" >&2
        exit 2
        ;;
esac

if [ ! -x "$checker" ]; then
    echo "house-policy checker has not been built: $checker" >&2
    exit 2
fi
if [ ! -f "$database" ]; then
    echo "compilation database is missing: $database" >&2
    exit 2
fi

source_list=$(mktemp -t gui-forms-house-policy-sources.XXXXXX)
sanitized_database_directory=
trap 'rm -f "$source_list"; if [ -n "$sanitized_database_directory" ]; then rm -rf "$sanitized_database_directory"; fi' EXIT HUP INT TERM

if [ "$platform" = mingw ]; then
    sanitized_database_directory=$(mktemp -d -t gui-forms-house-policy-database.XXXXXX)
    /usr/bin/jq '
        map(
            if has("command") then
                .command |= gsub(" -fno-keep-inline-dllexport"; "")
            else
                .
            end
            |
            if has("arguments") then
                .arguments |= map(select(. != "-fno-keep-inline-dllexport"))
            else
                .
            end
        )
    ' "$database" > "$sanitized_database_directory/compile_commands.json"
    checker_database_directory=$sanitized_database_directory
fi

if [ "$source_scope" = production ]; then
    source_prefix="$gui_forms_root/src/"
else
    source_prefix="$gui_forms_root/"
fi

/usr/bin/jq -r --arg prefix "$source_prefix" \
    '.[] | select(.file | startswith($prefix)) | .file' "$database" |
    /usr/bin/sort -u > "$source_list"

set --
while IFS= read -r source_file; do
    case "$source_file" in
        *.c) ;;
        */third_party/*|*/experiments/*|*/build/*|*/.build/*) ;;
        *) set -- "$@" "$source_file" ;;
    esac
done < "$source_list"

if [ -n "$pointer_rewrite_path" ]; then
    set -- --pointer-rewrite-output "$pointer_rewrite_path" "$@"
fi
if [ -n "$auto_rewrite_path" ]; then
    set -- --auto-rewrite-output "$auto_rewrite_path" "$@"
fi

if [ "$platform" = mingw ]; then
    "$checker" \
        --mode "$checker_mode" \
        --source-scope "$source_scope" \
        --quiet \
        --source-root "$gui_forms_root" \
        --json-output "$json_path" \
        --extra-arg=-resource-dir="$resource_directory" \
        --extra-arg=--target=x86_64-w64-windows-gnu \
        --extra-arg=--sysroot=/opt/homebrew/opt/mingw-w64/toolchain-x86_64 \
        -p "$checker_database_directory" \
        "$@"
else
    "$checker" \
        --mode "$checker_mode" \
        --source-scope "$source_scope" \
        --quiet \
        --source-root "$gui_forms_root" \
        --json-output "$json_path" \
        --extra-arg=-resource-dir="$resource_directory" \
        --extra-arg=-isysroot \
        --extra-arg="$sdk_root" \
        -p "$build_directory" \
        "$@"
fi
