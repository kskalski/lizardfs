#include "clock.h"

#include <thread>

using std::chrono::steady_clock;

steady_clock::time_point SteadyClock::Now() {
  return steady_clock::now();
}

void SteadyClock::Sleep(steady_clock::duration duration) {
  std::this_thread::sleep_for(duration);
}

steady_clock::time_point FakeClock::Now() {
  return current_time_;
}

void FakeClock::Sleep(steady_clock::duration duration) {
  steady_clock::time_point wait_until = current_time_ + duration;
  std::unique_lock<std::mutex> l(mu_);
  time_passed_cond_.wait(l, [this, wait_until]() { return current_time_ >= wait_until; });
}

void FakeClock::MoveForward(steady_clock::duration duration) {
  std::lock_guard<std::mutex> l(mu_);
  current_time_ += duration;
  time_passed_cond_.notify_all();
}
