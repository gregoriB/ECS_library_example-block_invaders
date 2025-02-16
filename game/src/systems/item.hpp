#pragma once

#include "../components.hpp"
#include "../core.hpp"
#include "../entities.hpp"
#include "../utilities.hpp"
#include <cstdlib>

namespace Systems::Item
{
inline void cleanup(CM &cm)
{
    Utilities::cleanupEffect<PowerupTimeoutEffect, PowerupEffect>(cm);
}

inline void spawnPowerup(CM &cm)
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

inline void processEvents(CM &cm)
{
    auto &powerupEventIds = cm.getEntityIds<PowerupEvent>();
    for (const auto &id : powerupEventIds)
        cm.add<PowerupEffect>(id);
}

inline auto update(CM &cm)
{
    processEvents(cm);
    spawnPowerup(cm);

    return cleanup;
}
}; // namespace Systems::Item
