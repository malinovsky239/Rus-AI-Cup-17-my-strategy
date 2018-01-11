#include "AddToSelectionByVehicleType.h"
#include "MyStrategy.h"

AddToSelectionByVehicleType::AddToSelectionByVehicleType(const model::VehicleType& vehicle_type,
                                                         const int world_side_length)
    : vehicle_type_(vehicle_type), world_side_length_(world_side_length) {}

void AddToSelectionByVehicleType::Execute(model::Move& move) const {
  move.setAction(model::ActionType::ADD_TO_SELECTION);
  move.setLeft(0);
  move.setTop(0);
  move.setRight(world_side_length_);
  move.setBottom(world_side_length_);
  move.setVehicleType(vehicle_type_);
}

std::string AddToSelectionByVehicleType::Name() const {
  return "AddToSelectionByVehicleType";
}
