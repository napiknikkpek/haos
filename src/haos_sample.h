#pragma once

#include <numbers>
#include <random>
#include <vector>

#include <range/v3/all.hpp>

auto haos_sample(int width, int height, int size, int r, int R, int S, float tp,
                 auto& gen) {
  int Col = width / (2 * R);
  int Row = height / (2 * R);
  float dx = 1.0 * width / Col;
  float dy = 1.0 * height / Row;

  std::uniform_int_distribution<int> dist{};

  return ranges::views::iota(0, Col * Row) |
         ranges::views::transform([&](int i) {
           Ball ball;
           ball.radius = r + dist(gen) % (R - r);
           int row = i / Col;
           int col = i % Col;
           X(ball.position) = col * dx + dx / 2;
           Y(ball.position) = row * dy + dy / 2;
           X(ball.velocity) = dist(gen) % (2 * S) - S;
           Y(ball.velocity) = dist(gen) % (2 * S) - S;
           ball.mass = std::numbers::pi * ball.radius * ball.radius;
           ball.tp = tp;
           return ball;
         }) |
         ranges::views::sample(size, gen) | ranges::to_vector;
};
