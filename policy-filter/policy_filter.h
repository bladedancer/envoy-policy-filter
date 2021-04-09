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

class PolicyFilter : public PassThroughFilter {
public:
  PolicyFilter(PolicyFilterConfigSharedPtr);
  ~PolicyFilter();

  // Http::StreamFilterBase
  void onDestroy() override;

  // Http::StreamEncoderFilter (Response)
  Http::FilterHeadersStatus encodeHeaders(Http::ResponseHeaderMap&, bool) override;

  // Http::StreamDecoderFilter (Requests)
  FilterHeadersStatus decodeHeaders(RequestHeaderMap&, bool) override;
  FilterDataStatus decodeData(Buffer::Instance&, bool) override;

private:
  const PolicyFilterConfigSharedPtr config_;

  const LowerCaseString headerKey() const;
  const std::string headerValue() const;
};

} // namespace Http
} // namespace Envoy
