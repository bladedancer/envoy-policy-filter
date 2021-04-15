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
  static void buildHttpHeaders(const Http::HeaderMap&, policyservice::InvokeRequest&);
  static void applyHeaderMutations(const policyservice::InvokeReply&, Http::HeaderMap&);
  static void applyCommonBodyResponse(const policyservice::InvokeReply&, Buffer::Instance&);
};

} // namespace ExternalPolicy
} // namespace HttpFilters
} // namespace Extensions
} // namespace Envoy