#pragma once

#include "core.hpp"
#include "renderer.hpp"
#include "update.hpp"
#include "utilities.hpp"
#include <stdexcept>

/**
 * @brief Setup the game and rendering, and run the game.
 */
class Game
{
  public:
    Benchmark run(int cycles)
    {
        if (!init())
            throw std::runtime_error("Game initialization failed");

        Benchmark benchmark;
        benchmark.run([&]() -> int { return loop(cycles); });

        return benchmark;
    }

    void run()
    {
        if (!init())
            throw std::runtime_error("Game initialization failed");

        loop();
    }

  private:
    bool init()
    {
        if (!m_renderManager.init())
            throw std::runtime_error("Renderer initialization failed!");

        Utilities::initializeGame(m_entityComponentManager, m_screenConfig);
        m_renderManager.startRender();

        return true;
    }

    /**
     * @brief Main game update loops where player input, system updates, and rerendering happens
     *
     * @param limit - Optional frame limit.  Game terminates when the limit is reached.
     */
    int loop(int limit = 0)
    {
        PRINT("\n $$$$$ STARTING GAME $$$$$ \n")

        int cycleCount{0};
        bool quit{false};
        float prevTime{0.0f};

        while (!quit)
        {
            if (cycleCount++ > limit && limit)
                break;

            float startTime = m_renderManager.tick();

            auto inputs = m_renderManager.pollInputs();
            Utilities::registerPlayerInputs(m_entityComponentManager, inputs);

            if (!Update::run(m_entityComponentManager))
            {
                PRINT("!! QUIT COMMAND ISSUED !!")
                quit = true;
                continue;
            };

            updateRenderer();
            waitIfNecessary(startTime);

            float delta = (startTime - prevTime) / 1000.0f;
            setDeltaTime(delta);

            prevTime = startTime;
        }

        PRINT("\n $$$$$ GAME OVER $$$$$ \n\n")

        m_renderManager.exit();

        return cycleCount;
    }

    void updateRenderer()
    {
        m_renderManager.clear();
        auto renders = Utilities::getRenderableElements(m_entityComponentManager);
        m_renderManager.render(renders);
    }

    void waitIfNecessary(int startTime)
    {
        int endTime = m_renderManager.tick();
        int timeDiff = endTime - startTime;
        if (timeDiff < m_screenConfig.ticks_per_frame)
            m_renderManager.wait(m_screenConfig.ticks_per_frame - timeDiff);
    }

    void setDeltaTime(float delta)
    {
        Utilities::setDeltaTime(m_entityComponentManager, delta);
    }

  private:
    ECS::Manager<EntityId> m_entityComponentManager{};
    ScreenConfig m_screenConfig{};
    Renderer::Manager<EntityId> m_renderManager{m_screenConfig};
};
