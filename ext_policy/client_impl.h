#pragma once

#include <memory>
#include <string>

#include "envoy/config/core/v3/grpc_service.pb.h"
#include "envoy/grpc/async_client_manager.h"
#include "ext_policy/external_policy.pb.h"
#include "envoy/stats/scope.h"

#include "common/grpc/typed_async_client.h"

#include "./client.h"

using policyservice::InvokeReply;
using policyservice::InvokeRequest;

namespace Envoy {
namespace Extensions {
namespace HttpFilters {
namespace ExternalPolicy {

using InvokeReplyPtr = std::unique_ptr<InvokeReply>;

class ExternalPolicyClientImpl : public ExternalPolicyClient {
public:
  ExternalPolicyClientImpl(Grpc::AsyncClientManager& client_manager,
                           const envoy::config::core::v3::GrpcService& grpc_service,
                           Stats::Scope& scope);

  ExternalPolicyStreamPtr start(ExternalPolicyCallbacks& callbacks,
                                const std::chrono::milliseconds& timeout) override;

private:
  Grpc::AsyncClientFactoryPtr factory_;
};

class ExternalPolicyStreamImpl : public ExternalPolicyStream,
                                 public Grpc::AsyncStreamCallbacks<InvokeReply>,
                                 public Logger::Loggable<Logger::Id::filter> {
public:
  ExternalPolicyStreamImpl(Grpc::AsyncClient<InvokeRequest, InvokeReply>&& client,
                           ExternalPolicyCallbacks& callbacks,
                           const std::chrono::milliseconds& timeout);
  void send(InvokeRequest&& request, bool end_stream) override;
  // Close the stream. This is idempotent and will return true if we
  // actually closed it.
  bool close() override;

  // AsyncStreamCallbacks
  void onReceiveMessage(InvokeReplyPtr&& message) override;

  // RawAsyncStreamCallbacks
  void onCreateInitialMetadata(Http::RequestHeaderMap& metadata) override;
  void onReceiveInitialMetadata(Http::ResponseHeaderMapPtr&& metadata) override;
  void onReceiveTrailingMetadata(Http::ResponseTrailerMapPtr&& metadata) override;
  void onRemoteClose(Grpc::Status::GrpcStatus status, const std::string& message) override;

private:
  ExternalPolicyCallbacks& callbacks_;
  Grpc::AsyncClient<InvokeRequest, InvokeReply> client_;
  Grpc::AsyncStream<InvokeRequest> stream_;
  bool stream_closed_ = false;
};

} // namespace ExternalPolicy
} // namespace HttpFilters
} // namespace Extensions
} // namespace Envoy
