#!/usr/bin/env bash
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

cat <<'END_OF_PREAMBLE'
# Obtaining google-cloud-cpp-spanner

There are two primary ways of obtaining `google-cloud-cpp-spanner`. You can use git:

```bash
git clone git@github.com:googleapis/google-cloud-cpp-spanner.git $HOME/project
```

Or obtain the tarball release:

```bash
mkdir -p $HOME/project
wget -q https://github.com/googleapis/google-cloud-cpp-spanner/archive/v0.4.0.tar.gz
tar -xf v0.4.0.tar.gz -C $HOME/project
```

# Installing google-cloud-cpp-spanner

When built with Bazel or as a CMake super build `google-cloud-cpp-spanner`
downloads and compiles all its dependencies ([see below](#required-libraries)
for a complete list). This makes it easier for users to "take the library for
a spin", and works well for users that "Live at Head", but does not work for
package maintainers or users that prefer to compile their dependencies once
and install them in `/usr/local/` or a similar directory.

This document provides instructions to install the dependencies of
`google-cloud-cpp-spanner`.

**If** all the dependencies of `google-cloud-cpp-spanner` are installed and
provide CMake support files, then compiling and installing the libraries
requires two commands:

```bash
cmake -H. -Bcmake-out
cmake --build cmake-out --target install
```

You may choose to parallelize the build by appending `-- -j ${NCPU}` to the
build command, where `NCPU` is an environment variable set to the number of
processors on your system. On Linux, you can obtain this information using the
`nproc` command or `sysctl -n hw.physicalcpu` on macOS.

Unfortunately getting your system to this state may require multiple steps,
the following sections describe how to install `google-cloud-cpp-spanner` on
several platforms.

## Using `google-cloud-cpp-spanner` in CMake-based projects.

Once you have installed `google-cloud-cpp-spanner` you can use the libraries from
your own projects using `find_package()` in your `CMakeLists.txt` file:

```CMake
cmake_minimum_required(VERSION 3.5)

find_package(spanner_client REQUIRED)

add_executable(my_program my_program.cc)
target_link_libraries(my_program spanner_client)
```

## Using `google-cloud-cpp-spanner` in Make-based projects.

Once you have installed `google-cloud-cpp-spanner` you can use the libraries in
your own Make-based projects using `pkg-config`:

```Makefile
GCPP_CXXFLAGS   := $(shell pkg-config spanner_client --cflags)
GCPP_CXXLDFLAGS := $(shell pkg-config spanner_client --libs-only-L)
GCPP_LIBS       := $(shell pkg-config spanner_client --libs-only-l)

my_program: my_program.cc
        $(CXX) $(CXXFLAGS) $(GCPP_CXXFLAGS) $(GCPP_CXXLDFLAGS) -o $@ $^ $(GCPP_LIBS)

```

## Using `google-cloud-cpp-spanner` in Bazel-based projects.

If you use `Bazel` for your builds you do not need to install
`google-cloud-cpp-spanner`. We provide a Starlark function to automatically
download and compile `google-cloud-cpp-spanner` as part of you Bazel build. Add
the following commands to your `WORKSPACE` file:

```Python
# Update the version and SHA256 digest as needed.
http_archive(
    name = "com_github_googleapis_google_cloud_cpp_spanner",
    url = "http://github.com/googleapis/google-cloud-cpp-spanner/archive/v0.2.0.tar.gz",
    strip_prefix = "google-cloud-cpp-0.2.0",
    sha256 = "TBD",
)

# Configure @com_google_googleapis to only compile C++ and gRPC:
load("@com_google_googleapis//:repository_rules.bzl", "switched_rules_by_language")
switched_rules_by_language(
    name = "com_google_googleapis_imports",
    cc = True,  # C++ support is only "Partially implemented", roll our own.
    grpc = True,
)

# Call the corresponding workspace function for each dependency
load("@com_github_googleapis_google_cloud_cpp_common//bazel:google_cloud_cpp_common_deps.bzl", "google_cloud_cpp_common_deps")
google_cloud_cpp_common_deps()
load("@com_github_grpc_grpc//bazel:grpc_deps.bzl", "grpc_deps")
grpc_deps()
load("@upb//bazel:workspace_deps.bzl", "upb_deps")
upb_deps()
load("@build_bazel_rules_apple//apple:repositories.bzl", "apple_rules_dependencies")
apple_rules_dependencies()
load("@build_bazel_apple_support//lib:repositories.bzl", "apple_support_dependencies")
apple_support_dependencies()
```

Then you can link the libraries from your `BUILD` files:

```Python
cc_binary(
    name = "my_program",
    srcs = [
        "my_program.cc",
    ],
    deps = [
        "@com_github_googleapis_google_cloud_cpp_spanner//google/cloud/spanner:spanner_client",
    ],
)
```

### Building with Bazel on macOS

To workaround a bug in Bazel ([bazelbuild/bazel#4341][bazel-bug-4341-link]) gRPC needs a
command-line flag when compiled using Bazel on macOS:

[bazel-bug-4341]: https://github.com/bazelbuild/bazel/issues/4341

```bash
bazel build --copt=-DGRPC_BAZEL_BUILD ...
```

Consider adding this option to your project's `.bazelrc` file.

## Required Libraries

`google-cloud-cpp-spanner` directly depends on the following libraries:

| Library | Minimum version | Description |
| ------- | --------------: | ----------- |
| gRPC    | 1.16.x | gRPC++ |

Note that these libraries may also depend on other libraries. The following
instructions include steps to install these indirect dependencies too.

When possible, the instructions below prefer to use pre-packaged versions of
these libraries and their dependencies. In some cases the packages do not exist,
or the package versions are too old to support `google-cloud-cpp`. If this is
the case, the instructions describe how you can manually download and install
these dependencies.

## Table of Contents

- [Fedora 30](#fedora-30)
- [openSUSE (Leap)](#opensuse-leap)
- [Ubuntu (18.04 - Bionic Beaver)](#ubuntu-1804---bionic-beaver)
- [Ubuntu (16.04 - Xenial Xerus)](#ubuntu-1604---xenial-xerus)
- [Debian (10 Buster)](#debian-10---buster)
- [Debian (9 Stretch)](#debian-9---stretch)
- [CentOS 8](#centos-8)
- [CentOS 7](#centos-7)
END_OF_PREAMBLE

readonly DOCKERFILES_DIR="${BINDIR}/../kokoro/install"

echo
echo "### Fedora (30)"
"${BINDIR}/extract-install.sh" "${DOCKERFILES_DIR}/Dockerfile.fedora"

echo
echo "### openSUSE (Leap)"
"${BINDIR}/extract-install.sh" "${DOCKERFILES_DIR}/Dockerfile.opensuse-leap"

echo
echo "### Ubuntu (18.04 - Bionic Beaver)"
"${BINDIR}/extract-install.sh" "${DOCKERFILES_DIR}/Dockerfile.ubuntu-bionic"

echo
echo "### Ubuntu (16.04 - Xenial Xerus)"
"${BINDIR}/extract-install.sh" "${DOCKERFILES_DIR}/Dockerfile.ubuntu-xenial"

echo
echo "### Debian (10 - Buster)"
"${BINDIR}/extract-install.sh" "${DOCKERFILES_DIR}/Dockerfile.debian-buster"

echo
echo "### Debian (9 - Stretch)"
"${BINDIR}/extract-install.sh" "${DOCKERFILES_DIR}/Dockerfile.debian-stretch"

echo
echo "### CentOS (8)"
"${BINDIR}/extract-install.sh" "${DOCKERFILES_DIR}/Dockerfile.centos-8"

echo
echo "### CentOS (7)"
"${BINDIR}/extract-install.sh" "${DOCKERFILES_DIR}/Dockerfile.centos-7"

