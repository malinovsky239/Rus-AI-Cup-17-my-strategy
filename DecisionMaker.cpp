#include "DecisionMaker.h"

#include "Action.h"
#include "GoTo.h"

#include <algorithm>

using std::vector;
using std::pair;
using std::make_pair;
using std::deque;

void DecisionMaker::InitializeHelperClasses(const World& world, const Game& game) {
  vehicle_value_estimator_ = std::make_shared<VehicleValueEstimator>();
  runtime_constants_ = std::make_shared<RuntimeConstants>(world, game);
  motionlesness_checker_ = std::make_shared<MotionlessnessChecker>(vehicle_by_id_, vehicle_coordinates_update_tick_by_id_,
                                                                   kAllVehicles.size());
  nuclear_attack_handler_ = std::make_shared<NuclearAttackHandler>(vehicle_by_id_, vehicle_value_estimator_,
                                                                   runtime_constants_, motionlesness_checker_);
}

void DecisionMaker::CheckMyVehiclesMotionlessness(const Player& me, const int current_tick) const {
  motionlesness_checker_->CheckMyVehiclesMotionlessness(me, current_tick);
}

void DecisionMaker::NuclearOperations(const Player& me, const int current_tick,
                                      deque<std::unique_ptr<Action>>& actions) const {
  const Vect bottom_right_figher = BottomRightVehiclePositionByType(me, VehicleType::FIGHTER);
  nuclear_attack_handler_->TrySendingNuclearCrew(me, current_tick, bottom_right_figher, actions);
  nuclear_attack_handler_->TryNuclearStrike(me, actions);
}

// Save the information about vehicles visible from the current tick
void DecisionMaker::AddNewVehicleInfo(const Vehicle& vehicle, const int current_tick) {
  const long long vehicle_id = vehicle.getId();
  vehicle_by_id_[vehicle_id] = vehicle;
  vehicle_coordinates_by_id_[vehicle_id] = Vect(vehicle);
  vehicle_coordinates_update_tick_by_id_[vehicle_id] = current_tick;
  update_tick_by_vehicle_id_[vehicle_id] = current_tick;
}

// Update the information about vehicles that we saw on previous tick as well
void DecisionMaker::UpdateVehicleInfo(const VehicleUpdate& vehicle_update, const int current_tick) {
  const long long vehicle_id = vehicle_update.getId();
  
  if (vehicle_update.getDurability() == 0) {
    // If the update tells that the vehicle was destroyed
    vehicle_by_id_.erase(vehicle_id);
    update_tick_by_vehicle_id_.erase(vehicle_id);
    vehicle_coordinates_by_id_.erase(vehicle_id);
    vehicle_coordinates_update_tick_by_id_.erase(vehicle_id);
  }
  else {
    // If the update tells that the vehicle's health and/or position changed
    vehicle_by_id_[vehicle_id] = Vehicle(vehicle_by_id_[vehicle_id], vehicle_update);
    update_tick_by_vehicle_id_[vehicle_id] = current_tick;
    const Vect& target = vehicle_coordinates_by_id_[vehicle_id];

    // Checks that the vehicle indeed moved after previous tick
    if (vehicle_by_id_[vehicle_id].getDistanceTo(target.x, target.y) > kSmallEps) {
      vehicle_coordinates_by_id_[vehicle_id] = Vect(vehicle_by_id_[vehicle_id]);
      vehicle_coordinates_update_tick_by_id_[vehicle_id] = current_tick;
    }
  }
}

int DecisionMaker::BaseUniformActionInterval() const {
  return runtime_constants_->kBaseUniformActionInterval;
}

Vect DecisionMaker::BottomRightVehiclePositionByType(const Player& me, const VehicleType& vehicle_type) const {
  Vect current_best;
  bool at_least_one_found = false;

  // Considering all my vehicles of desired type
  for (const auto& id_and_vehicle : vehicle_by_id_) {
    const Vehicle& vehicle = id_and_vehicle.second;
    if (vehicle.getPlayerId() == me.getId() && vehicle.getType() == vehicle_type) {
      if (!at_least_one_found) {
        // the first vehicle under consideration (it's indeed the rightmost one - for the moment)
        current_best = Vect(vehicle);
        at_least_one_found = true;
      }
      else {
        // if currently considered vehicle is far enough to the right from the temporary rightmost one
        if (current_best.x + kLargeEps < vehicle.getX() ||
            // or if it is not far enough to the left AND far enough downwards
            current_best.x < vehicle.getX() + kLargeEps && current_best.y + kLargeEps < vehicle.getY()) {
          current_best = Vect(vehicle);
        }
      }
    }
  }
  if (at_least_one_found) {
    return current_best;
  }
  return runtime_constants_->kWorldCenter; // it doesn't hurt to select anything in the center
                                           // if we haven't found desired vehicle
}

Vect DecisionMaker::ClosestEnemyPosition(const Player& me, const Vect& anchor_point) const {
  double shortest_distance = kInfiniteDistance;
  Vect best_target_position;
  for (const auto& id_and_vehicle : vehicle_by_id_) {
    const Vehicle& vehicle = id_and_vehicle.second;
    if (vehicle.getPlayerId() != me.getId()) {
      const Vect vehicle_position = Vect(vehicle);
      const double distance_to_vehicle = (vehicle_position - anchor_point).Length();
      if (distance_to_vehicle < shortest_distance) {
        best_target_position = vehicle_position;
        shortest_distance = distance_to_vehicle;
      }
    }
  }
  return best_target_position;
}

Vect DecisionMaker::MassCenterForVehiclesByTypes(const Player& player, const std::vector<VehicleType>& types) const {
  Vect sum_position;
  int cnt = 0;
  for (const auto& id_and_vehicle : vehicle_by_id_) {
    const Vehicle& vehicle = id_and_vehicle.second;
    if (vehicle.getPlayerId() == player.getId()) {
      bool matches_type = false;
      for (const VehicleType& desired_type : types) {
        if (vehicle.getType() == desired_type) {
          matches_type = true;
          break;
        }
      }
      if (matches_type) {
        sum_position += Vect(vehicle);
        cnt++;
      }
    }
  }
  if (cnt > 0) {
    return sum_position / cnt;
  }
  return runtime_constants_->kWorldCenter; // returns world center as "default" center of mass
                                           // for an empty group of vehicles
}

Vect DecisionMaker::MassCenterForVehiclesByType(const Player& player, const VehicleType& vehicle_type) const {
  const std::vector<VehicleType> desired_types = std::vector<VehicleType>{ vehicle_type };
  return MassCenterForVehiclesByTypes(player, desired_types);
}

Vect DecisionMaker::MassCenterForGroundVehicles(const Player& player) const {
  return MassCenterForVehiclesByTypes(player, kGroundVehicles);
}
