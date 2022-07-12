#pragma once

#include <boost/qvm/all.hpp>

struct Ball {
  boost::qvm::vec<float, 2> position;
  boost::qvm::vec<float, 2> velocity;
  float radius = 0;
  float mass = 0;
  float tp = 0;
};

inline void move_ball(float tp, Ball& ball) {
  X(ball.position) += X(ball.velocity) * (tp - ball.tp);
  Y(ball.position) += Y(ball.velocity) * (tp - ball.tp);
  ball.tp = tp;
}
