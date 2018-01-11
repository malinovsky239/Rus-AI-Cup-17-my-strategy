#pragma once
#ifndef _MY_STRATEGY_H_
#define _MY_STRATEGY_H_

#include "Strategy.h"
#include "Action.h"
#include "DecisionMaker.h"
#include <deque>
#include <memory>

using namespace model;

// Entry point of a strategy
class MyStrategy : public Strategy {
 public:
  // Entry point of a strategy on each tick
  void move(const Player& me, const World& world, const Game& game, Move& move) override;

 private:
  // Processes the information about world updates on each tick 
  void InitializeTick(const World& world) const;

  bool CanMakeMove(const int current_tick, const Game& game) const;

  std::deque<std::unique_ptr<Action>> actions_; // contains planned actions
  std::unique_ptr<DecisionMaker> decision_maker_;
};

#endif
