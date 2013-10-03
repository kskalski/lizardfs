#include "synchronization.h"

#include <mutex>

void Latch::operator =(const Latch& other) {
  count_ = other.count_;
}

void Latch::CountDown() {
  std::lock_guard<std::mutex> l(mu_);
  if (--count_ == 0) {
    cond_.notify_one();
  }
}

void Latch::Wait() {
  std::unique_lock<std::mutex> l(mu_);
  while (count_ > 0) {
    cond_.wait(l);
  }
}

