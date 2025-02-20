#pragma once

#include "components.hpp"
#include "core.hpp"
#include "entities.hpp"
#include "renderer.hpp"
#include "stages.hpp"
#include "ui.hpp"
#include <string_view>
#include <tuple>

/**
 * @brief Utilities are helper functions to be called from the main game class or the various systems
 */
namespace Utilities
{

/**
 * @brief Single place to register all transformation pipelines
 */
inline void registerTransformations(ComponentManager &cm)
{
    // Player powerup effect
    cm.registerTransformation<MovementComponent>([&](auto eId, MovementComponent comp) {
        auto [projectile] = cm.get<ProjectileComponent>(eId);
        if (!projectile)
            return comp;

        auto [playerId, _] = cm.getUnique<PlayerComponent>();
        auto &shooterId = projectile.peek(&ProjectileComponent::shooterId);
        if (shooterId != playerId || !cm.contains<PowerupEffect>(playerId))
            return comp;

        comp.speeds.y += 1000;
        return comp;
    });
}

inline int getTileSize(ComponentManager &cm, const std::vector<std::string_view> &stage)
{
    auto [_, gameMetaComps] = cm.getUnique<GameMetaComponent>();
    auto &size = gameMetaComps.peek(&GameMetaComponent::screen);
    return size.x / stage[0].size();
}

/**
 * @brief Build the game or UI from a template
 *
 * @tparam ConstructorGetterFn - Function which accepts a char and returns an entity constructor function
 *
 * @param templ - Reference to a template to build
 * @param getter - Getter function
 */
template <typename ConstructorGetterFn>
inline void buildFromTemplate(ComponentManager &cm, const std::vector<std::string_view> &templ,
                              ConstructorGetterFn &getter)
{
    auto tileSize = getTileSize(cm, templ);
    for (int row = 0; row < templ.size(); ++row)
    {
        for (int col = 0; col < templ[row].size(); ++col)
        {
            auto constructor = getter(templ[row][col]);
            if (!constructor)
                continue;

            constructor(cm, col * tileSize, row * tileSize, tileSize, tileSize);
        }
    }
};

/**
 * @brief All game initialization logic should be called from here
 *
 * @param screen - Screen config
 */
inline void initializeGame(ComponentManager &cm, ScreenConfig &screen)
{
    PRINT("STARTING GAME")
    auto stage = Stages::getStage(999);
    float screenW = screen.width;
    float screenH = screen.height;
    Vector2 size{screenW, screenH};
    createGame(cm, size, screen.width / stage[0].size());
    registerTransformations(cm);
    buildFromTemplate(cm, stage, Stages::getEntityConstructor);
    buildFromTemplate(cm, UI::getUI(1), UI::getEntityConstructor);
};

/**
 * @brief handles necessary current stage clearing and building of the next stage
 *
 * @param stage - New stage to load
 */
inline void goToStage(ComponentManager &cm, int stage)
{
    PRINT("STAGE:", stage, "LOADED")
    cm.clear<HiveMovementEffect>();
    cm.remove(cm.getEntityIds<HiveAIComponent>());
    buildFromTemplate(cm, Stages::getStage(stage), Stages::getEntityConstructor);
};

inline void setDeltaTime(ComponentManager &cm, float delta)
{
    auto [gameId, gameMetaComps] = cm.getUnique<GameMetaComponent>();
    gameMetaComps.mutate([&](GameMetaComponent &gameMetaComp) { gameMetaComp.deltaTime = delta; });
};

inline float getDeltaTime(ComponentManager &cm)
{
    auto [gameId, gameMetaComps] = cm.getUnique<GameMetaComponent>();
    return gameMetaComps.peek(&GameMetaComponent::deltaTime);
};

inline bool containsId(const auto &vec, EntityId id)
{
    for (const auto &vecId : vec)
        if (vecId == id)
            return true;

    return false;
}

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

/**
 * @brief Creates renderable elements, separated by non-UI and UI components.
 *
 * @return Container of rendereable elements
 */
inline std::vector<Renderer::RenderableElement> getRenderableElements(ComponentManager &cm)
{
    std::vector<Renderer::RenderableElement> worldElements{};
    std::vector<Renderer::RenderableElement> uiElements{};

    cm.getGroup<SpriteComponent, PositionComponent>().each(
        [&](EId eId, auto &spriteComps, auto &positionComps) {
            auto &rgba = spriteComps.peek(&SpriteComponent::rgba);
            auto [x, y, w, h] = positionComps.peek(&PositionComponent::bounds).get();
            Renderer::RenderableElement renderEl{x, y, w, h, rgba};
            auto isUIComps = cm.contains<UIComponent>(eId);
            auto &vec = isUIComps ? uiElements : worldElements;
            if (isUIComps)
            {
                auto [textComps] = cm.get<TextComponent>(eId);
                textComps.inspect([&](const TextComponent &textComp) {
                    renderEl.text = textComp.text;
                    renderEl.rgba = Renderer::RGBA{255, 255, 255, 255};
                });
            }

            vec.push_back(std::move(renderEl));
        });

    // UI elements are last to ensure they are overlaid on top
    worldElements.insert(worldElements.end(), uiElements.begin(), uiElements.end());
    return worldElements;
};

/**
 * @brief Iterate over each component set and cleanup and expired effects
 */
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
