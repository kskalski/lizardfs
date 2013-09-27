#ifndef CLOCK_H
#define CLOCK_H

#include <chrono>
#include <condition_variable>
#include <mutex>

class Clock {
 public:
  virtual ~Clock() {}
  virtual std::chrono::steady_clock::time_point Now() = 0;
  virtual void Sleep(std::chrono::steady_clock::duration duration) = 0;
};

class SteadyClock : public Clock {
 public:
  virtual ~SteadyClock();
  virtual std::chrono::steady_clock::time_point Now();
  virtual void Sleep(std::chrono::steady_clock::duration duration);
};

// Fake clock for testing. Allows setting up time and controlling its passage.
class FakeClock : public Clock {
 public:
  FakeClock() : current_time_(std::chrono::steady_clock::now()) {}
  explicit FakeClock(std::chrono::steady_clock::time_point init_time) : current_time_(init_time) {}
  virtual ~FakeClock() {}
  virtual std::chrono::steady_clock::time_point Now();
  virtual void Sleep(std::chrono::steady_clock::duration duration);

  void MoveForward(std::chrono::steady_clock::duration duration);

 private:
  std::chrono::steady_clock::time_point current_time_;
  std::condition_variable time_passed_cond_;
  std::mutex mu_;
};

#endif // CLOCK_H
