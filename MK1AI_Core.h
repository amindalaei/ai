#pragma once
#include <cstdint>
#include <deque>
#include <string>
#include <unordered_map>
#include <vector>

namespace mk1ai {

enum class ActionKind : uint8_t {
    Unknown, High, Low, Mid, Overhead, Throw, Jump, Backdash, ForwardDash,
    Projectile, Kameo, WakeupAttack, Block, ComboStarter
};

enum class Situation : uint8_t { Neutral, Punish, HitConfirm, Corner, Wakeup, AntiAir };

struct CombatSnapshot {
    float distance = 0.0f;          // Game units; integration layer must calibrate.
    float selfHealth = 1000.0f;
    float opponentHealth = 1000.0f;
    float meter = 0.0f;
    bool opponentAirborne = false;
    bool opponentInRecovery = false;
    bool selfInCorner = false;
    Situation situation = Situation::Neutral;
};

struct ComboRoute {
    std::string id;                // Stable route identifier from the character route catalog.
    std::string character;
    std::string inputSequence;     // Human-readable initially; execution mapping is integration-specific.
    float expectedDamage = 0.0f;
    float minDistance = 0.0f;
    float maxDistance = 99999.0f;
    float meterCost = 0.0f;
    float kameoCost = 0.0f;
    bool requiresPunish = false;
    bool requiresHitConfirm = false;
    bool requiresAirborne = false;
    bool enabled = true;
};

struct RouteStats {
    uint32_t attempts = 0;
    uint32_t successes = 0;
    float totalObservedDamage = 0.0f;
    float bestObservedDamage = 0.0f;
    uint32_t failures = 0;
};

struct ComboChoice {
    bool found = false;
    std::string routeId;
    float score = 0.0f;
    float estimatedSuccessRate = 0.0f;
    float estimatedDamage = 0.0f;
};

struct ActionStats {
    uint32_t count = 0;
    uint32_t punished = 0;
};

class AdaptiveCore {
public:
    explicit AdaptiveCore(size_t historyLimit = 64);

    void ObserveOpponentAction(ActionKind action);
    void ObservePunishedAction(ActionKind action);
    float OpponentActionRate(ActionKind action) const;
    float OpponentPunishedRate(ActionKind action) const;
    ActionKind MostFrequentOpponentAction() const;

    void AddComboRoute(const ComboRoute& route);
    void RecordComboAttempt(const std::string& routeId, bool success, float observedDamage);
    ComboChoice ChooseCombo(const CombatSnapshot& state, const std::string& character) const;
    const std::unordered_map<std::string, RouteStats>& GetRouteStats() const { return routeStats_; }

    // A simple performance signal for later adaptive-difficulty integration.
    void RecordRoundResult(bool aiWon, float aiHealthRemaining, float opponentHealthRemaining);
    float ChallengeAdjustment() const;

private:
    size_t historyLimit_;
    std::deque<ActionKind> recentActions_;
    std::unordered_map<ActionKind, ActionStats> actionStats_;
    std::vector<ComboRoute> routes_;
    std::unordered_map<std::string, RouteStats> routeStats_;
    uint32_t rounds_ = 0;
    uint32_t wins_ = 0;
    float performanceSum_ = 0.0f;
};

} // namespace mk1ai
