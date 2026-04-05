#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "${ROOT_DIR}"

echo "[1/3] Kill old auto-aim / decision related processes..."
pkill -f "detector_node|tracker_solver|predictor|behavior_tree|gimbal_driver|mapper_node|autoaim|sentry_all|ros2 launch" || true
pkill -f "ros2_ly_ws_sentary.*python3" || true
pkill -f "ros2_ly_ws_sentary.*launch" || true

sleep 1

echo "[2/3] Source workspace..."
if [[ -f "${ROOT_DIR}/install/setup.bash" ]]; then
  set +u
  source "${ROOT_DIR}/install/setup.bash"
  set -u
fi

echo "[3/3] Restart all (gated + league)..."
exec "${ROOT_DIR}/scripts/start.sh" gated --mode league --no-prompt
