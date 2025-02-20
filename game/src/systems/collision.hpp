#pragma once

#include "../components.hpp"
#include "../core.hpp"

namespace Systems::Collision
{
inline void cleanup(ComponentManager &cm)
{
}

inline bool checkFriendlyFire(ComponentManager &cm, auto &projectileComps, auto &hiveAiComps)
{
    if (!projectileComps || !hiveAiComps)
        return false;

    using Movement = decltype(ProjectileComponent::movement);
    auto &movement = projectileComps.peek(&ProjectileComponent::movement);
    return hiveAiComps && movement == Movement::DOWN;
}

// Check for collisions and assign damage events and/or powerup events if no friendly fire is detected
inline void handleCollisions(ComponentManager &cm)
{
    auto [collisionCheckEventSet] = cm.getAll<CollisionCheckEvent>();
    collisionCheckEventSet.each([&](EId eId1, auto &checkEvents) {
        auto [projectile1, hiveAiComps1] = cm.get<ProjectileComponent, HiveAIComponent>(eId1);
        auto [cX, cY, cW, cH] = checkEvents.peek(&CollisionCheckEvent::bounds).box();
        cm.getGroup<CollidableComponent, PositionComponent>().each(
            [&](EId eId2, auto &collidableComps2, auto &positionComps2) {
                if (eId1 == eId2)
                    return;

                auto [projectile2, hiveAiComps2] = cm.get<ProjectileComponent, HiveAIComponent>(eId2);
                if (checkFriendlyFire(cm, projectile2, hiveAiComps1) ||
                    checkFriendlyFire(cm, projectile1, hiveAiComps2))
                    return;

                auto [pX, pY, pW, pH] = positionComps2.peek(&PositionComponent::bounds).box();
                bool isX = (cX >= pX && cX <= pW) || (cW >= pX && cW <= pW);
                bool isY = (cY >= pY && cY <= pH) || (cH >= pY && cH <= pH);
                if (!isX || !isY)
                    return;

                if (cm.contains<PowerupComponent>(eId2))
                {
                    cm.add<PowerupEvent>(eId1);
                    cm.add<DamageEvent>(eId2, eId1);
                }

                EId dealer1 = projectile1 ? projectile1.peek(&ProjectileComponent::shooterId) : eId1;
                EId dealer2 = projectile2 ? projectile2.peek(&ProjectileComponent::shooterId) : eId2;
                cm.add<DamageEvent>(eId1, dealer2);
                cm.add<DamageEvent>(eId2, dealer1);
            });
    });
}

inline auto update(ComponentManager &cm)
{
    handleCollisions(cm);

    return cleanup;
};
}; // namespace Systems::Collision
