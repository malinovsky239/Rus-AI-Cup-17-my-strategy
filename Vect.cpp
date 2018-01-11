#include "Vect.h"
#include <cmath>

Vect::Vect() : x(0), y(0) {}

Vect::Vect(double x, double y) : x(x), y(y) {}

Vect::Vect(const model::Vehicle& v) : x(v.getX()), y(v.getY()) {}

Vect::Vect(const model::Facility& f) : x(f.getLeft()), y(f.getTop()) {}

Vect operator + (const Vect& a, const Vect& b) {
  return Vect(a.x + b.x, a.y + b.y);
}

Vect operator - (const Vect& a, const Vect& b) {
  return Vect(a.x - b.x, a.y - b.y);
}

Vect& Vect::operator += (const Vect& summand) {
  *this = *this + summand;
  return *this;
}

Vect operator / (const Vect& v, const double divisor) {
  return Vect(v.x / divisor, v.y / divisor);
}

Vect& Vect::operator /= (const double divisor) {
  *this = *this / divisor;
  return *this;
}

Vect operator * (const Vect& v, const double multiplier) {
  return Vect(v.x * multiplier, v.y * multiplier);
}

Vect& Vect::operator *= (const double multiplier) {
  *this = *this * multiplier;
  return *this;
}

double Vect::Length() const {
  return sqrt(x * x + y * y);
}

void Vect::Normalize() {
  const double len = Length();
  x /= len;
  y /= len;
}
