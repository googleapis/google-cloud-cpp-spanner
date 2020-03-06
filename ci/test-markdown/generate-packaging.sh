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

cat <<'END_OF_PREAMBLE'
# Packaging google-cloud-cpp-spanner

<!-- This is an automatically generated file -->
<!-- Make changes in ci/test-markdown/generate-packaging.sh -->

This document is intended for package maintainers or anyone else who might like
to "install" the google-cloud-cpp-spanner library.

There are two primary ways of obtaining `google-cloud-cpp-spanner`. You can use git:

```bash
git clone https://github.com/googleapis/google-cloud-cpp-spanner.git $HOME/google-cloud-cpp-spanner
```

Or obtain the tarball release:

```bash
mkdir -p $HOME/google-cloud-cpp-spanner
wget -q https://github.com/googleapis/google-cloud-cpp-spanner/archive/v0.8.0.tar.gz
tar -xf v0.8.0.tar.gz -C $HOME/google-cloud-cpp-spanner --strip=1
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

readonly BINDIR=$(dirname "$0")
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

