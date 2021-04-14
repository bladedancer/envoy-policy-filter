#include "./mutation_utils.h"

#include "envoy/http/header_map.h"

#include "common/http/header_utility.h"
#include "common/http/headers.h"
#include "common/protobuf/utility.h"

namespace Envoy {
namespace Extensions {
namespace HttpFilters {
namespace ExternalPolicy {

using Http::Headers;
using Http::LowerCaseString;
using policyservice::InvokeReply;
using policyservice::InvokeRequest;

void MutationUtils::buildHttpHeaders(const Http::HeaderMap& headers_in, InvokeRequest& req) {
  headers_in.iterate([&req](const Http::HeaderEntry& e) -> Http::HeaderMap::Iterate {
    (*req.mutable_headers())[std::string(e.key().getStringView())] =
        std::string(e.value().getStringView());
    return Http::HeaderMap::Iterate::Continue;
  });
}

void MutationUtils::applyHeaderMutations(const policyservice::InvokeReply& response,
                                         Http::HeaderMap& headers) {
  headers.clear();
  for (const auto& h : response.headers()) {
    headers.addCopy(LowerCaseString(h.first), h.second);
  }
}

} // namespace ExternalPolicy
} // namespace HttpFilters
} // namespace Extensions
} // namespace Envoy