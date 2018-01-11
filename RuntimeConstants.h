#pragma once
#ifndef _RUNTIME_CONSTANTS_H_
#define _RUNTIME_CONSTANTS_H_

#include "Strategy.h"
#include "Vect.h"

// Stores additional game constants.
// These constants depend on other constants returned by methods of model::World and model::Game.
class RuntimeConstants {
 public:
  RuntimeConstants(const model::World& world, const model::Game& game);

  const unsigned int kWorldSideLength;
  const unsigned int kFragmentSideLength, kFragmentsLinearCount; // square areas used by NuclearHandler
                                                                 // to approximate best point for the strike
  const unsigned int kDoubledFragmentSideLength, kDoubledFragmentsLinearCount;

  const Vect kWorldCenter;

  const int kBaseUniformActionInterval; // Required (by rules) pause between two consecutive actions
                                        // if a player wants to spend action points uniformly and 
                                        // doesn't control any Command Center (i.e. doesn't have bonuses)
};

#endif
