// Generated by the gRPC protobuf plugin.
// If you make any local change, they will be lost.
// source: nfa_msg.proto
// Original file comments:
// Define all the RPC calls that I defined in the new design document
#ifndef GRPC_nfa_5fmsg_2eproto__INCLUDED
#define GRPC_nfa_5fmsg_2eproto__INCLUDED

#include "nfa_msg.pb.h"

#include <grpc++/impl/codegen/async_stream.h>
#include <grpc++/impl/codegen/async_unary_call.h>
#include <grpc++/impl/codegen/proto_utils.h>
#include <grpc++/impl/codegen/rpc_method.h>
#include <grpc++/impl/codegen/service_type.h>
#include <grpc++/impl/codegen/status.h>
#include <grpc++/impl/codegen/stub_options.h>
#include <grpc++/impl/codegen/sync_stream.h>

namespace grpc {
class CompletionQueue;
class Channel;
class RpcService;
class ServerCompletionQueue;
class ServerContext;
}  // namespace grpc

namespace nfa_msg {

// The greeting service definition.
class Runtime_RPC GRPC_FINAL {
 public:
  class StubInterface {
   public:
    virtual ~StubInterface() {}
    // Sends a greeting
    virtual ::grpc::Status LivenessCheck(::grpc::ClientContext* context, const ::nfa_msg::LivenessRequest& request, ::nfa_msg::LivenessReply* response) = 0;
    std::unique_ptr< ::grpc::ClientAsyncResponseReaderInterface< ::nfa_msg::LivenessReply>> AsyncLivenessCheck(::grpc::ClientContext* context, const ::nfa_msg::LivenessRequest& request, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncResponseReaderInterface< ::nfa_msg::LivenessReply>>(AsyncLivenessCheckRaw(context, request, cq));
    }
    virtual ::grpc::Status SetMigrationTarget(::grpc::ClientContext* context, const ::nfa_msg::MigrationTarget& request, ::nfa_msg::MigrationNegotiationResult* response) = 0;
    std::unique_ptr< ::grpc::ClientAsyncResponseReaderInterface< ::nfa_msg::MigrationNegotiationResult>> AsyncSetMigrationTarget(::grpc::ClientContext* context, const ::nfa_msg::MigrationTarget& request, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncResponseReaderInterface< ::nfa_msg::MigrationNegotiationResult>>(AsyncSetMigrationTargetRaw(context, request, cq));
    }
    virtual ::grpc::Status MigrationNegotiate(::grpc::ClientContext* context, const ::nfa_msg::MigrationNegotiation& request, ::nfa_msg::MigrationNegotiationResult* response) = 0;
    std::unique_ptr< ::grpc::ClientAsyncResponseReaderInterface< ::nfa_msg::MigrationNegotiationResult>> AsyncMigrationNegotiate(::grpc::ClientContext* context, const ::nfa_msg::MigrationNegotiation& request, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncResponseReaderInterface< ::nfa_msg::MigrationNegotiationResult>>(AsyncMigrationNegotiateRaw(context, request, cq));
    }
  private:
    virtual ::grpc::ClientAsyncResponseReaderInterface< ::nfa_msg::LivenessReply>* AsyncLivenessCheckRaw(::grpc::ClientContext* context, const ::nfa_msg::LivenessRequest& request, ::grpc::CompletionQueue* cq) = 0;
    virtual ::grpc::ClientAsyncResponseReaderInterface< ::nfa_msg::MigrationNegotiationResult>* AsyncSetMigrationTargetRaw(::grpc::ClientContext* context, const ::nfa_msg::MigrationTarget& request, ::grpc::CompletionQueue* cq) = 0;
    virtual ::grpc::ClientAsyncResponseReaderInterface< ::nfa_msg::MigrationNegotiationResult>* AsyncMigrationNegotiateRaw(::grpc::ClientContext* context, const ::nfa_msg::MigrationNegotiation& request, ::grpc::CompletionQueue* cq) = 0;
  };
  class Stub GRPC_FINAL : public StubInterface {
   public:
    Stub(const std::shared_ptr< ::grpc::ChannelInterface>& channel);
    ::grpc::Status LivenessCheck(::grpc::ClientContext* context, const ::nfa_msg::LivenessRequest& request, ::nfa_msg::LivenessReply* response) GRPC_OVERRIDE;
    std::unique_ptr< ::grpc::ClientAsyncResponseReader< ::nfa_msg::LivenessReply>> AsyncLivenessCheck(::grpc::ClientContext* context, const ::nfa_msg::LivenessRequest& request, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncResponseReader< ::nfa_msg::LivenessReply>>(AsyncLivenessCheckRaw(context, request, cq));
    }
    ::grpc::Status SetMigrationTarget(::grpc::ClientContext* context, const ::nfa_msg::MigrationTarget& request, ::nfa_msg::MigrationNegotiationResult* response) GRPC_OVERRIDE;
    std::unique_ptr< ::grpc::ClientAsyncResponseReader< ::nfa_msg::MigrationNegotiationResult>> AsyncSetMigrationTarget(::grpc::ClientContext* context, const ::nfa_msg::MigrationTarget& request, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncResponseReader< ::nfa_msg::MigrationNegotiationResult>>(AsyncSetMigrationTargetRaw(context, request, cq));
    }
    ::grpc::Status MigrationNegotiate(::grpc::ClientContext* context, const ::nfa_msg::MigrationNegotiation& request, ::nfa_msg::MigrationNegotiationResult* response) GRPC_OVERRIDE;
    std::unique_ptr< ::grpc::ClientAsyncResponseReader< ::nfa_msg::MigrationNegotiationResult>> AsyncMigrationNegotiate(::grpc::ClientContext* context, const ::nfa_msg::MigrationNegotiation& request, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncResponseReader< ::nfa_msg::MigrationNegotiationResult>>(AsyncMigrationNegotiateRaw(context, request, cq));
    }

   private:
    std::shared_ptr< ::grpc::ChannelInterface> channel_;
    ::grpc::ClientAsyncResponseReader< ::nfa_msg::LivenessReply>* AsyncLivenessCheckRaw(::grpc::ClientContext* context, const ::nfa_msg::LivenessRequest& request, ::grpc::CompletionQueue* cq) GRPC_OVERRIDE;
    ::grpc::ClientAsyncResponseReader< ::nfa_msg::MigrationNegotiationResult>* AsyncSetMigrationTargetRaw(::grpc::ClientContext* context, const ::nfa_msg::MigrationTarget& request, ::grpc::CompletionQueue* cq) GRPC_OVERRIDE;
    ::grpc::ClientAsyncResponseReader< ::nfa_msg::MigrationNegotiationResult>* AsyncMigrationNegotiateRaw(::grpc::ClientContext* context, const ::nfa_msg::MigrationNegotiation& request, ::grpc::CompletionQueue* cq) GRPC_OVERRIDE;
    const ::grpc::RpcMethod rpcmethod_LivenessCheck_;
    const ::grpc::RpcMethod rpcmethod_SetMigrationTarget_;
    const ::grpc::RpcMethod rpcmethod_MigrationNegotiate_;
  };
  static std::unique_ptr<Stub> NewStub(const std::shared_ptr< ::grpc::ChannelInterface>& channel, const ::grpc::StubOptions& options = ::grpc::StubOptions());

  class Service : public ::grpc::Service {
   public:
    Service();
    virtual ~Service();
    // Sends a greeting
    virtual ::grpc::Status LivenessCheck(::grpc::ServerContext* context, const ::nfa_msg::LivenessRequest* request, ::nfa_msg::LivenessReply* response);
    virtual ::grpc::Status SetMigrationTarget(::grpc::ServerContext* context, const ::nfa_msg::MigrationTarget* request, ::nfa_msg::MigrationNegotiationResult* response);
    virtual ::grpc::Status MigrationNegotiate(::grpc::ServerContext* context, const ::nfa_msg::MigrationNegotiation* request, ::nfa_msg::MigrationNegotiationResult* response);
  };
  template <class BaseClass>
  class WithAsyncMethod_LivenessCheck : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service *service) {}
   public:
    WithAsyncMethod_LivenessCheck() {
      ::grpc::Service::MarkMethodAsync(0);
    }
    ~WithAsyncMethod_LivenessCheck() GRPC_OVERRIDE {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status LivenessCheck(::grpc::ServerContext* context, const ::nfa_msg::LivenessRequest* request, ::nfa_msg::LivenessReply* response) GRPC_FINAL GRPC_OVERRIDE {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    void RequestLivenessCheck(::grpc::ServerContext* context, ::nfa_msg::LivenessRequest* request, ::grpc::ServerAsyncResponseWriter< ::nfa_msg::LivenessReply>* response, ::grpc::CompletionQueue* new_call_cq, ::grpc::ServerCompletionQueue* notification_cq, void *tag) {
      ::grpc::Service::RequestAsyncUnary(0, context, request, response, new_call_cq, notification_cq, tag);
    }
  };
  template <class BaseClass>
  class WithAsyncMethod_SetMigrationTarget : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service *service) {}
   public:
    WithAsyncMethod_SetMigrationTarget() {
      ::grpc::Service::MarkMethodAsync(1);
    }
    ~WithAsyncMethod_SetMigrationTarget() GRPC_OVERRIDE {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status SetMigrationTarget(::grpc::ServerContext* context, const ::nfa_msg::MigrationTarget* request, ::nfa_msg::MigrationNegotiationResult* response) GRPC_FINAL GRPC_OVERRIDE {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    void RequestSetMigrationTarget(::grpc::ServerContext* context, ::nfa_msg::MigrationTarget* request, ::grpc::ServerAsyncResponseWriter< ::nfa_msg::MigrationNegotiationResult>* response, ::grpc::CompletionQueue* new_call_cq, ::grpc::ServerCompletionQueue* notification_cq, void *tag) {
      ::grpc::Service::RequestAsyncUnary(1, context, request, response, new_call_cq, notification_cq, tag);
    }
  };
  template <class BaseClass>
  class WithAsyncMethod_MigrationNegotiate : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service *service) {}
   public:
    WithAsyncMethod_MigrationNegotiate() {
      ::grpc::Service::MarkMethodAsync(2);
    }
    ~WithAsyncMethod_MigrationNegotiate() GRPC_OVERRIDE {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status MigrationNegotiate(::grpc::ServerContext* context, const ::nfa_msg::MigrationNegotiation* request, ::nfa_msg::MigrationNegotiationResult* response) GRPC_FINAL GRPC_OVERRIDE {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    void RequestMigrationNegotiate(::grpc::ServerContext* context, ::nfa_msg::MigrationNegotiation* request, ::grpc::ServerAsyncResponseWriter< ::nfa_msg::MigrationNegotiationResult>* response, ::grpc::CompletionQueue* new_call_cq, ::grpc::ServerCompletionQueue* notification_cq, void *tag) {
      ::grpc::Service::RequestAsyncUnary(2, context, request, response, new_call_cq, notification_cq, tag);
    }
  };
  typedef WithAsyncMethod_LivenessCheck<WithAsyncMethod_SetMigrationTarget<WithAsyncMethod_MigrationNegotiate<Service > > > AsyncService;
  template <class BaseClass>
  class WithGenericMethod_LivenessCheck : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service *service) {}
   public:
    WithGenericMethod_LivenessCheck() {
      ::grpc::Service::MarkMethodGeneric(0);
    }
    ~WithGenericMethod_LivenessCheck() GRPC_OVERRIDE {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status LivenessCheck(::grpc::ServerContext* context, const ::nfa_msg::LivenessRequest* request, ::nfa_msg::LivenessReply* response) GRPC_FINAL GRPC_OVERRIDE {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
  };
  template <class BaseClass>
  class WithGenericMethod_SetMigrationTarget : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service *service) {}
   public:
    WithGenericMethod_SetMigrationTarget() {
      ::grpc::Service::MarkMethodGeneric(1);
    }
    ~WithGenericMethod_SetMigrationTarget() GRPC_OVERRIDE {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status SetMigrationTarget(::grpc::ServerContext* context, const ::nfa_msg::MigrationTarget* request, ::nfa_msg::MigrationNegotiationResult* response) GRPC_FINAL GRPC_OVERRIDE {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
  };
  template <class BaseClass>
  class WithGenericMethod_MigrationNegotiate : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service *service) {}
   public:
    WithGenericMethod_MigrationNegotiate() {
      ::grpc::Service::MarkMethodGeneric(2);
    }
    ~WithGenericMethod_MigrationNegotiate() GRPC_OVERRIDE {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status MigrationNegotiate(::grpc::ServerContext* context, const ::nfa_msg::MigrationNegotiation* request, ::nfa_msg::MigrationNegotiationResult* response) GRPC_FINAL GRPC_OVERRIDE {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
  };
};

}  // namespace nfa_msg


#endif  // GRPC_nfa_5fmsg_2eproto__INCLUDED