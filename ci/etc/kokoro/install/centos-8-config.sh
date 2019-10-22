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

read -r -d '' INSTALL_DEVTOOLS_FRAGMENT <<'_EOF_'
# Install the minimal development tools, libcurl, OpenSSL, and the c-ares
# library (required by gRPC):

# ```bash
RUN dnf makecache && \
    dnf install -y cmake gcc-c++ git make openssl-devel pkgconfig \
        zlib-devel libcurl-devel c-ares-devel tar wget which
# ```

_EOF_

read -r -d '' INSTALL_BINARY_DEPENDENCIES <<'_EOF_'
_EOF_

INSTALL_SOURCE_DEPENDENCIES=$(
  cat <<'_EOF_'
# #### Protobuf

# Likewise, manually install protobuf:

# ```bash
_EOF_
  echo "${INSTALL_PROTOBUF_FROM_SOURCE}"

  cat <<'_EOF_'
# ```

# #### gRPC

# To install gRPC we first need to configure pkg-config to find the version of
# Protobuf we just installed in `/usr/local`:

# ```bash
ENV PKG_CONFIG_PATH=/usr/local/lib/pkgconfig:/usr/local/lib64/pkgconfig
ENV LD_LIBRARY_PATH=/usr/local/lib:/usr/local/lib64
# ```

# The we install it using using:

# ```bash
_EOF_
  echo "${INSTALL_GRPC_FROM_SOURCE}"
  echo '# ```'
  echo
)

read -r -d '' ENABLE_USR_LOCAL_FRAGMENT <<'_EOF_'
ENV PKG_CONFIG_PATH=/usr/local/lib64/pkgconfig
_EOF_
