#!/bin/sh
set -eu

codex_runs="$HOME/Developer/CodexRuns"
exec /usr/bin/open -n "$codex_runs/fmnew.app" --args \
  --root "$codex_runs/fmsandbox" \
  --engine-root-id fm1-contained \
  --allow-mutations \
  --quarantine "$codex_runs/fmquarantine"
