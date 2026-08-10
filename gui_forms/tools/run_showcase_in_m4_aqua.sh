#!/bin/sh
set -eu

# Run this from Terminal inside the M4 Mini's logged-in Aqua session. Launching
# the real bundle executable (not a symlink to it) preserves NSBundle resource
# discovery for the checked-in fonts.
showcase_executable='/Users/joshuahkuttenkuler/Developer/CodexBuilds/file_manager-2d80cdb86b7d/gui_forms/build/GUI.Forms Complete Showcase.app/Contents/MacOS/GUI.Forms Complete Showcase'
exec "$showcase_executable"
