#include "./client_impl.h"

namespace Envoy {
namespace Extensions {
namespace HttpFilters {
namespace ExternalPolicy {

static constexpr char kExternalMethod[] = "policyservice.PolicyServer.InvokePolicy";

ExternalPolicyClientImpl::ExternalPolicyClientImpl(
    Grpc::AsyncClientManager& client_manager,
    const envoy::config::core::v3::GrpcService& grpc_service, Stats::Scope& scope) {
  factory_ = client_manager.factoryForGrpcService(grpc_service, scope, true);
}

ExternalPolicyStreamPtr ExternalPolicyClientImpl::start(ExternalPolicyCallbacks& callbacks,
                                                        const std::chrono::milliseconds& timeout) {
  Grpc::AsyncClient<InvokeRequest, InvokeReply> grpcClient(factory_->create());
  return std::make_unique<ExternalPolicyStreamImpl>(std::move(grpcClient), callbacks, timeout);
}

ExternalPolicyStreamImpl::ExternalPolicyStreamImpl(
    Grpc::AsyncClient<InvokeRequest, InvokeReply>&& client, ExternalPolicyCallbacks& callbacks,
    const std::chrono::milliseconds& timeout)
    : callbacks_(callbacks) {
  client_ = std::move(client);
  auto descriptor = Protobuf::DescriptorPool::generated_pool()->FindMethodByName(kExternalMethod);
  Http::AsyncClient::StreamOptions options;
  options.setTimeout(timeout);

  stream_ = client_.start(*descriptor, *this, options);
}

void ExternalPolicyStreamImpl::send(policyservice::InvokeRequest&& request, bool end_stream) {
  stream_.sendMessage(std::move(request), end_stream);
}

bool ExternalPolicyStreamImpl::close() {
  if (!stream_closed_) {
    ENVOY_LOG(debug, "Closing gRPC stream");
    stream_->closeStream();
    stream_closed_ = true;
    return true;
  }
  return false;
}

void ExternalPolicyStreamImpl::onReceiveMessage(InvokeReplyPtr&& response) {
  callbacks_.onReceiveMessage(std::move(response));
}

void ExternalPolicyStreamImpl::onCreateInitialMetadata(Http::RequestHeaderMap&) {}
void ExternalPolicyStreamImpl::onReceiveInitialMetadata(Http::ResponseHeaderMapPtr&&) {}
void ExternalPolicyStreamImpl::onReceiveTrailingMetadata(Http::ResponseTrailerMapPtr&&) {}
void ExternalPolicyStreamImpl::onRemoteClose(Grpc::Status::GrpcStatus status,
                                             const std::string& message) {
  ENVOY_LOG(debug, "gRPC stream closed remotely with status {}: {}", status, message);
  stream_closed_ = true;
  if (status == Grpc::Status::Ok) {
    callbacks_.onGrpcClose();
  } else {
    callbacks_.onGrpcError(status);
  }
}

} // namespace ExternalPolicy
} // namespace HttpFilters
} // namespace Extensions
} // namespace Envoy
