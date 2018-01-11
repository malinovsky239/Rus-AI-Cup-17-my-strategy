#pragma once
#ifndef _MOTIONLESSNESS_CHECKER_H_
#define _MOTIONLESSNESS_CHECKER_H_

#include "Strategy.h"
#include <map>
#include <vector>

// Checks if a specific vehicle (or all vehicles of specific type)
// hasn't (haven't) moved for at least <kMotionCooldown> ticks
class MotionlessnessChecker {
 public:
  MotionlessnessChecker(const std::map<long long, model::Vehicle>& vehicle_by_id,
                        std::map<long long, int>& vehicle_coordinates_update_tick_by_id,
                        const int number_of_vehicle_types);

  void CheckMyVehiclesMotionlessness(const model::Player& me, const int current_tick);
  bool IsVehicleMotionless(const model::Vehicle& vehicle, const int current_tick) const;

  bool AreAllVehiclesOfTypeMotionless(const model::VehicleType& vehicle_type) const;
  bool AreAllVehiclesOfTypeMotionless(const size_t vehicle_type_index) const;

  bool AreAllVehiclesOfTypesMotionless(const std::vector<model::VehicleType>& types) const;

 private:
  const int kMotionCooldown = 31;
  const int kNumberOfVehicleTypes;

  const std::map<long long, model::Vehicle>& vehicle_by_id_;
  std::map<long long, int>& vehicle_coordinates_update_tick_by_id_;

  std::vector<bool> are_all_vehicles_of_type_motionless_;
};

#endif
