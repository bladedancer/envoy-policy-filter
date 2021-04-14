#pragma once

#include "envoy/buffer/buffer.h"
#include "envoy/http/header_map.h"
#include "ext_policy/external_policy.pb.h"

namespace Envoy {
namespace Extensions {
namespace HttpFilters {
namespace ExternalPolicy {

class MutationUtils {
public:
  // Convert a header map into a protobuf
  static void buildHttpHeaders(const Http::HeaderMap& headers_in, policyservice::InvokeRequest& request);
  // Modify header map based on a set of mutations from a protobuf
  static void applyHeaderMutations(const policyservice::InvokeReply& response, Http::HeaderMap& headers);
};

} // namespace ExternalPolicy
} // namespace HttpFilters
} // namespace Extensions
} // namespace Envoy