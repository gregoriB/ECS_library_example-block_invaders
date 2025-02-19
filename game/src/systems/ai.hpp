#pragma once

#include "../components.hpp"
#include "../core.hpp"
#include "../entities.hpp"
#include "../utilities.hpp"
#include "ecs/ecs.hpp"

namespace Systems::AI
{
inline void cleanup(ComponentManager &cm)
{
    Utilities::cleanupEffect<AITimeoutEffect, UFOTimeoutEffect>(cm);
}

inline void updateOutsideHiveAliens(ComponentManager &cm, EId hiveId, const HiveComponent &hiveComp)
{
    auto &ids = cm.getEntityIds<HiveAIComponent>();
    if (ids.empty())
    {
        cm.add<GameEvent>(hiveId, GameEvents::NEXT_STAGE);
        return;
    }

    // Add components to distinquish the aliens on the left and right sites of the hive
    auto [x, y, w, h] = hiveComp.bounds.box();
    for (const auto &eId : ids)
    {
        auto [positionComps] = cm.get<PositionComponent>(eId);
        auto [aiX, aiY, aiW, aiH] = positionComps.peek(&PositionComponent::bounds).box();
        if (aiX <= x)
            cm.add<LeftAlienComponent>(eId);
        if (aiW >= w)
            cm.add<RightAlienComponent>(eId);
    };
}

inline void updateHiveBounds(ComponentManager &cm, EId hiveId)
{
    auto [hiveComps] = cm.get<HiveComponent>(hiveId);
    hiveComps.mutate([&](HiveComponent &hiveComp) {
        constexpr float MIN_FLOAT = std::numeric_limits<float>::min();
        constexpr float MAX_FLOAT = std::numeric_limits<float>::max();

        Vector2 topLeft{MAX_FLOAT, MAX_FLOAT};
        Vector2 bottomRight{MIN_FLOAT, MIN_FLOAT};

        // Get the topleft and bottomright hive bounds from the alien positions
        cm.getGroup<HiveAIComponent, PositionComponent>().each([&](EId eId, auto &_, auto &positionComps) {
            positionComps.inspect([&](const PositionComponent &posComp) {
                auto [x, y, w, h] = posComp.bounds.box();
                if (x < topLeft.x)
                    topLeft.x = x;
                if (y < topLeft.y)
                    topLeft.y = y;
                if (w > bottomRight.x)
                    bottomRight.x = w;
                if (h > bottomRight.y)
                    bottomRight.y = h;
            });
        });

        hiveComp.bounds = Bounds{topLeft, Vector2{bottomRight.x - topLeft.x, bottomRight.y - topLeft.y}};
        updateOutsideHiveAliens(cm, hiveId, hiveComp);
    });
}

inline void handleHiveShift(ComponentManager &cm, auto &hiveMovementEffects)
{
    hiveMovementEffects.mutate([&](HiveMovementEffect &hiveMovementEffect) {
        using Movement = decltype(HiveMovementEffect::movement);
        switch (hiveMovementEffect.movement)
        {
        case Movement::LEFT:
            hiveMovementEffect.movement = Movement::DOWN;
            hiveMovementEffect.nextMove = Movement::RIGHT;
            break;
        case Movement::RIGHT:
            hiveMovementEffect.movement = Movement::DOWN;
            hiveMovementEffect.nextMove = Movement::LEFT;
            break;
        default:
            break;
        }
    });
}

template <typename Movement>
inline Vector2 calculateSpeed(ComponentManager &cm, const Vector2 &speed, const Movement &movement)
{
    auto [_, gameComps] = cm.getUnique<GameComponent>();
    float modifier = gameComps.peek(&GameComponent::currentStage) / 2.0f;
    if (modifier < 1)
        modifier = 1;

    Vector2 calculatedSpeed{0, 0};

    switch (movement)
    {
    case Movement::LEFT:
        calculatedSpeed.x = -1 * speed.x * modifier;
        break;
    case Movement::RIGHT:
        calculatedSpeed.x = speed.x * modifier;
        break;
    case Movement::DOWN:
        calculatedSpeed.y = speed.y;
        break;
    case Movement::UP:
        calculatedSpeed.y = -1 * speed.y;
        break;
    default:
        break;
    }

    return calculatedSpeed;
}

template <typename Movement>
inline bool checkIsOutOfBounds(ComponentManager &cm, EId hiveId, Movement &movement)
{
    auto [gameId, gameComps] = cm.getUnique<GameComponent>();
    auto [movementComps] = cm.get<MovementComponent>(hiveId);
    auto &hiveSpeeds = movementComps.peek(&MovementComponent::speeds);

    auto &hiveAiIds = movement == Movement::LEFT ? cm.getEntityIds<LeftAlienComponent>()
                                                 : cm.getEntityIds<RightAlienComponent>();
    if (!hiveAiIds.size())
        return false;

    auto [posComps] = cm.get<PositionComponent>(hiveAiIds[0]);
    return !!(posComps.find([&](const PositionComponent &positionComp) {
        auto [x, y] = calculateSpeed(cm, hiveSpeeds, movement);
        Bounds newBounds{
            positionComp.bounds.position.x + x,
            positionComp.bounds.position.y + y,
            positionComp.bounds.size.x + x,
            positionComp.bounds.size.y + y,
        };

        auto [gX, gY, gW, gH] = gameComps.peek(&GameComponent::bounds).box();
        auto [nX, nY, nW, nH] = newBounds.box();

        return nX <= gX || nY <= gY || nW >= gW || nH >= gH;
    }));
}

inline bool checkHiveOutOfBounds(ComponentManager &cm, EId hiveId, auto &hiveMovementEffects)
{
    if (!cm.exists<LeftAlienComponent>() || !cm.exists<RightAlienComponent>())
        updateHiveBounds(cm, hiveId);

    auto movement = hiveMovementEffects.peek(&HiveMovementEffect::movement);
    using Movement = decltype(movement);

    switch (movement)
    {
    case Movement::LEFT:
    case Movement::RIGHT:
        return checkIsOutOfBounds(cm, hiveId, movement);
    default:
        break;
    }

    return false;
}

inline void moveHiveAI(ComponentManager &cm, EId hiveId, auto &hiveMovementEffects)
{
    auto movement = hiveMovementEffects.peek(&HiveMovementEffect::movement);
    auto [movementComps] = cm.get<MovementComponent>(hiveId);
    auto &speeds = movementComps.peek(&MovementComponent::speeds);

    auto &allHiveAiIds = cm.getEntityIds<HiveAIComponent>();
    if (!allHiveAiIds.size())
        return;

    auto newSpeed = calculateSpeed(cm, speeds, movement);
    if (!newSpeed.x && !newSpeed.y)
        return;

    for (const auto &eId : cm.getEntityIds<HiveAIComponent>())
        cm.add<MovementEvent>(eId, std::move(newSpeed));
}

inline void updateHiveMovement(ComponentManager &cm, EId hiveId, auto &hiveMovementEffects)
{
    hiveMovementEffects.mutate([&](HiveMovementEffect &hiveMovementEffect) {
        auto &allHiveAiIds = cm.getEntityIds<HiveAIComponent>();
        auto hiveAICount = allHiveAiIds.size();
        if (!hiveAICount)
            return;

        using Movement = decltype(HiveMovementEffect::movement);
        if (hiveMovementEffect.movement == Movement::DOWN)
            hiveMovementEffect.movement = hiveMovementEffect.nextMove;

        float hiveTotal = 55.0f;
        float diff = hiveTotal - hiveAICount;
        diff = diff > 0 ? diff : 1.0f;
        float interval = 0.5f / (diff / 2);

        hiveMovementEffect.timer->update(interval);
    });
}

inline bool checkShouldHiveAIMove(ECS::Components<HiveMovementEffect> &hiveMovementEffects)
{
    return !!(hiveMovementEffects.find([&](const HiveMovementEffect &hiveMovementEffect) {
        return hiveMovementEffect.timer->hasElapsed();
    }));
}

inline void updateHive(ComponentManager &cm)
{
    auto [hiveMovementSet] = cm.getAll<HiveMovementEffect>();
    hiveMovementSet.each([&](EId hiveId, auto &hiveMovementEffects) {
        if (checkHiveOutOfBounds(cm, hiveId, hiveMovementEffects))
            handleHiveShift(cm, hiveMovementEffects);

        if (checkShouldHiveAIMove(hiveMovementEffects))
        {
            moveHiveAI(cm, hiveId, hiveMovementEffects);
            updateHiveMovement(cm, hiveId, hiveMovementEffects);
        }
    });
}

inline bool containsId(const auto &vec, EntityId id)
{
    for (const auto &vecId : vec)
        if (vecId == id)
            return true;

    return false;
}

inline void handleHiveAttack(ComponentManager &cm)
{
    auto [hiveId, hiveComps] = cm.getUnique<HiveComponent>();
    if (!hiveId)
        return;

    auto [aiTimeoutEffects] = cm.get<AITimeoutEffect>(hiveId);
    if (aiTimeoutEffects)
    {
        auto elapsedEffect = aiTimeoutEffects.find(
            [&](const AITimeoutEffect &AITimeoutEffect) { return AITimeoutEffect.timer->hasElapsed(); });

        if (!elapsedEffect)
            return;

        cm.remove<AITimeoutEffect>(hiveId);
    }

    auto &attackingAIs = cm.getEntityIds<HiveAIComponent, AttackEffect>();
    if (attackingAIs.size() >= 3)
        return;

    auto hiveAiIds = cm.getEntityIds<HiveAIComponent>();
    if (hiveAiIds.empty())
        return;

    for (auto iter = hiveAiIds.begin(); iter != hiveAiIds.end();)
    {
        auto id = hiveAiIds[*iter];
        if (containsId(attackingAIs, id))
            iter = hiveAiIds.erase(iter);
        else
            ++iter;
    }

    float randomIndex = std::rand() % hiveAiIds.size();
    cm.add<AttackEvent>(hiveAiIds[randomIndex], 0);

    float randomDelay = std::rand() % 10;
    cm.add<AITimeoutEffect>(hiveId, randomDelay);
}

inline void updateUFO(ComponentManager &cm)
{
    auto [gameId, gameMetaComps] = cm.getUnique<GameMetaComponent>();
    auto [ufoTimeoutEffect] = cm.get<UFOTimeoutEffect>(gameId);
    if (ufoTimeoutEffect)
        return;

    auto [ufoAISet] = cm.getAll<UFOAIComponent>();
    if (ufoAISet)
        return;

    const float &tileSize = gameMetaComps.peek(&GameMetaComponent::tileSize);
    createUfo(cm, 10.0f, tileSize / 2);
    cm.add<UFOTimeoutEffect>(gameId, 15);
}

inline void handleUFOAttack(ComponentManager &cm)
{
    auto [ufoAISet] = cm.getAll<UFOAIComponent>();
    ufoAISet.each([&](EId eId, auto &ufoAiComps) {
        if (cm.contains<AITimeoutEffect>(eId))
            return;

        float randomDelay = std::rand() % 5;
        cm.add<AttackEvent>(eId, 0);
        cm.add<AITimeoutEffect>(eId, randomDelay);
    });
}

inline auto update(ComponentManager &cm)
{
    updateHive(cm);
    updateUFO(cm);

    handleHiveAttack(cm);
    handleUFOAttack(cm);

    return cleanup;
};
}; // namespace Systems::AI
