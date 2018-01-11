#pragma once
#ifndef _SCALE_H_
#define _SCALE_H_

#include "Action.h"
#include "Vect.h"

// Scales the current selection
// relative to <center>
// with a specific scale <factor>
// (according to the rules: 0.1 <= factor <= 10)
class Scale : public Action {
 public:
  Scale(const double factor, const Vect& center);
  void Execute(model::Move& move) const override;
  std::string Name() const override;

 private:
  double factor_;
  Vect center_;
};

#endif