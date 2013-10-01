#ifndef ELECTOR_H_
#define ELECTOR_H_

#include <condition_variable>
#include <cstdint>
#include <vector>
#include <mutex>

#include "clock.h"

struct PrepareRequest;
struct AcceptRequest;
struct PrepareResponse;
struct AcceptResponse;
class ElectorStub;

// TODO(kskalski): Move to separate utilities file
// Allows threads to Wait() until CountDown() is called a specified number of times.
class Latch {
 public:
  Latch(size_t count) : count_(count) {}
  void operator =(const Latch &other) {
    count_ = other.count_;
  }

  void CountDown() {
    std::lock_guard<std::mutex> l(mu_);
    if (--count_ == 0) {
      cond_.notify_one();
    }
  }

  void Wait() {
    std::unique_lock<std::mutex> l(mu_);
    while (count_ > 0) {
      cond_.wait(l);
    }
  }

 private:
  size_t count_;
  std::mutex mu_;
  std::condition_variable cond_;
};

// Stores state received from single replica in duration of single prepare or accept phase.
struct ReplicaInfo {
  ReplicaInfo()
    : acked(false),
      is_master_count(0),
      is_master_until(Clock::time_point::max()) {}

  bool acked;
  uint32_t is_master_count;
  Clock::time_point is_master_until;
};

// Implements Paxos schema for electing master among several replicas. Optimized with some
// techniques described in http://www.eecs.harvard.edu/cs262/Readings/paxosmadelive.pdf
//
// The schema consists of two phases:
// - prepare (initiated by current replica when PerformPreparePhase() is called, replies
//   being handled by HandlePrepareReply() and using HandlePrepareRequest() for answering
//   similar requests from other replicas), which tries to estabilish new election proposal
//   among majority of replicas.
// - accept (similarly implemented by PerformAcceptPhrase(), HandleAcceptReply() and
//   HandleAcceptRequest() for processing single messages and HandleAllAcceptResponses() for
//   analysing all responses and deciding outcome), performed after successful prepare phase
//   and broadcasting new master as part of started election proposal (if it's still active
//   according to majority of replicas).
class Elector {
 public:
  // Provided collection of replicas contains objects implementating client interface for
  // communicating with other replicas of Elector functionality. Index corresponding to
  // own replica number is nullptr.
  Elector(Clock* clock, const std::vector<ElectorStub*>& replicas)
      : clock_(clock),
        replicas_(replicas),
        own_index_(FindOwnReplica(replicas)),
        stopping_(false),
        comm_in_progress_(0),
        replica_info_(replicas.size()),
        sequence_nr_(0),
        master_index_(-1),
        master_lease_valid_until_(Clock::time_point::min()) {
  }

  // Halts exection of Run and destroys the object.
  ~Elector();

  // Periodically checks if master is elected and performs new election or lease re-newal.
  // Blocks until destructor of object is invoked, should be started in dedicated thread.
  void Run();

  // Handlers for incoming Prepare/Accept RPCs from other replicas.
  PrepareResponse* HandlePrepareRequest(const PrepareRequest& req);
  AcceptResponse* HandleAcceptRequest(const AcceptRequest& req);

 private:
  static size_t FindOwnReplica(const std::vector<ElectorStub*>& replicas);

  // Convenience functions checking if master replica is known and has valid lease.
  bool IAmTheMasterLocked() const;
  bool IsMasterElectedLocked() const;

  // Handlers for async replies from other replicas for Prepare/Accept RPCs sent out earlier.
  void HandlePrepareReply(const PrepareResponse& resp, bool success);
  void HandleAcceptReply(const AcceptResponse& resp, bool success);

  // Broadcast information that this replica wants to start new election with given number.
  void PerformPreparePhase();
  void HandleAllPrepareResponses();

  // Broadcast information that this replica decided to be master, responses are still checked
  // to confirm that all replicas obey this sovereign act.
  void PerformAcceptPhrase();
  void HandleAllAcceptResponses();

  Clock* clock_;
  // All replicas including this one (replicas_[own_index_] == nullptr).
  const std::vector<ElectorStub*> replicas_;
  const size_t own_index_;
  bool stopping_;

  // We perform at most one proposal or accept broadcast at a time, each reply handler calls
  // latch to count down. Once all of RPCs are replied control continues to aggregate them.
  Latch comm_in_progress_;
  std::vector<ReplicaInfo> replica_info_;

  // Essentially sequence_nr_ reflects promise that this replica won't accept any new master
  // election sequence with lower or equal number.
  uint32_t sequence_nr_;
  // Sequence number used for our last election proposal, stays the same during our mastership.
  uint32_t my_proposal_sequence_nr_;
  // Current master: <0 not elected, >=0 index into replicas_
  int master_index_;
  Clock::time_point master_lease_valid_until_;
  std::mutex mu_;
  std::condition_variable stopped_cond_;
};

#endif /* ELECTOR_H_ */
