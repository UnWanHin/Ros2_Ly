#!/usr/bin/env bash

# AUTO-COMMENT: file overview
# This file belongs to the ROS2 sentry workspace codebase.
# Keep behavior and interface changes synchronized with related modules.

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
SCRIPT_NAME="$(basename "$0")"

TOPIC="/ly/outpost/target"
PITCH="10.0"
INTERVAL_SEC="1.0"
YAWS=(45 60 75 60)

# shellcheck disable=SC1091
source "${ROOT_DIR}/scripts/lib/ros_launch_common.sh"

usage() {
  cat <<EOF
Usage:
  ${SCRIPT_NAME} [options]

Purpose:
  Publish test /ly/outpost/target in loop for bridge verification.
  Yaw sequence is fixed: 45 -> 60 -> 75 -> 60 -> ...

Options:
  --topic TOPIC       Target topic. Default: ${TOPIC}
  --pitch DEG         Fixed pitch degree. Default: ${PITCH}
  --interval SEC      Publish interval (seconds). Default: ${INTERVAL_SEC}
  --help|-h           Show help

Example:
  ./${SCRIPT_NAME}
  ./${SCRIPT_NAME} --pitch 12 --interval 1.0
EOF
}

while [[ $# -gt 0 ]]; do
  case "$1" in
    --topic)
      TOPIC="$2"
      shift 2
      ;;
    --pitch)
      PITCH="$2"
      shift 2
      ;;
    --interval)
      INTERVAL_SEC="$2"
      shift 2
      ;;
    --help|-h)
      usage
      exit 0
      ;;
    *)
      echo "[ERROR] Unknown argument: $1" >&2
      usage >&2
      exit 2
      ;;
  esac
done

source_ros_workspace "${ROOT_DIR}"

echo "[OUTPOST-TARGET-TEST] topic=${TOPIC} pitch=${PITCH} interval=${INTERVAL_SEC}s" >&2
echo "[OUTPOST-TARGET-TEST] yaw loop: 45 -> 60 -> 75 -> 60 -> ..." >&2

index=0
while true; do
  yaw="${YAWS[$index]}"
  echo "[OUTPOST-TARGET-TEST][TX] yaw=${yaw} pitch=${PITCH}" >&2
  ros2 topic pub "${TOPIC}" auto_aim_common/msg/Target \
    "{status: true, buff_follow: false, yaw: ${yaw}, pitch: ${PITCH}}" -1 >/dev/null
  sleep "${INTERVAL_SEC}"
  index=$(((index + 1) % ${#YAWS[@]}))
done
