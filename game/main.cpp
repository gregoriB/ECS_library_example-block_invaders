 #include "src/game.hpp"

/** 
 * @brief Run benchmarks for the specified number of sets and frames
 *
 * @param sets - number of sets to run
 */
Benchmark runWithBenchmarks(int sets = 3, int frames = 500000)
{
    std::vector<Benchmark> benchmarks{};
    for (int i = 0; i < sets; ++i)
    {
        Game game{};
        benchmarks.push_back(game.run(frames));
    }

    Benchmark benches{};

    for (const auto& bench : benchmarks)
    {
        benches.cycles += bench.cycles;
        benches.average += bench.average;
    }

    benches.average /= sets;

    return benches;
}

int main() {

#ifdef ecs_with_benchmarks

    Benchmark bench = runWithBenchmarks();
    bench.printBenchmarks();

#else

    Game game{};
    game.run();

#endif
    
    return 0;
}

