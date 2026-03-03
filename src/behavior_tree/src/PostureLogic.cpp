#include "../include/Application.hpp"

namespace BehaviorTree {

SentryPosture Application::SelectDesiredPosture(const bool has_target) const {
    if (!config.PostureSettings.Enable) {
        return SentryPosture::Unknown;
    }

    const bool low_energy =
        teamBuff.RemainingEnergy == 0b10000 ||
        teamBuff.RemainingEnergy == 0b00000;

    if (strategyMode_ == StrategyMode::Protected || low_energy) {
        return SentryPosture::Defense;
    }

    if (strategyMode_ == StrategyMode::NaviTest) {
        return SentryPosture::Move;
    }

    if (aimMode == AimMode::RotateScan) {
        return SentryPosture::Move;
    }

    if (has_target) {
        return SentryPosture::Attack;
    }

    if (aimMode == AimMode::Buff || aimMode == AimMode::Outpost) {
        return SentryPosture::Attack;
    }

    return SentryPosture::Move;
}

void Application::UpdatePostureCommand(const bool has_target) {
    postureCommand = 0;
    if (!config.PostureSettings.Enable) return;

    const auto desired = SelectDesiredPosture(has_target);
    const auto now = std::chrono::steady_clock::now();
    const auto decision = postureManager_.Tick(now, desired, postureState);
    postureCommand = decision.Command;

    if (decision.Sent && LoggerPtr) {
        LoggerPtr->Info(
            "[Posture] command={}, desired={}, current={}, reason={}",
            static_cast<int>(postureCommand),
            PostureToString(desired),
            PostureToString(postureManager_.Runtime().Current),
            decision.Reason);
    }
}

}  // namespace BehaviorTree

