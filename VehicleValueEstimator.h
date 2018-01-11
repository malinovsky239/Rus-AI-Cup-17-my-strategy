#pragma once
#ifndef _VEHICLE_VALUE_ESTIMATOR_H_
#define _VEHICLE_VALUE_ESTIMATOR_H_

#include "Strategy.h"

// Estimates value of a specified vehicle (either player's or opponent's)
// based on its properties and manually selected heuristic costs.
// The more the value is, the more eager we are to damage this vehicle by Nuclear Strike.
// As one of consequences,
// our vehicles have negative values whereas the enemy's ones have positive values
class VehicleValueEstimator {
 public:
  int CalculateVehicleValue(const model::Vehicle& vehicle, const model::Player& me) const;

 private:
  const int kStartVehicleValue = 3;
  const int kQuickKillBonus = 2;
  const int kArrvBonus = -2;
};

#endif
