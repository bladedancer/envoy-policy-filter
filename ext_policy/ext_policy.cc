#include "./ext_policy.h"
#include "./mutation_utils.h"
#include "absl/strings/str_format.h"
#include "common/buffer/buffer_impl.h"

namespace Envoy {
namespace Extensions {
namespace HttpFilters {
namespace ExternalPolicy {

using policyservice::InvokeReply;
using policyservice::InvokeRequest;

using Http::FilterDataStatus;
using Http::FilterHeadersStatus;
using Http::RequestHeaderMap;
using Http::ResponseHeaderMap;

static const std::string kErrorPrefix = "ext_policy error";

void Filter::setDecoderFilterCallbacks(Http::StreamDecoderFilterCallbacks& callbacks) {
  decoder_callbacks_ = &callbacks;
}

void Filter::setEncoderFilterCallbacks(Http::StreamEncoderFilterCallbacks& callbacks) {
  encoder_callbacks_ = &callbacks;
}

void Filter::closeStream() {
  if (!stream_closed_) {
    if (stream_) {
      ENVOY_LOG(debug, "Closing gRPC stream to processing server");
      stream_->close();
      stats_.streams_closed_.inc();
    }
    stream_closed_ = true;
  }
}

void Filter::onDestroy() { closeStream(); }

FilterHeadersStatus Filter::decodeHeaders(RequestHeaderMap& headers, bool end_of_stream) {
  // We're at the start, so start the stream and send a headers message
  request_headers_ = &headers;
  stream_ = client_->start(*this, config_->grpcTimeout());
  stats_.streams_started_.inc();
  InvokeRequest req;

  MutationUtils::buildHttpHeaders(headers, req);
  req.set_id(std::string(headers.getRequestIdValue()));
  req.set_policy("get this from metadata?");
  req.set_endofstream(end_of_stream);

  filter_state_ = FilterState::Request;
  stream_->send(std::move(req), false);
  stats_.stream_msgs_sent_.inc();

  // Wait until we have a gRPC response before allowing any more callbacks
  return FilterHeadersStatus::StopIteration;
}

FilterDataStatus Filter::decodeData(Buffer::Instance& data, bool end_of_stream) {
  ENVOY_LOG(trace, "decodeData({}): end_stream = {}", data.length(), end_of_stream);

  InvokeRequest req;
  req.set_body(data.linearize(data.length()), data.length());
  req.set_endofstream(end_of_stream);

  stream_->send(std::move(req), false);
  stats_.stream_msgs_sent_.inc();

  return FilterDataStatus::StopIterationAndWatermark;
}

FilterHeadersStatus Filter::encodeHeaders(ResponseHeaderMap& headers, bool end_of_stream) {
  response_headers_ = &headers;

  if (stream_closed_) {
    // Perhaps we should keep it open - at the moment the demo server is closing it.
    stream_ = client_->start(*this, config_->grpcTimeout());
    stats_.streams_started_.inc();
  }

  InvokeRequest req;
  MutationUtils::buildHttpHeaders(headers, req);
  req.set_id(std::string(headers.getRequestIdValue()));
  req.set_policy("get resp policy this from metadata?");
  req.set_endofstream(end_of_stream);

  filter_state_ = FilterState::Response;
  stream_->send(std::move(req), false);
  stats_.stream_msgs_sent_.inc();

  // Wait until we have a gRPC response before allowing any more callbacks
  return FilterHeadersStatus::StopIteration;
}

FilterDataStatus Filter::encodeData(Buffer::Instance& data, bool end_of_stream) {
  ENVOY_LOG(trace, "encodeData({}): end_stream = {}", data.length(), end_of_stream);

  InvokeRequest req;
  req.set_body(data.linearize(data.length()), data.length());
  req.set_endofstream(end_of_stream);

  stream_->send(std::move(req), false);
  stats_.stream_msgs_sent_.inc();

  return FilterDataStatus::StopIterationAndWatermark;
}

void Filter::onReceiveMessage(std::unique_ptr<InvokeReply>&& r) {
  auto response = std::move(r);
  // bool message_valid = false;
  ENVOY_LOG(debug, "Received gRPC message.");

  if (filter_state_ == FilterState::Request) {
    // TODO - Update the headers
    if (request_headers_ != nullptr) {
      MutationUtils::applyHeaderMutations(*response, *request_headers_);
      request_headers_ = nullptr;
      decoder_callbacks_->clearRouteCache();
    }

    decoder_callbacks_->modifyDecodingBuffer([&response](Buffer::Instance& dec_buf) {
      Buffer::OwnedImpl body(response->body());
      dec_buf.drain(dec_buf.length());
      dec_buf.move(body);
    });

    if (response->endofstream()) {
      decoder_callbacks_->continueDecoding();
    }

  } else if (filter_state_ == FilterState::Response) {
    if (response_headers_ != nullptr) {
      MutationUtils::applyHeaderMutations(*response, *response_headers_);
      response_headers_ = nullptr;
    }

    Buffer::OwnedImpl data(response->body());
    encoder_callbacks_->modifyEncodingBuffer([&response](Buffer::Instance& enc_buf) {
      Buffer::OwnedImpl body(response->body());
      enc_buf.drain(enc_buf.length());
      enc_buf.move(body);
    });

    if (response->endofstream()) {
      encoder_callbacks_->continueEncoding();
    }
  }

  // TODO: Could manage stream state much better here.
}

void Filter::onGrpcError(Grpc::Status::GrpcStatus status) {
  ENVOY_LOG(debug, "Received gRPC error on stream: {}", status);
  stream_closed_ = true;
  stats_.streams_failed_.inc();
  if (config_->failureModeAllow()) {
    // Ignore this and treat as a successful close
    onGrpcClose();
    stats_.failure_mode_allowed_.inc();
  } else {
    filter_state_ = FilterState::Idle;
    decoder_callbacks_->sendLocalReply(Http::Code::InternalServerError, "", nullptr, absl::nullopt,
                                       absl::StrFormat("%s: gRPC error %i", kErrorPrefix, status));
  }
}

void Filter::onGrpcClose() {
  ENVOY_LOG(debug, "Received gRPC stream close");
  stream_closed_ = true;
  stats_.streams_closed_.inc();
  // TODO: CONTINUE
}

} // namespace ExternalPolicy
} // namespace HttpFilters
} // namespace Extensions
} // namespace Envoy