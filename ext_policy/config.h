#pragma once

#include <string>

#include "ext_policy/ext_policy.pb.h"
#include "ext_policy/ext_policy.pb.validate.h"

#include "extensions/filters/http/common/factory_base.h"

namespace Envoy {
namespace Extensions {
namespace HttpFilters {
namespace ExternalPolicy {

class ExternalPolicyFilterConfig
    : public Common::FactoryBase<demo::ext_policy::ExternalPolicy> {

public:
  ExternalPolicyFilterConfig() : FactoryBase("ext_policy") {}

private:
  static constexpr uint64_t DefaultTimeout = 200;

  Http::FilterFactoryCb createFilterFactoryFromProtoTyped(
      const demo::ext_policy::ExternalPolicy& proto_config,
      const std::string& stats_prefix, 
      Server::Configuration::FactoryContext& context) override;
};

} // namespace ExternalPolicy
} // namespace HttpFilters
} // namespace Extensions
} // namespace Envoy