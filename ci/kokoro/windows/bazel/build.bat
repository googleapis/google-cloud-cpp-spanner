REM Copyright 2019 Google LLC
REM
REM Licensed under the Apache License, Version 2.0 (the "License");
REM you may not use this file except in compliance with the License.
REM You may obtain a copy of the License at
REM
REM     http://www.apache.org/licenses/LICENSE-2.0
REM
REM Unless required by applicable law or agreed to in writing, software
REM distributed under the License is distributed on an "AS IS" BASIS,
REM WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
REM See the License for the specific language governing permissions and
REM limitations under the License.

REM Install Bazel using Chocolatey
choco install -y bazel --version 2.0.0
call refreshenv.cmd

REM Set it to "no" for any value other than "yes".
if "%RUN_SLOW_INTEGRATION_TESTS%" neq "yes" set RUN_SLOW_INTEGRATION_TESTS=no

echo %date% %time%
cd github\google-cloud-cpp-spanner

set TMP=T:\tmp

echo "Create the bazel output directory."
echo %date% %time%
if not exist "C:\b\" mkdir C:\b

@rem Use Visual Studio 2019
call "C:/Program Files (x86)/Microsoft Visual Studio/2019/Community/VC/Auxiliary/Build/vcvars64.bat"
set BAZEL_VS=C:\Program Files (x86)\Microsoft Visual Studio\2019\Community

echo %date% %time%
C:\ProgramData\chocolatey\bin\bazel version

echo "Downloading dependencies for the project."
echo %date% %time%
C:\ProgramData\chocolatey\bin\bazel --output_user_root=C:\b fetch -- ^
    //google/cloud/spanner/...:all

echo "Compiling the project."
echo %date% %time%
C:\ProgramData\chocolatey\bin\bazel --output_user_root=C:\b build --keep_going -- ^
    //google/cloud/spanner/...:all

if %errorlevel% neq 0 exit /b %errorlevel%

echo "Running unit tests"
echo %date% %time%
C:\ProgramData\chocolatey\bin\bazel --output_user_root=C:\b test ^
  --keep_going ^
  --test_output=errors ^
  --verbose_failures=true ^
  --test_tag_filters=-integration-tests ^
  -- //google/cloud/spanner/...:all

if %errorlevel% neq 0 exit /b %errorlevel%

echo "Running integration tests"
call "%KOKORO_GFILE_DIR%/spanner-integration-tests-config.bat"

@REM TODO(#1034) - Cleanup this workaround after we upgrade gRPC.
@REM Before 1.25.0 gRPC sometimes crashes when using the c-ares resolver on
@REM Windows
set GRPC_DNS_RESOLVER=native

@rem It seems like redirecting to a file is the easiest way to store the
@rem command output to a variable.
C:\ProgramData\chocolatey\bin\bazel --output_user_root=C:\b info output_base > t:\bazel-info.txt
set /p BAZEL_OUTPUT_DIR=<t:\bazel-info.txt
del t:\bazel-info.txt

echo %date% %time%
C:\ProgramData\chocolatey\bin\bazel --output_user_root=C:\b test ^
  --jobs=1 ^
  --keep_going ^
  --test_output=errors ^
  --verbose_failures=true ^
  --test_tag_filters=integration-tests ^
  --test_env GOOGLE_APPLICATION_CREDENTIALS=%KOKORO_GFILE_DIR%/spanner-credentials.json ^
  --test_env GOOGLE_CLOUD_PROJECT=%GOOGLE_CLOUD_PROJECT% ^
  --test_env GOOGLE_CLOUD_CPP_SPANNER_INSTANCE=%GOOGLE_CLOUD_CPP_SPANNER_INSTANCE% ^
  --test_env GOOGLE_CLOUD_CPP_SPANNER_IAM_TEST_SA=%GOOGLE_CLOUD_CPP_SPANNER_IAM_TEST_SA% ^
  --test_env GOOGLE_CLOUD_CPP_AUTO_RUN_EXAMPLES=yes ^
  --test_env RUN_SLOW_INTEGRATION_TESTS=%RUN_SLOW_INTEGRATION_TESTS% ^
  --test_env GRPC_DEFAULT_SSL_ROOTS_FILE_PATH=%BAZEL_OUTPUT_DIR%/external/com_github_grpc_grpc/etc/roots.pem ^
  -- //google/cloud/spanner/...:all

if %errorlevel% neq 0 exit /b %errorlevel%

@echo %date% %time%
@echo DONE DONE DONE "============================================="
@echo DONE DONE DONE "============================================="
@echo %date% %time%
