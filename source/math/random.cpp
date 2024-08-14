#include "random_generator.hpp"
#include <math/random.hpp>
#include <random>
namespace paranoixa {
static RandomGenerator randomGenerator;
RandomGenerator::RandomGenerator() {
  std::random_device rd;
  Random::seed(rd());
}

void Random::seed(unsigned int seed) { randomGenerator.sGenerator.seed(seed); }

float Random::getFloat() { return getFloatRange(0.0f, 1.0f); }

float Random::getFloatRange(float min, float max) {
  std::uniform_real_distribution<float> dist(min, max);
  return dist(randomGenerator.sGenerator);
}

int Random::getIntRange(int min, int max) {
  std::uniform_int_distribution<int> dist(min, max);
  int a = dist(randomGenerator.sGenerator);
  return a;
}

Vector2 Random::getVector(const Vector2 &min, const Vector2 &max) {
  Vector2 r = Vector2(getFloat(), getFloat());
  return min + (max - min) * r;
}

Vector3 Random::getVector(const Vector3 &min, const Vector3 &max) {
  Vector3 r = Vector3(getFloat(), getFloat(), getFloat());
  return min + (max - min) * r;
}

} // namespace paranoixa
