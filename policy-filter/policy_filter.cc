#include <string>

#include "policy_filter.h"

#include "envoy/server/filter_config.h"

namespace Envoy {
namespace Http {

PolicyFilterConfig::PolicyFilterConfig(
    const policy::PolicyConfig& proto_config)
    : key_(proto_config.key()), val_(proto_config.val()) {}

PolicyDecoderFilter::PolicyDecoderFilter(PolicyFilterConfigSharedPtr config)
    : config_(config) {}

PolicyDecoderFilter::~PolicyDecoderFilter() {}

void PolicyDecoderFilter::onDestroy() {}

const LowerCaseString PolicyDecoderFilter::headerKey() const {
  return LowerCaseString(config_->key());
}

const std::string PolicyDecoderFilter::headerValue() const {
  return config_->val();
}

FilterHeadersStatus PolicyDecoderFilter::decodeHeaders(RequestHeaderMap& headers, bool) {
  // add a header
  headers.addCopy(headerKey(), headerValue());

  return FilterHeadersStatus::Continue;
}

FilterDataStatus PolicyDecoderFilter::decodeData(Buffer::Instance&, bool) {
  return FilterDataStatus::Continue;
}

void PolicyDecoderFilter::setDecoderFilterCallbacks(StreamDecoderFilterCallbacks& callbacks) {
  decoder_callbacks_ = &callbacks;
}

} // namespace Http
} // namespace Envoy
