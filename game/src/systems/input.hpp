#pragma once

#include "../components.hpp"
#include "../core.hpp"
#include "../utilities.hpp"

namespace Systems::Input
{
inline void cleanup(ComponentManager &cm)
{
}

// Create player movement based on the input events
inline void movePlayer(ComponentManager &cm)
{
    float dt = Utilities::getDeltaTime(cm);
    auto [playerInputEventSet] = cm.getAll<PlayerInputEvent>();

    playerInputEventSet.each([&](EId eId, auto &playerInputEvents) {
        bool isDeactivated = cm.contains<DeactivatedComponent>(eId);

        auto [movementComps] = cm.get<MovementComponent>(eId);
        auto &speeds = movementComps.peek(&MovementComponent::speeds);
        float baseSpeed = speeds.x * dt;

        playerInputEvents.inspect([&](const PlayerInputEvent &inputEvent) {
            using Actions = decltype(PlayerInputEvent::action);
            switch (inputEvent.action)
            {
            case Actions::SHOOT:
                if (isDeactivated)
                    return;

                cm.add<AttackEvent>(eId, 3);
                break;
            case Actions::QUIT: {
                auto [gameId, _] = cm.getUnique<GameComponent>();
                cm.add<GameEvent>(gameId, GameEvents::QUIT);
                break;
            }
            default:
                break;
            }

            if (isDeactivated)
                return;

            using Movements = decltype(PlayerInputEvent::movement);
            switch (inputEvent.movement)
            {
            case Movements::LEFT:
                cm.add<MovementEvent>(eId, Vector2{-1 * baseSpeed, 0});
                break;
            case Movements::RIGHT:
                cm.add<MovementEvent>(eId, Vector2{baseSpeed, 0});
                break;
            default:
                break;
            }
        });
    });
}

inline auto update(ComponentManager &cm)
{
    movePlayer(cm);

    return cleanup;
};
}; // namespace Systems::Input
