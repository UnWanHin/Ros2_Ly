#!/usr/bin/env bash

# AUTO-COMMENT: file overview
# This file belongs to the ROS2 sentry workspace codebase.
# Keep behavior and interface changes synchronized with related modules.

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
SCRIPT_NAME="$(basename "$0")"

ENABLE_FIRE=1
USE_TARGET_STATUS=0
FIRE_HZ="20.0"
TARGET_TIMEOUT_SEC="1.0"
LAUNCH_ARGS=()

DEFAULT_BASE_CONFIG_FILE="${ROOT_DIR}/config/base_config.yaml"
DEFAULT_DETECTOR_CONFIG_FILE="${ROOT_DIR}/src/detector/config/detector_config.yaml"
DEFAULT_OUTPOST_CONFIG_FILE="${ROOT_DIR}/src/outpost_hitter/config/outpost_config.yaml"
DEFAULT_OVERRIDE_CONFIG_FILE="${ROOT_DIR}/config/override_config.yaml"

# shellcheck disable=SC1091
source "${ROOT_DIR}/scripts/lib/ros_launch_common.sh"

usage() {
  cat <<EOF
Usage:
  ${SCRIPT_NAME} [--fire|--no-fire] [--use-target-status|--ignore-target-status]
                 [--fire-hz <hz>] [--target-timeout-sec <sec>]
                 [-- <launch_args...>]

Purpose:
  Pure outpost test:
  - no behavior_tree
  - force /ly/aa/enable=false, /ly/ra/enable=false, /ly/outpost/enable=true, /ly/bt/target=7
  - bridge /ly/outpost/target -> /ly/control/* for gimbal debug/firing

Examples:
  ./${SCRIPT_NAME}
  ./${SCRIPT_NAME} --no-fire
  ./${SCRIPT_NAME} --fire-hz 15
  ./${SCRIPT_NAME} --use-target-status
  ./${SCRIPT_NAME} -- bridge_timeout_sec:=0.8 detector_config.show:=true
EOF
}

has_launch_arg_key() {
  local key="$1"
  local arg
  for arg in "${LAUNCH_ARGS[@]}"; do
    if [[ "${arg}" == "${key}:="* ]]; then
      return 0
    fi
  done
  return 1
}

while [[ $# -gt 0 ]]; do
  case "$1" in
    --fire)
      ENABLE_FIRE=1
      shift
      ;;
    --no-fire)
      ENABLE_FIRE=0
      shift
      ;;
    --use-target-status)
      USE_TARGET_STATUS=1
      shift
      ;;
    --ignore-target-status)
      USE_TARGET_STATUS=0
      shift
      ;;
    --fire-hz)
      if [[ $# -lt 2 ]]; then
        echo "[ERROR] --fire-hz requires a value" >&2
        exit 2
      fi
      FIRE_HZ="$2"
      shift 2
      ;;
    --target-timeout-sec)
      if [[ $# -lt 2 ]]; then
        echo "[ERROR] --target-timeout-sec requires a value" >&2
        exit 2
      fi
      TARGET_TIMEOUT_SEC="$2"
      shift 2
      ;;
    --help|-h)
      usage
      exit 0
      ;;
    --)
      shift
      LAUNCH_ARGS=("$@")
      break
      ;;
    *)
      LAUNCH_ARGS+=("$1")
      shift
      ;;
  esac
done

source_ros_workspace "${ROOT_DIR}"
cleanup_existing_stack "1" "/(gimbal_driver_node|detector_node|outpost_hitter_node|outpost_test_bridge|behavior_tree_node|buff_test_bridge)([[:space:]]|$)" "ros2 launch (detector|behavior_tree) (outpost_test|outpost|buff_test|competition_autoaim|sentry_all|chase_only|showcase)\\.launch.py"

if [[ -f "${DEFAULT_BASE_CONFIG_FILE}" ]] && ! has_launch_arg_key "base_config_file"; then
  LAUNCH_ARGS=("base_config_file:=${DEFAULT_BASE_CONFIG_FILE}" "${LAUNCH_ARGS[@]}")
fi
if [[ -f "${DEFAULT_DETECTOR_CONFIG_FILE}" ]] && ! has_launch_arg_key "detector_config_file"; then
  LAUNCH_ARGS=("detector_config_file:=${DEFAULT_DETECTOR_CONFIG_FILE}" "${LAUNCH_ARGS[@]}")
fi
if [[ -f "${DEFAULT_OUTPOST_CONFIG_FILE}" ]] && ! has_launch_arg_key "outpost_config_file"; then
  LAUNCH_ARGS=("outpost_config_file:=${DEFAULT_OUTPOST_CONFIG_FILE}" "${LAUNCH_ARGS[@]}")
fi
if [[ -f "${DEFAULT_OVERRIDE_CONFIG_FILE}" ]] && ! has_launch_arg_key "config_file"; then
  LAUNCH_ARGS=("config_file:=${DEFAULT_OVERRIDE_CONFIG_FILE}" "${LAUNCH_ARGS[@]}")
fi

if ! has_launch_arg_key "bridge_enable_fire"; then
  if (( ENABLE_FIRE == 1 )); then
    LAUNCH_ARGS=("bridge_enable_fire:=true" "${LAUNCH_ARGS[@]}")
  else
    LAUNCH_ARGS=("bridge_enable_fire:=false" "${LAUNCH_ARGS[@]}")
  fi
fi

if ! has_launch_arg_key "bridge_use_target_status"; then
  if (( USE_TARGET_STATUS == 1 )); then
    LAUNCH_ARGS=("bridge_use_target_status:=true" "${LAUNCH_ARGS[@]}")
  else
    LAUNCH_ARGS=("bridge_use_target_status:=false" "${LAUNCH_ARGS[@]}")
  fi
fi

if ! has_launch_arg_key "bridge_fire_hz"; then
  LAUNCH_ARGS=("bridge_fire_hz:=${FIRE_HZ}" "${LAUNCH_ARGS[@]}")
fi
if ! has_launch_arg_key "bridge_timeout_sec"; then
  LAUNCH_ARGS=("bridge_timeout_sec:=${TARGET_TIMEOUT_SEC}" "${LAUNCH_ARGS[@]}")
fi

exec ros2 launch detector outpost_test.launch.py "${LAUNCH_ARGS[@]}"
