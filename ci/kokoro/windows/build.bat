REM Copyright 2018 Google LLC
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

REM Set it to "no" for any value other than "yes".
if "%RUN_SLOW_INTEGRATION_TESTS%"!="yes" set RUN_SLOW_INTEGRATION_TESTS=no

echo %date% %time%
cd github\google-cloud-cpp-spanner

set CONFIG=Debug
set PROVIDER=package
call "c:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"

echo %date% %time%
cmd /c gcloud auth activate-service-account --key-file "%KOKORO_GFILE_DIR%/build-results-service-account.json"

call "%KOKORO_GFILE_DIR%/spanner-integration-tests-config.bat"
set GOOGLE_APPLICATION_CREDENTIALS=%KOKORO_GFILE_DIR%\spanner-credentials.json
set GOOGLE_CLOUD_CPP_AUTO_RUN_EXAMPLES=yes
set GRPC_DEFAULT_SSL_ROOTS_FILE_PATH=T:\src\github\vcpkg\installed\x64-windows-static\share\grpc\roots.pem

echo %date% %time%
powershell -exec bypass ci\kokoro\windows\build-dependencies.ps1
if %errorlevel% neq 0 exit /b %errorlevel%

echo %date% %time%
cmd /c gcloud auth revoke --all

echo %date% %time%
powershell -exec bypass ci\kokoro\windows\build-project.ps1
@rem Preserve the exit code of the test for later use because we want to
@rem delete the files in the %KOKORO_ARTIFACTS_DIR% on test failure too.
set test_errorlevel=%errorlevel%

@rem Kokoro rsyncs all the files in the %KOKORO_ARTIFACTS_DIR%, which takes a
@rem long time. The recommended workaround is to remove all the files that are
@rem not interesting artifacts.
if defined KOKORO_ARTIFACTS_DIR (
  echo %date% %time%
  cd "%KOKORO_ARTIFACTS_DIR%"
  powershell -Command "& {Get-ChildItem -Recurse -File -Exclude test.xml,sponge_log.xml,build.bat | Remove-Item -Recurse -Force}"
  if %errorlevel% neq 0 exit /b %errorlevel%
)

if %test_errorlevel% neq 0 exit /b %test_errorlevel%

@echo %date% %time%
@echo DONE DONE DONE "============================================="
@echo DONE DONE DONE "============================================="
@echo %date% %time%
