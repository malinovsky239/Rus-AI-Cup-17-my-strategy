#pragma once
#ifndef _VECT_H_
#define _VECT_H_

#include "Strategy.h"

// Auxiliary class aimed at adding vector support (in geometrical sense)
class Vect {
 public:
  double x, y;

  Vect();
  Vect(const double x, const double y);
  Vect(const model::Vehicle& v);
  Vect(const model::Facility& f);

  friend Vect operator + (const Vect& a, const Vect& b);
  friend Vect operator - (const Vect& a, const Vect& b);
  friend Vect operator / (const Vect& v, const double divisor);
  friend Vect operator * (const Vect& v, const double multiplier);

  Vect& operator += (const Vect& summand);
  Vect& operator /= (const double divisor);
  Vect& operator *= (const double multiplier);

  double Length() const;
  void Normalize();
};

const Vect kUnitVector = Vect(1, 1);

#endif
