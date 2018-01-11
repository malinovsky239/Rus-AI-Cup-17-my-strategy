#pragma once
#ifndef _GO_TO_H_
#define _GO_TO_H_

#include "Action.h"
#include "Vect.h"

// Orders current selection to change its position by vector `shift`
class GoTo : public Action {
 public:
  GoTo(const Vect& shift);
  void Execute(model::Move& move) const override;
  std::string Name() const override;

 private:
  Vect shift_;
};

#endif
