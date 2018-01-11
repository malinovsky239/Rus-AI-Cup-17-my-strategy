#pragma once
#ifndef _GO_TO_WITH_SPEED_LIMIT_H_
#define _GO_TO_WITH_SPEED_LIMIT_H_

#include "Action.h"
#include "Vect.h"

// Orders current selection to change its position by vector `shift`
// and limits speed of each individual vehicle within the selection.
// When selection contains vehicles of different types,
// it helps to keep them close.
class GoToWithSpeedLimit : public Action {
 public:
  GoToWithSpeedLimit(const Vect& shift, const double speed_limit);
  void Execute(model::Move& move) const override;
  std::string Name() const override;

 private:
  Vect shift_;
  double speed_limit_;
};

#endif
