#pragma once

#include "components.hpp"
#include "core.hpp"
#include "entities.hpp"
#include "renderer.hpp"
#include "stages.hpp"
#include "ui.hpp"
#include <functional>
#include <string_view>
#include <tuple>

namespace Utilities
{
inline auto &first(auto value)
{
    return std::get<0>(value);
}

inline void registerTransformations(ComponentManager &cm)
{
    cm.registerTransformation<MovementComponent>([&](auto eId, MovementComponent comp) {
        auto &projectile = first(cm.get<ProjectileComponent>(eId));
        if (!projectile)
            return comp;

        auto [playerId, _] = cm.getUnique<PlayerComponent>();
        auto &shooterId = projectile.peek(&ProjectileComponent::shooterId);
        if (shooterId != playerId || !first(cm.get<PowerupEffect>(playerId)))
            return comp;

        comp.speeds.y += 1000;
        return comp;
    });
}

inline void stageBuilder(ComponentManager &cm, const std::vector<std::string_view> &stage, int tileSize)
{
    for (int row = 0; row < stage.size(); ++row)
    {
        for (int col = 0; col < stage[row].size(); ++col)
        {
            auto constructor = Stages::getEntityConstructor(stage[row][col]);
            if (!constructor)
                continue;

            constructor(cm, col * tileSize, row * tileSize, tileSize, tileSize);
        }
    }
};

inline void uiBuilder(ComponentManager &cm, const std::vector<std::string_view> &ui, int tileSize)
{
    for (int row = 0; row < ui.size(); ++row)
    {
        for (int col = 0; col < ui[row].size(); ++col)
        {
            auto constructor = UI::getEntityConstructor(ui[row][col]);
            if (!constructor)
                continue;

            constructor(cm, col * tileSize, row * tileSize, tileSize, tileSize);
        }
    }
};

inline void stageBuilder(ComponentManager &cm, const std::vector<std::string_view> &stage)
{
    auto [_, gameMetaComps] = cm.getUnique<GameMetaComponent>();
    auto &size = gameMetaComps.peek(&GameMetaComponent::screen);
    return stageBuilder(cm, stage, size.x / stage[0].size());
}

inline void setup(ComponentManager &cm, ScreenConfig &screen)
{
    PRINT("STARTING GAME")
    auto stage = Stages::getStage(999);
    int tileSize = screen.width / stage[0].size();
    Vector2 size{static_cast<float>(screen.width), static_cast<float>(screen.height)};

    createGame(cm, size, tileSize);
    registerTransformations(cm);
    stageBuilder(cm, stage, tileSize);
    auto ui = UI::getUI(1);
    uiBuilder(cm, ui, tileSize);
};

inline void nextStage(ComponentManager &cm, int stage)
{
    PRINT("STAGE:", stage, "LOADED")
    cm.clear<HiveMovementEffect>();
    cm.remove(cm.getEntityIds<HiveAIComponent>());
    stageBuilder(cm, Stages::getStage(stage));
};

inline void updateDeltaTime(ComponentManager &cm, float delta)
{
    auto [gameId, gameMetaComps] = cm.getUnique<GameMetaComponent>();
    gameMetaComps.mutate([&](GameMetaComponent &gameMetaComp) { gameMetaComp.deltaTime = delta; });
};

inline float getDeltaTime(ComponentManager &cm)
{
    auto [gameId, gameMetaComps] = cm.getUnique<GameMetaComponent>();
    return gameMetaComps.peek(&GameMetaComponent::deltaTime);
};

inline void registerPlayerInputs(ComponentManager &cm, std::vector<Inputs> &inputs)
{
    auto [playerId, _] = cm.getUnique<PlayerComponent>();
    using Movements = decltype(PlayerInputEvent::movement);
    using Actions = decltype(PlayerInputEvent::action);
    for (const auto &input : inputs)
    {
        switch (input)
        {
        case Inputs::SHOOT:
            cm.add<PlayerInputEvent>(playerId, Actions::SHOOT);
            break;
        case Inputs::LEFT:
            cm.add<PlayerInputEvent>(playerId, Movements::LEFT);
            break;
        case Inputs::RIGHT:
            cm.add<PlayerInputEvent>(playerId, Movements::RIGHT);
            break;
        case Inputs::QUIT:
            cm.add<PlayerInputEvent>(playerId, Actions::QUIT);
            break;
        case Inputs::UP:
        case Inputs::DOWN:
        case Inputs::MENU:
        default:
            break;
        }
    }
};

inline void registerAIInputs(ComponentManager &cm, EId eId, std::vector<Inputs> &inputs)
{
    using Movements = decltype(AIInputEvent::movement);
    using Actions = decltype(AIInputEvent::action);
    for (const auto &input : inputs)
        switch (input)
        {
        case Inputs::SHOOT:
            cm.add<AIInputEvent>(eId, Actions::SHOOT);
            break;
        case Inputs::LEFT:
            cm.add<AIInputEvent>(eId, Movements::LEFT);
            break;
        case Inputs::RIGHT:
            cm.add<AIInputEvent>(eId, Movements::RIGHT);
            break;
        case Inputs::UP:
            cm.add<AIInputEvent>(eId, Movements::UP);
            break;
        case Inputs::DOWN:
            cm.add<AIInputEvent>(eId, Movements::DOWN);
            break;
        case Inputs::MENU:
        case Inputs::QUIT:
        default:
            break;
        }
};

inline bool getGameoverState(ComponentManager &cm)
{
    auto [gameId, gameComps] = cm.getUnique<GameComponent>();
    return gameComps.peek(&GameComponent::isGameOver);
};

inline std::vector<Renderer::RenderableElement> getRenderableElements(ComponentManager &cm)
{
    std::vector<Renderer::RenderableElement> worldElements{};
    std::vector<Renderer::RenderableElement> uiElements{};

    cm.getGroup<SpriteComponent, PositionComponent>().each(
        [&](EId eId, auto &spriteComps, auto &positionComps) {
            auto &rgba = spriteComps.peek(&SpriteComponent::rgba);
            auto [x, y, w, h] = positionComps.peek(&PositionComponent::bounds).get();
            Renderer::RenderableElement renderEl{x, y, w, h, rgba};
            auto &uiComps = first(cm.get<UIComponent>(eId));
            auto &vec = !!uiComps ? uiElements : worldElements;
            if (uiComps)
            {
                auto &textComps = first(cm.get<TextComponent>(eId));
                textComps.inspect([&](const TextComponent &textComp) {
                    renderEl.text = textComp.text;
                    renderEl.rgba = Renderer::RGBA{255, 255, 255, 255};
                });
            }

            vec.push_back(std::move(renderEl));
        });

    worldElements.insert(worldElements.end(), uiElements.begin(), uiElements.end());
    return worldElements;
};

template <typename... Ts> inline void cleanupEffect(ComponentManager &cm)
{
    std::apply(
        [&](auto &...set) {
            (set.each([&](EId eId, auto &effects) {
                effects.remove([&](auto &effect) {
                    if (effect.timer.has_value() && effect.timer->hasElapsed())
                        return true;

                    return effect.cleanup;
                });
            }),
             ...);
        },
        cm.getAll<Ts...>());
};

}; // namespace Utilities
