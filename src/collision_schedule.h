#pragma once

#include <functional>
#include <queue>
#include <variant>
#include <vector>

#include "ball.h"

struct No_collision {};
struct Border_collision {};

struct Collision_event {
  int id; // unique monotonic identifier
  int i1;
  std::variant<No_collision, Border_collision, int> i2;
  float tp; // collision time point

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
  float average_collision_time;

  Collision_event schedule_event_for(int i1, int id, float tp,
                                     std::vector<Ball> const& balls) const;

  void relax_timesheet_for(Collision_event const& e);

  void push_event_for(int i1, float tp, std::vector<Ball> const& balls);

 public:
  Collision_schedule(int width, int height, float tp, std::vector<Ball>& balls);

  // update balls positions if collision occurs
  void update(float tp, std::vector<Ball>& balls);
};
