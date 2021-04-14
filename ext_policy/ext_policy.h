#pragma once

#include <chrono>
#include <memory>

#include "ext_policy/ext_policy.pb.h"
#include "envoy/grpc/async_client.h"
#include "envoy/http/filter.h"
#include "envoy/stats/scope.h"
#include "envoy/stats/stats_macros.h"

#include "common/common/logger.h"

#include "extensions/filters/http/common/pass_through_filter.h"
#include "ext_policy/client.h"

namespace Envoy {
namespace Extensions {
namespace HttpFilters {
namespace ExternalPolicy {

#define ALL_EXT_POLICY_FILTER_STATS(COUNTER)                                                         \
  COUNTER(streams_started)                                                                           \
  COUNTER(streams_closed)                                                                            \
  COUNTER(streams_failed)                                                                            \
  COUNTER(stream_msgs_sent)                                                                          \
  COUNTER(policies_invoked)                                                                          \
  COUNTER(failure_mode_allowed)

struct ExtPolicyFilterStats {
  ALL_EXT_POLICY_FILTER_STATS(GENERATE_COUNTER_STRUCT)
};

class FilterConfig {
public:
  FilterConfig(const demo::ext_policy::ExternalPolicy& config,
               const std::chrono::milliseconds grpc_timeout, Stats::Scope& scope,
               const std::string& stats_prefix)
      : failure_mode_allow_(config.failure_mode_allow()), grpc_timeout_(grpc_timeout),
        stats_(generateStats(stats_prefix, config.stat_prefix(), scope)) {}

  bool failureModeAllow() const { return failure_mode_allow_; }

  const std::chrono::milliseconds& grpcTimeout() const { return grpc_timeout_; }

  const ExtPolicyFilterStats& stats() const { return stats_; }

private:
  ExtPolicyFilterStats generateStats(const std::string& prefix,
                                   const std::string& filter_stats_prefix, Stats::Scope& scope) {
    const std::string final_prefix = absl::StrCat(prefix, "ext_policy.", filter_stats_prefix);
    return {ALL_EXT_POLICY_FILTER_STATS(POOL_COUNTER_PREFIX(scope, final_prefix))};
  }

  const bool failure_mode_allow_;
  const std::chrono::milliseconds grpc_timeout_;

  ExtPolicyFilterStats stats_;
};

using FilterConfigSharedPtr = std::shared_ptr<FilterConfig>;

class Filter : public Logger::Loggable<Logger::Id::filter>,
               public Http::PassThroughFilter,
               public ExternalPolicyCallbacks {
public:
  Filter(const FilterConfigSharedPtr& config, ExternalPolicyClientPtr&& client)
      : config_(config), client_(std::move(client)), stats_(config->stats()) {}

  void onDestroy() override;

  void setDecoderFilterCallbacks(Http::StreamDecoderFilterCallbacks& callbacks) override {
    decoder_callbacks_ = &callbacks;
  }

  Http::FilterHeadersStatus decodeHeaders(Http::RequestHeaderMap& headers,
                                          bool end_stream) override;

  // ExternalPolicyCallbacks
  void onReceiveMessage(std::unique_ptr<policyservice::InvokeReply>&& response) override;
  void onGrpcError(Grpc::Status::GrpcStatus error) override;
  void onGrpcClose() override;

private:
  void closeStream();

  const FilterConfigSharedPtr config_;
  const ExternalPolicyClientPtr client_;
  ExtPolicyFilterStats stats_;

  Http::StreamDecoderFilterCallbacks* decoder_callbacks_ = nullptr;

  ExternalPolicyStreamPtr stream_;
  bool stream_closed_ = false;

  Http::HeaderMap* request_headers_ = nullptr;
};

} // namespace ExternalPolicy
} // namespace HttpFilters
} // namespace Extensions
} // namespace Envoy