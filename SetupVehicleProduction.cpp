#include "SetupVehicleProduction.h"

SetupVehicleProduction::SetupVehicleProduction(const long long factory_id, const model::VehicleType& vehicle_type)
    : factory_id_(factory_id), vehicle_type_(vehicle_type) {}

void SetupVehicleProduction::Execute(model::Move& move) const {
  move.setAction(model::ActionType::SETUP_VEHICLE_PRODUCTION);
  move.setFacilityId(factory_id_);
  move.setVehicleType(vehicle_type_);
}

std::string SetupVehicleProduction::Name() const {
  return "SetupVehicleProduction";
}
