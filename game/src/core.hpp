#pragma once

#include <algorithm>
#include <cassert>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <limits>
#include <memory>
#include <stack>
#include <typeindex>
#include <unordered_map>
#include <utility>
#include <vector>

#include <ecs/ecs.hpp>

using EntityId = uint32_t;
using EId = EntityId;
using ComponentManager = ECS::Manager<EntityId>;

#define PRINT(...) ECS::internal::Utilities::print(__VA_ARGS__);

/**
 * @brief Generic inputs to be converted into game inputs
 */
enum class Inputs
{
    UP,
    DOWN,
    LEFT,
    RIGHT,
    SHOOT,
    MENU,
    QUIT,
};

struct ScreenConfig
{
    const int width{640};
    const int height{480};
    const int fps{30000};
    const int ticks_per_frame{1000 / fps};
};

/**
 * @brief Provides 2-dimensional coordinates
 */
class Vector2
{
  public:
    float x{}, y{};

    Vector2()
    {
    }
    Vector2(float _x, float _y) : x(_x), y(_y)
    {
    }
};

/**
 * @brief Provides relative and absolute rectangle bounds
 */
class Bounds
{
  public:
    Vector2 position;
    Vector2 size;

    Bounds()
    {
    }
    Bounds(Vector2 _position, Vector2 _size) : position(_position), size(_size)
    {
    }
    Bounds(float _x, float _y, float _w, float _h) : position(_x, _y), size(_w, _h)
    {
    }

    const std::array<float, 4> box() const
    {
        return {position.x, position.y, position.x + size.x, position.y + size.y};
    }

    const std::array<float, 4> get() const
    {
        return {position.x, position.y, size.x, size.y};
    }
};

/**
 * @brief Provides simple benchmarking utilities
 */
class Benchmark
{
  public:
    float average;
    int cycles;

    void printBenchmarks()
    {
        PRINT("average frame time:", average, "for", cycles, "frames\n", "  average FPS:", getFramerate());
    }

    void printBenchData()
    {
        PRINT("AVERAGE:", average, "CYCLES:", cycles, "FRAMERATE:", getFramerate())
    }

    void run(std::function<float()> fn)
    {
        auto start = std::chrono::high_resolution_clock::now();

        cycles = fn();
        if (cycles == 0)
            cycles = 1;

        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float> duration = end - start;
        average = duration.count() / cycles;
    };

    float getFramerate()
    {
        return cycles / (average * cycles);
    }
};
