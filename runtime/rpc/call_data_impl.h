#ifndef CALL_DATA_IMPL
#define CALL_DATA_IMPL

#include "call_data_base.h"
#include "../bessport/kmod/llring.h"

template<class TReq, class TRep>
class derived_call_data : public call_data_base{
public:
  derived_call_data(Runtime_RPC::AsyncService* service, ServerCompletionQueue* cq)
    : call_data_base(service, cq), responder_(&ctx_){
    Proceed();
  }

  void Proceed() override{}

  ~derived_call_data() = default;

private:
  TReq request_;

  TRep reply_;

  ServerAsyncResponseWriter<TRep> responder_;
};

using nfa_msg::LivenessRequest;
using nfa_msg::LivenessReply;

template<>
void derived_call_data<LivenessRequest, LivenessReply>::Proceed(){
  if (status_ == CREATE) {
    status_ = PROCESS;
    service_->RequestLivenessCheck(&ctx_, &request_, &responder_, cq_, cq_, this);
  } else if (status_ == PROCESS) {
    new derived_call_data<LivenessRequest, LivenessReply>(service_, cq_);
    status_ = FINISH;
    responder_.Finish(reply_, Status::OK, this);
  } else {
    GPR_ASSERT(status_ == FINISH);
    delete this;
  }
}

using nfa_msg::AddOutputRtsReq;
using nfa_msg::AddOutputRtsRes;

template<>
void derived_call_data<AddOutputRtsReq, AddOutputRtsRes>::Proceed(){

}

/*class LivenessCheck : public call_data_base{
public:
  LivenessCheck(Runtime_RPC::AsyncService* service, ServerCompletionQueue* cq)
    : call_data_base(service, cq), responder_(&ctx_){
    Proceed();
  }

  ~LivenessCheck() = default;

  void Proceed() override {
    if (status_ == CREATE) {
      status_ = PROCESS;
      service_->RequestLivenessCheck(&ctx_, &request_, &responder_, cq_, cq_, this);
    } else if (status_ == PROCESS) {
      new LivenessCheck(service_, cq_);
      status_ = FINISH;
      responder_.Finish(reply_, Status::OK, this);
    } else {
      GPR_ASSERT(status_ == FINISH);
      delete this;
    }
  }

private:
  LivenessRequest request_;

  LivenessReply reply_;

  ServerAsyncResponseWriter<LivenessReply> responder_;
};*/

/*using nfa_msg::AddOutputRtsReq;
using nfa_msg::AddOutputRtsRes;

class AddOutputRts : public call_data_base{
public:
  AddOutputRts(Runtime_RPC::AsyncService* service, ServerCompletionQueue* cq)
    : call_data_base(service, cq), responder_(&ctx_){
    Proceed();
  }

private:
  AddOutputRtsReq request_;

  AddOutputRtsRes reply_;


};*/

#endif
