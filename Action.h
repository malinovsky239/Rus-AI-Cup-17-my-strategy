#pragma once
#ifndef _ACTION_H_
#define _ACTION_H_

#include "Strategy.h"
#include <string>

// Implements Command design pattern
// by turning each of possible sets of settings for a model::Move instance into an object
// that can be stored in the deque of planned actions
class Action {
 public:
  virtual ~Action() {}
  virtual void Execute(model::Move& move) const = 0;
  virtual std::string Name() const = 0;
};

#endif
