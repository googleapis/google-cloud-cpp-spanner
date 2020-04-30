#!/usr/bin/env bash
#
# Copyright 2020 Google LLC
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

cat <<_EOF_
# Google Cloud Spanner C++ Client Library

<!-- This file is automatically generated by ci/generate-markdown/$(basename "$0") -->

_EOF_

cat <<'_EOF_'
This directory contains an idiomatic C++ client library for interacting with
[Google Cloud Spanner](https://cloud.google.com/spanner/), which is a fully
managed, scalable, relational database service for regional and global
application data.

Please note that the Google Cloud C++ client libraries do **not** follow
[Semantic Versioning](http://semver.org/).

## Supported Platforms

* Windows, macOS, Linux
* C++11 (and higher) compilers (we test with GCC \>= 4.9, Clang >= 3.8, and MSVC \>= 2019)
* Environments with or without exceptions
* Bazel and CMake builds

## Documentation

* Official documentation about the [Cloud Spanner][cloud-spanner-docs] service
* [Reference doxygen documentation][doxygen-link] for each release of this client library
* Detailed header comments in our [public `.h`][source-link] files

[doxygen-link]: https://googleapis.dev/cpp/google-cloud-spanner/latest/
[cloud-spanner-docs]: https://cloud.google.com/spanner/docs/
[source-link]: https://github.com/googleapis/google-cloud-cpp-spanner/tree/master/google/cloud/spanner

## Quickstart

The [quickstart/](quickstart/) directory contains a minimal environment to
help you quickly get started using this client library. The following is the
"Hello World" program you'll be running, which should give you a taste of this
library.

```cc
_EOF_

# TODO(googleapis/google-cloud-cpp#3973): move the quickstart directory to
# google/cloud/spanner.
#
# Dumps the contents of quickstart.cc starting at the first #include, so we
# skip the license header comment.
sed -n '/^#/,$p' "${BINDIR}/../../quickstart/quickstart.cc"

cat <<'_EOF_'
````

## Contributing changes

See [`CONTRIBUTING.md`](../../../CONTRIBUTING.md) for details on how to
contribute to this project, including how to build and test your changes
as well as how to properly format your code.

## Licensing

Apache 2.0; see [`LICENSE`](../../../LICENSE) for details.

_EOF_
