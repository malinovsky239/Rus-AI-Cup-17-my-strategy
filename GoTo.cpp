#include "GoTo.h"

GoTo::GoTo(const Vect& shift) : shift_(shift) {}

void GoTo::Execute(model::Move& move) const {
  move.setAction(model::ActionType::MOVE);
  move.setX(shift_.x);
  move.setY(shift_.y);
}

std::string GoTo::Name() const {
  return "GoTo";
}
