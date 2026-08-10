#!/bin/sh
set -eu

# Screen Sharing/Aqua launcher for the deterministic m4build mirror. Running
# the binary through a symlink changes NSBundle discovery and hides bundled
# fonts, so the executable path itself must remain inside the real .app.
demoboard_executable='/Users/joshuahkuttenkuler/Developer/CodexBuilds/file_manager-2d80cdb86b7d/gui_forms/build/File Manager Demoboard.app/Contents/MacOS/File Manager Demoboard'
exec "$demoboard_executable"
