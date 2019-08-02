#!/bin/bash
#
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
#
# This script is meant to be run as bazel's `--workspace_status_command=<cmd>`
# command. It should output space-separated "KEY value" pairs that bazel will
# store in the file bazel-out/stable-status.txt, which can be accessed from
# genrules(). For more info, see:
# https://docs.bazel.build/versions/master/user-manual.html#flag--workspace_status_command

readonly commit="$(git rev-parse --short HEAD 2> /dev/null || echo unknown)"
echo "STABLE_GIT_COMMIT ${commit}"
