#!/bin/sh
set -eu

repository_root=$(git rev-parse --show-toplevel)
build_helper=/Users/ultimussecundai/.local/bin/m4build
build_directory=gui_forms/build
test_pattern=${GUI_FORMS_TEST_PATTERN:-gui_forms_(composition_controls|showcase_interaction|gallery_interaction)_tests}
jobs=${GUI_FORMS_BUILD_JOBS:-10}
verification_tmp=$(mktemp -d "${TMPDIR:-/tmp}/gui-forms-verify.XXXXXX")
trap 'rm -rf "$verification_tmp"' EXIT HUP INT TERM

run_or_report() {
    label=$1
    shift
    log="$verification_tmp/$label.log"
    if ! "$@" >"$log" 2>&1; then
        echo "GUI.Forms M4 verification failed during $label" >&2
        tail -n 100 "$log" >&2
        exit 1
    fi
}

cd "$repository_root"
run_or_report build "$build_helper" -- \
    /opt/homebrew/bin/cmake --build "$build_directory" --parallel "$jobs"
run_or_report tests "$build_helper" -- \
    /opt/homebrew/bin/ctest --test-dir "$build_directory" \
    --output-on-failure -R "$test_pattern"

echo "GUI.Forms M4 verification: PASS"
