#pragma once

#include "../components.hpp"
#include "../core.hpp"
#include "../utilities.hpp"

namespace Systems::Game
{
inline void cleanup(ComponentManager &cm)
{
}

// Handle game state for game over/quit events and stage transitions
inline auto update(ComponentManager &cm)
{
    auto [gameEventSet] = cm.getAll<GameEvent>();
    gameEventSet.each([&](EId eId, auto &gameEvents) {
        gameEvents.inspect([&](const GameEvent &gameEvent) {
            auto [_, gameComps] = cm.getUnique<GameComponent>();
            gameComps.mutate([&](GameComponent &gameComp) {
                switch (gameEvent.event)
                {
                case GameEvents::QUIT: {
                    PRINT("GAME QUIT")
                    gameComps.mutate([&](GameComponent &gameComp) { gameComp.isGameOver = true; });
                    break;
                }
                case GameEvents::GAME_OVER: {
                    PRINT("GAME OVER")
                    Utilities::goToStage(cm, -999);
                    auto [playerId, _] = cm.getUnique<PlayerComponent>();
                    cm.add<DeactivatedComponent>(playerId);
                    break;
                }
                case GameEvents::NEXT_STAGE: {
                    auto [startTriggerId, _] = cm.getUnique<StartGameTriggerComponent>();
                    if (eId == startTriggerId)
                    {
                        gameComp.currentStage = 1;
                        Utilities::goToStage(cm, gameComp.currentStage);
                        auto entities = cm.getEntityIds<TitleScreenComponent>();
                        cm.remove(entities);
                    }
                    else
                    {
                        PRINT("STAGE CLEARED!!")
                        Utilities::goToStage(cm, ++gameComp.currentStage);
                    }
                    break;
                }
                default:
                    break;
                }
            });
        });
    });

    return cleanup;
};
}; // namespace Systems::Game
