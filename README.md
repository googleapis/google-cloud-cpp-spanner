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

This directory contains the implementation of the Cloud Spanner C++
client library.

## Status

This library is under active development, it is known to be incomplete,
and is subject to change without notice. We do not recommend that you use
this library for experimental code, nor for production workloads.

[![Kokoro CI status][kokoro-integration-shield]][kokoro-integration-link]
[![Kokoro CI status][kokoro-clang-tidy-shield]][kokoro-clang-tidy-link]
[![Kokoro CI status][kokoro-cmake-shield]][kokoro-cmake-link]
[![Kokoro CI status][kokoro-asan-shield]][kokoro-asan-link]
[![Kokoro CI status][kokoro-tsan-shield]][kokoro-tsan-link]
[![Kokoro CI status][kokoro-msan-shield]][kokoro-msan-link]
[![Kokoro CI status][kokoro-ubsan-shield]][kokoro-ubsan-link]
[![Kokoro CI status][kokoro-cxx17-shield]][kokoro-cxx17-link]
[![Kokoro CI status][kokoro-gcc48-shield]][kokoro-gcc48-link]
[![Kokoro CI status][kokoro-windows-bazel-shield]][kokoro-windows-bazel-link]

[kokoro-integration-shield]: https://storage.googleapis.com/cloud-cpp-kokoro-status/spanner-integration.svg
[kokoro-integration-link]:   https://storage.googleapis.com/cloud-cpp-kokoro-status/spanner-integration-link.html
[kokoro-clang-tidy-shield]: https://storage.googleapis.com/cloud-cpp-kokoro-status/spanner-clang-tidy.svg
[kokoro-clang-tidy-link]:   https://storage.googleapis.com/cloud-cpp-kokoro-status/spanner-clang-tidy-link.html
[kokoro-cmake-shield]: https://storage.googleapis.com/cloud-cpp-kokoro-status/spanner-cmake.svg
[kokoro-cmake-link]:   https://storage.googleapis.com/cloud-cpp-kokoro-status/spanner-cmake-link.html
[kokoro-asan-shield]: https://storage.googleapis.com/cloud-cpp-kokoro-status/spanner-asan.svg
[kokoro-asan-link]:   https://storage.googleapis.com/cloud-cpp-kokoro-status/spanner-asan-link.html
[kokoro-tsan-shield]: https://storage.googleapis.com/cloud-cpp-kokoro-status/spanner-tsan.svg
[kokoro-tsan-link]:   https://storage.googleapis.com/cloud-cpp-kokoro-status/spanner-tsan-link.html
[kokoro-msan-shield]: https://storage.googleapis.com/cloud-cpp-kokoro-status/spanner-msan.svg
[kokoro-msan-link]:   https://storage.googleapis.com/cloud-cpp-kokoro-status/spanner-msan-link.html
[kokoro-ubsan-shield]: https://storage.googleapis.com/cloud-cpp-kokoro-status/spanner-ubsan.svg
[kokoro-ubsan-link]:   https://storage.googleapis.com/cloud-cpp-kokoro-status/spanner-ubsan-link.html
[kokoro-cxx17-shield]: https://storage.googleapis.com/cloud-cpp-kokoro-status/spanner-cxx17.svg
[kokoro-cxx17-link]:   https://storage.googleapis.com/cloud-cpp-kokoro-status/spanner-cxx17-link.html
[kokoro-gcc48-shield]: https://storage.googleapis.com/cloud-cpp-kokoro-status/spanner-gcc48.svg
[kokoro-gcc48-link]:   https://storage.googleapis.com/cloud-cpp-kokoro-status/spanner-gcc48-link.html
[kokoro-windows-bazel-shield]: https://storage.googleapis.com/cloud-cpp-kokoro-status/spanner-windows-bazel.svg
[kokoro-windows-bazel-link]:   https://storage.googleapis.com/cloud-cpp-kokoro-status/spanner-windows-bazel-link.html

## Documentation

* Please consult the [Cloud Spanner website][cloud-spanner-docs] for
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

