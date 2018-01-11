#pragma once
#ifndef _SETUP_VEHICLE_PRODUCTION_H_
#define _SETUP_VEHICLE_PRODUCTION_H_

#include "Action.h"

// Starts producing vehicles of specified type on the factory with specified ID
class SetupVehicleProduction : public Action {
 public:
  SetupVehicleProduction(const long long factory_id, const model::VehicleType& vehicle_type);
  void Execute(model::Move& move) const override;
  std::string Name() const override;

 private:
  long long factory_id_;
  model::VehicleType vehicle_type_;
};

#endif