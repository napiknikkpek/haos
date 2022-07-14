#include "collision_schedule.h"

#include <algorithm>
#include <cmath>
#include <functional>
#include <limits>
#include <numbers>

#include <boost/hana/functional/overload.hpp>
#include <range/v3/all.hpp>

using namespace ranges::views;

namespace {

float solve_average_collision_time(int width, int height,
                                   std::vector<Ball> const& balls) {
  // TODO this is just placeholder for average_collision_time
  auto E = ranges::accumulate(balls, 0.0f, std::plus<>{},
                              [](auto const& ball) {
                                return ball.mass * mag_sqr(ball.velocity);
                              }) /
           balls.size();

  auto m = ranges::accumulate(balls, 0.0f, std::plus<>{}, &Ball::mass) /
           balls.size();

  auto v = std::sqrt(E / m) / std::sqrt(2);

  return std::min(width, height) / v;
}

void apply_border_collision(int width, int height, Ball& ball) {
  if (std::min(X(ball.position), width - X(ball.position)) <
      std::min(Y(ball.position), height - Y(ball.position))) {
    X(ball.velocity) *= -1;
  } else {
    Y(ball.velocity) *= -1;
  }
}

void apply_encounter_collision(Ball& left, Ball& right) {
  boost::qvm::vec<float, 2> dir = normalized(right.position - left.position);

  float v1 = dot(left.velocity, dir);
  float v2 = dot(right.velocity, dir);

  v2 -= v1;

  float u1 = (2 * v2) / (1 + left.mass / right.mass);
  float u2 = v2 - (left.mass / right.mass) * u1;

  v2 += v1;
  u1 += v1;
  u2 += v1;

  left.velocity = left.velocity - v1 * dir + u1 * dir;
  right.velocity = right.velocity - v2 * dir + u2 * dir;
}

float schedule_border_event(int width, int height, Ball const& ball) {
  float tx = ((X(ball.velocity) > 0)
                  ? std::max(0.0f, width - ball.radius - X(ball.position))
                  : std::max(0.0f, X(ball.position) - ball.radius)) /
             std::abs(X(ball.velocity));
  float ty = ((Y(ball.velocity) > 0)
                  ? std::max(0.0f, height - ball.radius - Y(ball.position))
                  : std::max(0.0f, Y(ball.position) - ball.radius)) /
             std::abs(Y(ball.velocity));

  return ball.tp + std::min(tx, ty);
}

float schedule_collision_event(Ball const& left, Ball const& right) {
  // TODO now we find collision point as box collision, but we can exactly
  // solve circle collision

  boost::qvm::vec<float, 2> v = right.velocity - left.velocity;

  float tp = 0;
  boost::qvm::vec<float, 2> lp = left.position;
  boost::qvm::vec<float, 2> rp = right.position;

  if (left.tp < right.tp) {
    lp += left.velocity * (right.tp - left.tp);
    tp = right.tp;
  } else {
    rp += right.velocity * (left.tp - right.tp);
    tp = left.tp;
  }

  float dist = mag(lp - rp);

  boost::qvm::vec<float, 2> dir = normalized(lp - rp);

  float vx = dot(v, dir);
  float vy = mag(v - vx * dir);

  if (vx <= 0.0f) return std::numeric_limits<float>::max();

  float t = (dist - left.radius - right.radius) / vx;

  if ((t < 0) || (vy * t > (left.radius + right.radius))) {
    return std::numeric_limits<float>::max();
  }

  return tp + t;
}

}  // namespace

Collision_event Collision_schedule::schedule_event_for(
    int i1, int id, float tp, std::vector<Ball> const& balls) const {
  auto const& left = balls[i1];
  Collision_event e{id, i1, No_collision{}, tp + average_collision_time};

  auto t2 = schedule_border_event(width, height, left);
  if (t2 < e.tp) {
    e.i2 = Border_collision{};
    e.tp = t2;
  }

  for (int i2 = 0; i2 < static_cast<int>(balls.size()); ++i2) {
    if (i2 == i1) continue;
    auto const& right = balls[i2];
    auto t2 = schedule_collision_event(left, right);
    if (t2 < timesheet[i2].tp && t2 < e.tp) {
      e.tp = t2;
      e.i2 = i2;
    }
  }
  return e;
}

void Collision_schedule::relax_timesheet_for(Collision_event const& e) {
  timesheet[e.i1] = {e.id, e.tp};
  if (int const* i2 = std::get_if<int>(&e.i2)) {
    timesheet[*i2] = {e.id, e.tp};
  }
}

void Collision_schedule::push_event_for(int i1, float tp,
                                        std::vector<Ball> const& balls) {
  auto e = schedule_event_for(i1, id++, tp, balls);
  relax_timesheet_for(e);
  Q.push(e);
}

Collision_schedule::Collision_schedule(int width, int height, float tp,
                                       std::vector<Ball> const& balls)
    : timesheet(balls.size(), {-1, std::numeric_limits<float>::max()}),
      width{width},
      height{height} {
  average_collision_time = solve_average_collision_time(width, height, balls);
  for (auto i1 = 0u; i1 < balls.size(); ++i1) {
    push_event_for(i1, tp, balls);
  }
}

void Collision_schedule::update(float tp, std::vector<Ball>& balls) {
  while (Q.top().tp < tp) {
    auto event = Q.top();
    Q.pop();

    std::visit(boost::hana::overload(
                   [](No_collision) {
                     // do nothing
                   },
                   [&, this](Border_collision) {
                     if (event.id == timesheet[event.i1].id) {
                       auto& ball = balls[event.i1];
                       move_ball(tp, ball);
                       X(ball.position) = std::clamp(
                           X(ball.position), ball.radius, width - ball.radius);
                       Y(ball.position) = std::clamp(
                           Y(ball.position), ball.radius, height - ball.radius);
                       apply_border_collision(width, height, ball);
                     }
                   },
                   [&](int i2) {
                     if (event.id == timesheet[event.i1].id &&
                         event.id == timesheet[i2].id) {
                       move_ball(tp, balls[event.i1]);
                       move_ball(tp, balls[i2]);
                       apply_encounter_collision(balls[event.i1], balls[i2]);
                     }
                   }),
               event.i2);

    if (event.id == timesheet[event.i1].id) {
      push_event_for(event.i1, event.tp, balls);
    }

    if (int const* i2 = std::get_if<int>(&event.i2);
        i2 && (event.id == timesheet[*i2].id)) {
      push_event_for(*i2, event.tp, balls);
    }
  }
}
