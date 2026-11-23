#include "MK1AI_Core.h"
#include <cassert>
int main() {
    mk1ai::AdaptiveCore ai;
    ai.AddComboRoute({"scorp_300", "Scorpion", "verified-route-placeholder", 320.0f,
                      0.0f, 500.0f, 1.0f, 0.0f, true, false, false, true});
    mk1ai::CombatSnapshot s;
    s.distance = 100.0f;
    s.meter = 2.0f;
    s.situation = mk1ai::Situation::Punish;
    auto choice = ai.ChooseCombo(s, "Scorpion");
    assert(choice.found && choice.routeId == "scorp_300");
    ai.RecordComboAttempt("scorp_300", true, 327.0f);
    auto learned = ai.ChooseCombo(s, "Scorpion");
    assert(learned.found && learned.estimatedDamage == 327.0f);
    ai.ObserveOpponentAction(mk1ai::ActionKind::Jump);
    ai.ObserveOpponentAction(mk1ai::ActionKind::Jump);
    assert(ai.MostFrequentOpponentAction() == mk1ai::ActionKind::Jump);
}
