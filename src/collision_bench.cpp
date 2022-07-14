#include <random>
#include <vector>

#include <benchmark/benchmark.h>
#include <range/v3/all.hpp>

#include "collision_schedule.h"
#include "haos_sample.h"

using namespace ranges::views;

void BM_collision_bench(benchmark::State& state) {
  int const width = 5'000;
  int const height = 5'000;
  float tp = 0;
  std::mt19937 gen{};

  auto balls = haos_sample(width, height, 10'000, 5, 10, 100, tp, gen);

  Collision_schedule schedule{width, height, tp, balls};

  for (auto _ : state) {
    tp += 1;
    schedule.update(tp, balls);
  }
}

BENCHMARK(BM_collision_bench);

BENCHMARK_MAIN();
