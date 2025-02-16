#pragma once

#include "../components.hpp"
#include "../core.hpp"

namespace Systems::Death
{
inline void cleanup(CM &cm)
{
    auto &deadIds = cm.getEntityIds<DeathComponent>();
    for (const auto &id : deadIds)
        cm.remove(id);
}

inline auto update(CM &cm)
{
    auto [deathSet] = cm.getAll<DeathEvent>();
    deathSet.each([&](EId eId, ECS::ComponentsWrapper<DeathEvent> &deathEvents) {
        auto [playerId, _] = cm.getUnique<PlayerComponent>();
        if (eId == playerId)
        {
            deathEvents.inspect(
                [&](const DeathEvent &deathEvent) { PRINT("PLAYER KILLED BY ", deathEvent.killedBy) });
            cm.add<PlayerEvent>(eId, PlayerEvents::DEATH);
            return;
        }

        auto [startTriggerId, _] = cm.getUnique<StartGameTriggerComponent>();
        if (eId == startTriggerId)
        {
            cm.add<GameEvent>(eId, GameEvents::NEXT_STAGE);
        }

        deathEvents.inspect([&](const DeathEvent &deathEvent) {
            if (!cm.contains<PointsComponent>(eId))
                return;

            cm.add<ScoreEvent>(deathEvent.killedBy, eId);
        });

        cm.add<DeathComponent>(eId);
    });

    return cleanup;
};
}; // namespace Systems::Death
