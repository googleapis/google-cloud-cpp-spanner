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
# Install the minimal development tools, OpenSSL and libcurl:

# ```bash
RUN apt update && \
    apt install -y automake build-essential cmake git gcc g++ \
        libcurl4-openssl-dev libssl-dev libtool make \
        pkg-config tar wget zlib1g-dev
# ```

_EOF_

read -r -d '' INSTALL_BINARY_DEPENDENCIES <<'_EOF_'
_EOF_

INSTALL_SOURCE_DEPENDENCIES=$(
  cat <<'_EOF_'
# #### Protobuf

# Likewise, manually install Protobuf:

# ```bash
_EOF_
  echo "${INSTALL_PROTOBUF_FROM_SOURCE}"

  cat <<'_EOF_'
# ```

# #### c-ares

# Recent versions of gRPC require c-ares >= 1.11, while Ubuntu-16.04
# distributes c-ares-1.10. Manually install a newer version:

# ```bash
_EOF_
  echo "${INSTALL_C_ARES_FROM_SOURCE}"

  cat <<'_EOF_'
# ```

# #### gRPC

# Can be manually installed using:

# ```bash
_EOF_
  echo "${INSTALL_GRPC_FROM_SOURCE}"
  echo '# ```'
  echo
)

read -r -d '' ENABLE_USR_LOCAL_FRAGMENT <<'_EOF_'
_EOF_
