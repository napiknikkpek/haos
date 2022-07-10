#pragma once

#include <functional>
#include <queue>
#include <vector>

#include "ball.h"

struct Collision_event {

  // Collision event contains 2 entities:
  // 1. collision with border, then i2 == -1
  // 2. collision between balls, then i2 != -1

  int id; // unique monotonic identifier
  int i1; 
  int i2;
  float tp; // collision time point

  explicit operator bool() const { return std::isfinite(tp); }

  friend auto operator<=>(Collision_event const& left,
                          Collision_event const& right) {
    return left.tp <=> right.tp;
  }
};

class Collision_schedule {
  std::priority_queue<Collision_event, std::vector<Collision_event>,
                      std::greater<Collision_event>>
      Q;

  struct Record {
    int id = -1;
    float tp = 0;
  };
  std::vector<Record> timesheet;
  int id = 0;
  int width;
  int height;

  void apply_border_collision(Ball& ball) const;

  static void apply_encounter_collision(Ball& left, Ball& right);

  void apply_collision(Collision_event const& e,
                       std::vector<Ball>& balls) const;

  static float schedule_collision_event(float tp, Ball const& left,
                                        Ball const& right);

  float schedule_border_event(float tp, Ball const& ball) const;

  Collision_event schedule_event_for(int i1, int id, float tp,
                                     std::vector<Ball> const& balls) const;

  bool is_active(Collision_event const& e) const;

  void relax_timesheet_for(Collision_event const& e);

  void push_event_for(int i1, float tp, std::vector<Ball> const& balls);

 public:
  Collision_schedule(int width, int height, float tp,
                     std::vector<Ball> const& balls);

  // update balls positions if collision occurs, return time point for last
  // collision
  float update(float tp, float t2, std::vector<Ball>& balls);
};
