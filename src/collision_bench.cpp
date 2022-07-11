#include <numbers>
#include <random>
#include <vector>

#include <benchmark/benchmark.h>
#include <range/v3/all.hpp>

#include "collision_schedule.h"

using namespace ranges::views;

void BM_collision_bench(benchmark::State& state) {
  std::minstd_rand gen{};
  std::uniform_int_distribution<int> dist{};

  int const Width = 1024;
  int const Height = 768;

  auto balls = generate_n(
                   [&] {
                     Ball ball;
                     X(ball.position) = dist(gen) % Width;
                     Y(ball.position) = dist(gen) % Height;
                     float speed = 30 + dist(gen) % 30;
                     X(ball.velocity) = speed * (-5 + (dist(gen) % 10)) / 3.;
                     Y(ball.velocity) = speed * (-5 + (dist(gen) % 10)) / 3.;
                     ball.radius = 5 + dist(gen) % 5;
                     ball.mass = std::numbers::pi * ball.radius * ball.radius;
                     return ball;
                   },
                   200) |
               ranges::to_vector;

  float tp = 0;

  Collision_schedule schedule{Width, Height, tp, balls};

  for (auto _ : state) {
    tp = schedule.update(tp, tp + 100, balls);
  }
}

BENCHMARK(BM_collision_bench);

BENCHMARK_MAIN();
