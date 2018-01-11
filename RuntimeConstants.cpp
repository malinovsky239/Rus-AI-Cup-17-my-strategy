#include "RuntimeConstants.h"
#include <cmath>

RuntimeConstants::RuntimeConstants(const model::World& world, const model::Game& game)
    : kWorldSideLength(world.getWidth()),
      kFragmentSideLength(static_cast<unsigned int>(sqrt(kWorldSideLength))),
      kFragmentsLinearCount(static_cast<unsigned int>(sqrt(kWorldSideLength))),
      kDoubledFragmentSideLength(kFragmentSideLength * 2),
      kDoubledFragmentsLinearCount(kFragmentsLinearCount / 2),
      kWorldCenter(Vect(kWorldSideLength / 2, kWorldSideLength / 2)),
      kBaseUniformActionInterval(game.getActionDetectionInterval() / game.getBaseActionCount()) {}
