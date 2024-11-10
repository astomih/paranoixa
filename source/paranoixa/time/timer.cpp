#include <time/time.hpp>
#include <time/timer.hpp>
namespace paranoixa {
Timer::Timer() : startTime(0.f), time(0.f), isStarted(false) {}
Timer::Timer(float time) : startTime(0.f), time(0.f), isStarted(false) {
  set_time(time);
}
Timer::~Timer() = default;
void Timer::start() {
  startTime = Time::milli();
  isStarted = true;
}
void Timer::set_time(float milliSecond) { this->time = milliSecond; }
bool Timer::check() {
  return !isStarted ? false : time <= Time::milli() - startTime;
}
void Timer::stop() { isStarted = false; }
} // namespace paranoixa
