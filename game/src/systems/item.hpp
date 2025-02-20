#pragma once

#include "../components.hpp"
#include "../core.hpp"
#include "../entities.hpp"
#include "../utilities.hpp"
#include <cstdlib>

namespace Systems::Item
{
inline void cleanup(ComponentManager &cm)
{
    Utilities::cleanupEffect<PowerupTimeoutEffect, PowerupEffect>(cm);
}

// Creates a new power in specified intervals IF the player doesn't already have a powerup
inline void spawnPowerup(ComponentManager &cm)
{
    auto [gameId, gameMetaComps] = cm.getUnique<GameMetaComponent>();
    if (cm.contains<PowerupTimeoutEffect>(gameId))
        return;

    auto [playerId, _] = cm.getUnique<PlayerComponent>();
    if (cm.contains<PowerupEffect>(playerId))
        return;

    auto [positionComps] = cm.get<PositionComponent>(playerId);
    auto &playerPos = positionComps.peek(&PositionComponent::bounds);

    auto &screenSize = gameMetaComps.peek(&GameMetaComponent::screen);
    float tileSize = gameMetaComps.peek(&GameMetaComponent::tileSize);

    float randomX = std::rand() % static_cast<int>(screenSize.x - tileSize);
    createPowerup(cm, Bounds{randomX + tileSize, playerPos.position.y, tileSize, tileSize});
    cm.add<PowerupTimeoutEffect>(gameId);
}

inline void processEvents(ComponentManager &cm)
{
    auto &powerupEventIds = cm.getEntityIds<PowerupEvent>();
    for (const auto &id : powerupEventIds)
        cm.add<PowerupEffect>(id);
}

inline auto update(ComponentManager &cm)
{
    processEvents(cm);
    spawnPowerup(cm);

    return cleanup;
}
}; // namespace Systems::Item
