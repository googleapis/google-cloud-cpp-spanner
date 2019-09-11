# Google Cloud Spanner C++ Client Library

Cloud Spanner is the only enterprise-grade, globally-distributed, and
strongly consistent database service built for the cloud specifically
to combine the benefits of relational database structure with
non-relational horizontal scale. This combination delivers
high-performance transactions and strong consistency across rows,
regions, and continents with an industry-leading 99.999% availability
SLA, no planned downtime, and enterprise-grade security. Cloud Spanner
revolutionizes database administration and management and makes
application development more efficient.

This repository contains the implementation of the Cloud Spanner C++
client library.

## Status

This library is under active development, it is known to be incomplete,
and is subject to change without notice. We do not recommend that you use
this library for experimental code, nor for production workloads.

<!-- Start of automatically generated content by ci/generate-badges.sh -->

**Core Builds**
[![CI status docker/asan][docker/asan-shield]][docker/asan-link]
[![CI status docker/bazel-dependency][docker/bazel-dependency-shield]][docker/bazel-dependency-link]
[![CI status docker/clang-3.8][docker/clang-3.8-shield]][docker/clang-3.8-link]
[![CI status docker/clang-tidy][docker/clang-tidy-shield]][docker/clang-tidy-link]
[![CI status docker/cmake][docker/cmake-shield]][docker/cmake-link]
[![CI status docker/cmake-super][docker/cmake-super-shield]][docker/cmake-super-link]
[![CI status docker/coverage][docker/coverage-shield]][docker/coverage-link]
[![CI status docker/cxx17][docker/cxx17-shield]][docker/cxx17-link]
[![CI status docker/gcc-4.8][docker/gcc-4.8-shield]][docker/gcc-4.8-link]
[![CI status docker/integration][docker/integration-shield]][docker/integration-link]
[![CI status docker/msan][docker/msan-shield]][docker/msan-link]
[![CI status docker/noex][docker/noex-shield]][docker/noex-link]
[![CI status docker/tsan][docker/tsan-shield]][docker/tsan-link]
[![CI status docker/ubsan][docker/ubsan-shield]][docker/ubsan-link]
[![CI status macos/bazel][macos/bazel-shield]][macos/bazel-link]
[![CI status macos/cmake-super][macos/cmake-super-shield]][macos/cmake-super-link]
[![CI status windows/bazel][windows/bazel-shield]][windows/bazel-link]
[![CI status windows/cmake][windows/cmake-shield]][windows/cmake-link]
[![Code Coverage Status][codecov-io-badge]][codecov-io-link]
[![Link to Reference Documentation][doxygen-shield]][doxygen-link]

**Install Instructions**
[![CI status install/fedora][install/fedora-shield]][install/fedora-link]

[docker/asan-link]: https://storage.googleapis.com/cloud-cpp-kokoro-status/spanner/docker/asan-link.html
[docker/asan-shield]: https://storage.googleapis.com/cloud-cpp-kokoro-status/spanner/docker/asan.svg
[docker/bazel-dependency-link]: https://storage.googleapis.com/cloud-cpp-kokoro-status/spanner/docker/bazel-dependency-link.html
[docker/bazel-dependency-shield]: https://storage.googleapis.com/cloud-cpp-kokoro-status/spanner/docker/bazel-dependency.svg
[docker/clang-3.8-link]: https://storage.googleapis.com/cloud-cpp-kokoro-status/spanner/docker/clang-3.8-link.html
[docker/clang-3.8-shield]: https://storage.googleapis.com/cloud-cpp-kokoro-status/spanner/docker/clang-3.8.svg
[docker/clang-tidy-link]: https://storage.googleapis.com/cloud-cpp-kokoro-status/spanner/docker/clang-tidy-link.html
[docker/clang-tidy-shield]: https://storage.googleapis.com/cloud-cpp-kokoro-status/spanner/docker/clang-tidy.svg
[docker/cmake-link]: https://storage.googleapis.com/cloud-cpp-kokoro-status/spanner/docker/cmake-link.html
[docker/cmake-shield]: https://storage.googleapis.com/cloud-cpp-kokoro-status/spanner/docker/cmake.svg
[docker/cmake-super-link]: https://storage.googleapis.com/cloud-cpp-kokoro-status/spanner/docker/cmake-super-link.html
[docker/cmake-super-shield]: https://storage.googleapis.com/cloud-cpp-kokoro-status/spanner/docker/cmake-super.svg
[docker/coverage-link]: https://storage.googleapis.com/cloud-cpp-kokoro-status/spanner/docker/coverage-link.html
[docker/coverage-shield]: https://storage.googleapis.com/cloud-cpp-kokoro-status/spanner/docker/coverage.svg
[docker/cxx17-link]: https://storage.googleapis.com/cloud-cpp-kokoro-status/spanner/docker/cxx17-link.html
[docker/cxx17-shield]: https://storage.googleapis.com/cloud-cpp-kokoro-status/spanner/docker/cxx17.svg
[docker/gcc-4.8-link]: https://storage.googleapis.com/cloud-cpp-kokoro-status/spanner/docker/gcc-4.8-link.html
[docker/gcc-4.8-shield]: https://storage.googleapis.com/cloud-cpp-kokoro-status/spanner/docker/gcc-4.8.svg
[docker/integration-link]: https://storage.googleapis.com/cloud-cpp-kokoro-status/spanner/docker/integration-link.html
[docker/integration-shield]: https://storage.googleapis.com/cloud-cpp-kokoro-status/spanner/docker/integration.svg
[docker/msan-link]: https://storage.googleapis.com/cloud-cpp-kokoro-status/spanner/docker/msan-link.html
[docker/msan-shield]: https://storage.googleapis.com/cloud-cpp-kokoro-status/spanner/docker/msan.svg
[docker/noex-link]: https://storage.googleapis.com/cloud-cpp-kokoro-status/spanner/docker/noex-link.html
[docker/noex-shield]: https://storage.googleapis.com/cloud-cpp-kokoro-status/spanner/docker/noex.svg
[docker/tsan-link]: https://storage.googleapis.com/cloud-cpp-kokoro-status/spanner/docker/tsan-link.html
[docker/tsan-shield]: https://storage.googleapis.com/cloud-cpp-kokoro-status/spanner/docker/tsan.svg
[docker/ubsan-link]: https://storage.googleapis.com/cloud-cpp-kokoro-status/spanner/docker/ubsan-link.html
[docker/ubsan-shield]: https://storage.googleapis.com/cloud-cpp-kokoro-status/spanner/docker/ubsan.svg
[install/fedora-link]: https://storage.googleapis.com/cloud-cpp-kokoro-status/spanner/install/fedora-link.html
[install/fedora-shield]: https://storage.googleapis.com/cloud-cpp-kokoro-status/spanner/install/fedora.svg
[macos/bazel-link]: https://storage.googleapis.com/cloud-cpp-kokoro-status/spanner/macos/bazel-link.html
[macos/bazel-shield]: https://storage.googleapis.com/cloud-cpp-kokoro-status/spanner/macos/bazel.svg
[macos/cmake-super-link]: https://storage.googleapis.com/cloud-cpp-kokoro-status/spanner/macos/cmake-super-link.html
[macos/cmake-super-shield]: https://storage.googleapis.com/cloud-cpp-kokoro-status/spanner/macos/cmake-super.svg
[codecov-io-badge]: https://codecov.io/gh/googleapis/google-cloud-cpp-spanner/branch/master/graph/badge.svg
[codecov-io-link]: https://codecov.io/gh/googleapis/google-cloud-cpp-spanner
[doxygen-shield]: https://img.shields.io/badge/documentation-master-brightgreen.svg
[doxygen-link]: https://googleapis.github.io/google-cloud-cpp-spanner/latest/
[windows/bazel-shield]: https://storage.googleapis.com/cloud-cpp-kokoro-status/spanner-windows-bazel.svg
[windows/bazel-link]:   https://storage.googleapis.com/cloud-cpp-kokoro-status/spanner-windows-bazel-link.html
[windows/cmake-shield]: https://storage.googleapis.com/cloud-cpp-kokoro-status/spanner-windows-cmake.svg
[windows/cmake-link]:   https://storage.googleapis.com/cloud-cpp-kokoro-status/spanner-windows-cmake-link.html

<!-- End of automatically generated content -->"

## Documentation

The [reference documentation][doxygen-link] for this client library is available online.
Please consult the [Cloud Spanner website][cloud-spanner-docs] for
general documentation on Cloud Spanner features, APIS, other client
libraries, etc.

[cloud-spanner-docs]: https://cloud.google.com/spanner/docs/

## Contributing changes

See [`CONTRIBUTING.md`](CONTRIBUTING.md) for details on how to contribute to
this project, including how to build and test your changes as well as how to
properly format your code.

## Licensing

Apache 2.0; see [`LICENSE`](LICENSE) for details.

## Release Notes

### v0.1.x - TBD

## Versioning

Please note that the Google Cloud C++ client libraries do **not** follow
[Semantic Versioning](http://semver.org/).

**GA**: Libraries defined at a GA quality level are expected to be stable and
any backwards-incompatible changes will be noted in the documentation. Major
changes to the API will signaled by changing major version number
(e.g. 1.x.y -> 2.0.0).

**Beta**: Libraries defined at a Beta quality level are expected to be mostly
stable and we're working towards their release candidate. We will address issues
and requests with a higher priority.

**Alpha**: Libraries defined at an Alpha quality level are still a
work-in-progress and are more likely to get backwards-incompatible updates.
Additionally, it's possible for Alpha libraries to get deprecated and deleted
before ever being promoted to Beta or GA.

