#include "collision_schedule.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <numbers>

void Collision_schedule::apply_border_collision(Ball& ball) const {
  if (std::min(X(ball.position), width - X(ball.position)) <
      std::min(Y(ball.position), height - Y(ball.position))) {
    X(ball.velocity) *= -1;
  } else {
    Y(ball.velocity) *= -1;
  }
}

void Collision_schedule::apply_encounter_collision(Ball& left, Ball& right) {
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

void Collision_schedule::apply_collision(Collision_event const& e,
                                         std::vector<Ball>& balls) const {
  if (e.i2 == -1) {
    apply_border_collision(balls[e.i1]);
  } else {
    apply_encounter_collision(balls[e.i1], balls[e.i2]);
  }
}

float Collision_schedule::schedule_collision_event(float tp, Ball const& left,
                                                   Ball const& right) {
  // TODO now we find collision point as box collision, but we can exactly
  // solve circle collision
  boost::qvm::vec<float, 2> v = right.velocity - left.velocity;

  boost::qvm::vec<float, 2> dir = normalized(left.position - right.position);

  float vx = dot(v, dir);
  float vy = mag(v - vx * dir);

  if (vx <= 0.0f) return INFINITY;

  float dist = mag(left.position - right.position);
  float t = (dist - left.radius - right.radius) / vx;

  if ((t < 0) || (vy * t > (left.radius + right.radius))) {
    return INFINITY;
  }

  return tp + t;
}

float Collision_schedule::schedule_border_event(float tp,
                                                Ball const& ball) const {
  float tx = ((X(ball.velocity) > 0) ? (width - ball.radius - X(ball.position))
                                     : (X(ball.position) - ball.radius)) /
             std::abs(X(ball.velocity));
  float ty = ((Y(ball.velocity) > 0) ? (height - ball.radius - Y(ball.position))
                                     : (Y(ball.position) - ball.radius)) /
             std::abs(Y(ball.velocity));

  return tp + std::min(tx, ty);
}

Collision_event Collision_schedule::schedule_event_for(
    int i1, int id, float tp, std::vector<Ball> const& balls) const {
  auto const& left = balls[i1];
  Collision_event e{id, i1, -1, schedule_border_event(tp, left)};

  for (int x = 0; x < static_cast<int>(balls.size()); ++x) {
    if (x == i1) continue;

    auto const& right = balls[x];
    auto t2 = schedule_collision_event(tp, left, right);
    if (timesheet[x].tp <= t2 || e.tp <= t2) continue;

    e.tp = t2;
    e.i2 = x;
  }

  return e;
}

bool Collision_schedule::is_active(Collision_event const& e) const {
  if (e.i2 != -1) {
    return (timesheet[e.i1].id == e.id) && (timesheet[e.i2].id == e.id);
  } else {
    return timesheet[e.i1].id == e.id;
  }
}

void Collision_schedule::relax_timesheet_for(Collision_event const& e) {
  timesheet[e.i1] = {e.id, e.tp};
  if (e.i2 != -1) {
    timesheet[e.i2] = {e.id, e.tp};
  }
}

Collision_schedule::Collision_schedule(int width, int height, float tp,
                                       std::vector<Ball> const& balls)
    : timesheet(balls.size(), {-1, std::numeric_limits<float>::max()}),
      width{width},
      height{height} {
  for (auto i1 = 0u; i1 < balls.size(); ++i1) {
    auto e = schedule_event_for(i1, id++, tp, balls);
    if (e) {
      relax_timesheet_for(e);
      Q.push(e);
    }
  }
}

void Collision_schedule::push_event_for(int i1, float tp,
                                        std::vector<Ball> const& balls) {
  auto e = schedule_event_for(i1, id++, tp, balls);
  if (e) {
    relax_timesheet_for(e);
    Q.push(e);
  } else {
    timesheet[i1] = {-1, std::numeric_limits<float>::max()};
  }
}

float Collision_schedule::update(float tp, float t2, std::vector<Ball>& balls) {
  while (Q.top().tp < t2) {
    auto event = Q.top();
    Q.pop();

    for (auto& ball : balls) {
      move_ball(event.tp - tp, ball);
    }
    tp = event.tp;

    if (is_active(event)) {
      apply_collision(event, balls);
    }

    if (event.id == timesheet[event.i1].id) {
      push_event_for(event.i1, tp, balls);
    }

    if ((event.i2 != -1) && (event.id == timesheet[event.i2].id)) {
      push_event_for(event.i2, tp, balls);
    }
  }
  return tp;
}
