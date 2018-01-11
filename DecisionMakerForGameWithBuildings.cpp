#include "DecisionMakerForGameWithBuildings.h"

#include "Select.h"
#include "GoTo.h"
#include "Scale.h"
#include "SelectByVehicleType.h"
#include "SetupVehicleProduction.h"

#include <algorithm>
#include <vector>

using std::vector;
using std::deque;
using std::pair;

void DecisionMakerForGameWithBuildings::MakeDecisions(const Player& me, const World& world, const Game& game,
                                                      Move& move, std::deque<std::unique_ptr<Action>>& actions) {
  const int current_tick = world.getTickIndex();

  if (current_tick == 0) {
    // Orders initial relative positions of different types of vehicles
    // so that we know in which order we should send them to occupy buildings
    for (const VehicleType& type : kGroundVehicles) {
      const Vect pos = BottomRightVehiclePositionByType(me, type);
      vehicle_type_representatives_positions_.emplace_back(std::make_pair(pos.x, pos.y), type);
    }
    std::sort(vehicle_type_representatives_positions_.begin(), vehicle_type_representatives_positions_.end());
    reverse(vehicle_type_representatives_positions_.begin(), vehicle_type_representatives_positions_.end());
  }

  const vector<Facility>& facilities = world.getFacilities();

  if (current_tick < kLaunchIterationDuration * kLaunchIterations && current_tick % kLaunchInterval == 0) {
    const int number_of_facilities = facilities.size();
    // Determines vehicle type which order is now
    const int continuous_same_type_launches_duration = kLaunchIterationDuration / kGroundVehicles.size();
    const VehicleType& type = vehicle_type_representatives_positions_[
      (current_tick % kLaunchIterationDuration) / continuous_same_type_launches_duration].second;

    // Sets random facility as a destination
    const Facility& target_facility = facilities[rand() % number_of_facilities];
    const Vect destination = Vect(target_facility.getLeft() + game.getFacilityWidth() / 2,
                                  target_facility.getTop() + game.getFacilityHeight() / 2);

    // Selects vehicles of specific type
    // closest to destination
    // and sends them to occupy the facility
    const pair<Vect, Vect> bounds = BoundsForMultipleUnitsClosestToPoint(me, type, destination, kBrigadeSize);
    actions.push_back(std::make_unique<Select>(bounds.first, bounds.second - bounds.first));
    const Vect selected_group_position = (bounds.first + bounds.second) / 2;
    actions.push_back(std::make_unique<GoTo>(destination - selected_group_position));
  }

  for (const Facility& facility : facilities) {
    const long long facility_id = facility.getId();
    if (is_facility_mine_.count(facility_id) == 0) {
      is_facility_mine_[facility_id] = false;
    }
    // Detects recently occupied factories
    if (facility.getOwnerPlayerId() == me.getId()) {
      if (!is_facility_mine_[facility_id]) {
        is_facility_mine_[facility_id] = true;
        if (facility.getType() == FacilityType::VEHICLE_FACTORY) {
          // and starts producing tanks there
          actions.push_back(std::make_unique<SetupVehicleProduction>(facility_id, VehicleType::TANK));
        }
      }
    }
    else {
      is_facility_mine_[facility_id] = false;
    }
  }

  // If initial stage is over (i.e. all ground vehicles were given orders)
  // and there's not too many planned actions (if too many relocation requests are queued,
  // deque becomes polluted with meaningless duplicate orders, and they block nuclear strikes in turn)
  if (current_tick >= kLaunchIterationDuration * kLaunchIterations &&
    actions.size() < kMaxDequeSizeToOrderRelocation) {
    bool found_starting_point = false;
    Vect starting_point;
    Vect starting_selection_top_left;
    Vect starting_selection_diagonal;
    int cnt_selected_units = 0;
    bool is_selection_outside_facilities = false;

    if (current_tick % kRelocateOrdersInterval == 0) {
      // Finds the fragment with the largest number of ground vehicles standing outside all facilities
      vector<vector<int>> vehicles_outside_facilities_count(
        runtime_constants_->kFragmentsLinearCount,
        vector<int>(runtime_constants_->kFragmentsLinearCount));
      for (const auto& id_and_vehicle : vehicle_by_id_) {
        const Vehicle& vehicle = id_and_vehicle.second;
        if (vehicle.getPlayerId() == me.getId() && !IsAirVehicle(vehicle) &&
            motionlesness_checker_->IsVehicleMotionless(vehicle, current_tick)) {
          bool outside_all_facilities = true;
          for (const Facility& facility : facilities) {
            if (IsPositionInsideFacility(Vect(vehicle), facility, game)) {
              outside_all_facilities = false;
              break;
            }
          }
          if (outside_all_facilities) {
            const int x = static_cast<int>(vehicle.getX()) / runtime_constants_->kFragmentSideLength;
            const int y = static_cast<int>(vehicle.getY()) / runtime_constants_->kFragmentSideLength;
            vehicles_outside_facilities_count[x][y]++;
          }
        }
      }
      int mx_vehicles_outside_facilities_count = 0;
      int best_fragment_x = 0, best_fragment_y = 0;
      for (size_t i = 0; i < runtime_constants_->kFragmentsLinearCount; i++) {
        for (size_t j = 0; j < runtime_constants_->kFragmentsLinearCount; j++) {
          if (vehicles_outside_facilities_count[i][j] > mx_vehicles_outside_facilities_count) {
            mx_vehicles_outside_facilities_count = vehicles_outside_facilities_count[i][j];
            best_fragment_x = i;
            best_fragment_y = j;
          }
        }
      }

      if (mx_vehicles_outside_facilities_count > kMinTroopsToTouchOutsideFacilities) {
        // Decides to select that fragment
        starting_selection_top_left = Vect(best_fragment_x * runtime_constants_->kFragmentSideLength,
                                           best_fragment_y * runtime_constants_->kFragmentSideLength);
        starting_selection_diagonal = kUnitVector * runtime_constants_->kFragmentSideLength;
        found_starting_point = true;
        cnt_selected_units = mx_vehicles_outside_facilities_count;
        is_selection_outside_facilities = true;
      }
    }

    // Otherwise relocate troops from occupied facilities
    if (!found_starting_point) {
      // Start from different facilities (depending on tick index) so that all factories are evenly occupied
      const size_t end_index = (current_tick / runtime_constants_->kBaseUniformActionInterval) % facilities.size();
      const size_t start_index = (end_index + 1) % facilities.size();
      for (size_t i = start_index;; i = (i + 1) % facilities.size()) {
        const Facility& facility = facilities[i];
        if (facility.getOwnerPlayerId() == me.getId()) {
          int cnt_my_units = 0;
          for (const auto& id_and_vehicle : vehicle_by_id_) {
            if (IsPositionInsideFacility(Vect(id_and_vehicle.second), facility, game)) {
              cnt_my_units++;
            }
          }

          // Stops if found ANY troops in a Control Center (can leave immediately)
          // or ENOUGH troops in a Factory
          if ((facility.getType() == FacilityType::CONTROL_CENTER && cnt_my_units > 0) ||
              cnt_my_units > kMinTroopsSizeToRelocateFromFactory) {
            // Selects entire facility
            starting_selection_top_left = Vect(facility.getLeft(), facility.getTop());
            starting_selection_diagonal = Vect(game.getFacilityWidth(), game.getFacilityHeight());
            found_starting_point = true;
            cnt_selected_units = cnt_my_units;
            break;
          }
        }
        if (i == end_index) {
          break; // All facilities have been considered exactly once
        }
      }
    }

    // If found free units to select, chooses where to send them
    if (found_starting_point) {
      starting_point = starting_selection_top_left + starting_selection_diagonal / 2;

      bool found_destination = false;
      Vect next_destination;
      Facility best_facility;
      double min_dist = kInfiniteDistance;

      // First tries to send them to unoccupied facility
      for (const Facility& other_facility : world.getFacilities()) {
        if (other_facility.getOwnerPlayerId() != me.getId() &&
            other_facility.getOwnerPlayerId() != world.getOpponentPlayer().getId()) {
          Vect path = Vect(other_facility) - starting_point;
          if (path.Length() < min_dist) {
            min_dist = path.Length();
            best_facility = other_facility;
            found_destination = true;
          }
        }
      }
      if (!found_destination) {
        // If there is no such facility, considers enemy's facilities
        for (const Facility& other_facility : world.getFacilities()) {
          if (other_facility.getOwnerPlayerId() == world.getOpponentPlayer().getId()) {
            Vect path = Vect(other_facility) - starting_point;
            if (path.Length() < min_dist) {
              min_dist = path.Length();
              best_facility = other_facility;
              found_destination = true;
            }
          }
        }
      }
      if (found_destination) {
        next_destination = Vect(best_facility.getLeft(), best_facility.getTop()) +
                           Vect(game.getFacilityWidth() / 2, game.getFacilityHeight() / 2);
      }
      else if (cnt_selected_units > kMinTroopsSizeToAttackEnemy) {
        // Decides to attack the enemy if selected troops are strong enough
        found_destination = true;
        next_destination = ClosestEnemyPosition(me, starting_point);
      }

      if (found_destination) {
        actions.push_back(std::make_unique<Select>(starting_selection_top_left, starting_selection_diagonal));
        if (is_selection_outside_facilities) {
          // If selected units are standing outside all facilities,
          // chances are high that they block each other. So let's give them twice more space!
          actions.push_back(std::make_unique<Scale>(2, starting_point));
        }
        actions.push_back(std::make_unique<GoTo>(next_destination - starting_point));
      }
    }
  }

  // Sends helicopters patrolling between facilities
  if (current_tick > kHelicoptersStartTick && current_tick % kHelicoptersSwitchInterval == 0) {
    actions.push_back(std::make_unique<SelectByVehicleType>(VehicleType::HELICOPTER, runtime_constants_->kWorldSideLength));
    const int number_of_facilities = world.getFacilities().size();
    const Facility& target_facility = world.getFacilities()[rand() % number_of_facilities];
    const Vect cur_pos = MassCenterForVehiclesByType(me, VehicleType::HELICOPTER);
    const Vect target_pos = Vect(target_facility.getLeft(), target_facility.getTop()) +
                            Vect(game.getFacilityWidth() / 2, game.getFacilityHeight() / 2);
    actions.push_back(std::make_unique<GoTo>(target_pos - cur_pos));
  }
}

bool DecisionMakerForGameWithBuildings::IsPositionInsideFacility(const Vect& pos, const Facility& facility,
                                                                 const Game& game) const {
  const Vect top_left = Vect(facility);
  const Vect bottom_right = top_left + Vect(game.getFacilityWidth(), game.getFacilityHeight());
  return top_left.x < pos.x && pos.x < bottom_right.x && top_left.y < pos.y && pos.y < bottom_right.y;
}

double DecisionMakerForGameWithBuildings::DistanceBetweenFacilities(const Facility& facility1,
                                                                    const Facility& facility2) const {
  const Vect path = Vect(facility1) - Vect(facility2);
  return path.Length();
}

bool DecisionMakerForGameWithBuildings::IsAirVehicle(const Vehicle& vehicle) const {
  for (const VehicleType& vehicle_type : kAirVehicles) {
    if (vehicle_type == vehicle.getType()) {
      return true;
    }
  }
  return false;
}

pair<Vect, Vect> DecisionMakerForGameWithBuildings::BoundsForMultipleUnitsClosestToPoint(
    const Player& me,
    const VehicleType& vehicle_type,
    const Vect& anchor_point,
    size_t size) {
  vector<pair<double, long long>> potential_group_members; // {distance to anchor point; ID}

  // add my vehicles of desired type that haven't moved yet to the above-defined vector
  for (const auto& id_and_vehicle : vehicle_by_id_) {
    const Vehicle& vehicle = id_and_vehicle.second;
    if (vehicle.getPlayerId() == me.getId() && vehicle.getType() == vehicle_type &&
      vehicle_coordinates_update_tick_by_id_[vehicle.getId()] == 0) {
      potential_group_members.emplace_back((Vect(vehicle) - anchor_point).Length(), vehicle.getId());
    }
  }

  // chooses <size> closest vehicles (or less if vector size is smaller) 
  size = std::min(size, potential_group_members.size());
  std::sort(potential_group_members.begin(), potential_group_members.end());
  potential_group_members.resize(size);

  // initializes bounds for the rectangle
  double min_x = runtime_constants_->kWorldSideLength, max_x = 0;
  double min_y = runtime_constants_->kWorldSideLength, max_y = 0;

  // updates bounds
  for (const auto& potential_group_member : potential_group_members) {
    const Vect vehicle_position = Vect(vehicle_by_id_[potential_group_member.second]);
    min_x = std::min(min_x, vehicle_position.x);
    max_x = std::max(max_x, vehicle_position.x);
    min_y = std::min(min_y, vehicle_position.y);
    max_y = std::max(max_y, vehicle_position.y);
  }

  return std::make_pair(Vect(min_x, min_y), Vect(max_x, max_y));
}
