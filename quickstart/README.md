# HOWTO: using google-cloud-cpp-spanner in your project

This directory contains small examples showing how to use the Cloud Spanner C++ client library in your own project.
These instructions assumne that you have some experience as a C++ developer and that you have a working C++ toolchain
(compiler, linker, etc.) installed on your platform.

## Using with Bazel

1. Install Bazel using [the instructions][bazel-install] from the `bazel.build` website.

2. Compile this example using Bazel:

   ```bash
   cd $HOME/google-cloud-cpp-spanner/quickstart
   bazel build ...
   ```

3. Run the example, change the place holder to appropriate values:

   ```bash
   bazel run :quickstart -- [GCP PROJECT] [CLOUD SPANNER INSTANCE] [CLOUD SPANNER DATABASE]
   ```

If you are using Windows or macOS there are additional instructions at the end of this document.

## Using with CMake

1. Install CMake. The package managers for most Linux distributions include a package for CMake.
   Likewise, you can install CMake on Windows using a package manager such as
   [chocolatey][choco-cmake-link], and on macOS using [homebrew][homebrew-cmake-link]. You can also obtain the software
   directly from the [cmake.org][https://cmake.org/download/].

2. Install the dependencies with your favorite tools. As an example, if you use
   [vcpkg](https://github.com/Microsoft/vcpkg.git):

   ```bash
   cd $HOME/vcpkg
   ./vcpkg install google-cloud-cpp-spanner
   ```

3. Configure CMake, if necessary, configure the directory where you installed the dependencies:

   ```bash
   cd $HOME/gooogle-cloud-cpp-spanner/quickstart
   cmake -H. -B.build -DCMAKE_TOOLCHAIN_FILE=$HOME/vcpkg/scripts/buildsystems/vcpkg.cmake
   cmake --build .build
   ```

4. Run the example, change the place holder to appropriate values:

   ```bash
   .build/quickstart [GCP PROJECT] [CLOUD SPANNER INSTANCE] [CLOUD SPANNER DATABASE]
   ```

If you are using Windows or macOS there are additional instructions at the end of this document.

## Platform Specific Notes

### macOS

gRPC [requires][grpc-roots-pem-bug] an environment variable to configure the trust store for SSL certificates, you
can download and configure this using:

```console
curl -Lo roots.pem https://raw.githubusercontent.com/grpc/grpc/master/etc/roots.pem
export GRPC_DEFAULT_SSL_ROOTS_FILE_PATH="$PWD/roots.pem"
```

### Windows

gRPC [requires][grpc-roots-pem-bug] an environment variable to configure the trust store for SSL certificates, you
can download and configure this using:

```console
@powershell -NoProfile -ExecutionPolicy unrestricted -Command ^
    (new-object System.Net.WebClient).Downloadfile( ^
        'https://raw.githubusercontent.com/grpc/grpc/master/etc/roots.pem', ^
        'roots.pem')
set GRPC_DEFAULT_SSL_ROOTS_FILE_PATH=%cd%\roots.pem
```

[bazel-install]: https://docs.bazel.build/versions/master/install.html
[spanner-quickstart-link]: https://cloud.google.com/spanner/docs/quickstart-console
[grpc-roots-pem-bug]: https://github.com/grpc/grpc/issues/16571
[choco-cmake-link]: https://chocolatey.org/packages/cmake
[homebrew-cmake-link]: https://formulae.brew.sh/formula/cmake
[cmake-download-link]: https://cmake.org/download/
