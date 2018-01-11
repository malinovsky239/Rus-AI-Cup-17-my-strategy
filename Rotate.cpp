#include "Rotate.h"

Rotate::Rotate(const double angle, const Vect& center) : angle_(angle), center_(center) {}

void Rotate::Execute(model::Move& move) const {
  move.setAction(model::ActionType::ROTATE);
  move.setAngle(angle_);
  move.setX(center_.x);
  move.setY(center_.y);
}

std::string Rotate::Name() const {
  return "Rotate";
}
