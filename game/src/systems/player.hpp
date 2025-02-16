#pragma once

#include "../components.hpp"
#include "../core.hpp"

namespace Systems::Player
{
inline void cleanup(CM &cm)
{
}

inline auto update(CM &cm)
{
    auto [playerId, playerComps] = cm.getUnique<PlayerComponent>();
    auto [playerEventSet] = cm.getAll<PlayerEvent>();
    playerEventSet.each([&](EId eId, auto &playerEvents) {
        playerEvents.inspect([&](const PlayerEvent &playerEvent) {
            using Event = decltype(playerEvent.event);
            switch (playerEvent.event)
            {
            case Event::DEATH: {
                auto [livesComps] = cm.get<LivesComponent>(playerId);
                livesComps.mutate([&](LivesComponent &livesComp) { --livesComp.count; });
                auto &lifeCount = livesComps.peek(&LivesComponent::count);
                cm.add<UIEvent>(eId, UIEvents::UPDATE_LIVES);
                if (lifeCount <= 0)
                    cm.add<GameEvent>(eId, GameEvents::GAME_OVER);

                break;
            }
            default:
                break;
            }
        });
    });

    return cleanup;
};
}; // namespace Systems::Player
