#pragma once

#include <boost/qvm/all.hpp>

struct Ball {
  boost::qvm::vec<float, 2> position;
  boost::qvm::vec<float, 2> velocity;
  float radius = 0;
  float mass = 0;
};

inline void move_ball(float delta, Ball& ball) {
  X(ball.position) = X(ball.position) + X(ball.velocity) * delta;
  Y(ball.position) = Y(ball.position) + Y(ball.velocity) * delta;
}
