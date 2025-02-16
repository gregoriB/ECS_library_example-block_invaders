#pragma once

#include "../components.hpp"
#include "../core.hpp"

namespace Systems::Position
{
inline void cleanup(CM &cm)
{
}

inline auto update(CM &cm)
{
    auto [positionEventSet] = cm.getAll<PositionEvent>();
    positionEventSet.each([&](EId eId, auto &positionEvents) {
        positionEvents.inspect([&](const PositionEvent &positionEvent) {
            auto [positionComps] = cm.get<PositionComponent>(eId);
            positionComps.mutate([&](PositionComponent &positionComp) {
                positionComp.bounds.position.x = positionEvent.coords.x;
                positionComp.bounds.position.y = positionEvent.coords.y;
            });
        });
    });

    return cleanup;
};
}; // namespace Systems::Position
