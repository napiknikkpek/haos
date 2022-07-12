#include <numbers>
#include <random>
#include <vector>

#include <benchmark/benchmark.h>
#include <range/v3/all.hpp>

#include "collision_schedule.h"

using namespace ranges::views;

void BM_collision_bench(benchmark::State& state) {
  std::mt19937 gen{};
  std::uniform_int_distribution<int> dist{};

  int const width = 1024;
  int const height = 768;

  auto balls = generate_n(
                   [&] {
                     Ball ball;
                     ball.radius = 5 + dist(gen) % 5;
                     X(ball.position) =
                         ball.radius +
                         dist(gen) % static_cast<int>(width - 2 * ball.radius);
                     Y(ball.position) =
                         ball.radius +
                         dist(gen) % static_cast<int>(height - 2 * ball.radius);
                     float speed = 30 + dist(gen) % 30;
                     X(ball.velocity) = speed * (-5 + (dist(gen) % 10)) / 3.;
                     Y(ball.velocity) = speed * (-5 + (dist(gen) % 10)) / 3.;
                     ball.mass = std::numbers::pi * ball.radius * ball.radius;
                     ball.tp = 0;
                     return ball;
                   },
                   200) |
               ranges::to_vector;

  float tp = 0;

  Collision_schedule schedule{width, height, tp, balls};

  for (auto _ : state) {
    tp += 100;
    schedule.update(tp, balls);
  }
}

BENCHMARK(BM_collision_bench);

BENCHMARK_MAIN();
