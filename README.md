# Envoy filter example

This project demonstrates the linking of additional HTTP filters with the Envoy binary.
A new filter `sample` which adds a HTTP header is introduced.
Integration tests demonstrating the filter's end-to-end behavior are
also provided.

## Building

To build the Envoy static binary:

1. `git submodule update --init`
2. `cd envoy && git checkout v1.17.1 && cd ..`
3. ```ENVOY_DOCKER_BUILD_DIR=`pwd`/build ./envoy/ci/run_envoy_docker.sh "bazel build //ext_policy:envoy"```

## Debug Build

`ENVOY_DOCKER_BUILD_DIR=`pwd`/build ./envoy/ci/run_envoy_docker.sh "bazel build -c dbg //ext_policy:envoy //ext_policy:envoy.dwp"`


## Testing
To run the `policy` integration test:

`./envoy/ci/run_envoy_docker.sh "bazel test //policy-filter:policy_filter_integration_test"`

## How it works

The [Envoy repository](https://github.com/envoyproxy/envoy/) is provided as a submodule.
The [`WORKSPACE`](WORKSPACE) file maps the `@envoy` repository to this local path.

The [`BUILD`](BUILD) file introduces a new Envoy static binary target, `envoy`,
that links together the new filter and `@envoy//source/exe:envoy_main_entry_lib`. The
`echo2` filter registers itself during the static initialization phase of the
Envoy binary as a new filter.

## How to write and use an HTTP filter

- The main task is to write a class that implements the interface
 [`Envoy::Http::StreamDecoderFilter`][StreamDecoderFilter] as in
 [`http_filter.h`](http_filter.h) and [`http_filter.cc`](http_filter.cc),
 which contains functions that handle http headers, data, and trailers.
 Note that this is an example of decoder filters, 
 and to write encoder filters or decoder/encoder filters
 you need to implement 
 [`Envoy::Http::StreamEncoderFilter`][StreamEncoderFilter] or
 [`Envoy::Http::StreamFilter`][StreamFilter] instead.
- You also need a class that implements 
 `Envoy::Server::Configuration::NamedHttpFilterConfigFactory`
 to enable the Envoy binary to find your filter,
 as in [`http_filter_config.cc`](http_filter_config.cc).
 It should be linked to the Envoy binary by modifying [`BUILD`][BUILD] file.
- Finally, you need to modify the Envoy config file to add your filter to the
 filter chain for a particular HTTP route configuration. For instance, if you
 wanted to change [the front-proxy example][front-envoy.yaml] to chain our
 `sample` filter, you'd need to modify its config to look like

```yaml
http_filters:
- name: sample          # before envoy.router because order matters!
  typed_config:
    "@type": type.googleapis.com/sample.Decoder
    key: via
    val: sample-filter
- name: envoy.router
  typed_config: {}
```
 

[StreamDecoderFilter]: https://github.com/envoyproxy/envoy/blob/b2610c84aeb1f75c804d67effcb40592d790e0f1/include/envoy/http/filter.h#L300
[StreamEncoderFilter]: https://github.com/envoyproxy/envoy/blob/b2610c84aeb1f75c804d67effcb40592d790e0f1/include/envoy/http/filter.h#L413
[StreamFilter]: https://github.com/envoyproxy/envoy/blob/b2610c84aeb1f75c804d67effcb40592d790e0f1/include/envoy/http/filter.h#L462
[BUILD]: https://github.com/envoyproxy/envoy-filter-example/blob/d76d3096c4cbd647d26b44b3f801c3afbc81d3e2/http-filter-example/BUILD#L15-L18
[front-envoy.yaml]: https://github.com/envoyproxy/envoy/blob/b2610c84aeb1f75c804d67effcb40592d790e0f1/examples/front-proxy/front-envoy.yaml#L28
