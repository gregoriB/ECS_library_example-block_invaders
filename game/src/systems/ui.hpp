#pragma once

#include "../components.hpp"
#include "../core.hpp"

namespace Systems::UI
{
inline void cleanup(ComponentManager &cm)
{
}

inline auto update(ComponentManager &cm)
{
    auto [uiEventSet] = cm.getAll<UIEvent>();
    uiEventSet.each([&](EId eId, auto &uiEvents) {
        uiEvents.inspect([&](const UIEvent &uiEvent) {
            auto [playerId, playerComps] = cm.getUnique<PlayerComponent>();
            using Event = decltype(uiEvent.event);
            switch (uiEvent.event)
            {
            case Event::UPDATE_SCORE: {
                auto [scoreComps] = cm.get<ScoreComponent>(playerId);
                auto &score = scoreComps.peek(&ScoreComponent::score);
                auto [playerScoreId, _] = cm.getUnique<PlayerScoreCardComponent>();
                auto [textComps] = cm.get<TextComponent>(playerScoreId);
                textComps.mutate(
                    [&](TextComponent &textComp) { textComp.text = "SCORE: " + std::to_string(score); });
                break;
            }
            case Event::UPDATE_LIVES: {
                auto [livesComps] = cm.get<LivesComponent>(playerId);
                auto &lives = livesComps.peek(&LivesComponent::count);
                auto [playerLifeCardId, _] = cm.getUnique<PlayerLifeCardComponent>();
                auto [textComps] = cm.get<TextComponent>(playerLifeCardId);
                textComps.mutate(
                    [&](TextComponent &textComp) { textComp.text = "LIVES: " + std::to_string(lives); });
                break;
            }
            }
        });
    });

    return cleanup;
}
}; // namespace Systems::UI
