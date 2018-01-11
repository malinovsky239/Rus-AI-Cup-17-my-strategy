#include "DecisionMakerForGameWithoutBuildings.h"

#include "GoTo.h"
#include "GoToWithSpeedLimit.h"
#include "Select.h"
#include "SelectByVehicleType.h"
#include "AddToSelectionByVehicleType.h"
#include "Scale.h"
#include "Rotate.h"

#include <algorithm>

using std::vector;
using std::pair;

DecisionMakerForGameWithoutBuildings::DecisionMakerForGameWithoutBuildings() {
  regrouping_stage_by_vehicle_type_ = std::vector<RegroupingStage>(kAllVehicles.size());
}

void DecisionMakerForGameWithoutBuildings::MakeDecisions(const Player& me, const World& world, const Game& game,
                                                         Move& move, std::deque<std::unique_ptr<Action>>& actions) {
  // advances motionless vehicles to the next regrouping stage
  MakeRegroupingStageTransitions();

  const int current_tick = world.getTickIndex();

  // Groups ground and air vehicles in different regions of the map
  // on the initial step
  if (current_tick == 0) {
    // Places air vehicles directly below each other
    // leaving space between them to scale each group of 100 units later
    const Vect fighter_mass_center = MassCenterForVehiclesByType(me, VehicleType::FIGHTER);
    const Vect helicopter_mass_center = MassCenterForVehiclesByType(me, VehicleType::HELICOPTER);
    Vect helicopter_destination = RelToWorld(kRelativeAirDestinations[0]);
    Vect fighter_destination = RelToWorld(kRelativeAirDestinations[1]);
    isHelicoptersDestinationAboveFighters = false;
    if (fighter_mass_center.y > helicopter_mass_center.y) {
      // Vehicles shouldn't overtake each other (they may get stuck otherwise),
      // so the group which is initially lower should go to a lower destination point
      std::swap(fighter_destination, helicopter_destination);
      isHelicoptersDestinationAboveFighters = true;
    }
    actions.push_back(std::make_unique<SelectByVehicleType>(VehicleType::FIGHTER, RelToWorld(1.0)));
    actions.push_back(std::make_unique<GoTo>(fighter_destination - fighter_mass_center));
    actions.push_back(std::make_unique<SelectByVehicleType>(VehicleType::HELICOPTER, RelToWorld(1.0)));
    actions.push_back(std::make_unique<GoTo>(helicopter_destination - helicopter_mass_center));

    // Determines vertical order of different types of ground vehicles,
    // so that they won't get stuck while regrouping
    vector<pair<double, VehicleType>> order;
    for (const VehicleType& vehicle_type : kGroundVehicles) {
      Vect mass_center = MassCenterForVehiclesByType(me, vehicle_type);
      order.emplace_back(mass_center.y, vehicle_type);
    }
    std::sort(order.begin(), order.end());
    std::reverse(order.begin(), order.end());

    // Places ground vehicles of different types on different vertical levels,
    // the lower the group was initially, farther it goes
    for (size_t i = 0; i < order.size(); i++) {
      const double y = order[i].first;
      const VehicleType ground_vehicle_type = order[i].second;
      actions.push_back(std::make_unique<SelectByVehicleType>(ground_vehicle_type, RelToWorld(1.0)));
      actions.push_back(std::make_unique<GoTo>(
        Vect(0, RelToWorld(kRelativeLowestGroundDestination - kRelativeGroundStep * i) - y)));
    }
  }

  // Regroups air vehicles
  if (motionlesness_checker_->AreAllVehiclesOfTypeMotionless(VehicleType::HELICOPTER) &&
      air_crew_state_ == INITIAL) {
    // Prevents air vehicles from switching regrouping stages twice in a row
    FakeLastUpdatedTickForMyVehiclesOfTypes(me, current_tick, { VehicleType::HELICOPTER });
    // Gives new orders to air vehicles
    switch (regrouping_stage_by_vehicle_type_[static_cast<size_t>(VehicleType::HELICOPTER)]) {
      case READY_FOR_SHIFT_BY_X: {
        // No need in shift by X, so proceed to the next step right away:
        // scales both groups of air units (to make collapsing possible)
        for (int i = 0; i < 2; i++) {
          const Vect center = RelToWorld(kRelativeAirDestinations[i]);
          const Vect diagonal = kUnitVector * RelToWorld(kRelativeGroupSide);
          actions.push_back(std::make_unique<Select>(center - diagonal / 2, diagonal));
          actions.push_back(std::make_unique<Scale>(2, center));
        }
        regrouping_stage_by_vehicle_type_[static_cast<size_t>(VehicleType::HELICOPTER)] = SCALING;
        break;
      }

      case READY_FOR_ADJUSTMENT_BY_X: {
        // Shifts helicopters a bit so that they can fly into the gaps between fighters
        actions.push_back(std::make_unique<SelectByVehicleType>(VehicleType::HELICOPTER, RelToWorld(1.0)));
        actions.push_back(std::make_unique<GoTo>(RelToWorld(kRelativeGroupAdjustment)));
        regrouping_stage_by_vehicle_type_[static_cast<size_t>(VehicleType::HELICOPTER)] = ADJUSTMENT_BY_X;
        break;
      }

      case READY_FOR_COLLAPSING: {
        // Moves helicopters into the gaps between fighters
        actions.push_back(std::make_unique<SelectByVehicleType>(VehicleType::HELICOPTER, RelToWorld(1.0)));
        Vect diff = RelToWorld(kRelativeAirDestinations[0] - kRelativeAirDestinations[1]);
        if (!isHelicoptersDestinationAboveFighters) {
          diff *= -1;
        }
        actions.push_back(std::make_unique<GoTo>(diff));
        regrouping_stage_by_vehicle_type_[static_cast<size_t>(VehicleType::HELICOPTER)] = COLLAPSING;
        break;
      }

      case READY_FOR_ROTATING: {
        // Rotates mixed group of helicopters and fighters by 45 degrees (towards the enemy initial location)
        const size_t center_index = isHelicoptersDestinationAboveFighters ? 0 : 1;
        const Vect center = RelToWorld(kRelativeAirDestinations[center_index]);
        const Vect diagonal = kUnitVector * RelToWorld(kRelativeGroupSide);
        actions.push_back(std::make_unique<Select>(center - diagonal / 2, diagonal));
        actions.push_back(std::make_unique<Rotate>(kRotationAngle, center));
        regrouping_stage_by_vehicle_type_[static_cast<size_t>(VehicleType::HELICOPTER)] = ROTATING;
        break;
      }

      case READY_FOR_DESCALING: {
        // Makes mixed group of helicopters and fighters more dense
        const size_t center_index = isHelicoptersDestinationAboveFighters ? 0 : 1;
        const Vect center = RelToWorld(kRelativeAirDestinations[center_index]);
        const Vect diagonal = kUnitVector * RelToWorld(kRelativeGroupSide);
        actions.push_back(std::make_unique<Select>(center - diagonal / 2, diagonal));
        actions.push_back(std::make_unique<Scale>(kDescalingRatio, center));
        regrouping_stage_by_vehicle_type_[static_cast<size_t>(VehicleType::HELICOPTER)] = DESCALING;
      }

      case HAS_DENSE_FORMATION: {
        // Switches from regrouping to approach-retreat strategy
        air_crew_state_ = FROM_ENEMY;
      }
    }
  }

  if (air_crew_state_ == TO_ENEMY &&
      DistanceBetweenMyAirVehiclesAndEnemyVehicles(me) < game.getFighterAerialAttackRange()) {
    // If some of my aerial vehicles are close enough to attack the enemy, starts retreating
    actions.push_back(std::make_unique<SelectByVehicleType>(VehicleType::HELICOPTER, RelToWorld(1.0)));
    actions.push_back(std::make_unique<AddToSelectionByVehicleType>(VehicleType::FIGHTER, RelToWorld(1.0)));
    actions.push_back(std::make_unique<GoToWithSpeedLimit>(kUnitVector * RelToWorld(kRelativeRetreat),
                                                           game.getHelicopterSpeed()));
    air_crew_state_ = FROM_ENEMY;
  }
  if (air_crew_state_ == FROM_ENEMY &&
      DistanceBetweenMyAirVehiclesAndEnemyVehicles(me) > game.getFighterVisionRange()) {
    // If none of my aerial vehicles see the enemy, starts approaching
    // to the spot with maximum cumulative value
    actions.push_back(std::make_unique<SelectByVehicleType>(VehicleType::HELICOPTER, RelToWorld(1.0)));
    actions.push_back(std::make_unique<AddToSelectionByVehicleType>(VehicleType::FIGHTER, RelToWorld(1.0)));
    actions.push_back(std::make_unique<GoToWithSpeedLimit>(
      nuclear_attack_handler_->FindSquareWithLargestPotentialForNuclearStrike(me) -
      MassCenterForVehiclesByTypes(me, { VehicleType::HELICOPTER, VehicleType::FIGHTER }),
      game.getHelicopterSpeed()));
    air_crew_state_ = TO_ENEMY;
  }

  // Regroups ground vehicles
  const RegroupingStage arrv_stage = regrouping_stage_by_vehicle_type_[static_cast<size_t>(VehicleType::ARRV)];
  if (AllMyGroundVehiclesOnSpecificRegroupingStage(arrv_stage) &&
      motionlesness_checker_->AreAllVehiclesOfTypesMotionless(kGroundVehicles)) {
    FakeLastUpdatedTickForMyGroundVehicles(me, current_tick);
    switch (arrv_stage) {
      case READY_FOR_SHIFT_BY_X: {
        // Moves ground vehicles horizontally so that they end up directly below each other
        for (const VehicleType& vehicle_type : kGroundVehicles) {
          actions.push_back(std::make_unique<SelectByVehicleType>(vehicle_type, RelToWorld(1.0)));
          const Vect mass_center = MassCenterForVehiclesByType(me, vehicle_type);
          actions.push_back(std::make_unique<GoTo>(Vect(RelToWorld(kRelativeGroundX) - mass_center.x, 0)));
          regrouping_stage_by_vehicle_type_[static_cast<int>(vehicle_type)] = SHIFT_BY_X;
        }
        break;
      }

      case READY_FOR_SCALING: {
        // Scales ground vehicle groups, so that they can infiltrate each other
        for (const VehicleType& vehicle_type : kGroundVehicles) {
          regrouping_stage_by_vehicle_type_[static_cast<size_t>(vehicle_type)] = SCALING;
          actions.push_back(std::make_unique<SelectByVehicleType>(vehicle_type, RelToWorld(1.0)));
          actions.push_back(std::make_unique<Scale>(kGroundVehicles.size(), MassCenterForVehiclesByType(me, vehicle_type)));
        }
        break;
      }

      case READY_FOR_ADJUSTMENT_BY_X: {
        // Shift ground vehicle groups, so that they can infiltrate each other by moving vertically
        for (const VehicleType& vehicle_type : kGroundVehicles) {
          regrouping_stage_by_vehicle_type_[static_cast<size_t>(vehicle_type)] = ADJUSTMENT_BY_X;
          actions.push_back(std::make_unique<SelectByVehicleType>(vehicle_type, RelToWorld(1.0)));
          const Vect adjustment = RelToWorld(kRelativeGroupAdjustment) *
            std::max(0, int(vehicle_type) - 2); // 0 for VehicleType(0), 1 for VehicleType(3) and 2 for VehicleType(4)
                                                // because VehicleType(1) and VehicleType(2) stand for air vehicles          
          actions.push_back(std::make_unique<GoTo>(adjustment));
        }
        break;
      }

      case READY_FOR_COLLAPSING: {
        // Merges groups of ground vehicles
        for (const VehicleType& vehicle_type : kGroundVehicles) {
          regrouping_stage_by_vehicle_type_[static_cast<size_t>(vehicle_type)] = COLLAPSING;
          actions.push_back(std::make_unique<SelectByVehicleType>(vehicle_type, RelToWorld(1.0)));
          const Vect mass_center = MassCenterForVehiclesByType(me, vehicle_type);
          actions.push_back(std::make_unique<GoTo>(Vect(0, RelToWorld(kRelativeGroundY) - mass_center.y)));
        }
        break;
      }

      case READY_FOR_ROTATING: {
        // Rotates groups of ground vehicles so that descaling will be more effective
        const Vect center = RelToWorld(Vect(kRelativeGroundX, kRelativeGroundY));
        const Vect diagonal = kUnitVector * RelToWorld(kRelativeScaledGroupSide);
        actions.push_back(std::make_unique<Select>(center - diagonal / 2, diagonal));
        actions.push_back(std::make_unique<Rotate>(kRotationAngle, center));
        for (const VehicleType& vehicle_type : kGroundVehicles) {
          regrouping_stage_by_vehicle_type_[static_cast<size_t>(vehicle_type)] = ROTATING;
        }
        break;
      }

      case READY_FOR_DESCALING: {
        // Makes group of ground vehicles more dense
        const Vect center = RelToWorld(Vect(kRelativeGroundX, kRelativeGroundY));
        const Vect diagonal = kUnitVector * RelToWorld(kRelativeScaledGroupSide);
        actions.push_back(std::make_unique<Select>(center - diagonal / 2, diagonal));
        actions.push_back(std::make_unique<Scale>(kDescalingRatio, center));
        for (const VehicleType& vehicle_type : kGroundVehicles) {
          regrouping_stage_by_vehicle_type_[static_cast<size_t>(vehicle_type)] = DESCALING;
        }
        break;
      }

      case HAS_DENSE_FORMATION: {
        // Rotates group of ground vehicles by 90 degrees so that it turns the most dense side to the enemy
        const Vect center = RelToWorld(Vect(kRelativeGroundX, kRelativeGroundY));
        const Vect diagonal = kUnitVector * RelToWorld(kRelativeScaledGroupSide);
        actions.push_back(std::make_unique<Select>(center - diagonal / 2, diagonal));
        actions.push_back(std::make_unique<Rotate>(kRotationAngle * 2, center));
        for (const VehicleType& vehicle_type : kGroundVehicles) {
          regrouping_stage_by_vehicle_type_[static_cast<size_t>(vehicle_type)] = READY_FOR_ATTACK;
        }
        break;
      }
    }
  }

  // If we are not winning and regrouping is over for ground vehicles, brings them into attack
  if (current_tick > game.getTickCount() / 4 && current_tick % kKillerGroupUpdateFrequency == 0 &&
      me.getScore() <= world.getOpponentPlayer().getScore()) {
    actions.push_back(std::make_unique<SelectByVehicleType>(VehicleType::ARRV, RelToWorld(1.0)));
    actions.push_back(std::make_unique<AddToSelectionByVehicleType>(VehicleType::TANK, RelToWorld(1.0)));
    actions.push_back(std::make_unique<AddToSelectionByVehicleType>(VehicleType::IFV, RelToWorld(1.0)));
    const Vect source = MassCenterForGroundVehicles(me);
    const Vect destination = ClosestEnemyPosition(me, source);
    Vect direction = destination - source;
    direction.Normalize();
    direction *= RelToWorld(kKillerGroupStep);
    actions.push_back(std::make_unique<GoToWithSpeedLimit>(direction, kGroundVehiclesSpeedLimit));
  }
}

void DecisionMakerForGameWithoutBuildings::MakeRegroupingStageTransitions() {
  for (size_t i = 0; i < kAllVehicles.size(); i++) {
    if (motionlesness_checker_->AreAllVehiclesOfTypeMotionless(i)) {
      switch (regrouping_stage_by_vehicle_type_[i]) {
        case SHIFT_BY_Y:
          regrouping_stage_by_vehicle_type_[i] = READY_FOR_SHIFT_BY_X;
          break;
        case SHIFT_BY_X:
          regrouping_stage_by_vehicle_type_[i] = READY_FOR_SCALING;
          break;
        case SCALING:
          regrouping_stage_by_vehicle_type_[i] = READY_FOR_ADJUSTMENT_BY_X;
          break;
        case ADJUSTMENT_BY_X:
          regrouping_stage_by_vehicle_type_[i] = READY_FOR_COLLAPSING;
          break;
        case COLLAPSING:
          regrouping_stage_by_vehicle_type_[i] = READY_FOR_ROTATING;
          break;
        case ROTATING:
          regrouping_stage_by_vehicle_type_[i] = READY_FOR_DESCALING;
          break;
        case DESCALING:
          regrouping_stage_by_vehicle_type_[i] = HAS_DENSE_FORMATION;
          break;
      }
    }
  }
}

bool DecisionMakerForGameWithoutBuildings::AllMyVehiclesOfTypesOnSpecificRegroupingStage(
  const RegroupingStage& stage, const std::vector<VehicleType>& types) const {
  for (const VehicleType& type : types) {
    if (regrouping_stage_by_vehicle_type_[static_cast<size_t>(type)] != stage) {
      return false;
    }
  }
  return true;
}

bool DecisionMakerForGameWithoutBuildings::AllMyGroundVehiclesOnSpecificRegroupingStage(
  const RegroupingStage& stage) const {
  return AllMyVehiclesOfTypesOnSpecificRegroupingStage(stage, kGroundVehicles);
}

bool DecisionMakerForGameWithoutBuildings::AllMyAirVehiclesOnSpecificRegroupingStage(
  const RegroupingStage& stage) const {
  return AllMyVehiclesOfTypesOnSpecificRegroupingStage(stage, kAirVehicles);
}

double DecisionMakerForGameWithoutBuildings::DistanceBetweenMyAirVehiclesAndEnemyVehicles(
  const Player& me) const {
  double min_distance = kInfiniteDistance;

  vector<vector<bool>> tried(runtime_constants_->kFragmentsLinearCount,
                             vector<bool>(runtime_constants_->kFragmentsLinearCount));
  vector<vector<int>> cnt(runtime_constants_->kFragmentsLinearCount,
                          vector<int>(runtime_constants_->kFragmentsLinearCount));

  // Considers all my aerial vehicles
  for (const auto& id_and_vehicle : vehicle_by_id_) {
    const Vehicle& vehicle = id_and_vehicle.second;
    const VehicleType& type = vehicle.getType();
    if ((type == VehicleType::FIGHTER || type == VehicleType::HELICOPTER) &&
        vehicle.getPlayerId() == me.getId()) {
      // And maps them onto square fragments
      const Vect pos = Vect(vehicle);
      cnt[size_t(pos.x / runtime_constants_->kFragmentSideLength)]
        [size_t(pos.y / runtime_constants_->kFragmentSideLength)]++;
    }
  }

  // Considers all my aerial vehicles again
  for (const auto& id_and_vehicle : vehicle_by_id_) {
    const Vehicle& vehicle = id_and_vehicle.second;
    const VehicleType& vehicle_type = vehicle.getType();
    if ((vehicle_type == VehicleType::FIGHTER || vehicle_type == VehicleType::HELICOPTER) &&
        vehicle.getPlayerId() == me.getId()) {
      const Vect pos = Vect(vehicle);
      const unsigned int fragment_x = int(pos.x / runtime_constants_->kFragmentSideLength);
      const unsigned int fragment_y = int(pos.y / runtime_constants_->kFragmentSideLength);
      // If it looks like this unit belongs to the main (largest) group of aerial vehicles,
      // and we haven't looked at any unit in this fragment yet
      if (cnt[fragment_x][fragment_y] >= kMainAirCrewMinUnits && !tried[fragment_x][fragment_y]) {
        // Try to update min_distance and never look at this fragment again
        tried[fragment_x][fragment_y] = true;
        const Vect path_to_closest_enemy = ClosestEnemyPosition(me, pos) - pos;
        min_distance = std::min(min_distance, path_to_closest_enemy.Length());
      }
    }
  }
  return min_distance;
}

void DecisionMakerForGameWithoutBuildings::FakeLastUpdatedTickForMyVehiclesOfTypes(
    const Player& me, const int current_tick,
    const std::vector<VehicleType>& types) {
  // Consider all my vehicles matching one of the <types>
  for (const auto& id_and_vehicle : vehicle_by_id_) {
    const Vehicle& vehicle = id_and_vehicle.second;
    if (vehicle.getPlayerId() == me.getId()) {
      const long long id = id_and_vehicle.first;
      bool matches_type = false;
      for (const auto& type : types) {
        if (vehicle.getType() == type) {
          matches_type = true;
          break;
        }
      }
      if (matches_type) {
        // Tell this vehicle that its coordinate was just updated (even though most likely it wasn't)
        update_tick_by_vehicle_id_[id] = current_tick;
        vehicle_coordinates_update_tick_by_id_[id] = current_tick;
      }
    }
  }
}

void DecisionMakerForGameWithoutBuildings::FakeLastUpdatedTickForMyGroundVehicles(
    const Player& me, const int current_tick) {
  FakeLastUpdatedTickForMyVehiclesOfTypes(me, current_tick, kGroundVehicles);
}

void DecisionMakerForGameWithoutBuildings::FakeLastUpdatedTickForAllMyVehicles(
    const Player& me, const int current_tick) {
  FakeLastUpdatedTickForMyVehiclesOfTypes(me, current_tick, kAllVehicles);
}

double DecisionMakerForGameWithoutBuildings::RelToWorld(const double x) const {
  return x * runtime_constants_->kWorldSideLength;
}

Vect DecisionMakerForGameWithoutBuildings::RelToWorld(const Vect& x) const {
  return x * runtime_constants_->kWorldSideLength;
}
