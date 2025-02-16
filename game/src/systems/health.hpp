#pragma once

#include "../components.hpp"
#include "../core.hpp"
#include <cstdint>

namespace Systems::Health
{
inline void cleanup(CM &cm)
{
}

inline auto update(CM &cm)
{
    cm.getGroup<HealthEvent, HealthComponent>().each([&](EId eId, auto &healthEvents, auto &healthComps) {
        healthEvents.inspect([&](const HealthEvent &healthEvent) {
            healthComps.mutate([&](HealthComponent &healthComp) {
                healthComp.current += healthEvent.amount;
                if (healthComp.current <= 0)
                    cm.add<DeathEvent>(eId, healthEvent.dealerId);

                if (!cm.contains<ObstacleComponent>(eId))
                    return;

                // Update obstacle color to reflect damage
                auto [spriteComps] = cm.get<SpriteComponent>(eId);
                spriteComps.mutate([&](SpriteComponent &spriteComp) {
                    auto [r, g, b, a] = spriteComp.rgba;
                    uint8_t change = 20;
                    spriteComp.rgba.r -= r >= change ? change : 0;
                    spriteComp.rgba.g -= g >= change ? change : 0;
                    spriteComp.rgba.b -= b >= change ? change : 0;
                    spriteComp.rgba.a -= a >= change ? change : 0;
                });
            });
        });
    });

    return cleanup;
};
}; // namespace Systems::Health
