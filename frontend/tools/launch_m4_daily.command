#!/bin/sh
set -eu

tool_directory=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
exec /bin/sh "$tool_directory/launch_m4_dogfood.sh" daily
