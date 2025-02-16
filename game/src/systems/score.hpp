#pragma once

#include "../components.hpp"
#include "../core.hpp"

namespace Systems::Score
{
inline void cleanup(CM &cm)
{
}

inline auto update(CM &cm)
{
    auto [scoreEventSet] = cm.getAll<ScoreEvent>();
    scoreEventSet.each([&](EId eId, auto &scoreEvents) {
        scoreEvents.inspect([&](const ScoreEvent &scoreEvent) {
            auto [pointsComps] = cm.get<PointsComponent>(scoreEvent.pointsId);
            if (!pointsComps)
                return;

            auto [points, multiplier] =
                pointsComps.peek(&PointsComponent::points, &PointsComponent::multiplier);
            auto [scoreComps] = cm.get<ScoreComponent>(eId);
            scoreComps.mutate([&](ScoreComponent &scoreComp) { scoreComp.score += (points * multiplier); });

            auto [playerId, _] = cm.getUnique<PlayerComponent>();
            if (eId == playerId)
                cm.add<UIEvent>(eId, UIEvents::UPDATE_SCORE);
        });
    });

    return cleanup;
}
}; // namespace Systems::Score
