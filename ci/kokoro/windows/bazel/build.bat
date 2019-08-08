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

echo %date% %time%
cd github\google-cloud-cpp-spanner

echo "Create the bazel output directory."
echo %date% %time%
if not exist "C:\b\" mkdir C:\b

@rem Use Visual Studio 2019
call "C:/Program Files (x86)/Microsoft Visual Studio/2019/Community/VC/Auxiliary/Build/vcvars64.bat"
set BAZEL_VS=C:\Program Files (x86)\Microsoft Visual Studio\2019\Community

echo %date% %time%
bazel version

echo "Compiling the project."
echo %date% %time%
bazel --output_user_root=C:\b build --keep_going -- ^
    //google/cloud/spanner/...:all

if %errorlevel% neq 0 exit /b %errorlevel%

@echo %date% %time%
@echo DONE DONE DONE "============================================="
@echo DONE DONE DONE "============================================="
@echo %date% %time%
