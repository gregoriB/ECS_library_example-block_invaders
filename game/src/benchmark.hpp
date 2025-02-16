#pragma once

#include "core.hpp"

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
