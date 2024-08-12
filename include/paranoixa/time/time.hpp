#ifndef PARANOIXA_TIME_HPP
#define PARANOIXA_TIME_HPP
#include <chrono>
#include <cstdint>
#include <functional>

namespace paranoixa {
/**
 * @brief About time
 *
 */
class Time {
public:
  /**
   * @brief Get time about launch app to now as seconds
   */
  static float seconds();

  /**
   * @brief Get time about launch app to now as milli seconds
   */
  static uint32_t milli();

  /**
   * @brief Get the function time
   *
   * @param function
   * @return double time
   */
  static inline double getFunctionTime(const std::function<void()> &function) {
    std::chrono::system_clock::time_point start, end;
    start = std::chrono::system_clock::now();
    function();
    end = std::chrono::system_clock::now();
    return std::chrono::duration_cast<std::chrono::microseconds>(end - start)
        .count();
  }
};
} // namespace paranoixa

#endif // PARANOIXA_TIME_HPP
