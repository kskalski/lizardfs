#ifndef ELECTOR_PROTO_H_
#define ELECTOR_PROTO_H_

#include <cstdint>
#include <cstring>
#include <functional>

class PersistentConnection;

// TODO(kskalski): I want PROTOCOL BUFFERS!

// Requests and responses below realize Paxos algorithm with slight extensions.

// Request starting new election proposal with specified sequence number. If majority
// of replicas didn't receive similar requests with same or higher sequence number, then
// master election can proceed further.
struct PrepareRequest {
  uint32_t sequence_nr;
  int proposer_index;
};

// Prepare request is acked when no previous request with same or higher sequence number
// was acked.
struct PrepareResponse {
  bool ack;
  int responder_index;
  // Fields below reflect current state of responding replica, not necessarily connected
  // with request. Max seen number is used to propagate the highest occuring sequence number
  // to proposer (response isn't acked if it is same or higher than proposed one).
  uint32_t max_seen_sequence_nr;
  // Carries information about already elected master (if any).
  int master_index;
  uint64_t master_lease_valid_until;
};

// When some replica gathers majority ack replies in prepare phase, it sends out accept
// requests designating given replica index (usually own) as master. The basic guarantee
// of master uniqueness comes from ordering of successful prepare phases. Successful accept
// phase is always after successful prepare phase with same number and before any successful
// prepare phase with higher number.
struct AcceptRequest {
  uint32_t sequence_nr;
  int master_index;
};

// Accept reuqest is acked if it is just extending mastership of existing master or if no
// master is elected it has the same or higher sequence number as latest acked prepare
// reuqest on given replica.
struct AcceptResponse {
  bool ack;
  int responder_index;
  uint64_t master_lease_valid_until;
  uint32_t max_seen_sequence_nr;
};

// Client for sending messages to other replicas running Elector service
class ElectorStub {
 public:
   ElectorStub(PersistentConnection* connection) : connection_(connection) {}
   virtual ~ElectorStub() {}

   virtual void SendPrepareRequest(const PrepareRequest& req,
                                   std::function<void(const PrepareResponse&, bool)> callback);
   virtual void SendAcceptRequest(const AcceptRequest& req,
                                  std::function<void(const AcceptResponse&, bool)> callback);

 private:
   PersistentConnection* connection_;
};

#endif /* ELECTOR_PROTO_H_ */
