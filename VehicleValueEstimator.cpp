#include "VehicleValueEstimator.h"

int VehicleValueEstimator::CalculateVehicleValue(const model::Vehicle& vehicle, const model::Player& me) const {
  const bool almost_dying = vehicle.getDurability() * 2 < vehicle.getMaxDurability();
  const bool is_mine = vehicle.getPlayerId() == me.getId();
  const bool self_healing = vehicle.getType() == model::VehicleType::ARRV;

  int coeff = kStartVehicleValue;

  // If the vehicle has less than half of initial durability,
  // it is more probable that it will be destroyed by the wave from the nuclear blast.
  if (almost_dying) {
    coeff += kQuickKillBonus;
  }

  // Armored Repair and Recovery Vehicle and its neighbors are not vulnerable enough to the nuclear strike
  // because ARRV will heal itself and its damaged neighbors during the interval between two strikes
  if (self_healing) {
    coeff -= kArrvBonus;
  }

  // All factors taken into account above
  // have the exact opposite influence in case of player's vehicles.
  if (is_mine) {
    coeff *= -1;
  }

  return coeff;
}
