#pragma once
#ifndef _NUCLEAR_ATTACK_HANDLER_H_
#define _NUCLEAR_ATTACK_HANDLER_H_

#include "Vect.h"
#include "Action.h"
#include "VehicleValueEstimator.h"
#include "RuntimeConstants.h"
#include "MotionlessnessChecker.h"
#include <deque>
#include <map>
#include <vector>
#include <memory>

class NuclearAttackHandler {
 public:
  NuclearAttackHandler(const std::map<long long, model::Vehicle>& vehicle_by_id,
                       const std::shared_ptr<VehicleValueEstimator>& vehicle_value_estimator,
                       const std::shared_ptr<RuntimeConstants>& runtime_constants,
                       const std::shared_ptr<MotionlessnessChecker>& motionlessness_checker);

  // Subdivides the world into <Length-of-the-world-side> equal squares.
  // Chooses the one where the nuclear strike will be the most effective (more damage for opponent, less damage for us).
  // Returns coordinates of a point inside that square.
  Vect FindSquareWithLargestPotentialForNuclearStrike(const model::Player& me);

  // If nuclear strike is possible soon,
  // selects a small group of fighters and
  // sends them to the square with the largest potential for nuclear strike.
  void TrySendingNuclearCrew(const model::Player& me, const int current_tick,
                             const Vect& launcher_position, std::deque<std::unique_ptr<Action>>& actions);

  // If nuclear strike is possible right now and will bring enough damage,
  // this method orders it immediately.
  void TryNuclearStrike(const model::Player& me, std::deque<std::unique_ptr<Action>>& actions) const;

 private:
  // Changes `x` to the closest integer from [l; r] segment
  void Clamp(int& x, const int l, const int r) const;

  const int kEarliestNuclearCrewMissionTick = 100;
  const int kTimeToDeliverNukes = 50;
  const int kMinEnemiesCountDeservingNukes = 10;

  const int kNuclearLauncherSelectionSize = 30;

  const std::map<long long, model::Vehicle>& vehicle_by_id_;

  std::vector<std::vector<model::Vehicle>> representatives_in_fragment_;
  std::vector<std::vector<bool>> have_representatives_in_fragment_;
  std::vector<std::vector<int>> force_balance_by_fragment_;

  std::shared_ptr<VehicleValueEstimator> vehicle_value_estimator_;
  const std::shared_ptr<RuntimeConstants> runtime_constants_;
  const std::shared_ptr<MotionlessnessChecker> motionlessness_checker_;
};

#endif
