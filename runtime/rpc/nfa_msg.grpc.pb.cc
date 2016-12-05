// Generated by the gRPC protobuf plugin.
// If you make any local change, they will be lost.
// source: nfa_msg.proto

#include "nfa_msg.pb.h"
#include "nfa_msg.grpc.pb.h"

#include <grpc++/impl/codegen/async_stream.h>
#include <grpc++/impl/codegen/async_unary_call.h>
#include <grpc++/impl/codegen/channel_interface.h>
#include <grpc++/impl/codegen/client_unary_call.h>
#include <grpc++/impl/codegen/method_handler_impl.h>
#include <grpc++/impl/codegen/rpc_service_method.h>
#include <grpc++/impl/codegen/service_type.h>
#include <grpc++/impl/codegen/sync_stream.h>
namespace nfa_msg {

static const char* Runtime_RPC_method_names[] = {
  "/nfa_msg.Runtime_RPC/LivenessCheck",
  "/nfa_msg.Runtime_RPC/SetMigrationTarget",
  "/nfa_msg.Runtime_RPC/MigrationNegotiate",
};

std::unique_ptr< Runtime_RPC::Stub> Runtime_RPC::NewStub(const std::shared_ptr< ::grpc::ChannelInterface>& channel, const ::grpc::StubOptions& options) {
  std::unique_ptr< Runtime_RPC::Stub> stub(new Runtime_RPC::Stub(channel));
  return stub;
}

Runtime_RPC::Stub::Stub(const std::shared_ptr< ::grpc::ChannelInterface>& channel)
  : channel_(channel), rpcmethod_LivenessCheck_(Runtime_RPC_method_names[0], ::grpc::RpcMethod::NORMAL_RPC, channel)
  , rpcmethod_SetMigrationTarget_(Runtime_RPC_method_names[1], ::grpc::RpcMethod::NORMAL_RPC, channel)
  , rpcmethod_MigrationNegotiate_(Runtime_RPC_method_names[2], ::grpc::RpcMethod::NORMAL_RPC, channel)
  {}

::grpc::Status Runtime_RPC::Stub::LivenessCheck(::grpc::ClientContext* context, const ::nfa_msg::LivenessRequest& request, ::nfa_msg::LivenessReply* response) {
  return ::grpc::BlockingUnaryCall(channel_.get(), rpcmethod_LivenessCheck_, context, request, response);
}

::grpc::ClientAsyncResponseReader< ::nfa_msg::LivenessReply>* Runtime_RPC::Stub::AsyncLivenessCheckRaw(::grpc::ClientContext* context, const ::nfa_msg::LivenessRequest& request, ::grpc::CompletionQueue* cq) {
  return new ::grpc::ClientAsyncResponseReader< ::nfa_msg::LivenessReply>(channel_.get(), cq, rpcmethod_LivenessCheck_, context, request);
}

::grpc::Status Runtime_RPC::Stub::SetMigrationTarget(::grpc::ClientContext* context, const ::nfa_msg::MigrationTarget& request, ::nfa_msg::MigrationNegotiationResult* response) {
  return ::grpc::BlockingUnaryCall(channel_.get(), rpcmethod_SetMigrationTarget_, context, request, response);
}

::grpc::ClientAsyncResponseReader< ::nfa_msg::MigrationNegotiationResult>* Runtime_RPC::Stub::AsyncSetMigrationTargetRaw(::grpc::ClientContext* context, const ::nfa_msg::MigrationTarget& request, ::grpc::CompletionQueue* cq) {
  return new ::grpc::ClientAsyncResponseReader< ::nfa_msg::MigrationNegotiationResult>(channel_.get(), cq, rpcmethod_SetMigrationTarget_, context, request);
}

::grpc::Status Runtime_RPC::Stub::MigrationNegotiate(::grpc::ClientContext* context, const ::nfa_msg::MigrationNegotiation& request, ::nfa_msg::MigrationNegotiationResult* response) {
  return ::grpc::BlockingUnaryCall(channel_.get(), rpcmethod_MigrationNegotiate_, context, request, response);
}

::grpc::ClientAsyncResponseReader< ::nfa_msg::MigrationNegotiationResult>* Runtime_RPC::Stub::AsyncMigrationNegotiateRaw(::grpc::ClientContext* context, const ::nfa_msg::MigrationNegotiation& request, ::grpc::CompletionQueue* cq) {
  return new ::grpc::ClientAsyncResponseReader< ::nfa_msg::MigrationNegotiationResult>(channel_.get(), cq, rpcmethod_MigrationNegotiate_, context, request);
}

Runtime_RPC::Service::Service() {
  (void)Runtime_RPC_method_names;
  AddMethod(new ::grpc::RpcServiceMethod(
      Runtime_RPC_method_names[0],
      ::grpc::RpcMethod::NORMAL_RPC,
      new ::grpc::RpcMethodHandler< Runtime_RPC::Service, ::nfa_msg::LivenessRequest, ::nfa_msg::LivenessReply>(
          std::mem_fn(&Runtime_RPC::Service::LivenessCheck), this)));
  AddMethod(new ::grpc::RpcServiceMethod(
      Runtime_RPC_method_names[1],
      ::grpc::RpcMethod::NORMAL_RPC,
      new ::grpc::RpcMethodHandler< Runtime_RPC::Service, ::nfa_msg::MigrationTarget, ::nfa_msg::MigrationNegotiationResult>(
          std::mem_fn(&Runtime_RPC::Service::SetMigrationTarget), this)));
  AddMethod(new ::grpc::RpcServiceMethod(
      Runtime_RPC_method_names[2],
      ::grpc::RpcMethod::NORMAL_RPC,
      new ::grpc::RpcMethodHandler< Runtime_RPC::Service, ::nfa_msg::MigrationNegotiation, ::nfa_msg::MigrationNegotiationResult>(
          std::mem_fn(&Runtime_RPC::Service::MigrationNegotiate), this)));
}

Runtime_RPC::Service::~Service() {
}

::grpc::Status Runtime_RPC::Service::LivenessCheck(::grpc::ServerContext* context, const ::nfa_msg::LivenessRequest* request, ::nfa_msg::LivenessReply* response) {
  (void) context;
  (void) request;
  (void) response;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}

::grpc::Status Runtime_RPC::Service::SetMigrationTarget(::grpc::ServerContext* context, const ::nfa_msg::MigrationTarget* request, ::nfa_msg::MigrationNegotiationResult* response) {
  (void) context;
  (void) request;
  (void) response;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}

::grpc::Status Runtime_RPC::Service::MigrationNegotiate(::grpc::ServerContext* context, const ::nfa_msg::MigrationNegotiation* request, ::nfa_msg::MigrationNegotiationResult* response) {
  (void) context;
  (void) request;
  (void) response;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}


}  // namespace nfa_msg
