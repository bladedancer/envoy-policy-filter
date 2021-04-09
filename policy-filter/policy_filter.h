#pragma once

#include <string>

#include "extensions/filters/http/common/pass_through_filter.h"

#include "policy-filter/policy_filter.pb.h"

namespace Envoy {
namespace Http {

class PolicyFilterConfig {
public:
  PolicyFilterConfig(const policy::PolicyConfig& proto_config);

  const std::string& key() const { return key_; }
  const std::string& val() const { return val_; }

private:
  const std::string key_;
  const std::string val_;
};

using PolicyFilterConfigSharedPtr = std::shared_ptr<PolicyFilterConfig>;

class PolicyDecoderFilter : public PassThroughDecoderFilter {
public:
  PolicyDecoderFilter(PolicyFilterConfigSharedPtr);
  ~PolicyDecoderFilter();

  // Http::StreamFilterBase
  void onDestroy() override;

  // Http::StreamDecoderFilter
  FilterHeadersStatus decodeHeaders(RequestHeaderMap&, bool) override;
  FilterDataStatus decodeData(Buffer::Instance&, bool) override;
  void setDecoderFilterCallbacks(StreamDecoderFilterCallbacks&) override;

private:
  const PolicyFilterConfigSharedPtr config_;
  StreamDecoderFilterCallbacks* decoder_callbacks_;

  const LowerCaseString headerKey() const;
  const std::string headerValue() const;
};

} // namespace Http
} // namespace Envoy
