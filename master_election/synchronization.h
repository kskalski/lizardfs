#ifndef SYNCHRONIZATION_H
#define SYNCHRONIZATION_H

#include <mutex>
#include <condition_variable>

// Allows threads to Wait() until CountDown() is called a specified number of times.
class Latch {
 public:
  explicit Latch(size_t count) : count_(count) {}
  void operator =(const Latch& other);

  void CountDown();

  // Blocks until CountDown is called expected number of times.
  void Wait();

 private:
  size_t count_;
  std::mutex mu_;
  std::condition_variable cond_;
};

#endif // SYNCHRONIZATION_H
