#ifndef PARANOIXA_RANDOM_HPP
#define PARANOIXA_RANDOM_HPP
#include "vector2.hpp"
#include "vector3.hpp"

namespace paranoixa {
class Random {
public:
  // Seed the generator with the specified int
  // NOTE: You should generally not need to manually use this
  static void seed(unsigned int seed);

  // Get a float between 0.0f and 1.0f
  static float getFloat();

  // Get a float from the specified range
  static float getFloatRange(float min, float max);

  // Get an int from the specified range
  static int getIntRange(int min, int max);

  // Get a random vector given the min/max bounds
  static Vector2 getVector(const Vector2 &min, const Vector2 &max);
  static Vector3 getVector(const Vector3 &min, const Vector3 &max);
};
} // namespace paranoixa
#endif // !PARANOIXA_RANDOM_HPP
