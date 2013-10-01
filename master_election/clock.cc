#ifndef _GLIBCXX_USE_NANOSLEEP
#define _GLIBCXX_USE_NANOSLEEP
#endif

#include "clock.h"

#include <thread>

Clock::time_point RealClock::Now() const {
  return Clock::std_clock::now();
}

void RealClock::Sleep(duration forDuration) {
  std::this_thread::sleep_for(forDuration);
}

Clock::time_point FakeClock::Now() const {
  return current_time_;
}

void FakeClock::Sleep(duration forDuration) {
  const Clock::time_point wait_until = current_time_ + forDuration;
  std::unique_lock<std::mutex> l(mu_);
  time_passed_cond_.wait(l, [this, wait_until]() { return current_time_ >= wait_until; });
}

void FakeClock::MoveForward(duration by) {
  std::lock_guard<std::mutex> l(mu_);
  current_time_ += by;
  time_passed_cond_.notify_all();
}
