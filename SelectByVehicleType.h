#pragma once
#ifndef _SELECT_BY_VEHICLE_TYPE_H_
#define _SELECT_BY_VEHICLE_TYPE_H_

#include "Action.h"

// Selects all vehicles of a specific type
class SelectByVehicleType : public Action {
 public:
  SelectByVehicleType(const model::VehicleType& vehicle_type, const int world_side_length);
  void Execute(model::Move& move) const override;
  std::string Name() const override;

 private:
  model::VehicleType vehicle_type_;
  int world_side_length_;
};

#endif