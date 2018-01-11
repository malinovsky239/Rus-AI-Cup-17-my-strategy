#pragma once
#ifndef _VEHICLE_INFO_H_
#define _VEHICLE_INFO_H_

#include "Strategy.h"
#include "Vect.h"
#include "Action.h"
#include "NuclearAttackHandler.h"
#include "VehicleValueEstimator.h"
#include "RuntimeConstants.h"
#include "MotionlessnessChecker.h"

#include <map>
#include <deque>
#include <vector>
#include <memory>

using namespace model;

// Core class for the entire strategy:
// - Interacts with helper classes
// (RuntimeConstants, MotionlessnessChecker, NuclearAttackHandler, and VehicleValueEstimator).
// - Connects MyStrategy (i.e. the entry point) and
// two classes (derived from this one) that define rules-specific strategies (with/without buildings).
// - Methods and fields defined here are used by both above-mentioned classes.
class DecisionMaker {
 public:
  virtual ~DecisionMaker() = default;

  // Derived classes define rules-specific strategies here
  virtual void MakeDecisions(const Player& me, const World& world, const Game& game,
                             Move& move, std::deque<std::unique_ptr<Action>>& actions) = 0;

  // Initializes classes
  void InitializeHelperClasses(const World& world, const Game& game);

  // Updates indicators of motionlessness for each type of vehicles
  void CheckMyVehiclesMotionlessness(const Player& me, const int current_tick) const;

  // Sends vehicles to launch nuclear strike and strikes when possible
  void NuclearOperations(const Player& me, const int current_tick,
                         std::deque<std::unique_ptr<Action>>& actions) const;

  // Processes all the information updates on all visible vehicles every tick
  // (these functions exist only because of the way that the game uses to inform players about updates)
  void AddNewVehicleInfo(const Vehicle& vehicle, const int current_tick);
  void UpdateVehicleInfo(const VehicleUpdate& vehicle_update, const int current_tick);

  // Returns required pause between two consecutive actions
  // (assuming that the player distributes action points evenly throughout the entire game duration)
  int BaseUniformActionInterval() const;

 protected:
  // Finds a position of the rightmost vehicle of specific type.
  // If there are several vehicles with X-coordinate close to the maximum,
  // picks the bottommost one
  Vect BottomRightVehiclePositionByType(const Player& me, const VehicleType& vehicle_type) const;

  // Finds a position of the enemy closest to the specified point
  Vect ClosestEnemyPosition(const Player& me, const Vect& anchor_point) const;

  // Returns mass center for specific group of vehicles (assuming that all vehicles have the same mass)
  Vect MassCenterForVehiclesByTypes(const Player& player, const std::vector<VehicleType>& types) const;
  Vect MassCenterForVehiclesByType(const Player& player, const VehicleType& vehicle_type) const;
  Vect MassCenterForGroundVehicles(const Player& player) const;

  const std::vector<VehicleType> kGroundVehicles = { VehicleType::ARRV, VehicleType::IFV, VehicleType::TANK };
  const std::vector<VehicleType> kAirVehicles = { VehicleType::FIGHTER, VehicleType::HELICOPTER };
  const std::vector<VehicleType> kAllVehicles = { VehicleType::ARRV, VehicleType::IFV, VehicleType::TANK, VehicleType::FIGHTER, VehicleType::HELICOPTER };

  const double kSmallEps = 1e-3;
  const double kLargeEps = 0.5;
  const double kInfiniteDistance = 1e5; // larger than any possible distance in this game's world

  // helper classes
  std::shared_ptr<NuclearAttackHandler> nuclear_attack_handler_;
  std::shared_ptr<VehicleValueEstimator> vehicle_value_estimator_;
  std::shared_ptr<RuntimeConstants> runtime_constants_;
  std::shared_ptr<MotionlessnessChecker> motionlesness_checker_;

  // states of all visible vehicles in the world
  std::map<long long, Vehicle> vehicle_by_id_;
  std::map<long long, int> update_tick_by_vehicle_id_; // anything (health/position) updated
  std::map<long long, Vect> vehicle_coordinates_by_id_;
  std::map<long long, int> vehicle_coordinates_update_tick_by_id_;
};

#endif
