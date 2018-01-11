#include "GoToWithSpeedLimit.h"

GoToWithSpeedLimit::GoToWithSpeedLimit(const Vect& shift, const double speed_limit)
    : shift_(shift), speed_limit_(speed_limit) {}

void GoToWithSpeedLimit::Execute(model::Move& move) const {
  move.setAction(model::ActionType::MOVE);
  move.setX(shift_.x);
  move.setY(shift_.y);
  move.setMaxSpeed(speed_limit_);
}

std::string GoToWithSpeedLimit::Name() const {
  return "GoToWithSpeedLimit";
}
