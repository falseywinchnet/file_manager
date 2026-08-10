#!/bin/sh
set -eu

if [ "$(uname -s)" != Darwin ] || [ "$(uname -m)" != arm64 ]; then
  echo "This dogfood installer is restricted to an Apple-silicon Mac." >&2
  exit 2
fi
if [ "$#" -ne 1 ]; then
  echo "usage: $0 /absolute/approved/source/root" >&2
  exit 2
fi

case "$1" in
  /*) ;;
  *) echo "approved source root must be absolute" >&2; exit 2 ;;
esac

SOURCE_ROOT=$(cd "$1" && pwd -P)
SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd -P)
ENGINE_DIR=$(dirname "$SCRIPT_DIR")
GO_BIN=${GO_BIN:-/opt/homebrew/bin/go}
LABEL=com.filemanager.engine.m4-dogfood
DOMAIN="gui/$(id -u)"
APP_ROOT="$HOME/Library/Application Support/FileManager/Engine/m4-dogfood"
STORE_ROOT="$APP_ROOT/store"
RUNTIME_DIR="$APP_ROOT/runtime"
LOG_DIR="$APP_ROOT/logs"
MANIFEST="$APP_ROOT/deployment.json"
BINARY="$HOME/.local/bin/fileman-engine"
PLIST="$HOME/Library/LaunchAgents/$LABEL.plist"

mkdir -p "$HOME/.local/bin" "$HOME/Library/LaunchAgents" "$APP_ROOT" "$STORE_ROOT" "$RUNTIME_DIR" "$LOG_DIR" "$ENGINE_DIR/build/bin"
chmod 700 "$APP_ROOT" "$STORE_ROOT" "$RUNTIME_DIR" "$LOG_DIR"

(cd "$ENGINE_DIR" && "$GO_BIN" build -trimpath -o "$ENGINE_DIR/build/bin/fileman-engine" ./cmd/fileman-engine)
install -m 0755 "$ENGINE_DIR/build/bin/fileman-engine" "$BINARY"

"$BINARY" create-macos-manifest \
  --deployment-id m4-dogfood \
  --root-id m4-file-manager-source \
  --root-path "$SOURCE_ROOT" \
  --store-root "$STORE_ROOT" \
  --runtime-dir "$RUNTIME_DIR" \
  --output "$MANIFEST"

"$BINARY" write-launchd-plist \
  --label "$LABEL" \
  --binary "$BINARY" \
  --manifest "$MANIFEST" \
  --stdout "$LOG_DIR/stdout.log" \
  --stderr "$LOG_DIR/stderr.log" \
  --output "$PLIST"
/usr/bin/plutil -lint "$PLIST"

if /bin/launchctl print "$DOMAIN/$LABEL" >/dev/null 2>&1; then
  /bin/launchctl bootout "$DOMAIN/$LABEL"
fi
/bin/launchctl bootstrap "$DOMAIN" "$PLIST"
/bin/launchctl kickstart -k "$DOMAIN/$LABEL"

attempt=0
STATUS_OUTPUT=
while [ "$attempt" -lt 600 ]; do
  if STATUS_OUTPUT=$("$BINARY" call-local --runtime-dir "$RUNTIME_DIR" --authority query \
    --timeout 2s --request '{"id":"install-status","method":"engine.status"}' 2>/dev/null); then
    break
  fi
  attempt=$((attempt + 1))
  sleep 0.1
done
if [ -z "$STATUS_OUTPUT" ]; then
  echo "Engine endpoint did not authenticate as ready; inspect $LOG_DIR/stderr.log" >&2
  exit 1
fi

echo "$STATUS_OUTPUT"
echo "Installed $LABEL for $SOURCE_ROOT"
echo "State: $APP_ROOT"
echo "Plist: $PLIST"
