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

`google-cloud-cpp` directly depends on the following libraries:

| Library | Minimum version | Description |
| ------- | --------------: | ----------- |
| gRPC    | 1.16.x | gRPC++ for Cloud Bigtable |

Note that these libraries may also depend on other libraries. The following
instructions include steps to install these indirect dependencies too.

When possible, the instructions below prefer to use pre-packaged versions of
these libraries and their dependencies. In some cases the packages do not exist,
or the package versions are too old to support `google-cloud-cpp`. If this is
the case, the instructions describe how you can manually download and install
these dependencies.

## Table of Contents

- [Fedora 30](#fedora-30)

### Fedora (30)

Install the minimal development tools:

```bash
sudo dnf makecache && \
sudo dnf install -y cmake gcc-c++ git make openssl-devel pkgconfig \
        zlib-devel
```

Fedora 30 includes packages for gRPC, libcurl, and OpenSSL that are recent
enough for the project. Install these packages and additional development
tools to compile the dependencies:

```bash
sudo dnf makecache && \
sudo dnf install -y grpc-devel grpc-plugins \
        libcurl-devel protobuf-compiler tar wget zlib-devel
```

#### googleapis

We need a recent version of the Google Cloud Platform proto C++ libraries:

```bash
cd $HOME/Downloads
wget -q https://github.com/googleapis/cpp-cmakefiles/archive/v0.1.5.tar.gz
tar -xf v0.1.5.tar.gz
cd $HOME/Downloads/cpp-cmakefiles-0.1.5
cmake \
    -DBUILD_SHARED_LIBS=YES \
    -H. -Bcmake-out
sudo cmake --build cmake-out --target install -- -j ${NCPU:-4}
sudo ldconfig
```

#### googletest

We need a recent version of GoogleTest to compile the unit and integration
tests.

```bash
cd $HOME/Downloads
wget -q https://github.com/google/googletest/archive/release-1.10.0.tar.gz
tar -xf release-1.10.0.tar.gz
cd $HOME/Downloads/googletest-release-1.10.0
cmake \
      -DCMAKE_BUILD_TYPE="Release" \
      -DBUILD_SHARED_LIBS=yes \
      -H. -Bcmake-out
sudo cmake --build cmake-out --target install -- -j ${NCPU:-4}
sudo ldconfig
```

#### google-cloud-cpp-common

The project also depends on google-cloud-cpp-common, the libraries shared by
all the Google Cloud C++ client libraries:

```bash
cd $HOME/Downloads
wget -q https://github.com/googleapis/google-cloud-cpp-common/archive/v0.13.0.tar.gz
tar -xf v0.13.0.tar.gz
cd $HOME/Downloads/google-cloud-cpp-common-0.13.0
cmake -H. -Bcmake-out \
    -DBUILD_TESTING=OFF \
    -DGOOGLE_CLOUD_CPP_TESTING_UTIL_ENABLE_INSTALL=ON
sudo cmake --build cmake-out --target install -- -j ${NCPU:-4}
sudo ldconfig
```

#### Compile and install the main project

We can now compile, test, and install `google-cloud-cpp-spanner`.

```bash
cd $HOME/project
cmake -H. -B/o
cmake --build /o -- -j "${NCPU:-4}"
cd /o
ctest -LE integration-tests --output-on-failure
cd $HOME/project
sudo cmake --build /o --target install
```

