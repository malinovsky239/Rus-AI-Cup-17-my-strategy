#include "Scale.h"

Scale::Scale(const double factor, const Vect& center) : factor_(factor), center_(center) {}

void Scale::Execute(model::Move& move) const {
  move.setAction(model::ActionType::SCALE);
  move.setFactor(factor_);
  move.setX(center_.x);
  move.setY(center_.y);
}

std::string Scale::Name() const {
  return "Scale";
}
