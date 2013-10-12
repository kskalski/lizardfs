#include "elector.h"

#include <thread>
#include <vector>

#include "clock.h"
#include "elector-proto.h"

class ProxyElectorStub : public ElectorStub {
 public:
  ProxyElectorStub() : ElectorStub(nullptr) {}

  virtual void SendPrepareRequest(const PrepareRequest& req,
                                  std::function<void(const PrepareResponse&, bool)> callback) {
    PrepareResponse* resp = object_->HandlePrepareRequest(req);
    callback(*resp, true);
    delete resp;
  }

  virtual void SendAcceptRequest(const AcceptRequest& req,
                                 std::function<void(const AcceptResponse&, bool)> callback) {
    AcceptResponse* resp = object_->HandleAcceptRequest(req);
    callback(*resp, true);
    delete resp;
  }

  void SetObject(Elector* object) {
    object_ = object;
  }

 private:
  Elector* object_;
};

void IntegrationTest() {
  RealClock clock;
  std::vector<ProxyElectorStub> stubs(3);
  std::vector<std::thread> threads(3);
  {
    Elector elector0(&clock, std::vector<ElectorStub*>({nullptr, &stubs[1], &stubs[2]}));
    Elector elector1(&clock, std::vector<ElectorStub*>({&stubs[0], nullptr, &stubs[2]}));
    Elector elector2(&clock, std::vector<ElectorStub*>({&stubs[0], &stubs[1], nullptr}));
    int i = 0;
    for (auto* elector : {&elector0, &elector1, &elector2}) {
      stubs[i].SetObject(elector);
      threads[i] = std::thread(std::bind(&Elector::Run, elector));
      ++i;
    }
    for (auto* elector : {&elector0, &elector1, &elector2}) {
      elector->Stop();
    }
  }
  for (auto& thread : threads) {
    thread.join();
  }
}

int main() {
  IntegrationTest();

  return 0;
}
