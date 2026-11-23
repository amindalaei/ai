#include "MK1AI_Core.h"
#include <algorithm>
#include <cmath>
#include <limits>

namespace mk1ai {

AdaptiveCore::AdaptiveCore(size_t historyLimit) : historyLimit_(historyLimit ? historyLimit : 1) {}

void AdaptiveCore::ObserveOpponentAction(ActionKind action) {
    if (action == ActionKind::Unknown) return;
    ++actionStats_[action].count;
    recentActions_.push_back(action);
    while (recentActions_.size() > historyLimit_) recentActions_.pop_front();
}

void AdaptiveCore::ObservePunishedAction(ActionKind action) {
    if (action == ActionKind::Unknown) return;
    ++actionStats_[action].punished;
}

float AdaptiveCore::OpponentActionRate(ActionKind action) const {
    uint32_t total = 0;
    for (const auto& item : actionStats_) total += item.second.count;
    if (!total) return 0.0f;
    auto it = actionStats_.find(action);
    return it == actionStats_.end() ? 0.0f : float(it->second.count) / float(total);
}

float AdaptiveCore::OpponentPunishedRate(ActionKind action) const {
    auto it = actionStats_.find(action);
    if (it == actionStats_.end() || !it->second.count) return 0.0f;
    return float(it->second.punished) / float(it->second.count);
}

ActionKind AdaptiveCore::MostFrequentOpponentAction() const {
    ActionKind best = ActionKind::Unknown;
    uint32_t bestCount = 0;
    for (const auto& item : actionStats_) {
        if (item.second.count > bestCount) {
            best = item.first;
            bestCount = item.second.count;
        }
    }
    return best;
}

void AdaptiveCore::AddComboRoute(const ComboRoute& route) {
    auto it = std::find_if(routes_.begin(), routes_.end(), [&](const ComboRoute& r) { return r.id == route.id; });
    if (it == routes_.end()) routes_.push_back(route);
    else *it = route;
    routeStats_.try_emplace(route.id);
}

void AdaptiveCore::RecordComboAttempt(const std::string& routeId, bool success, float observedDamage) {
    auto& stats = routeStats_[routeId];
    ++stats.attempts;
    if (success) {
        ++stats.successes;
        stats.totalObservedDamage += std::max(0.0f, observedDamage);
        stats.bestObservedDamage = std::max(stats.bestObservedDamage, observedDamage);
    } else {
        ++stats.failures;
    }
}

ComboChoice AdaptiveCore::ChooseCombo(const CombatSnapshot& state, const std::string& character) const {
    ComboChoice best;
    best.score = -std::numeric_limits<float>::infinity();

    for (const auto& route : routes_) {
        if (!route.enabled || route.character != character) continue;
        if (state.distance < route.minDistance || state.distance > route.maxDistance) continue;
        if (state.meter < route.meterCost) continue;
        if (route.requiresPunish && state.situation != Situation::Punish) continue;
        if (route.requiresHitConfirm && state.situation != Situation::HitConfirm) continue;
        if (route.requiresAirborne && !state.opponentAirborne) continue;

        auto it = routeStats_.find(route.id);
        const RouteStats stats = it == routeStats_.end() ? RouteStats{} : it->second;
        // Beta-style smoothing avoids treating one success/failure as certainty.
        const float successRate = float(stats.successes + 2) / float(stats.attempts + 4);
        const float observedDamage = stats.successes
            ? stats.totalObservedDamage / float(stats.successes)
            : route.expectedDamage;
        const float damage = observedDamage > 0.0f ? observedDamage : route.expectedDamage;

        // Damage matters, but reliability and resource cost matter too.
        float score = damage * (0.55f + 0.45f * successRate);
        score -= route.meterCost * 0.35f;
        score -= route.kameoCost * 0.20f;
        if (route.requiresPunish && state.situation == Situation::Punish) score += 12.0f;
        if (route.requiresHitConfirm && state.situation == Situation::HitConfirm) score += 8.0f;
        if (damage >= 300.0f) score += 5.0f; // preference, never a forced pick

        if (score > best.score) {
            best.found = true;
            best.routeId = route.id;
            best.score = score;
            best.estimatedSuccessRate = successRate;
            best.estimatedDamage = damage;
        }
    }
    return best;
}

void AdaptiveCore::RecordRoundResult(bool aiWon, float aiHealthRemaining, float opponentHealthRemaining) {
    ++rounds_;
    if (aiWon) ++wins_;
    const float denom = std::max(1.0f, aiHealthRemaining + opponentHealthRemaining);
    performanceSum_ += (aiHealthRemaining - opponentHealthRemaining) / denom;
}

float AdaptiveCore::ChallengeAdjustment() const {
    if (!rounds_) return 0.0f;
    const float winRate = float(wins_) / float(rounds_);
    const float avgMargin = performanceSum_ / float(rounds_);
    // Bounded signal in [-1, +1]; the host decides how to map this to difficulty.
    return std::max(-1.0f, std::min(1.0f, (winRate - 0.5f) * 1.2f + avgMargin * 0.5f));
}

} // namespace mk1ai
