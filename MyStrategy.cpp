#include "MyStrategy.h"

#include "DecisionMakerForGameWithBuildings.h"
#include "DecisionMakerForGameWithoutBuildings.h"

using namespace std;

void MyStrategy::move(const Player& me, const World& world, const Game& game, Move& move) {
  const int current_tick = world.getTickIndex();
  if (current_tick == 0) {
    if (!world.getFacilities().empty()) {
      decision_maker_ = std::make_unique<DecisionMakerForGameWithBuildings>();
    }
    else {
      decision_maker_ = std::make_unique<DecisionMakerForGameWithoutBuildings>();
    }
    decision_maker_->InitializeHelperClasses(world, game);
  }

  InitializeTick(world);

  bool performed_action = false;
  if (CanMakeMove(current_tick, game)) {
    decision_maker_->CheckMyVehiclesMotionlessness(me, current_tick);
    decision_maker_->NuclearOperations(me, current_tick, actions_);
    decision_maker_->MakeDecisions(me, world, game, move, actions_);

    // Executes an action with the highest priority.
    // It is either the earliest one (by the time when it was planned) or
    // the most urgent one (Nuclear Strike).
    if (!actions_.empty()) {
      actions_.front()->Execute(move);
      actions_.pop_front();
      performed_action = true;
    }
  }
  if (!performed_action) {
    move.setAction(ActionType::NONE);
  }
}

void MyStrategy::InitializeTick(const World& world) const {
  const int current_tick = world.getTickIndex();

  for (const Vehicle& vehicle : world.getNewVehicles()) {
    decision_maker_->AddNewVehicleInfo(vehicle, current_tick);
  }

  for (const VehicleUpdate& vehicleUpdate : world.getVehicleUpdates()) {
    decision_maker_->UpdateVehicleInfo(vehicleUpdate, current_tick);
  }
}

// Checks if the move can be made
// assuming that we want to spend actions points as evenly as possible
bool MyStrategy::CanMakeMove(const int current_tick, const Game& game) const {
  return current_tick % decision_maker_->BaseUniformActionInterval() == 0;
}
