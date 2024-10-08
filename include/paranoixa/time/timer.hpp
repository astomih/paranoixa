#ifndef PARANOIXA_TIME_TIMER_HPP
#define PARANOIXA_TIME_TIMER_HPP

namespace paranoixa {
/**
 * @brief Timer class
 *
 */
class Timer {
public:
  /**
   * @brief Construct a new timer object
   *
   */
  Timer();
  /**
   * @brief Construct a new timer object
   *
   * @param time Set the time in seconds
   */
  Timer(float time);
  /**
   * @brief Destroy the timer object
   *
   */
  ~Timer();
  /**
   * @brief Start the timer
   *
   */
  void start();
  /**
   * @brief Set the time object
   *
   * @param milliSecond Set the time in milliseconds
   */
  void set_time(float milliseconds);
  /**
   * @brief Is the timer finished
   *
   * @return true The timer is finished
   * @return false The timer is not finished
   */
  bool check();
  /**
   * @brief Is the timer started
   *
   * @return true The timer is started
   * @return false The timer is not started
   */
  bool is_started() { return isStarted; }
  /**
   * @brief Stop the timer
   *
   */
  void stop();

private:
  float startTime;
  float time;
  bool isStarted;
};
} // namespace paranoixa
#endif // PARANOIXA_TIME_TIMER_HPP
