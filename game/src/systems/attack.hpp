#pragma once

#include "../components.hpp"
#include "../core.hpp"
#include "../entities.hpp"
#include "../utilities.hpp"

namespace Systems::Attack
{
inline void cleanup(ComponentManager &cm)
{
    Utilities::cleanupEffect<AttackEffect>(cm);
}

inline void removeExpiredAttackAffects(ComponentManager &cm)
{
    auto [attackEffectSet] = cm.getAll<AttackEffect>();
    attackEffectSet.each([&](EId eId, auto &attackEffects) {
        // clang-format off
        attackEffects
            .filter([&](const AttackEffect &effect) { 
                // If the projectile entity no longer exists, the attack effect
                // needs to be cleaned up. This limits attacking to a single shot
                // on the screen at a time
                auto [projectileComps] = cm.get<ProjectileComponent>(effect.attackId);
                return !projectileComps || effect.timer->hasElapsed();
            })
            .mutate([&](auto &effect) { effect.cleanup = true; });
        // clang-format on
    });
}

// Take attack events and convert those into attack, then create attack effects which hold attack info
inline void processAttacks(ComponentManager &cm)
{
    auto [attackEventSet] = cm.getAll<AttackEvent>();
    attackEventSet.each([&](EId eId, auto &attackEvents) {
        attackEvents.inspect([&](const AttackEvent &attackEvent) {
            auto [attackEffects] = cm.get<AttackEffect>(eId);
            if (attackEffects)
                return;

            auto [positionComps, attackComps] = cm.get<PositionComponent, AttackComponent>(eId);
            auto &bounds = positionComps.peek(&PositionComponent::bounds);
            auto direction = attackComps.peek(&AttackComponent::direction);

            using Movements = decltype(direction);
            EntityId projectileId;
            switch (direction)
            {
            case (Movements::UP):
                projectileId = createUpwardProjectile(cm, eId, bounds);
                break;
            case (Movements::DOWN):
                projectileId = createDownwardProjectile(cm, eId, bounds);
                break;
            default:
                return;
            }

            cm.add<AttackEffect>(eId, projectileId, attackEvent.timeout);
        });
    });
}

inline auto update(ComponentManager &cm)
{
    processAttacks(cm);
    removeExpiredAttackAffects(cm);

    return cleanup;
};
} // namespace Systems::Attack
