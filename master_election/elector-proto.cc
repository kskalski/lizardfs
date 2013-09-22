#include "elector-proto.h"

#include <iostream>
#include <functional>

#include "persistent-connection.h"

void ElectorStub::SendPrepareRequest(
    const PrepareRequest& req,
    std::function<void(const PrepareResponse&, bool)> callback) {
  std::cout << "send prepare\n";
  PrepareResponse resp;
  char buffer[1 + sizeof(PrepareRequest)];
  buffer[sizeof(PrepareRequest)] = 'P';
  memcpy(buffer, &req, sizeof(PrepareRequest));
  bool success = connection_->SendMessage(buffer, sizeof(PrepareRequest) + 1,
                                          &resp, sizeof(PrepareResponse));
  callback(resp, success);
}

void ElectorStub::SendAcceptRequest(
    const AcceptRequest& req,
    std::function<void(const AcceptResponse&, bool)> callback) {
  std::cout << "send accept\n";
  AcceptResponse resp;
  char buffer[1 + sizeof(AcceptRequest)];
  buffer[sizeof(AcceptRequest)] = 'A';
  memcpy(buffer, &req, sizeof(AcceptRequest));
  bool success = connection_->SendMessage(buffer, sizeof(AcceptRequest) + 1,
                                          &resp, sizeof(AcceptResponse));
  callback(resp, success);
}
