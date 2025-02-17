#pragma once

#include "benchmark.hpp"
#include "core.hpp"
#include "renderer.hpp"
#include "update.hpp"
#include "utilities.hpp"

class Game
{
  public:
    Benchmark run(int cycles)
    {
        if (!init())
            throw std::runtime_error("INIT FAILED");

        Benchmark benchmark;
        benchmark.run([&]() -> int { return loop(cycles); });

        return benchmark;
    }

    void run()
    {
        if (!init())
            throw std::runtime_error("INIT FAILED");

        loop();
    }

  private:
    bool init()
    {
        if (!m_renderManager.init())
            throw 12345;

        Utilities::setup(m_entityComponentManager, m_screenConfig);
        m_renderManager.startRender();

        return true;
    }

    int loop(int limit = 0)
    {
        PRINT("\n $$$$$ STARTING GAME $$$$$ \n")

        int cycleCount{0};
        bool quit{false};
        float prevTime = m_renderManager.tick();

        while (!quit)
        {
            cycleCount++;
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

            m_renderManager.clear();
            auto renders = Utilities::getRenderableElements(m_entityComponentManager);
            m_renderManager.render(renders);

            int endTime = m_renderManager.tick();
            int timeDiff = endTime - startTime;
            if (timeDiff < SCREEN_TICKS_PER_FRAME)
                m_renderManager.wait(SCREEN_TICKS_PER_FRAME - timeDiff);

            float delta = (startTime - prevTime) / 1000.0f;
            setDeltaTime(delta);
            prevTime = startTime;
        }

        PRINT("\n $$$$$ GAME OVER $$$$$ \n\n")

        m_renderManager.exit();

        return cycleCount;
    }

    void setDeltaTime(float delta)
    {
        Utilities::updateDeltaTime(m_entityComponentManager, delta);
    }

  private:
    ECS::Manager<EntityId> m_entityComponentManager{};
    ScreenConfig m_screenConfig{};
    Renderer::Manager<EntityId> m_renderManager{m_screenConfig};
};
