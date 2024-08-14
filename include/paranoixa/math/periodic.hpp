#ifndef PARANOIXA_PERIODIC_HPP
#define PARANOIXA_PERIODIC_HPP
#include "../time/time.hpp"
namespace paranoixa {
class Periodic {
public:
  static float sin0_1(const float periodSec, const float t = Time::seconds());

  static float cos0_1(const float periodSec, const float t = Time::milli());
};
} // namespace paranoixa
#endif // PARANOIXA_PERIODIC_HPP