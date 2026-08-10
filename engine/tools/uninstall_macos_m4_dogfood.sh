#!/bin/sh
set -eu

LABEL=com.filemanager.engine.m4-dogfood
DOMAIN="gui/$(id -u)"
APP_ROOT="$HOME/Library/Application Support/FileManager/Engine/m4-dogfood"
RUNTIME_DIR="$APP_ROOT/runtime"
PLIST="$HOME/Library/LaunchAgents/$LABEL.plist"

if /bin/launchctl print "$DOMAIN/$LABEL" >/dev/null 2>&1; then
  /bin/launchctl bootout "$DOMAIN/$LABEL"
fi
rm -f "$PLIST" \
  "$RUNTIME_DIR/query.sock" "$RUNTIME_DIR/admin.sock" \
  "$RUNTIME_DIR/query.token" "$RUNTIME_DIR/admin.token" \
  "$RUNTIME_DIR/discovery.json"

echo "Removed the LaunchAgent and volatile endpoints."
echo "Preserved the manifest, durable store, logs, and shared binary under $APP_ROOT and $HOME/.local/bin."
echo "Delete those explicit paths separately only when their evidence is no longer needed."
