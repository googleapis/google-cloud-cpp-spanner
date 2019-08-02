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
# Given an argument that is the path to a compiler, tries to determine
# the compiler's version by looking for the first string that looks like
# X.Y with more optional numbers, '.', '-', and '+'.

if [[ $# -ne 1 ]]; then
  echo "Usage: $0 <path-to-compiler>"
  exit 1
fi

version="$("${1}" --version 2> /dev/null \
  | grep -Eo "[0-9]+\.[0-9.+-]+" \
  | head -1)"

if [[ -n "${version}" ]]; then
  echo "${version}"
else
  echo unknown
fi
