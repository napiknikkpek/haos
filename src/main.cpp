#include <numbers>
#include <random>
#include <vector>

#include <SFML/Graphics.hpp>
#include <range/v3/all.hpp>

#include "ball.h"
#include "collision_schedule.h"

using namespace ranges::views;

constexpr int WINDOW_X = 1024;
constexpr int WINDOW_Y = 768;
constexpr int MAX_BALLS = 300;
constexpr int MIN_BALLS = 100;

void draw_ball(Ball const& ball, sf::RenderWindow& window) {
  sf::CircleShape gball;
  gball.setRadius(ball.radius);
  gball.setPosition(X(ball.position) - ball.radius,
                    Y(ball.position) - ball.radius);
  window.draw(gball);
}

int main() {
  sf::RenderWindow window(sf::VideoMode(WINDOW_X, WINDOW_Y),
                          "ball collision demo");

  sf::Clock clock;

  float tp = clock.getElapsedTime().asSeconds();

  std::uniform_int_distribution<int> dist{};
  std::random_device gen{};

  auto balls = generate_n(
                   [&] {
                     Ball ball;
                     X(ball.position) = dist(gen) % WINDOW_X;
                     Y(ball.position) = dist(gen) % WINDOW_Y;
                     float speed = 30 + dist(gen) % 30;
                     X(ball.velocity) = speed * (-5 + (dist(gen) % 10)) / 3.;
                     Y(ball.velocity) = speed * (-5 + (dist(gen) % 10)) / 3.;
                     ball.radius = 5 + dist(gen) % 5;
                     ball.mass = std::numbers::pi * ball.radius * ball.radius;
                     return ball;
                   },
                   dist(gen) % (MAX_BALLS - MIN_BALLS) + MIN_BALLS) |
               ranges::to_vector;

  Collision_schedule schedule{WINDOW_X, WINDOW_Y, tp, balls};

  // window.setFramerateLimit(60);

  while (window.isOpen()) {
    sf::Event event;
    while (window.pollEvent(event)) {
      if (event.type == sf::Event::Closed) {
        window.close();
      }
    }

    float t2 = clock.getElapsedTime().asSeconds();

    float t1 = schedule.update(tp, t2, balls);

    for (auto& ball : balls) {
      move_ball(t2 - t1, ball);
    }
    tp = t2;

    window.clear();

    for (auto const& ball : balls) {
      draw_ball(ball, window);
    }

    window.display();
  }
  return 0;
}
