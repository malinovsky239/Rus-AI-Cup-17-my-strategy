#pragma once
#ifndef _SELECT_H_
#define _SELECT_H_

#include "Action.h"
#include "Vect.h"

// Selects all vehicles within rectangle with specified <top-left-corner> and <diagonal>
class Select : public Action {
 public:
  Select(const Vect& top_left, const Vect& diagonal);
  void Execute(model::Move& move) const override;
  std::string Name() const override;

 private:
  Vect top_left_corner_;
  Vect diagonal_;
};

#endif
