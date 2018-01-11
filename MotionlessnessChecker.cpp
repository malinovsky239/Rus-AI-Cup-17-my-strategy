#include "MotionlessnessChecker.h"

MotionlessnessChecker::MotionlessnessChecker(const std::map<long long, model::Vehicle>& vehicle_by_id,
                                             std::map<long long, int>& vehicle_coordinates_update_tick_by_id,
                                             const int number_of_vehicle_types)
    : kNumberOfVehicleTypes(number_of_vehicle_types),
      vehicle_by_id_(vehicle_by_id),
      vehicle_coordinates_update_tick_by_id_(vehicle_coordinates_update_tick_by_id) {}

void MotionlessnessChecker::CheckMyVehiclesMotionlessness(const model::Player& me, const int current_tick) {
  are_all_vehicles_of_type_motionless_ = std::vector<bool>(kNumberOfVehicleTypes, true);
  for (const auto& id_and_vehicle : vehicle_by_id_) {
    const model::Vehicle& vehicle = id_and_vehicle.second;    
    if (vehicle.getPlayerId() == me.getId()) {
      if (!IsVehicleMotionless(vehicle, current_tick)) {
        are_all_vehicles_of_type_motionless_[static_cast<size_t>(vehicle.getType())] = false;
      }
    }
  }
}

bool MotionlessnessChecker::IsVehicleMotionless(const model::Vehicle& vehicle, const int current_tick) const {
  return vehicle_coordinates_update_tick_by_id_[vehicle.getId()] < current_tick - kMotionCooldown;
}

bool MotionlessnessChecker::AreAllVehiclesOfTypeMotionless(const model::VehicleType& vehicle_type) const {
  return are_all_vehicles_of_type_motionless_[static_cast<size_t>(vehicle_type)];
}

bool MotionlessnessChecker::AreAllVehiclesOfTypeMotionless(const size_t vehicle_type_index) const {
  return are_all_vehicles_of_type_motionless_[vehicle_type_index];
}

bool MotionlessnessChecker::AreAllVehiclesOfTypesMotionless(const std::vector<model::VehicleType>& types) const {
  for (const model::VehicleType& type : types) {
    if (!AreAllVehiclesOfTypeMotionless(type)) {
      return false;
    }
  }
  return true;
}
