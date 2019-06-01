# How to setup your workstation to build with CMake.

This document describes how to setup your workstation to build the Google Cloud
Spanner C++ client library using CMake. The intended audience is developers of
the client library that want to verify their changes will work with CMake and/or
prefer to use CMake for whatever reason. The document assumes you are using a
Linux workstation running Ubuntu, changing the instructions for other
distributions or operating systems is left as an exercise to the reader, and
PRs to improve this document are most welcome!

Unless explicitly stated, this document assumes you running these commands at
the top-level directory of the project, as in:

```console
cd $HOME
git clone git@github:<github-username>/google-cloud-cpp-spanner.git
cd google-cloud-cpp-spanner
```

## Running the CI build

If you just want to run one of the CI builds based on CMake then just install
Docker (once):

```console
# Make sure the docker server and client are installed in your workstation, you
# only need to do this once:
sudo apt update && sudo apt install docker.io
```

and run the same script the CI build uses:

```console
cd $HOME/google-cloud-cpp-spanner
env BUILD_NAME=fedora ./ci/kokoro/cmake/installed-dependencies/build.sh
```

The build takes a few minutes the first time, as it needs to create a Docker
image with all the development tools and dependencies installed. Future builds
(with no changes to the source) take only a few seconds.

## Using `vcpkg` to install the dependencies

`vcpkg` is a package manager that installs dependencies from source. All the
dependencies of `google-cloud-cpp-spanner` are available through `vcpkg`.
However, some of the dependencies, such as `google-cloud-cpp` are only updated
monthly. If at some point the Spanner client depends on recent changes on
`google-cloud-cpp` this strategy will not work for you.

With that out of the way, installing Ninja is recommended, because it makes the
`vcpkg` builds faster, but not required:

```console
sudo apt update && sudo apt install ninja-build
```

Download `vcpkg` from GitHub:

```console
cd $HOME
git clone https://github.com/Microsoft/vcpkg.git
```

Bootstrap `vcpkg`:

```console
cd vcpkg
./bootstrap-vcpkg.sh
```

Once `vcpkg` is built, install all dependencies:

```console
./vcpkg install google-cloud-cpp gtest
```

Configure the project, using `vcpkg` to find the dependencies, we recommend
using a sub-directory of `cmake-out` because the CI builds also use
sub-directories of `cmake-out/`:

```console
cd $HOME/google-cloud-cpp-spanner
cmake -H. -Bcmake-out/vcpkg \
  -DCMAKE_TOOLCHAIN_FILE=$HOME/vcpkg/scripts/buildsystems/vcpkg.cmake
```

Then compile the project as usual:

```console
cmake --build cmake-out/vcpkg -- -j $(nproc)
```

You can run the tests using either:

```console
env -C cmake-out/vcpkg ctest --output-on-failure -j $(nproc)
```

Or:

```console
cmake --build cmake-out/vcpkg --target test
```

## Installing in `/usr/local`.

> **WARNING**: These instructions were tested using the
> `Dockerfile.test-instructions` file found in this directory. Beware of
> transcription mistakes when copying to this document.

Install the development tools and the dependencies provided by your system:

```console
sudo apt update && \
    sudo apt install -y build-essential cmake git gcc g++ cmake \
        libc-ares-dev libc-ares2 libcurl4-openssl-dev libssl-dev make \
        pkg-config tar wget zlib1g-dev
```

Compile crc32c manually:

```console
cd /var/tmp/build
wget -q https://github.com/google/crc32c/archive/1.0.6.tar.gz
tar -xf 1.0.6.tar.gz
cd /var/tmp/build/crc32c-1.0.6
cmake \
      -DCMAKE_BUILD_TYPE=Release \
      -DBUILD_SHARED_LIBS=yes \
      -DCRC32C_BUILD_TESTS=OFF \
      -DCRC32C_BUILD_BENCHMARKS=OFF \
      -DCRC32C_USE_GLOG=OFF \
      -H. -Bcmake-out/crc32c
sudo cmake --build cmake-out/crc32c --target install -- -j $(nproc)
sudo ldconfig
```

Remove any system-wide Protobuf libraries or tools, they are too old to be of
any use:

```console
sudo apt remove \
    libprotobuf-c-dev protobuf-compiler-grpc \
    protobuf-compiler protobuf-c-compiler
```

Compile and install protobuf:

```console
cd /var/tmp/build
cd wget -q https://github.com/google/protobuf/archive/v3.6.1.tar.gz
tar -xf v3.6.1.tar.gz
cd /var/tmp/build/protobuf-3.6.1/cmake
cmake \
        -DCMAKE_BUILD_TYPE=Release \
        -DBUILD_SHARED_LIBS=yes \
        -Dprotobuf_BUILD_TESTS=OFF \
        -H. -Bcmake-out
sudo cmake --build cmake-out --target install -- -j $(nproc)
sudo ldconfig
```

Compile and install gRPC:

```console
cd /var/tmp/build
wget -q https://github.com/grpc/grpc/archive/v1.19.1.tar.gz
tar -xf v1.19.1.tar.gz
cd /var/tmp/build/grpc-1.19.1
make -j $(nproc)
sudo make install
sudo ldconfig
```

Compile and install `google-cloud-cpp`:

```console
cd /var/tmp/build
wget -q https://github.com/googleapis/google-cloud-cpp/archive/v0.9.0.tar.gz
tar -xf v0.9.0.tar.gz
cd /var/tmp/build/google-cloud-cpp-0.9.0
cmake -H. -Bcmake-out \
    -DGOOGLE_CLOUD_CPP_DEPENDENCY_PROVIDER=package \
    -DBUILD_TESTING=OFF
cmake --build cmake-out -- -j $(nproc)
sudo cmake --build cmake-out --target install
sudo ldconfig
```

Download and compile googletest:

```console
cd /var/tmp/build
wget -q https://github.com/google/googletest/archive/b6cd405286ed8635ece71c72f118e659f4ade3fb.tar.gz
tar -xf b6cd405286ed8635ece71c72f118e659f4ade3fb.tar.gz
cd /var/tmp/build/googletest-b6cd405286ed8635ece71c72f118e659f4ade3fb
cmake \
        -H. -Bcmake-out
sudo cmake --build cmake-out --target install -- -j $(nproc)
```

Now you can compile the project as usual:

```console
cd $HOME/google-cloud-cpp-spanner
cmake -H. -Bcmake-out/local
cmake --build cmake-out/local -- -j "$(nproc)"
cmake --build cmake-out/local --target test
```

## Install the dependencies in `$HOME/local`.

<!-- TODO(#40) - create instructions for this case -->
