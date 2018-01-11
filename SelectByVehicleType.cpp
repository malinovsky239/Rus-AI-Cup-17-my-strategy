#include "SelectByVehicleType.h"
#include "MyStrategy.h"

SelectByVehicleType::SelectByVehicleType(const model::VehicleType& vehicle_type, const int world_side_length)
    : vehicle_type_(vehicle_type), world_side_length_(world_side_length) {}

void SelectByVehicleType::Execute(model::Move& move) const {
  move.setAction(model::ActionType::CLEAR_AND_SELECT);
  move.setLeft(0);
  move.setTop(0);
  move.setRight(world_side_length_);
  move.setBottom(world_side_length_);
  move.setVehicleType(vehicle_type_);
}

std::string SelectByVehicleType::Name() const {
  return "SelectByVehicleType";
}
