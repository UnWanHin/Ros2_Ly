#include "../include/Application.hpp"

#include <algorithm>
#include <cctype>

namespace {

std::string NormalizeProfile(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(),
        [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return value;
}

BehaviorTree::CompetitionProfile ParseCompetitionProfile(const std::string& value) {
    const auto normalized = NormalizeProfile(value);
    if (normalized == "league") {
        return BehaviorTree::CompetitionProfile::League;
    }
    return BehaviorTree::CompetitionProfile::Regional;
}

bool IsValidBaseGoal(const std::uint8_t goal_id) {
    return goal_id <= LangYa::OccupyArea.ID;
}

}  // namespace


namespace LangYa {

    // 自定义 from_json 函数，用于自动转换 JSON 到结构体
    void from_json(const json& j, AimDebug& ad) {
        ad.StopFire = j.value("StopFire", ad.StopFire);
        ad.StopRotate = j.value("StopRotate", ad.StopRotate);
        ad.StopScan = j.value("StopScan", ad.StopScan);
        ad.HitOutpost = j.value("HitOutpost", ad.HitOutpost);
        ad.HitBuff = j.value("HitBuff", ad.HitBuff);
        ad.HitCar = j.value("HitCar", ad.HitCar);
    }

    void from_json(const json& j, Rate& r) {
        r.FireRate = j.value("FireRate", r.FireRate);
        r.TreeTickRate = j.value("TreeTickRate", r.TreeTickRate);
        r.NaviCommandRate = j.value("NaviCommandRate", r.NaviCommandRate);
    }
    void from_json(const json& j, GameStrategy& gs) {
        gs.HitBuff = j.value("HitBuff", gs.HitBuff);
        gs.HitOutpost = j.value("HitOutpost", gs.HitOutpost);
        gs.TestNavi = j.value("TestNavi", gs.TestNavi);
        gs.HitSentry = j.value("HitSentry", gs.HitSentry);
        gs.Protected = j.value("Protected", gs.Protected);
    }
    void from_json(const json& j, NaviSetting& ns) {
        ns.UseXY = j.value("UseXY", ns.UseXY);
    }

    void from_json(const json& j, LeagueStrategySetting& ls) {
        ls.UseHealthRecovery = j.value("UseHealthRecovery", ls.UseHealthRecovery);
        ls.HealthRecoveryThreshold = j.value("HealthRecoveryThreshold", ls.HealthRecoveryThreshold);
        ls.UseAmmoRecovery = j.value("UseAmmoRecovery", ls.UseAmmoRecovery);
        ls.AmmoRecoveryThreshold = j.value("AmmoRecoveryThreshold", ls.AmmoRecoveryThreshold);
        ls.MainGoal = j.value("MainGoal", ls.MainGoal);
        ls.GoalHoldSec = j.value("GoalHoldSec", ls.GoalHoldSec);
        if (j.contains("PatrolGoals")) {
            j.at("PatrolGoals").get_to(ls.PatrolGoals);
        }
    }

    void from_json(const json& j, PostureSetting& ps) {
        ps.Enable = j.value("Enable", ps.Enable);
        ps.SwitchCooldownSec = j.value("SwitchCooldownSec", ps.SwitchCooldownSec);
        ps.MaxSinglePostureSec = j.value("MaxSinglePostureSec", ps.MaxSinglePostureSec);
        ps.EarlyRotateSec = j.value("EarlyRotateSec", ps.EarlyRotateSec);
        ps.MinHoldSec = j.value("MinHoldSec", ps.MinHoldSec);
        ps.PendingAckTimeoutMs = j.value("PendingAckTimeoutMs", ps.PendingAckTimeoutMs);
        ps.RetryIntervalMs = j.value("RetryIntervalMs", ps.RetryIntervalMs);
        ps.MaxRetryCount = j.value("MaxRetryCount", ps.MaxRetryCount);
        ps.OptimisticAck = j.value("OptimisticAck", ps.OptimisticAck);
        ps.TargetKeepMs = j.value("TargetKeepMs", ps.TargetKeepMs);
        ps.DamageKeepSec = j.value("DamageKeepSec", ps.DamageKeepSec);
        ps.LowHealthThreshold = j.value("LowHealthThreshold", ps.LowHealthThreshold);
        ps.VeryLowHealthThreshold = j.value("VeryLowHealthThreshold", ps.VeryLowHealthThreshold);
        ps.LowAmmoThreshold = j.value("LowAmmoThreshold", ps.LowAmmoThreshold);
        ps.ScoreHysteresis = j.value("ScoreHysteresis", ps.ScoreHysteresis);
    }

    void from_json(const json& j, Config& c) {
        if (j.contains("AimDebug")) {
            j.at("AimDebug").get_to(c.AimDebugSettings);
        }
        if (j.contains("Rate")) {
            j.at("Rate").get_to(c.RateSettings);
        }
        if (j.contains("GameStrategy")) {
            j.at("GameStrategy").get_to(c.GameStrategySettings);
        }
        c.ScanCounter = j.value("ScanCounter", c.ScanCounter);
        if (j.contains("NaviSetting")) {
            j.at("NaviSetting").get_to(c.NaviSettings);
        }
        if (j.contains("LeagueStrategy")) {
            j.at("LeagueStrategy").get_to(c.LeagueStrategySettings);
        }
        if (j.contains("Posture")) {
            j.at("Posture").get_to(c.PostureSettings);
        }
        c.CompetitionProfile = j.value("CompetitionProfile", c.CompetitionProfile);
    }
}

namespace BehaviorTree {
    using namespace LangYa;
    using json = nlohmann::json;

    bool Application::ConfigurationInit() {
        std::ifstream ifs(config_file_);
        if (!ifs.is_open()) {
            LoggerPtr->Error("Failed to open config.json, file location: {}", config_file_);
            return false;
        }
        LoggerPtr->Info("Open config.json, file location: {}", config_file_);

        // 解析 JSON 文件
        json j;
        ifs >> j;
        config = j.get<Config>();
        LoggerPtr->Debug("------ AimDebug ------");
        LoggerPtr->Debug("StopFire: {}", config.AimDebugSettings.StopFire);
        LoggerPtr->Debug("StopRotate: {}", config.AimDebugSettings.StopRotate);
        LoggerPtr->Debug("StopScan: {}", config.AimDebugSettings.StopScan);
        LoggerPtr->Debug("HitOutpost: {}", config.AimDebugSettings.HitOutpost);
        LoggerPtr->Debug("HitBuff: {}", config.AimDebugSettings.HitBuff);
        LoggerPtr->Debug("HitCar: {}", config.AimDebugSettings.HitCar);
        LoggerPtr->Debug("------ Rate ------");
        LoggerPtr->Debug("FireRate: {}", config.RateSettings.FireRate);
        LoggerPtr->Debug("TickRate: {}", config.RateSettings.TreeTickRate);
        LoggerPtr->Debug("NaviCommandRate: {}", config.RateSettings.NaviCommandRate);
        LoggerPtr->Debug("------ GameStrategy ------");
        LoggerPtr->Debug("ScanCounter: {}", config.ScanCounter);
        LoggerPtr->Debug("HitOutpost: {}", config.GameStrategySettings.HitOutpost);
        LoggerPtr->Debug("HitBuff: {}", config.GameStrategySettings.HitBuff);
        LoggerPtr->Debug("HitSentry: {}", config.GameStrategySettings.HitSentry);
        LoggerPtr->Debug("TestNavi: {}", config.GameStrategySettings.TestNavi);
        LoggerPtr->Debug("Protected: {}", config.GameStrategySettings.Protected);
        LoggerPtr->Debug("------ NaviSetting ------");
        LoggerPtr->Debug("UseXY: {}", config.NaviSettings.UseXY);
        LoggerPtr->Debug("------ LeagueStrategy ------");
        LoggerPtr->Debug("UseHealthRecovery: {}", config.LeagueStrategySettings.UseHealthRecovery);
        LoggerPtr->Debug("HealthRecoveryThreshold: {}", config.LeagueStrategySettings.HealthRecoveryThreshold);
        LoggerPtr->Debug("UseAmmoRecovery: {}", config.LeagueStrategySettings.UseAmmoRecovery);
        LoggerPtr->Debug("AmmoRecoveryThreshold: {}", config.LeagueStrategySettings.AmmoRecoveryThreshold);
        LoggerPtr->Debug("MainGoal: {}", static_cast<int>(config.LeagueStrategySettings.MainGoal));
        LoggerPtr->Debug("GoalHoldSec: {}", config.LeagueStrategySettings.GoalHoldSec);
        LoggerPtr->Debug("------ PostureSetting ------");
        LoggerPtr->Debug("Enable: {}", config.PostureSettings.Enable);
        LoggerPtr->Debug("SwitchCooldownSec: {}", config.PostureSettings.SwitchCooldownSec);
        LoggerPtr->Debug("MaxSinglePostureSec: {}", config.PostureSettings.MaxSinglePostureSec);
        LoggerPtr->Debug("EarlyRotateSec: {}", config.PostureSettings.EarlyRotateSec);
        LoggerPtr->Debug("MinHoldSec: {}", config.PostureSettings.MinHoldSec);
        LoggerPtr->Debug("PendingAckTimeoutMs: {}", config.PostureSettings.PendingAckTimeoutMs);
        LoggerPtr->Debug("RetryIntervalMs: {}", config.PostureSettings.RetryIntervalMs);
        LoggerPtr->Debug("MaxRetryCount: {}", config.PostureSettings.MaxRetryCount);
        LoggerPtr->Debug("OptimisticAck: {}", config.PostureSettings.OptimisticAck);
        LoggerPtr->Debug("TargetKeepMs: {}", config.PostureSettings.TargetKeepMs);
        LoggerPtr->Debug("DamageKeepSec: {}", config.PostureSettings.DamageKeepSec);
        LoggerPtr->Debug("LowHealthThreshold: {}", config.PostureSettings.LowHealthThreshold);
        LoggerPtr->Debug("VeryLowHealthThreshold: {}", config.PostureSettings.VeryLowHealthThreshold);
        LoggerPtr->Debug("LowAmmoThreshold: {}", config.PostureSettings.LowAmmoThreshold);
        LoggerPtr->Debug("ScoreHysteresis: {}", config.PostureSettings.ScoreHysteresis);
        LoggerPtr->Debug("------ End ------");
        LoggerPtr->Debug("Configuration completed.");
        fireRateClock.reset(config.RateSettings.FireRate);
        treeTickRateClock.reset(config.RateSettings.TreeTickRate);
        naviCommandRateClock.reset(config.RateSettings.NaviCommandRate);

        if (config.LeagueStrategySettings.GoalHoldSec <= 0) {
            LoggerPtr->Warning("Invalid LeagueStrategy.GoalHoldSec={}, fallback to 15.", config.LeagueStrategySettings.GoalHoldSec);
            config.LeagueStrategySettings.GoalHoldSec = 15;
        }
        if (!IsValidBaseGoal(config.LeagueStrategySettings.MainGoal)) {
            LoggerPtr->Warning(
                "Invalid LeagueStrategy.MainGoal={}, fallback to OccupyArea.",
                static_cast<int>(config.LeagueStrategySettings.MainGoal));
            config.LeagueStrategySettings.MainGoal = LangYa::OccupyArea.ID;
        }
        std::vector<std::uint8_t> sanitized_patrol_goals;
        sanitized_patrol_goals.reserve(config.LeagueStrategySettings.PatrolGoals.size());
        for (const auto goal_id : config.LeagueStrategySettings.PatrolGoals) {
            if (!IsValidBaseGoal(goal_id)) {
                LoggerPtr->Warning("Ignore invalid LeagueStrategy.PatrolGoals item={}.", static_cast<int>(goal_id));
                continue;
            }
            if (std::find(sanitized_patrol_goals.begin(), sanitized_patrol_goals.end(), goal_id) == sanitized_patrol_goals.end()) {
                sanitized_patrol_goals.push_back(goal_id);
            }
        }
        config.LeagueStrategySettings.PatrolGoals = std::move(sanitized_patrol_goals);

        const std::string config_profile = NormalizeProfile(config.CompetitionProfile);
        const std::string effective_profile = competitionProfileOverride_.empty()
            ? config_profile
            : NormalizeProfile(competitionProfileOverride_);
        if (!competitionProfileOverride_.empty() && !config_profile.empty() &&
            effective_profile != config_profile) {
            LoggerPtr->Warning(
                "competition_profile override '{}' replaces config profile '{}'.",
                competitionProfileOverride_, config.CompetitionProfile);
        }
        const auto selected_profile = effective_profile.empty() ? std::string("regional") : effective_profile;
        config.CompetitionProfile = selected_profile;
        competitionProfile_ = ParseCompetitionProfile(config.CompetitionProfile);
        LoggerPtr->Info("Effective CompetitionProfile: {}", CompetitionProfileToString(competitionProfile_));
        if (competitionProfile_ == CompetitionProfile::League && config.NaviSettings.UseXY) {
            LoggerPtr->Warning("League profile is using UseXY=true. Goal-ID mode is recommended for league.");
        }

        // 与旧逻辑保持一致：SetPositionRepeat 的初始优先级
        if (competitionProfile_ == CompetitionProfile::League) {
            strategyMode_ = StrategyMode::LeagueSimple;
        } else if (config.GameStrategySettings.HitSentry) {
            strategyMode_ = StrategyMode::HitSentry;
        } else if (config.GameStrategySettings.TestNavi) {
            strategyMode_ = StrategyMode::NaviTest;
        } else if (config.GameStrategySettings.Protected) {
            strategyMode_ = StrategyMode::Protected;
        } else {
            strategyMode_ = StrategyMode::HitHero;
        }
        LoggerPtr->Info("Initial StrategyMode: {}", StrategyModeToString(strategyMode_));

        postureManager_.Configure(config.PostureSettings);
        postureManager_.Reset(std::chrono::steady_clock::now(), SentryPosture::Move);
        leaguePatrolGoalIndex_ = 0;
        leaguePatrolGoalInitialized_ = false;

        return true;
    }
}
