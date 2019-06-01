# CMake Continuous Integration Support Files.

This directory contains the CI support files to test the Google Cloud Spanner
C++ client library when built using CMake.

There are two basic configurations:

<!-- TODO(#42) - create a super build based CI -->
- Install all the dependencies and the build the client library.
- Use a super build to compile the client library.
