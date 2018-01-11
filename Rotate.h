#pragma once
#ifndef _ROTATE_H_
#define _ROTATE_H_

#include "Action.h"
#include "Vect.h"

// Orders current selection to rotate around <center> by <angle> radians
class Rotate : public Action {
 public:
  Rotate(const double angle, const Vect& center);
  void Execute(model::Move& move) const override;
  std::string Name() const override;

 private:
  double angle_;
  Vect center_;
};

#endif
