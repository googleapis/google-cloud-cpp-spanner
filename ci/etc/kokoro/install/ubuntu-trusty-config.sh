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
# Install the minimal development tools.
# We use the `ubuntu-toolchain-r` PPA to get a modern version of CMake:

# ```bash
RUN apt update && apt install -y software-properties-common
RUN add-apt-repository ppa:ubuntu-toolchain-r/test -y
RUN apt update && \
    apt install -y automake cmake3 git gcc g++ libtool make pkg-config \
        tar wget zlib1g-dev
# ```

_EOF_

read -r -d '' INSTALL_BINARY_DEPENDENCIES <<'_EOF_'
_EOF_

INSTALL_SOURCE_DEPENDENCIES=$(
  cat <<'_EOF_'
# #### OpenSSL

# Ubuntu:14.04 ships with a very old version of OpenSSL, this version is not
# supported by gRPC. We need to compile and install OpenSSL-1.0.2 from source.

# ```bash
WORKDIR /var/tmp/build
RUN wget -q https://www.openssl.org/source/openssl-1.0.2n.tar.gz
RUN tar xf openssl-1.0.2n.tar.gz
WORKDIR /var/tmp/build/openssl-1.0.2n
RUN ./config --shared
RUN make -j ${NCPU:-4}
RUN make install
# ```

# Note that by default OpenSSL installs itself in `/usr/local/ssl`. Installing
# on a more conventional location, such as `/usr/local` or `/usr`, can break
# many programs in your system. OpenSSL 1.0.2 is actually incompatible with
# with OpenSSL 1.0.0 which is the version expected by the programs already
# installed by Ubuntu 14.04.

# In any case, as the library installs itself in this non-standard location, we
# also need to configure CMake and other build program to find this version of
# OpenSSL:

# ```bash
ENV OPENSSL_ROOT_DIR=/usr/local/ssl
ENV PKG_CONFIG_PATH=/usr/local/ssl/lib/pkgconfig
# ```

# #### libcurl.

# Because google-cloud-cpp uses both gRPC and curl, we need to compile libcurl
# against the same version of OpenSSL:

# ```bash
WORKDIR /var/tmp/build
RUN wget -q https://curl.haxx.se/download/curl-7.61.0.tar.gz
RUN tar xf curl-7.61.0.tar.gz
WORKDIR /var/tmp/build/curl-7.61.0
RUN ./configure --prefix=/usr/local/curl
RUN make -j ${NCPU:-4}
RUN make install
RUN ldconfig
# ```

# #### Protobuf

# Likewise, manually install Protobuf:

# ```bash
_EOF_
  echo "${INSTALL_PROTOBUF_FROM_SOURCE}"

  cat <<'_EOF_'
# ```

# #### c-ares

# Recent versions of gRPC require c-ares >= 1.11, while CentOS-7
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
