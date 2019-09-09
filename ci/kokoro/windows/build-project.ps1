#!/usr/bin/env powershell
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

# Stop on errors. This is similar to `set -e` on Unix shells.
$ErrorActionPreference = "Stop"

Write-Host "================================================================"
Write-Host "Install Pscx module for current user $(Get-Date -Format o)."
Install-Module Pscx -Force -AllowClobber -Scope CurrentUser
if ($LastExitCode) {
    throw "Error installing Pscx module $LastExitCode"
}

Write-Host "================================================================"
Write-Host "Run CMake to create build scripts $(Get-Date -Format o)."

# This script expects vcpkg to be installed in ..\vcpkg, discover the full
# path to that directory:
$dir = Split-Path (Get-Item -Path ".\" -Verbose).FullName

$cmake_flags=@(
    # On Windows ninja is a much faster generator.
    "-GNinja",
    "-Hci/kokoro/windows",
    # Use short path names because we only have 255 characters.
    "-Bcmake-out/w",
    # We need to compile OpenSSL using vcpkg because compiling from scratch is hard.
    "-DCMAKE_TOOLCHAIN_FILE=`"$dir\vcpkg\scripts\buildsystems\vcpkg.cmake`"",
    "-DVCPKG_TARGET_TRIPLET=x64-windows-static",
    "-DCMAKE_PREFIX_PATH=c:/build/vcpkg/installed/x64-windows-static",
    # Explicitly set the compiler, otherwise CMake picks up CLang or GCC if installed.
    "-DCMAKE_C_COMPILER=cl.exe",
    "-DCMAKE_CXX_COMPILER=cl.exe",
    "-DCMAKE_BUILD_TYPE=Release"
)
# Configure CMake and create the build directory.
Write-Host "================================================================"
Write-Host "Configuring CMake with ${cmake_flags} $(Get-Date -Format o)"
cmake ${cmake_flags}
if ($LastExitCode) {
    throw "cmake config failed with exit code $LastExitCode"
}

Write-Host "================================================================"
Write-Host "Compiling the project $(Get-Date -Format o)."
cmake --build cmake-out/w
if ($LastExitCode) {
    throw "cmake failed with exit code $LastExitCode"
}

if (Test-Path env:RUN_INTEGRATION_TESTS ) {
  Write-Host "================================================================"
  Write-Host "Run integration tests $(Get-Date -Format o)."
  cd cmake-out/w
  ctest --output-on-failure -L integration-tests -j 1
  if ($LastExitCode) {
      throw "Integration tests failed with exit code $LastExitCode"
  }
}
