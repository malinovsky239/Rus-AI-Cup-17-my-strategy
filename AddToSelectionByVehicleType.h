#pragma once
#ifndef _ADD_TO_SELECTION_BY_VEHICLE_TYPE_H_
#define _ADD_TO_SELECTION_BY_VEHICLE_TYPE_H_

#include "Action.h"

// Adds all vehicles of specific type to selection
class AddToSelectionByVehicleType : public Action {
 public:
  AddToSelectionByVehicleType(const model::VehicleType& vehicle_type, const int world_side_length);
  void Execute(model::Move& move) const override;
  std::string Name() const override;

 private:
  model::VehicleType vehicle_type_;
  int world_side_length_;
};

#endif