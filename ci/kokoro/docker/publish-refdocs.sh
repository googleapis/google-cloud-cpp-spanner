#!/usr/bin/env bash
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

set -eu

# For docuploader to work
export LC_ALL=C.UTF-8
export LANG=C.UTF-8

if [[ -z "${CREDENTIALS_FILE:-}" ]]; then
  CREDENTIALS_FILE="${KOKORO_KEYSTORE_DIR}/73713_docuploader_service_account"
fi

if [[ -z "${STAGING_BUCKET:-}" ]]; then
  STAGING_BUCKET="docs-staging"
fi

if [[ -z "${PROJECT_ROOT+x}" ]]; then
  readonly PROJECT_ROOT="$(cd "$(dirname "$0")/../../.."; pwd)"
fi
source "${PROJECT_ROOT}/ci/kokoro/define-docker-variables.sh"

# Exit successfully (and silently) if there are no documents to upload.
if [[ "${GENERATE_DOCS:-}" != "yes" ]]; then
  # No documentation generated by this build, skip upload.
  exit 0
fi

if [[ -n "${KOKORO_GITHUB_PULL_REQUEST_NUMBER:-}" ]]; then
  # Do not publish new documentation on PR builds.
  exit 0
fi

# Because Kokoro checks out the code in `detached HEAD` mode there is no easy
# way to discover what is the current branch (and Kokoro does not expose the
# branch as an enviroment variable, like other CI systems do). We use the
# following trick:
# - Find out the current commit using git rev-parse HEAD.
# - Find out what branches contain that commit.
# - Exclude "HEAD detached" branches (they are not really branches).
# - Typically this is the single branch that was checked out by Kokoro.
BRANCH="$(git branch --no-color --contains "$(git rev-parse HEAD)" | \
    grep -v 'HEAD detached' || exit 0)"
BRANCH="${BRANCH/  /}"
readonly BRANCH

echo "================================================================"
echo "Installing docuploader package $(date)."
python3 -m pip install gcp-docuploader

cd "${BUILD_OUTPUT}/google/cloud/spanner/html"

echo "================================================================"
echo "Detecting version $(date)."

if [[ "${BRANCH}" == "master" ]]; then
  docs_version="master"
else
  # Extract the version from index.html
  docs_version=$(sed -n 's:.*<span id="projectnumber">\(.*\)</span>.*:\1:p' \
    index.html)
fi

echo "================================================================"
echo "Creating docs metadata $(date)."

python3 -m docuploader create-metadata \
  --name "google-cloud-spanner" \
  --version "${docs_version}" \
  --language cpp

echo "================================================================"
echo "Publishing the docs $(date)."

python3 -m docuploader upload . \
  --credentials "${CREDENTIALS_FILE}" \
  --staging-bucket "${STAGING_BUCKET}"

echo "================================================================"
echo "Succeeded $(date)."
