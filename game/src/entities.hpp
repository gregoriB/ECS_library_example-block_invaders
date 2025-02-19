#pragma once

#include "components.hpp"
#include "core.hpp"
#include "renderer.hpp"

inline void createGame(ComponentManager &cm, Vector2 &size, int tileSize)
{
    EntityId gameId = cm.createEntity();

    PRINT("CREATE GAME", gameId)
    cm.add<GameMetaComponent>(gameId, size, tileSize);
    cm.add<GameComponent>(gameId, Bounds{0, 0, size.x, size.y});
    cm.add<UFOTimeoutEffect>(gameId, 12);
    cm.add<PowerupTimeoutEffect>(gameId);
}

inline EntityId hive(ComponentManager &cm, float x, float y, float w, float h)
{
    EntityId hiveId = cm.createEntity();
    PRINT("CREATE HIVE", hiveId)
    cm.clear<HiveComponent, HiveMovementEffect>();
    auto [_, gameMetaComps] = cm.getUnique<GameMetaComponent>();
    auto &size = gameMetaComps.peek(&GameMetaComponent::screen);
    cm.add<HiveComponent>(hiveId);
    cm.add<HiveMovementEffect>(hiveId, Movements::RIGHT);
    cm.add<MovementComponent>(hiveId, Vector2{size.x / 200, size.y / 50});
    cm.add<AttackEffect>(hiveId, 0, 3);

    return hiveId;
}

inline EntityId player(ComponentManager &cm, float x, float y, float w, float h)
{
    EntityId id = cm.createEntity();

    PRINT("CREATE PLAYER", id)
    cm.add<CollidableComponent>(id);
    cm.add<PlayerComponent>(id);
    cm.add<PositionComponent>(id, Bounds{x - (w / 4), y + (h / 2), w * 1.5f, h - (h / 2)});
    cm.add<SpriteComponent>(id, Renderer::RGBA{0, 255, 0, 255});
    cm.add<MovementComponent>(id, Vector2{w * 10, w * 10});
    cm.add<AttackComponent>(id, Movements::UP);
    cm.add<HealthComponent>(id, 10);
    cm.add<DamageComponent>(id, 25.0f);
    cm.add<ScoreComponent>(id, 0);
    cm.add<LivesComponent>(id, 3);

    return id;
};

inline EntityId playerScore(ComponentManager &cm, float x, float y, float w, float h)
{
    EntityId id = cm.createEntity();

    PRINT("CREATE PLAYER SCORE", id)
    cm.add<PositionComponent>(id, Bounds{x, y, w, h});
    cm.add<SpriteComponent>(id, Renderer::RGBA{0, 0, 0, 0});
    cm.add<UIComponent>(id);
    cm.add<TextComponent>(id, "SCORE: 0");
    cm.add<PlayerScoreCardComponent>(id);

    return id;
};

inline EntityId playerLives(ComponentManager &cm, float x, float y, float w, float h)
{
    EntityId id = cm.createEntity();

    PRINT("CREATE PLAYER LIVES", id)
    cm.add<PositionComponent>(id, Bounds{x, y, w, h});
    cm.add<SpriteComponent>(id, Renderer::RGBA{0, 0, 0, 0});
    cm.add<UIComponent>(id);
    cm.add<TextComponent>(id, "LIVES: 3");
    cm.add<PlayerLifeCardComponent>(id);

    return id;
};

inline EntityId createUfo(ComponentManager &cm, float x, float y)
{
    EntityId id = cm.createEntity();
    PRINT("UFO SPAWNED", id)
    cm.add<UFOAIComponent>(id);
    auto [_, gameMetaComps] = cm.getUnique<GameMetaComponent>();
    auto &size = gameMetaComps.peek(&GameMetaComponent::screen);
    const float &tileSize = gameMetaComps.peek(&GameMetaComponent::tileSize);
    float diff = 15;
    float newW = tileSize + diff;
    float newX = x - newW;
    cm.add<CollidableComponent>(id);
    cm.add<PositionComponent>(id, Bounds{newX, y, newW, tileSize});
    cm.add<AttackComponent>(id, Movements::DOWN);
    cm.add<HealthComponent>(id, 10);
    cm.add<DamageComponent>(id, 100);
    cm.add<PointsComponent>(id, 150);
    cm.add<MovementComponent>(id, Vector2{tileSize * 4, tileSize * 4});
    cm.add<MovementEffect>(id, Vector2{tileSize * size.x, tileSize / 2});
    cm.add<SpriteComponent>(id, Renderer::RGBA{255, 0, 0, 255});
    float randomDelay = std::rand() % 5;
    cm.add<AttackEffect>(id, randomDelay);

    return id;
};

inline EntityId hiveAlien(ComponentManager &cm, float x, float y, float w, float h)
{
    EntityId id = cm.createEntity();
    auto [hiveId, _] = cm.getUnique<HiveComponent>();
    float diff = 7;
    cm.add<CollidableComponent>(id);
    cm.add<AIComponent>(id);
    cm.add<HiveAIComponent>(id, hiveId);
    cm.add<PositionComponent>(id, Bounds{x - diff, y, w + diff, h});
    cm.add<MovementComponent>(id, Vector2{w / 2, w});
    cm.add<AttackComponent>(id, Movements::DOWN);
    cm.add<HealthComponent>(id, 10);
    cm.add<DamageComponent>(id, 25.0f);

    return id;
};

inline EntityId hiveAlienSmall(ComponentManager &cm, float x, float y, float w, float h)
{
    EntityId id = hiveAlien(cm, x, y, w, h);
    cm.add<PointsComponent>(id, 10);
    cm.add<SpriteComponent>(id, Renderer::RGBA{205, 205, 205, 255});

    return id;
}

inline EntityId hiveAlienMedium(ComponentManager &cm, float x, float y, float w, float h)
{
    EntityId id = hiveAlien(cm, x, y, w, h);
    cm.add<PointsComponent>(id, 20);
    cm.add<SpriteComponent>(id, Renderer::RGBA{230, 230, 230, 255});

    return id;
}

inline EntityId hiveAlienLarge(ComponentManager &cm, float x, float y, float w, float h)
{
    EntityId id = hiveAlien(cm, x, y, w, h);
    cm.add<PointsComponent>(id, 40);
    cm.add<SpriteComponent>(id, Renderer::RGBA{255, 255, 255, 255});

    return id;
}

inline EntityId collidableObstacleBlock(ComponentManager &cm, float x, float y, float w, float h)
{
    EntityId id = cm.createEntity();
    cm.add<ObstacleComponent>(id);
    cm.add<CollidableComponent>(id);
    cm.add<DamageComponent>(id, 1);

    return id;
}

inline EntityId titleBlockSm(ComponentManager &cm, float x, float y, float w, float h)
{
    EntityId id = collidableObstacleBlock(cm, x, y, w, h);
    cm.add<SpriteComponent>(id, Renderer::RGBA{0, 255, 0, 255});
    cm.add<PositionComponent>(id, Bounds{x, y, w - 5, h});
    cm.add<HealthComponent>(id, 50);
    cm.add<TitleScreenComponent>(id);

    return id;
}

inline EntityId titleBlock(ComponentManager &cm, float x, float y, float w, float h)
{
    EntityId id = collidableObstacleBlock(cm, x, y, w, h);
    cm.add<SpriteComponent>(id, Renderer::RGBA{0, 255, 0, 255});
    cm.add<PositionComponent>(id, Bounds{x, y, w, h});
    cm.add<HealthComponent>(id, 50);
    cm.add<TitleScreenComponent>(id);

    return id;
}

inline EntityId redBlock(ComponentManager &cm, float x, float y, float w, float h)
{
    EntityId id = collidableObstacleBlock(cm, x, y, w, h);
    cm.add<SpriteComponent>(id, Renderer::RGBA{255, 0, 0, 255});
    cm.add<PositionComponent>(id, Bounds{x, y, w, h});
    cm.add<HealthComponent>(id, 1);
    cm.add<PointsComponent>(id, 1);

    return id;
}

inline EntityId startBlock(ComponentManager &cm, float x, float y, float w, float h)
{
    EntityId id = collidableObstacleBlock(cm, x, y, w, h);
    cm.add<SpriteComponent>(id, Renderer::RGBA{67, 189, 255, 255});
    cm.add<PositionComponent>(id, Bounds{x, y, w, h});
    cm.add<HealthComponent>(id, 1);
    cm.add<StartGameTriggerComponent>(id);
    cm.add<TitleScreenComponent>(id);

    return id;
}

inline EntityId greenBlock(ComponentManager &cm, float x, float y, float w, float h)
{
    EntityId id = collidableObstacleBlock(cm, x, y, w, h);
    cm.add<SpriteComponent>(id, Renderer::RGBA{0, 255, 0, 255});
    cm.add<PositionComponent>(id, Bounds{x, y, w, h});
    cm.add<HealthComponent>(id, 100);

    return id;
}

inline EntityId createProjectile(ComponentManager &cm, Bounds bounds)
{
    EntityId id = cm.createEntity();
    auto [w, h] = bounds.size;
    cm.add<CollidableComponent>(id);
    cm.add<MovementComponent>(id, Vector2{0, w * 10});
    cm.add<SpriteComponent>(id, Renderer::RGBA{255, 255, 255, 255});
    cm.add<HealthComponent>(id, 1);
    return id;
};

inline EntityId createUpwardProjectile(ComponentManager &cm, EntityId shooterId, Bounds bounds)
{
    auto [x, y, w, h] = bounds.get();
    float newW = w / 5;
    float newH = h * 2;
    float newY = y - newH - 1;
    float newX = x + (w / 2) - (newW / 2);
    EntityId id = createProjectile(cm, bounds);
    cm.add<MovementEffect>(id, Vector2{newX, -10000});
    cm.add<PositionComponent>(id, Bounds{newX, newY, newW, newH});
    using Movements = decltype(ProjectileComponent::movement);
    cm.add<ProjectileComponent>(id, shooterId, Movements::UP);

    return id;
}

inline EntityId createDownwardProjectile(ComponentManager &cm, EntityId shooterId, Bounds bounds)
{
    auto [x, y, w, h] = bounds.get();
    float newW = w / 5;
    float newH = h;
    float newY = y + newH;
    float newX = x + (w / 2) - (newW / 2);
    EntityId id = createProjectile(cm, bounds);
    cm.add<MovementEffect>(id, Vector2{newX, 10000});
    cm.add<PositionComponent>(id, Bounds{newX, newY + 1, newW, newH});
    using Movements = decltype(ProjectileComponent::movement);
    cm.add<ProjectileComponent>(id, shooterId, Movements::DOWN);
    cm.add<PointsComponent>(id, 10);

    return id;
}

inline EntityId createPowerup(ComponentManager &cm, Bounds bounds)
{
    EntityId id = cm.createEntity();
    PRINT("POWERUP SPAWNED", id)
    cm.add<CollidableComponent>(id);
    cm.add<HealthComponent>(id, 1);
    cm.add<SpriteComponent>(id, Renderer::RGBA{255, 255, 0, 255});
    cm.add<PositionComponent>(id, bounds);
    cm.add<PowerupComponent>(id);

    return id;
}
