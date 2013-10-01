#ifndef CLOCK_H
#define CLOCK_H

#include <chrono>
#include <condition_variable>
#include <mutex>

class Clock {
 public:
  typedef std::chrono::steady_clock std_clock;
  typedef std_clock::duration duration;
  typedef std_clock::time_point time_point;

  virtual ~Clock() {}
  virtual time_point Now() const = 0;
  virtual void Sleep(duration forDuration) = 0;
};

class RealClock : public Clock {
 public:
  virtual ~RealClock() {}
  virtual time_point Now() const;
  virtual void Sleep(duration forDuration);
};

// Fake clock for testing. Allows setting up time and controlling its passage.
class FakeClock : public Clock {
 public:
  FakeClock() : current_time_(std_clock::now()) {}
  explicit FakeClock(time_point init_time) : current_time_(init_time) {}
  virtual ~FakeClock() {}
  virtual time_point Now() const;
  virtual void Sleep(duration forDuration);

  void MoveForward(duration by);

 private:
  time_point current_time_;
  std::condition_variable time_passed_cond_;
  std::mutex mu_;
};

#endif // CLOCK_H
