#include "./config.h"

#include <string>

#include "./client_impl.h"
#include "./ext_policy.h"

namespace Envoy {
namespace Extensions {
namespace HttpFilters {
namespace ExternalPolicy {

Http::FilterFactoryCb ExternalPolicyFilterConfig::createFilterFactoryFromProtoTyped(
    const demo::ext_policy::ExternalPolicy& proto_config,
    const std::string& stats_prefix,
    Server::Configuration::FactoryContext& context) {
  const uint32_t timeout_ms =
      PROTOBUF_GET_MS_OR_DEFAULT(proto_config.grpc_service(), timeout, DefaultTimeout);
  const auto filter_config = std::make_shared<FilterConfig>(
      proto_config, std::chrono::milliseconds(timeout_ms), context.scope(), stats_prefix);

  return [filter_config, grpc_service = proto_config.grpc_service(),
          &context](Http::FilterChainFactoryCallbacks& callbacks) {
    auto client = std::make_unique<ExternalPolicyClientImpl>(
        context.clusterManager().grpcAsyncClientManager(), grpc_service, context.scope());

    callbacks.addStreamFilter(
        Http::StreamFilterSharedPtr{std::make_shared<Filter>(filter_config, std::move(client))});
  };
}

// REGISTER_FACTORY(ExternalPolicyFilterConfig,
//                  Server::Configuration::NamedHttpFilterConfigFactory){"ext_policy"};

/**
 * Static registration for this sample filter. @see RegisterFactory.
 */
static Registry::RegisterFactory<ExternalPolicyFilterConfig, Server::Configuration::NamedHttpFilterConfigFactory>
    register_;

} // namespace ExternalPolicy
} // namespace HttpFilters
} // namespace Extensions
} // namespace Envoy