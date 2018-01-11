#pragma once
#ifndef _DECISION_MAKER_FOR_GAME_WITH_BUILDINGS_H_
#define _DECISION_MAKER_FOR_GAME_WITH_BUILDINGS_H_

#include "DecisionMaker.h"
#include <memory>

class DecisionMakerForGameWithBuildings : public DecisionMaker {
 public:  
  void MakeDecisions(const Player& me, const World& world, const Game& game,
                     Move& move, std::deque<std::unique_ptr<Action>>& actions) override;

 private:
  bool IsPositionInsideFacility(const Vect& pos, const Facility& facility, const Game& game) const;
  double DistanceBetweenFacilities(const Facility& facility1, const Facility& facility2) const;
  bool IsAirVehicle(const Vehicle& vehicle) const;

  // Finds bounding rectangle for <size+> vehicles of specified type that
  // haven't moved yet and are as close as possible to a specified anchor point.
  // Returns coordinates of the top left and bottom right corners.
  std::pair<Vect, Vect> BoundsForMultipleUnitsClosestToPoint(const Player& me, const VehicleType& vehicle_type,
                                                             const Vect& anchor_point, size_t size);

  // Constants describing strategy's initial stage timeline (sending brigades to occupy buildings).
  // Here's a brief overview of the initial stage: it consists of 3 similar iterations.
  // On each iteration, we consider each type of ground vehicles one by one.
  // For each type, we perform the following operation 5 times:
  // - select small brigade containing units of the current type,
  // - send this brigade to occupy a facility.
  // Between each pair of consecutive operation there is the same interval.
  const int kLaunchIterations = 3;
  const int kLaunchIterationDuration = 600;
  const int kLaunchInterval = 40;
  const int kBrigadeSize = 10;

  const size_t kMaxDequeSizeToOrderRelocation = 5;

  const int kRelocateOrdersInterval = 100;
  const int kMinTroopsToTouchOutsideFacilities = 5;
  const int kMinTroopsSizeToRelocateFromFactory = 50;
  const int kMinTroopsSizeToAttackEnemy = 20;

  const int kHelicoptersStartTick = 1500;
  const int kHelicoptersSwitchInterval = 500;

  std::map<long long, bool> is_facility_mine_;

  // Helps to determine initial relative positions of different types of vehicles
  std::vector<std::pair<std::pair<double, double>, VehicleType>> vehicle_type_representatives_positions_;
};

#endif
