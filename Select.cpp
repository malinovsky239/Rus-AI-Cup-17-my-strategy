#include "Select.h"

Select::Select(const Vect& top_left, const Vect& diagonal) : top_left_corner_(top_left), diagonal_(diagonal) {}

void Select::Execute(model::Move& move) const {
  move.setAction(model::ActionType::CLEAR_AND_SELECT);
  move.setLeft(top_left_corner_.x);
  move.setTop(top_left_corner_.y);
  const Vect bottom_right_corner = top_left_corner_ + diagonal_;
  move.setRight(bottom_right_corner.x);
  move.setBottom(bottom_right_corner.y);
}

std::string Select::Name() const {
  return "Select";
}
