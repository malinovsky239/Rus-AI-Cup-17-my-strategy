#include "NuclearAttackHandler.h"

#include "Select.h"
#include "GoTo.h"
#include "NuclearStrike.h"

using model::Player;
using model::Vehicle;

using std::vector;

NuclearAttackHandler::NuclearAttackHandler(const std::map<long long, Vehicle>& vehicle_by_id,
                                           const std::shared_ptr<VehicleValueEstimator>& vehicle_value_estimator,
                                           const std::shared_ptr<RuntimeConstants>& runtime_constants,
                                           const std::shared_ptr<MotionlessnessChecker>& motionlessness_checker)
    : vehicle_by_id_(vehicle_by_id),
      vehicle_value_estimator_(vehicle_value_estimator),
      runtime_constants_(runtime_constants),
      motionlessness_checker_(motionlessness_checker) {
  representatives_in_fragment_ = vector<vector<Vehicle>>(
    runtime_constants->kFragmentsLinearCount,
    vector<Vehicle>(runtime_constants->kFragmentsLinearCount));
}

Vect NuclearAttackHandler::FindSquareWithLargestPotentialForNuclearStrike(const Player& me) {
  vector<vector<Vect>> sum_enemy_position_by_fragment(runtime_constants_->kFragmentsLinearCount,
    vector<Vect>(runtime_constants_->kFragmentsLinearCount));
  vector<vector<int>> cnt_enemies_by_fragment(runtime_constants_->kFragmentsLinearCount,
    vector<int>(runtime_constants_->kFragmentsLinearCount));
  have_representatives_in_fragment_ = vector<vector<bool>>(runtime_constants_->kFragmentsLinearCount,
    vector<bool>(runtime_constants_->kFragmentsLinearCount));
  force_balance_by_fragment_ = vector<vector<int>>(runtime_constants_->kFragmentsLinearCount,
    vector<int>(runtime_constants_->kFragmentsLinearCount));

  // consider all vehicles
  for (const auto& id_and_vehicle : vehicle_by_id_) {
    const Vehicle& vehicle = id_and_vehicle.second;

    // map vehicle position onto one of the World fragments
    int x_cell = int(vehicle.getX()) / runtime_constants_->kFragmentSideLength;
    int y_cell = int(vehicle.getY()) / runtime_constants_->kFragmentSideLength;
    Clamp(x_cell, 0, runtime_constants_->kFragmentsLinearCount - 1);
    Clamp(y_cell, 0, runtime_constants_->kFragmentsLinearCount - 1);
     
    force_balance_by_fragment_[x_cell][y_cell] += vehicle_value_estimator_->CalculateVehicleValue(vehicle, me);
        
    if (vehicle.getPlayerId() != me.getId()) {
      // include opponent's vehicle into mass center calculation
      sum_enemy_position_by_fragment[x_cell][y_cell] += Vect(vehicle);
      cnt_enemies_by_fragment[x_cell][y_cell]++;
    }
    else {
      // remember potential nuclear strike launcher for this fragment
      have_representatives_in_fragment_[x_cell][y_cell] = true;
      representatives_in_fragment_[x_cell][y_cell] = vehicle;
    }
  }

  int best_fragment_balance = 0;

  // default to bottom right corner (opponent's initial position)
  int best_fragment_x = runtime_constants_->kFragmentsLinearCount - 1;
  int best_fragment_y = runtime_constants_->kFragmentsLinearCount - 1;
  
  for (size_t i = 0; i < runtime_constants_->kFragmentsLinearCount; i++) {
    for (size_t j = 0; j < runtime_constants_->kFragmentsLinearCount; j++) {
      if (force_balance_by_fragment_[i][j] > best_fragment_balance) {
        best_fragment_balance = force_balance_by_fragment_[i][j];
        best_fragment_x = i;
        best_fragment_y = j;
      }
    }
  }

  if (cnt_enemies_by_fragment[best_fragment_x][best_fragment_y] > 0) {
    // return mass center of all enemies within the square with the best balance
    return sum_enemy_position_by_fragment[best_fragment_x][best_fragment_y] /
           cnt_enemies_by_fragment[best_fragment_x][best_fragment_y];
  }

  // return top-left corner of the square with the best balance
  return Vect(best_fragment_x * runtime_constants_->kFragmentSideLength,
              best_fragment_y * runtime_constants_->kFragmentSideLength);
}

void NuclearAttackHandler::TrySendingNuclearCrew(const Player& me, const int current_tick,
                                                 const Vect& launcher_position,
                                                 std::deque<std::unique_ptr<Action>>& actions) {
  // If nuclear strike will be allowed by the time when nuclear brigade reaches the target
  // and there's no planned actions (so we won't have to wait for long when time for the strike comes)
  if (me.getRemainingNuclearStrikeCooldownTicks() <= kTimeToDeliverNukes && actions.empty()) {
    // Don't send Nuclear Crew at the very beginning of the game. Give orders to other vehicles first!
    if (current_tick >= kEarliestNuclearCrewMissionTick &&
        motionlessness_checker_->AreAllVehiclesOfTypeMotionless(model::VehicleType::FIGHTER)) {
      
      // select units within a small rectangle with center at the selected Fighter position
      const Vect diagonal = kUnitVector * kNuclearLauncherSelectionSize;
      const Vect top_left = launcher_position - diagonal / 2;
      actions.push_back(std::make_unique<Select>(top_left, diagonal));

      const Vect point_to_strike = FindSquareWithLargestPotentialForNuclearStrike(me);
      actions.push_back(std::make_unique<GoTo>(point_to_strike - launcher_position));
    }
  }
}

void NuclearAttackHandler::TryNuclearStrike(const Player& me,
                                            std::deque<std::unique_ptr<Action>>& actions) const {
  if (me.getRemainingNuclearStrikeCooldownTicks() == 0) {
    // if the first action in the deque is Selection or the deque is Empty,
    // it means that we finished working with the currently selected troops,
    // so interfering with Nuclear Strike won't break
    // any of already existing plans for the currently selected troops
    if (actions.empty() || actions[0]->Name().find("Select") != std::string::npos) {
      vector<vector<bool>> tried_nuclear_strike_launcher(
        runtime_constants_->kDoubledFragmentsLinearCount,
        vector<bool>(runtime_constants_->kDoubledFragmentsLinearCount, false));
      int enemies_in_range_best_cnt = 0;
      int enemies_in_range_best_balance = 0;
      Vehicle best_launcher;
      Vect best_sum_position;

      // consider all my vehicles
      for (const auto& id_and_vehicle : vehicle_by_id_) {
        const Vehicle& launcher = id_and_vehicle.second;
        if (launcher.getPlayerId() == me.getId()) {
          // map vehicle position onto one of the large fragments
          const int x_cell = launcher.getX() / runtime_constants_->kDoubledFragmentSideLength;
          const int y_cell = launcher.getY() / runtime_constants_->kDoubledFragmentSideLength;
          
          // don't try more than one vehicle as a launcher in each large fragment
          // (it's a heuristic to speed up the launcher selection process)
          if (tried_nuclear_strike_launcher[x_cell][y_cell]) {
            continue;
          }
          tried_nuclear_strike_launcher[x_cell][y_cell] = true;
          
          int cnt = 0;
          int balance = 0;
          Vect sumPosition;

          // consider all vehicles (both mine and opponent's) that may be damaged by the nuclear strike
          for (const auto& id_and_target_vehicle : vehicle_by_id_) {
            const Vehicle target = id_and_target_vehicle.second;
            if (launcher.getDistanceTo(target) < launcher.getVisionRange() / 2) {
              balance += vehicle_value_estimator_->CalculateVehicleValue(target, me);
              if (target.getPlayerId() != me.getId()) {
                cnt++;
                sumPosition += Vect(target);
              }
            }
          }
          if (balance > enemies_in_range_best_balance) {
            enemies_in_range_best_cnt = cnt;
            enemies_in_range_best_balance = balance;
            best_launcher = launcher;
            best_sum_position = sumPosition;
          }
        }
      }
      // order nuclear strike with the best possible outcome for us, assuming that
      // - it hits the specified minimum number of enemy vehicles,
      // and
      // - the balance is positive.
      if (enemies_in_range_best_cnt >= kMinEnemiesCountDeservingNukes) {
        actions.push_front(
          std::make_unique<NuclearStrike>(best_sum_position / enemies_in_range_best_cnt, best_launcher.getId()));
      }
    }
  }
}

void NuclearAttackHandler::Clamp(int& x, const int l, const int r) const {
  if (x < l) x = l;
  if (x > r) x = r;
}
