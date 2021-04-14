#pragma once

#include <memory>

#include "envoy/common/pure.h"
#include "envoy/grpc/status.h"
#include "ext_policy/external_policy.pb.h"

namespace Envoy {
namespace Extensions {
namespace HttpFilters {
namespace ExternalPolicy {

class ExternalPolicyStream {
public:
  virtual ~ExternalPolicyStream() = default;
  virtual void send(policyservice::InvokeRequest&& request, bool end_stream) PURE;
  virtual bool close() PURE;
};

using ExternalPolicyStreamPtr = std::unique_ptr<ExternalPolicyStream>;

class ExternalPolicyCallbacks {
public:
  virtual ~ExternalPolicyCallbacks() = default;
  virtual void onReceiveMessage(std::unique_ptr<policyservice::InvokeReply>&& response) PURE;
  virtual void onGrpcError(Grpc::Status::GrpcStatus error) PURE;
  virtual void onGrpcClose() PURE;
};

class ExternalPolicyClient {
public:
  virtual ~ExternalPolicyClient() = default;
  virtual ExternalPolicyStreamPtr start(ExternalPolicyCallbacks& callbacks,
                                        const std::chrono::milliseconds& timeout) PURE;
};

using ExternalPolicyClientPtr = std::unique_ptr<ExternalPolicyClient>;

} // namespace ExternalPolicy
} // namespace HttpFilters
} // namespace Extensions
} // namespace Envoy
