#ifndef ELECTOR_H_
#define ELECTOR_H_

#include <condition_variable>
#include <cstdint>
#include <vector>
#include <mutex>

#include "clock.h"
#include "synchronization.h"

struct PrepareRequest;
struct AcceptRequest;
struct PrepareResponse;
struct AcceptResponse;
class ElectorStub;

// Stores information about single replica aggregated from replies of prepare or accept phase.
struct ReplicaInfo {
  ReplicaInfo()
    : acked(false),
      is_master_count(0),
      is_master_until(Clock::time_point::max()) {}

  // Whether this replica replied positively on request.
  bool acked;
  // How many replies indicate that this replica is master.
  uint32_t is_master_count;
  // Time point until which all replicas treating this replica as master will do so.
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
//   analysing all responses and deciding outcome), performed after successful prepare phase.
//   Broadcasts new master (self) as part of started election proposal (which will be accepted
//   if proposal is still active according to majority of replicas).
class Elector {
 public:
  // Provided collection of replicas contains objects implementating client interface for
  // communicating with other replicas of Elector functionality. Index corresponding to
  // own replica number is nullptr.
  Elector(Clock* clock, const std::vector<ElectorStub*>& replicas)
      : replicas_(replicas),
        own_index_(FindOwnReplica(replicas)),
        clock_(clock),
        stopping_(false),
        rpcs_in_progress_(0),
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

  // Mark stop of loop executed in Run() and block until it finishes.
  void Stop();

  // Handlers for incoming Prepare/Accept RPCs from other replicas.
  PrepareResponse* HandlePrepareRequest(const PrepareRequest& req);
  AcceptResponse* HandleAcceptRequest(const AcceptRequest& req);

 private:
  static size_t FindOwnReplica(const std::vector<ElectorStub*>& replicas);

  // Convenience functions checking if master replica is known and has valid lease.
  bool IAmTheMasterLocked() const;
  bool IsMasterElectedLocked() const;

  // Broadcast information that this replica wants to start new election with next sequence number.
  void PerformPreparePhase();
  // Handlers for async replies from other replicas for Prepare RPCs sent out earlier.
  void HandlePrepareReply(const PrepareResponse& resp, bool success);
  void HandleAllPrepareResponses();

  // Broadcast information that this replica decided to be master, responses are still checked
  // to confirm that all replicas obey this sovereign act.
  void PerformAcceptPhrase();
  void HandleAcceptReply(const AcceptResponse& resp, bool success);
  void HandleAllAcceptResponses();

  // All replicas including this one (replicas_[own_index_] == nullptr).
  const std::vector<ElectorStub*> replicas_;
  const size_t own_index_;

  Clock* clock_;
  bool stopping_;
  std::condition_variable stopped_cond_;

  // We perform at most one proposal or accept broadcast at a time, each reply handler calls
  // latch to count down. Once all of RPCs are replied control continues to aggregate them.
  Latch rpcs_in_progress_;
  std::vector<ReplicaInfo> replica_info_;

  // Essentially sequence_nr_ reflects promise that this replica won't accept any new master
  // election request with lower or equal sequence number.
  uint32_t sequence_nr_;
  // Sequence number used for our last election proposal, stays the same during our mastership
  // until we decide to re-run prepare phase.
  uint32_t my_proposal_sequence_nr_;
  // Current master: <0 not elected, >=0 index into replicas_
  int master_index_;
  Clock::time_point master_lease_valid_until_;
  std::mutex mu_;
};

#endif /* ELECTOR_H_ */
