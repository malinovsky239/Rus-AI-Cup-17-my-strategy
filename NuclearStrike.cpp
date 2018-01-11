#include "NuclearStrike.h"

NuclearStrike::NuclearStrike(const Vect& target, const long long launcher_id) : target_(target), launcher_id_(launcher_id) {}

void NuclearStrike::Execute(model::Move& move) const {
  move.setAction(model::ActionType::TACTICAL_NUCLEAR_STRIKE);
  move.setVehicleId(launcher_id_);
  move.setX(target_.x);
  move.setY(target_.y);
}

std::string NuclearStrike::Name() const {
  return "NuclearStrike";
}
