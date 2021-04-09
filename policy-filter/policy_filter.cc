#include <string>

#include "policy_filter.h"

#include "envoy/server/filter_config.h"

namespace Envoy {
namespace Http {

PolicyFilterConfig::PolicyFilterConfig(
    const policy::PolicyConfig& proto_config)
    : key_(proto_config.key()), val_(proto_config.val()) {}

PolicyFilter::PolicyFilter(PolicyFilterConfigSharedPtr config)
    : config_(config) {}

PolicyFilter::~PolicyFilter() {}

void PolicyFilter::onDestroy() {}

const LowerCaseString PolicyFilter::headerKey() const {
  return LowerCaseString(config_->key());
}

const std::string PolicyFilter::headerValue() const {
  return config_->val();
}

FilterHeadersStatus PolicyFilter::encodeHeaders(ResponseHeaderMap& headers, bool) {
  // add a header
  headers.addCopy(headerKey(), headerValue());

  return FilterHeadersStatus::Continue;
}

FilterHeadersStatus PolicyFilter::decodeHeaders(RequestHeaderMap& headers, bool) {
  // add a header
  headers.addCopy(headerKey(), headerValue());

  return FilterHeadersStatus::Continue;
}

FilterDataStatus PolicyFilter::decodeData(Buffer::Instance&, bool) {
  return FilterDataStatus::Continue;
}


} // namespace Http
} // namespace Envoy
