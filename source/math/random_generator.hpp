#ifndef PARANOIXA_RANDOM_GENERATOR_HPP
#define PARANOIXA_RANDOM_GENERATOR_HPP
#include <random>

namespace paranoixa {
class RandomGenerator {
public:
  RandomGenerator();
  std::mt19937 sGenerator;
};
} // namespace paranoixa
#endif // !PARANOIXA_RANDOM_GENERATOR_HPP
