#!/usr/bin/env bash
# Builds (and optionally uploads) a SIMULATE ON bench test firmware, see README.md
#
#   build_sim.sh [--config PATH] [--upload]
#
# The sketch is copied to a temporary directory and Sim.config.h is appended to a copy of
# the chosen Config.h, so the working tree is never modified.
set -euo pipefail

repo="$(cd "$(dirname "$0")/../../.." && pwd)"
config="$repo/Config.h"
upload=false

while [ $# -gt 0 ]; do
  case "$1" in
    --config) config="$2"; shift 2 ;;
    --upload) upload=true; shift ;;
    *) echo "usage: $0 [--config PATH] [--upload]" >&2; exit 2 ;;
  esac
done

fqbn="teensy:avr:teensy41:usb=serial3,speed=600,opt=o2std,keys=en-gb"
work="${TMPDIR:-/tmp}/ocs-sim"
sketch="$work/OCS"

mkdir -p "$sketch"
rsync -a --delete --exclude '/.git' --exclude '/build' --exclude '/.github' --exclude '/.vscode' "$repo/" "$sketch/"
cp "$config" "$sketch/Config.h"
cat "$repo/src/plugins/sim/Sim.config.h" >> "$sketch/Config.h"

arduino-cli compile --fqbn "$fqbn" --build-path "$work/build" "$sketch"

if $upload; then
  port="$(arduino-cli board list | awk '$2 == "teensy" { print $1; exit }')"
  if [ -z "$port" ]; then echo "No Teensy found by 'arduino-cli board list'" >&2; exit 1; fi
  arduino-cli upload --fqbn "$fqbn" --input-dir "$work/build" -p "$port" "$sketch"
fi
