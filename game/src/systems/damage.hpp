#pragma once

#include "../components.hpp"
#include "../core.hpp"

namespace Systems::Damage
{
inline void cleanup(CM &cm)
{
}

inline auto update(CM &cm)
{
    auto [damageEventSet] = cm.getAll<DamageEvent>();
    damageEventSet.each([&](EId eId, auto &damageEvents) {
        damageEvents.inspect([&](const DamageEvent &damageEvent) {
            auto [damageComps] = cm.get<DamageComponent>(damageEvent.dealerId);
            if (!damageComps)
                return;

            auto &amount = damageComps.peek(&DamageComponent::amount);
            cm.add<HealthEvent>(eId, -1 * amount, damageEvent.dealerId);
        });
    });

    return cleanup;
};
}; // namespace Systems::Damage
