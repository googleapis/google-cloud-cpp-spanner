#!/usr/bin/env bash
#
# Copyright 2019 Google LLC
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

set -eu

readonly BINDIR=$(dirname "$0")

badge() {
  local -r distro="$1"
  cat <<_EOF_

[![Kokoro readme ${distro} status][kokoro-readme-${distro}-shield]][kokoro-readme-${distro}-link]

[kokoro-readme-${distro}-shield]: https://storage.googleapis.com/cloud-cpp-kokoro-status/spanner/readme/${distro}.svg
[kokoro-readme-${distro}-link]: https://storage.googleapis.com/cloud-cpp-kokoro-status/spanner/readme/${distro}-link.html
_EOF_
}

readonly DOCKERFILES_DIR="${BINDIR}/../kokoro/readme"

cat <<_EOF_
# Google Cloud Spanner C++ Client Library

<!-- This file is automatically generated by ci/test-readme/$(basename "$0") -->

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

_EOF_

"${BINDIR}"/../generate-badges.sh

cat <<'_EOF_'

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

On Windows and macOS gRPC [requires][grpc-roots-pem-bug] an environment variable
to find the to manually set the root of trust for SSL. On macOS use:

```console
wget -q https://raw.githubusercontent.com/grpc/grpc/master/etc/roots.pem
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
_EOF_
