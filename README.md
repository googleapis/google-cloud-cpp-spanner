# Google Cloud Spanner C++ Client Library

<!-- This file is automatically generated by ci/test-readme/generate-readme.sh -->

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

This library supports Cloud Spanner at the [Beta](README.md#versioning) quality
level. This library should perform well for production workloads and we expect
to make minimal API changes. Please note that, as is often the case with C++
libraries, we do **not** follow semantic versioning in the Cloud C++ client
libraries. We make every effort to document backwards-incompatible API changes
in the [release notes](README.md#release-notes) below.


**Core Builds**
[![CI status docker/asan][docker/asan-shield]][docker/asan-link]
[![CI status docker/bazel-dependency][docker/bazel-dependency-shield]][docker/bazel-dependency-link]
[![CI status docker/check-api][docker/check-api-shield]][docker/check-api-link]
[![CI status docker/clang-3.8][docker/clang-3.8-shield]][docker/clang-3.8-link]
[![CI status docker/clang-tidy][docker/clang-tidy-shield]][docker/clang-tidy-link]
[![CI status docker/cmake-super][docker/cmake-super-shield]][docker/cmake-super-link]
[![CI status docker/cmake][docker/cmake-shield]][docker/cmake-link]
[![CI status docker/coverage][docker/coverage-shield]][docker/coverage-link]
[![CI status docker/cxx17][docker/cxx17-shield]][docker/cxx17-link]
[![CI status docker/gcc-4.8][docker/gcc-4.8-shield]][docker/gcc-4.8-link]
[![CI status docker/integration][docker/integration-shield]][docker/integration-link]
[![CI status docker/msan][docker/msan-shield]][docker/msan-link]
[![CI status docker/ninja][docker/ninja-shield]][docker/ninja-link]
[![CI status docker/noex][docker/noex-shield]][docker/noex-link]
[![CI status docker/publish-refdocs][docker/publish-refdocs-shield]][docker/publish-refdocs-link]
[![CI status docker/tsan][docker/tsan-shield]][docker/tsan-link]
[![CI status docker/ubsan][docker/ubsan-shield]][docker/ubsan-link]
[![CI status macos/bazel][macos/bazel-shield]][macos/bazel-link]
[![CI status macos/cmake-super][macos/cmake-super-shield]][macos/cmake-super-link]
[![CI status windows/bazel][windows/bazel-shield]][windows/bazel-link]
[![CI status windows/cmake][windows/cmake-shield]][windows/cmake-link]
[![Code Coverage Status][codecov-io-badge]][codecov-io-link]
[![Link to Reference Documentation][doxygen-shield]][doxygen-link]

**Install Instructions**
[![CI status install/centos-7][install/centos-7-shield]][install/centos-7-link]
[![CI status install/centos-8][install/centos-8-shield]][install/centos-8-link]
[![CI status install/debian-buster][install/debian-buster-shield]][install/debian-buster-link]
[![CI status install/debian-stretch][install/debian-stretch-shield]][install/debian-stretch-link]
[![CI status install/fedora][install/fedora-shield]][install/fedora-link]
[![CI status install/opensuse-leap][install/opensuse-leap-shield]][install/opensuse-leap-link]
[![CI status install/opensuse-tumbleweed][install/opensuse-tumbleweed-shield]][install/opensuse-tumbleweed-link]
[![CI status install/ubuntu-bionic][install/ubuntu-bionic-shield]][install/ubuntu-bionic-link]
[![CI status install/ubuntu-xenial][install/ubuntu-xenial-shield]][install/ubuntu-xenial-link]

[docker/asan-link]: https://storage.googleapis.com/cloud-cpp-kokoro-status/spanner/docker/asan-link.html
[docker/asan-shield]: https://storage.googleapis.com/cloud-cpp-kokoro-status/spanner/docker/asan.svg
[docker/bazel-dependency-link]: https://storage.googleapis.com/cloud-cpp-kokoro-status/spanner/docker/bazel-dependency-link.html
[docker/bazel-dependency-shield]: https://storage.googleapis.com/cloud-cpp-kokoro-status/spanner/docker/bazel-dependency.svg
[docker/check-api-link]: https://storage.googleapis.com/cloud-cpp-kokoro-status/spanner/docker/check-api-link.html
[docker/check-api-shield]: https://storage.googleapis.com/cloud-cpp-kokoro-status/spanner/docker/check-api.svg
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
[docker/ninja-link]: https://storage.googleapis.com/cloud-cpp-kokoro-status/spanner/docker/ninja-link.html
[docker/ninja-shield]: https://storage.googleapis.com/cloud-cpp-kokoro-status/spanner/docker/ninja.svg
[docker/noex-link]: https://storage.googleapis.com/cloud-cpp-kokoro-status/spanner/docker/noex-link.html
[docker/noex-shield]: https://storage.googleapis.com/cloud-cpp-kokoro-status/spanner/docker/noex.svg
[docker/publish-refdocs-link]: https://storage.googleapis.com/cloud-cpp-kokoro-status/spanner/docker/publish-refdocs-link.html
[docker/publish-refdocs-shield]: https://storage.googleapis.com/cloud-cpp-kokoro-status/spanner/docker/publish-refdocs.svg
[docker/tsan-link]: https://storage.googleapis.com/cloud-cpp-kokoro-status/spanner/docker/tsan-link.html
[docker/tsan-shield]: https://storage.googleapis.com/cloud-cpp-kokoro-status/spanner/docker/tsan.svg
[docker/ubsan-link]: https://storage.googleapis.com/cloud-cpp-kokoro-status/spanner/docker/ubsan-link.html
[docker/ubsan-shield]: https://storage.googleapis.com/cloud-cpp-kokoro-status/spanner/docker/ubsan.svg
[install/centos-7-link]: https://storage.googleapis.com/cloud-cpp-kokoro-status/spanner/install/centos-7-link.html
[install/centos-7-shield]: https://storage.googleapis.com/cloud-cpp-kokoro-status/spanner/install/centos-7.svg
[install/centos-8-link]: https://storage.googleapis.com/cloud-cpp-kokoro-status/spanner/install/centos-8-link.html
[install/centos-8-shield]: https://storage.googleapis.com/cloud-cpp-kokoro-status/spanner/install/centos-8.svg
[install/debian-buster-link]: https://storage.googleapis.com/cloud-cpp-kokoro-status/spanner/install/debian-buster-link.html
[install/debian-buster-shield]: https://storage.googleapis.com/cloud-cpp-kokoro-status/spanner/install/debian-buster.svg
[install/debian-stretch-link]: https://storage.googleapis.com/cloud-cpp-kokoro-status/spanner/install/debian-stretch-link.html
[install/debian-stretch-shield]: https://storage.googleapis.com/cloud-cpp-kokoro-status/spanner/install/debian-stretch.svg
[install/fedora-link]: https://storage.googleapis.com/cloud-cpp-kokoro-status/spanner/install/fedora-link.html
[install/fedora-shield]: https://storage.googleapis.com/cloud-cpp-kokoro-status/spanner/install/fedora.svg
[install/opensuse-leap-link]: https://storage.googleapis.com/cloud-cpp-kokoro-status/spanner/install/opensuse-leap-link.html
[install/opensuse-leap-shield]: https://storage.googleapis.com/cloud-cpp-kokoro-status/spanner/install/opensuse-leap.svg
[install/opensuse-tumbleweed-link]: https://storage.googleapis.com/cloud-cpp-kokoro-status/spanner/install/opensuse-tumbleweed-link.html
[install/opensuse-tumbleweed-shield]: https://storage.googleapis.com/cloud-cpp-kokoro-status/spanner/install/opensuse-tumbleweed.svg
[install/ubuntu-bionic-link]: https://storage.googleapis.com/cloud-cpp-kokoro-status/spanner/install/ubuntu-bionic-link.html
[install/ubuntu-bionic-shield]: https://storage.googleapis.com/cloud-cpp-kokoro-status/spanner/install/ubuntu-bionic.svg
[install/ubuntu-xenial-link]: https://storage.googleapis.com/cloud-cpp-kokoro-status/spanner/install/ubuntu-xenial-link.html
[install/ubuntu-xenial-shield]: https://storage.googleapis.com/cloud-cpp-kokoro-status/spanner/install/ubuntu-xenial.svg
[macos/bazel-link]: https://storage.googleapis.com/cloud-cpp-kokoro-status/spanner/macos/bazel-link.html
[macos/bazel-shield]: https://storage.googleapis.com/cloud-cpp-kokoro-status/spanner/macos/bazel.svg
[macos/cmake-super-link]: https://storage.googleapis.com/cloud-cpp-kokoro-status/spanner/macos/cmake-super-link.html
[macos/cmake-super-shield]: https://storage.googleapis.com/cloud-cpp-kokoro-status/spanner/macos/cmake-super.svg
[codecov-io-badge]: https://codecov.io/gh/googleapis/google-cloud-cpp-spanner/branch/master/graph/badge.svg
[codecov-io-link]: https://codecov.io/gh/googleapis/google-cloud-cpp-spanner
[doxygen-shield]: https://img.shields.io/badge/documentation-master-brightgreen.svg
[doxygen-link]: https://googleapis.dev/cpp/google-cloud-spanner/latest/
[windows/bazel-shield]: https://storage.googleapis.com/cloud-cpp-kokoro-status/spanner-windows-bazel.svg
[windows/bazel-link]:   https://storage.googleapis.com/cloud-cpp-kokoro-status/spanner-windows-bazel-link.html
[windows/cmake-shield]: https://storage.googleapis.com/cloud-cpp-kokoro-status/spanner-windows-cmake.svg
[windows/cmake-link]:   https://storage.googleapis.com/cloud-cpp-kokoro-status/spanner-windows-cmake-link.html

**Build with the latest bazel release**
[![Build status][bazelci-shield]][bazelci-link]

[bazelci-shield]: https://badge.buildkite.com/414b5f8e146fdf26017f069e3b464d7df1caa6bfa5ec627ef9.svg
[bazelci-link]: https://buildkite.com/bazel/github-dot-com-googleapis-google-cloud-cpp-spanner

## Documentation

The [reference documentation][doxygen-link] for this client library is available
online. Please consult the [Cloud Spanner website][cloud-spanner-docs] for
general documentation on Cloud Spanner features, APIs, other client libraries,
etc.

[cloud-spanner-docs]: https://cloud.google.com/spanner/docs/

## Running the Examples

Install Bazel using [these instructions][bazel-install]. If needed, install the
C++ toolchain for your platform too. Then compile the library examples using:

```console
bazel build //google/cloud/spanner/samples:samples
```

On Windows you may need to add some flags to workaround the filename length
limits.

> You must provide this option in **all** Bazel invocations shown below.

```console
mkdir C:\b
bazel --output_user_root=C:\b build //google/cloud/spanner/samples:samples
```

If you are using a Bazel version before 2.2.0 you may need to run the following command
to workaround [bazelbuild/bazel#10621](https://github.com/bazelbuild/bazel/issues/10621).

```console
bazel --output_user_root=C:\b test //google/cloud/spanner/samples:samples
```

On Windows and macOS gRPC [requires][grpc-roots-pem-bug] an environment variable
to find the root of trust for SSL. On macOS use:

```console
curl -Lo roots.pem https://raw.githubusercontent.com/grpc/grpc/master/etc/roots.pem
export GRPC_DEFAULT_SSL_ROOTS_FILE_PATH="$PWD/roots.pem"
```

While on Windows use:

```console
@powershell -NoProfile -ExecutionPolicy unrestricted -Command ^
    (new-object System.Net.WebClient).Downloadfile( ^
        'https://raw.githubusercontent.com/grpc/grpc/master/etc/roots.pem', ^
        'roots.pem')
set GRPC_DEFAULT_SSL_ROOTS_FILE_PATH=%cd%\roots.pem
```

You will need a Google Cloud Project with billing and the spanner API enabled.
Please consult the Spanner [quickstart guide][spanner-quickstart-link] for
detailed instructions on how to enable billing for your project.
Once your project is properly configured you can run the examples using
`bazel run`, for example:

```console
bazel run //google/cloud/spanner/samples:samples -- create-instance [PROJECT ID] [SPANNER INSTANCE] "My Test Instance"
bazel run //google/cloud/spanner/samples:samples -- create-database [PROJECT ID] [SPANNER INSTANCE] [DATABASE ID]
```

Running the command without options will list the available examples:

```console
bazel run //google/cloud/spanner/samples:samples --
```

[bazel-install]: https://docs.bazel.build/versions/master/install.html
[spanner-quickstart-link]: https://cloud.google.com/spanner/docs/quickstart-console
[grpc-roots-pem-bug]: https://github.com/grpc/grpc/issues/16571

## Using the Library for Development

If you plan to use this library in your development workflow please read the
[installation instructions](INSTALL.md). This also cover how to integrate the
library with CMake and Bazel workflows.

## Contributing changes

See [`CONTRIBUTING.md`](CONTRIBUTING.md) for details on how to contribute to
this project, including how to build and test your changes as well as how to
properly format your code.

## Licensing

Apache 2.0; see [`LICENSE`](LICENSE) for details.

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
