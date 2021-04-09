#include <string>

#include "envoy/registry/registry.h"
#include "envoy/server/filter_config.h"

#include "policy-filter/policy_filter.pb.h"
#include "policy-filter/policy_filter.pb.validate.h"
#include "policy_filter.h"

namespace Envoy {
namespace Server {
namespace Configuration {

class PolicyFilterConfigFactory : public NamedHttpFilterConfigFactory {
public:
  Http::FilterFactoryCb createFilterFactoryFromProto(const Protobuf::Message& proto_config,
                                                     const std::string&,
                                                     FactoryContext& context) override {

    return createFilter(Envoy::MessageUtil::downcastAndValidate<const policy::PolicyConfig&>(
                            proto_config, context.messageValidationVisitor()),
                        context);
  }

  /**
   *  Return the Protobuf Message that represents your config incase you have config proto
   */
  ProtobufTypes::MessagePtr createEmptyConfigProto() override {
    return ProtobufTypes::MessagePtr{new policy::PolicyConfig()};
  }

  std::string name() const override { return "policy"; }

private:
  Http::FilterFactoryCb createFilter(const policy::PolicyConfig& proto_config, FactoryContext&) {
    Http::PolicyFilterConfigSharedPtr config =
        std::make_shared<Http::PolicyFilterConfig>(
            Http::PolicyFilterConfig(proto_config));

    return [config](Http::FilterChainFactoryCallbacks& callbacks) -> void {
      auto filter = new Http::PolicyDecoderFilter(config);
      callbacks.addStreamDecoderFilter(Http::StreamDecoderFilterSharedPtr{filter});
    };
  }
};

/**
 * Static registration for this sample filter. @see RegisterFactory.
 */
static Registry::RegisterFactory<PolicyFilterConfigFactory, NamedHttpFilterConfigFactory>
    register_;

} // namespace Configuration
} // namespace Server
} // namespace Envoy
