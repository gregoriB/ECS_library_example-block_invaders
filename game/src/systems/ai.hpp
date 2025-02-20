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
    Utilities::cleanupEffect<AITimeoutEffect, UFOTimeoutEffect, UFOAttackTimeoutEffect>(cm);
}

// Adds components to left and right aliens to denote their position
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

// Sets the boundaries of the hive based on the outmost alien positions
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

// Transitions the hive movement into the next direction
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

inline float getDifficultyModifier(ComponentManager &cm)
{
    auto [_, gameComps] = cm.getUnique<GameComponent>();
    float modifier = gameComps.peek(&GameComponent::currentStage) / 2.0f;
    if (modifier < 1)
        modifier = 1;

    return modifier;
}

// Calculates movement speed based on the input speed, movement configuration object, and current stage speed
// modifier
template <typename Movement>
inline Vector2 calculateSpeed(ComponentManager &cm, const Vector2 &speed, const Movement &movement)
{
    auto [_, gameComps] = cm.getUnique<GameComponent>();
    float modifier = getDifficultyModifier(cm);

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
inline bool checkHiveAgainstScreenBoundaries(ComponentManager &cm, EId hiveId, Movement &movement)
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

// Check the leftmost and rightmost alien positions to see if the hive is out of bounds
inline bool checkIsHiveOutOfBounds(ComponentManager &cm, EId hiveId, auto &hiveMovementEffects)
{
    if (!cm.exists<LeftAlienComponent>() || !cm.exists<RightAlienComponent>())
        updateHiveBounds(cm, hiveId);

    auto movement = hiveMovementEffects.peek(&HiveMovementEffect::movement);
    using Movement = decltype(movement);

    switch (movement)
    {
    case Movement::LEFT:
    case Movement::RIGHT:
        return checkHiveAgainstScreenBoundaries(cm, hiveId, movement);
    default:
        break;
    }

    return false;
}

// Moves each hive alien based on the hive movement effect
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

// Updates the hive movement data
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

// Handles moving hive aliens and updating the hive movement data
inline void updateHive(ComponentManager &cm)
{
    auto [hiveMovementSet] = cm.getAll<HiveMovementEffect>();
    hiveMovementSet.each([&](EId hiveId, auto &hiveMovementEffects) {
        if (checkIsHiveOutOfBounds(cm, hiveId, hiveMovementEffects))
            handleHiveShift(cm, hiveMovementEffects);

        if (checkShouldHiveAIMove(hiveMovementEffects))
        {
            moveHiveAI(cm, hiveId, hiveMovementEffects);
            updateHiveMovement(cm, hiveId, hiveMovementEffects);
        }
    });
}

// Choose a random alien to attack. The alien cannot already be attacking, and there
// are limits to how freqently the hive can attack, and how many aliens can be attacking
// at the same time.  All of that is handled here.
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

        if (Utilities::containsId(attackingAIs, id))
            iter = hiveAiIds.erase(iter);
        else
            ++iter;
    }

    float randomIndex = std::rand() % hiveAiIds.size();
    cm.add<AttackEvent>(hiveAiIds[randomIndex], 0);

    float randomDelay = std::rand() % 10;
    cm.add<AITimeoutEffect>(hiveId, randomDelay);
}

// Creates a new UFO if allowed
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

// Creates a UFO attack if the UFO is not in a timeout state
inline void handleUFOAttack(ComponentManager &cm)
{
    auto [ufoAISet] = cm.getAll<UFOAIComponent>();
    ufoAISet.each([&](EId eId, auto &ufoAiComps) {
        if (cm.contains<UFOAttackTimeoutEffect>(eId))
            return;

        float modifier = getDifficultyModifier(cm);
        int maxInterval = 5000;
        // Get random number in milliseconds
        float randInterval = std::rand() % maxInterval;
        // Convert to seconds
        randInterval = randInterval / 1000;
        cm.add<AttackEvent>(eId, 0);
        cm.add<UFOAttackTimeoutEffect>(eId, randInterval / modifier);
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
