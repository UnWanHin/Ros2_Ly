#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
SCRIPT_NAME="$(basename "$0")"

OUTPUT_MODE="screen"
USE_GIMBAL="true"
USE_CALIB="true"
TEAM_RED="true"
DEBUG_TEAM_BLUE="false"
WEB_SHOW="true"
DRAW_IMAGE="true"
CONFIG_FILE=""
PARAM_MANAGER_CMD=""

usage() {
  cat <<EOF
Usage:
  ${SCRIPT_NAME} [options] [-- <extra ros2 launch args...>]

Options:
  --config <path>             YAML config file (optional, default launch value).
  --output <screen|log>       Node output mode. Default: screen.
  --use-gimbal <true|false>   Start gimbal_driver. Default: true.
  --use-calib <true|false>    Start shooting_table_calib node. Default: true.
  --team <red|blue>           Team color for calib node. Default: red.
  --web-show <true|false>     Enable web show. Default: true.
  --draw-image <true|false>   Enable image draw. Default: true.
  --param-manager-cmd "<cmd>" Optional command to start param manager in background.
  -h, --help                  Show help.

Examples:
  ./${SCRIPT_NAME}
  ./${SCRIPT_NAME} --team blue --web-show false
  ./${SCRIPT_NAME} --config /path/to/auto_aim_config.yaml
  ./${SCRIPT_NAME} --param-manager-cmd "bash /home/liu/workspace/scripts/param_manager.bash"
EOF
}

EXTRA_LAUNCH_ARGS=()
while [[ $# -gt 0 ]]; do
  case "$1" in
    --config)
      CONFIG_FILE="$2"
      shift 2
      ;;
    --output)
      OUTPUT_MODE="$2"
      shift 2
      ;;
    --use-gimbal)
      USE_GIMBAL="$2"
      shift 2
      ;;
    --use-calib)
      USE_CALIB="$2"
      shift 2
      ;;
    --team)
      case "$2" in
        red|RED)
          TEAM_RED="true"
          DEBUG_TEAM_BLUE="false"
          ;;
        blue|BLUE)
          TEAM_RED="false"
          DEBUG_TEAM_BLUE="true"
          ;;
        *)
          echo "[ERROR] --team only supports red|blue" >&2
          exit 2
          ;;
      esac
      shift 2
      ;;
    --web-show)
      WEB_SHOW="$2"
      shift 2
      ;;
    --draw-image)
      DRAW_IMAGE="$2"
      shift 2
      ;;
    --param-manager-cmd)
      PARAM_MANAGER_CMD="$2"
      shift 2
      ;;
    -h|--help)
      usage
      exit 0
      ;;
    --)
      shift
      EXTRA_LAUNCH_ARGS=("$@")
      break
      ;;
    *)
      echo "[ERROR] Unknown argument: $1" >&2
      usage >&2
      exit 2
      ;;
  esac
done

if [[ ! -f "${ROOT_DIR}/install/setup.bash" ]]; then
  echo "[ERROR] ${ROOT_DIR}/install/setup.bash not found. Please run colcon build first." >&2
  exit 1
fi

# `install/setup.bash` may reference unset vars (for example COLCON_TRACE).
# Define it as empty by default so nounset is satisfied without enabling trace spam.
: "${COLCON_TRACE:=}"
export COLCON_TRACE
set +u
source "${ROOT_DIR}/install/setup.bash"
set -u

PARAM_MANAGER_PID=""
cleanup() {
  if [[ -n "${PARAM_MANAGER_PID}" ]] && kill -0 "${PARAM_MANAGER_PID}" 2>/dev/null; then
    kill "${PARAM_MANAGER_PID}" 2>/dev/null || true
    wait "${PARAM_MANAGER_PID}" 2>/dev/null || true
  fi
}
trap cleanup EXIT INT TERM

if [[ -n "${PARAM_MANAGER_CMD}" ]]; then
  echo "[INFO] Starting param manager: ${PARAM_MANAGER_CMD}"
  bash -lc "${PARAM_MANAGER_CMD}" &
  PARAM_MANAGER_PID=$!
fi

LAUNCH_ARGS=(
  "output:=${OUTPUT_MODE}"
  "use_gimbal:=${USE_GIMBAL}"
  "use_calib:=${USE_CALIB}"
  "team_red:=${TEAM_RED}"
  "debug_team_blue:=${DEBUG_TEAM_BLUE}"
  "web_show:=${WEB_SHOW}"
  "draw_image:=${DRAW_IMAGE}"
)

if [[ -n "${CONFIG_FILE}" ]]; then
  if [[ ! -f "${CONFIG_FILE}" ]]; then
    echo "[ERROR] config file not found: ${CONFIG_FILE}" >&2
    exit 1
  fi
  LAUNCH_ARGS+=("config_file:=${CONFIG_FILE}")
fi

echo "Starting Shooting Table Calibration System (ROS2)..."
echo "===================================================="
echo "[INFO] output=${OUTPUT_MODE} use_gimbal=${USE_GIMBAL} use_calib=${USE_CALIB} team_red=${TEAM_RED} debug_team_blue=${DEBUG_TEAM_BLUE} web_show=${WEB_SHOW} draw_image=${DRAW_IMAGE}"
if [[ -n "${CONFIG_FILE}" ]]; then
  echo "[INFO] config_file=${CONFIG_FILE}"
fi

cd "${ROOT_DIR}"
exec ros2 launch shooting_table_calib shooting_table_calib.launch.py \
  "${LAUNCH_ARGS[@]}" \
  "${EXTRA_LAUNCH_ARGS[@]}"
