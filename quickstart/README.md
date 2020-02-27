# HOWTO: usie google-cloud-cpp-spanner in your project

This directory contains small examples showing how to use the Cloud Spanner C++ client library in your own project.

## Using with Bazel

## Using with CMake

Install the dependencies with your favorite tools, for example, if you use
[vcpkg](https://github.com/Microsoft/vcpkg.git) you can use:

```bash
cd $HOME/vcpkg
./vcpkg install google-cloud-cpp-spanner
```

Configure CMake, if necessary, configure the directory where you installed the dependencies:

```bash
cd $HOME/gooogle-cloud-cpp-spanner/quickstart
cmake -H. -B.build -DCMAKE_TOOLCHAIN_FILE=$HOME/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build .build
```

## Platform Specific Notes

### macOS

gRPC requires an environment variable to configure the trust store for SSL certificates, you
can download and configure this using:

```console
curl -Lo roots.pem https://raw.githubusercontent.com/grpc/grpc/master/etc/roots.pem
export GRPC_DEFAULT_SSL_ROOTS_FILE_PATH="$PWD/roots.pem"
```

### Windows

gRPC requires an environment variable to configure the trust store for SSL certificates, you
can download and configure this using:

```console
@powershell -NoProfile -ExecutionPolicy unrestricted -Command ^
    (new-object System.Net.WebClient).Downloadfile( ^
        'https://raw.githubusercontent.com/grpc/grpc/master/etc/roots.pem', ^
        'roots.pem')
set GRPC_DEFAULT_SSL_ROOTS_FILE_PATH=%cd%\roots.pem
```

