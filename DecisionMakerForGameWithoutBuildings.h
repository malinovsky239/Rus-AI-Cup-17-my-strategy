#pragma once
#ifndef _DECISION_MAKER_FOR_GAME_WITHOUT_BUILDINGS_H_
#define _DECISION_MAKER_FOR_GAME_WITHOUT_BUILDINGS_H_

#include "DecisionMaker.h"
#include <memory>

class DecisionMakerForGameWithoutBuildings : public DecisionMaker {
 public:
  DecisionMakerForGameWithoutBuildings();
  void MakeDecisions(const Player& me, const World& world, const Game& game,
                     Move& move, std::deque<std::unique_ptr<Action>>& actions) override;

 private:
  // Used separately for both ground and aerial vehicles
  enum RegroupingStage {
    SHIFT_BY_Y,
    READY_FOR_SHIFT_BY_X,
    SHIFT_BY_X,
    READY_FOR_SCALING,
    SCALING,
    READY_FOR_ADJUSTMENT_BY_X,
    ADJUSTMENT_BY_X,
    READY_FOR_COLLAPSING,
    COLLAPSING,
    READY_FOR_ROTATING,
    ROTATING,
    READY_FOR_DESCALING,
    DESCALING,
    HAS_DENSE_FORMATION,
    READY_FOR_ATTACK
  };

  enum AirCrewState {
    INITIAL,
    READY,
    TO_ENEMY,
    FROM_ENEMY
  };

  void MakeRegroupingStageTransitions();

  bool AllMyVehiclesOfTypesOnSpecificRegroupingStage(const RegroupingStage& stage,
                                                     const std::vector<VehicleType>& types) const;
  bool AllMyGroundVehiclesOnSpecificRegroupingStage(const RegroupingStage& stage) const;
  bool AllMyAirVehiclesOnSpecificRegroupingStage(const RegroupingStage& stage) const;

  // gives approximate result!
  double DistanceBetweenMyAirVehiclesAndEnemyVehicles(const Player& me) const;

  // Imitates that a certain group of vehicles has just moved.
  // This function is useful when a group received an order but it is queued,
  // and therefore that group can still be treated as motionless
  // which is undesirable for Regrouping Stage Transitions
  void FakeLastUpdatedTickForMyVehiclesOfTypes(const Player& me, const int current_tick,
                                               const std::vector<VehicleType>& types);
  void FakeLastUpdatedTickForMyGroundVehicles(const Player& me, const int current_tick);
  void FakeLastUpdatedTickForAllMyVehicles(const Player& me, const int current_tick);

  // Maps relative coordinates (from 0.0 to 1.0) onto World coordinate system
  double RelToWorld(const double x) const;
  Vect RelToWorld(const Vect& x) const;

  const int kMainAirCrewMinUnits = 20; // Threshold used to tell if we look at the main group of aerial vehicles or not

  const double kRotationAngle = 0.785; // ~ 45 degrees
  const double kDescalingRatio = 0.1;

  // values relative to the playing field side
  const double kRelativeGroupSide = 0.15; // side of each of 5 initial groups of units
  const double kRelativeScaledGroupSide = 0.25;
  const Vect kRelativeGroupAdjustment = Vect(0.005, 0);
  const Vect kRelativeAirDestinations[2] = { Vect(0.35, 0.4), Vect(0.35, 0.2) };
  const double kRelativeLowestGroundDestination = 0.45;
  const double kRelativeGroundStep = 0.17;
  const double kRelativeGroundX = 0.125;
  const double kRelativeGroundY = 0.3; // used specifically for collapsing
  const double kRelativeRetreat = -0.1;
  const double kKillerGroupStep = 0.05;

  const int kKillerGroupUpdateFrequency = 100; // when all ground vehicles are sent into attack,
                                               // its destination should be updated regularly
  const double kGroundVehiclesSpeedLimit = 0.15; // when all ground vehicles are sent into attack,
                                                 // we don't won't slow ones to lag behind

  bool isHelicoptersDestinationAboveFighters;
  AirCrewState air_crew_state_ = INITIAL;
  std::vector<RegroupingStage> regrouping_stage_by_vehicle_type_;
};

#endif
