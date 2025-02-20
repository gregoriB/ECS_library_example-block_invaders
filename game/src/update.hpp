#pragma once

#include "components.hpp"
#include "core.hpp"
#include "systems/ai.hpp"
#include "systems/attack.hpp"
#include "systems/collision.hpp"
#include "systems/damage.hpp"
#include "systems/death.hpp"
#include "systems/game.hpp"
#include "systems/health.hpp"
#include "systems/input.hpp"
#include "systems/item.hpp"
#include "systems/movement.hpp"
#include "systems/player.hpp"
#include "systems/position.hpp"
#include "systems/score.hpp"
#include "systems/ui.hpp"

#include <functional>

/**
 * @brief Handles updating all game systems in the correct order, and cleaning up after updates
 */
namespace Update
{
using CleanupFunc = std::function<void(ComponentManager &)>;

/**
 * @brief Run the system cleanup function and clear any components which need clearing
 *
 * @tparam CleanupFuncs - Container of cleanup functions
 *
 * @param clenaupFuncs - Cleanup functions
 */
template <typename CleanupFuncs> inline void cleanup(ComponentManager &cm, CleanupFuncs &cleanupFuncs)
{
    for (auto &func : cleanupFuncs)
        func(cm);

    cm.clear<ECS::Tags::Event>();
}

/**
 * @brief Handles updating all systems in order, cleanup, and returns a bool to communicate the game exit
 * state
 *
 * @return bool - Game over state
 */
inline bool run(ComponentManager &cm)
{
    // clang-format off
    std::array<CleanupFunc, 14> cleanupFuncs{
        Systems::AI::update(cm),
        Systems::Input::update(cm),
        Systems::Attack::update(cm),
        Systems::Movement::update(cm),
        Systems::Position::update(cm),
        Systems::Collision::update(cm),
        Systems::Damage::update(cm),
        Systems::Health::update(cm),
        Systems::Death::update(cm),
        Systems::Score::update(cm),
        Systems::Player::update(cm),
        Systems::Item::update(cm),
        Systems::UI::update(cm),
        Systems::Game::update(cm),
    };

    // clang-format on
    cleanup(cm, cleanupFuncs);

    auto [gameId, gameComps] = cm.getUnique<GameComponent>();

    return !gameComps.peek(&GameComponent::isGameOver);
}
}; // namespace Update
