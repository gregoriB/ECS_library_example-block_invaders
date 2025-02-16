#pragma once

#include "../components.hpp"
#include "../core.hpp"
#include "../utilities.hpp"

namespace Systems::Movement
{
inline void cleanup(CM &cm)
{
    Utilities::cleanupEffect<MovementEffect>(cm);
}

inline auto checkOutOfBounds(const Bounds &countainer, const Bounds &subject)
{
    auto [cX, cY, cW, cH] = countainer.box();
    auto [sX, sY, sW, sH] = subject.box();

    return sX <= cX || sY <= cY || sW >= cW || sH >= cH;
}

inline Bounds calculateNewBounds(auto &movementEvents, const PositionComponent &positionComp)
{
    MovementEvent reduced = movementEvents.reduce([&](MovementEvent &acc, const MovementEvent &current) {
        acc.coords.x += current.coords.x;
        acc.coords.y += current.coords.y;
    });

    auto [rX, rY] = reduced.coords;
    auto [pX, pY, pW, pH] = positionComp.bounds.get();

    return Bounds{pX + rX, pY + rY, pW, pH};
}

inline void applyMovementEffects(CM &cm)
{
    auto dt = Utilities::getDeltaTime(cm);
    cm.getGroup<MovementEffect, MovementComponent, PositionComponent>().each(
        [&](EId eId, auto &movementEffects, auto &movementComps, auto &positionComps) {
            movementEffects.inspect([&](const MovementEffect &movementEffect) {
                auto &speeds = movementComps.peek(&MovementComponent::speeds);
                auto &position = positionComps.peek(&PositionComponent::bounds).position;
                auto &targetPos = movementEffect.trajectory;

                // clang-format off
                Vector2 diff{position.x - targetPos.x, position.y - targetPos.y};
                Vector2 directions{
                    diff.x < 0 ? 1.0f : diff.x > 0 ? -1.0f : 0,
                    diff.y < 0 ? 1.0f : diff.y > 0 ? -1.0f : 0,
                };
                // clang-format on

                auto xMove = speeds.x * dt;
                auto yMove = speeds.y * dt;
                cm.add<MovementEvent>(eId, Vector2{xMove * directions.x, yMove * directions.y});
            });
        });
}

inline void updateOtherMovement(CM &cm)
{
    auto [movementEventSet] = cm.getAll<MovementEvent>();
    movementEventSet.each([&](EId eId, auto &movementEvents) {
        auto [positionComps] = cm.get<PositionComponent>(eId);
        positionComps.inspect([&](const PositionComponent &positionComp) {
            auto [gameId, gameComps] = cm.getUnique<GameComponent>();
            auto &gameBounds = gameComps.peek(&GameComponent::bounds);
            auto [gX, gY, gW, gH] = gameBounds.box();

            auto newBounds = calculateNewBounds(movementEvents, positionComp);
            auto [newX, newY, newW, newH] = newBounds.box();
            auto [w, h] = positionComp.bounds.size;

            // TODO Task : Move boundary checks to collision system
            if (checkOutOfBounds(gameBounds, newBounds))
            {
                if (!cm.contains<ProjectileComponent>(eId) && !cm.contains<UFOAIComponent>(eId))
                    return;

                if (newH < gY || newY > gH || newW < gX || newX > gW)
                {
                    cm.add<DeathEvent>(eId);
                    return;
                }
            }

            Vector2 newPos{newX, newY};
            cm.add<CollisionCheckEvent>(eId, Bounds{newPos, Vector2{w, h}});
            cm.add<PositionEvent>(eId, std::move(newPos));
        });
    });
}

inline auto update(CM &cm)
{
    applyMovementEffects(cm);
    updateOtherMovement(cm);

    return cleanup;
};
}; // namespace Systems::Movement
