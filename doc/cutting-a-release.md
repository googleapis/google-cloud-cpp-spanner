# Creating a new release of google-cloud-cpp-spanner

Unless there are no changes, we create releases for `google-cloud-cpp-spanner`
every month, or if there is a major announcement or change to the status of the
library.

The intended audience of this document are developers in the
`google-cloud-cpp-spanner` project that need to create a new release. The
audience is expected to be familiar with the project itself, [git][git-docs],
[GitHub][github-guides], and [semantic versioning](https://semver.org).

## Preparing for a release

First you should collect and update the release notes for the project. Prepare
a pull request (PR) with the necessary changes to the README file.

Assuming you are working on your own fork of the `google-cloud-cpp-spanner`
project, and `upstream` points to the `googleapis/google-cloud-cpp-spanner`
remote, these commands should be useful in identifying important changes:

```bash
git log --no-merges --format="format:* %s" \
    $(git describe --tags --abbrev=0 upstream/master)..HEAD \
    upstream/master
```

Write an edited version of these changes to `ci/test-readme/README.md.in`,
consider skipping information that is not useful for consumers of the library,
such as CI system changes and internal cleanups.

### Update README.md

Update the top-level README file:

```bash
./ci/test-readme/generate-readme.sh >README.md
```

### Update INSTALL.md

Update the instructions to install the library:

```bash
./ci/test-readme/generate-install.sh >INSTALL.md
```

It is not recommended that you create the release branch before this PR is
*merged*, but in some circumstances it might be needed, for example, if a large
change that could destabilize the release is about to be merged, or if we want
to create the release at an specific point in the revision history.

## Creating the release

We next need to create the release tag, the release branch, and create the
release in the GitHub UI. These steps are handled automatically for us by the
`./release/release.sh` script that lives in the
[googleapis/google-cloud-cpp-common](https://github.com/googleapis/google-cloud-cpp-common/blob/master/release/release.sh)
repo. The following steps assume you have that script in your path or that
you're specifying the full path to that script.

*No PR is needed for this step.*

First run the following command -- which will *NOT* make any changes to any
repos -- and verify that the output and *version numbers* look correct.

```bash
$ release.sh googleapis/google-cloud-cpp-spanner
```

If the output from the previous command looks OK, rerun the command with the
`-f` flag, which will make the changes and push them to the remote repo.

```bash
$ release.sh -f googleapis/google-cloud-cpp-spanner
```

**NOTE:** This script can be run from any directory. It operates only on the
specified repo.

## Add the release notes to the GitHub release

The `release.sh` script that was run in the previous step should have created a
new "pre-release" at
https://github.com/googleapis/google-cloud-cpp-spanner/releases. Add the release
notes to the Release.

After you create the release, capture the SHA256 checksums of the
tarball and zip files, and edit the notes to include them. These
commands might be handy:

```bash
TAG="<release-tag>" # change this to the actual tag, e.g., "v0.5.0"
wget -q -O - "https://github.com/googleapis/google-cloud-cpp-spanner/archive/${TAG}.tar.gz" | sha256sum
wget -q -O - "https://github.com/googleapis/google-cloud-cpp-spanner/archive/${TAG}.zip" | sha256sum
```

Ask your colleagues to review the release notes. There should be few/no edits
needed at this point since the release notes were already reviewed in step 1.

### Publish the release

Uncheck the pre-release checkbox and publish.

## Generate and upload the documentation to googleapis.dev

Manually run a Kokoro job
`cloud-devrel/client-libraries/cpp/google-cloud-cpp-spanner/publish-refdocs` in
the Cloud C++ internal testing dashboard and specify the branch name
(e.g. `v0.2.x`) in the `Committish` field. This job will generate and upload
the doxygen documentation to the staging bucket for googleapis.dev hosting. The
uploaded documentation will generally be live in an hour at the
[googleapis.dev site](https://googleapis.dev/cpp/google-cloud-spanner/latest/).

## Bump the version numbers in `master`

Working in your fork of `gooogle-cloud-cpp-spanner`: bump the version numbers
to the *next* version (i.e., one version past the release you just did above),
and send the PR for review against `master`. For an example, look at
[#943](https://github.com/googleapis/google-cloud-cpp-spanner/pull/943)

## Review the branch protections

We use the [GitHub Branch Settings][github-branch-settings] to protect the
release branches against accidental mistakes. From time to time changes in the
release branch naming conventions may require you to change these settings.
Please note that we use more strict settings for release branches than for
`master`, in particular:

* We require at least one review, but stale reviews are dismissed.
* The `Require status checks to pass before merging` option is set.
  This prevents merges into the release branches that break the build.
  * The `Require branches to be up to date before merging` sub-option
    is set. This prevents two merges that do not conflict, but nevertheless
    break if both are pushed, to actually merge.
  * The `Kokoro Ubuntu`, `Kokoro Windows`, `cla/google`, and
    `continuous-integration/travis-ci` checks are required to pass.

* The `Include administrators` checkbox is turned on, we want to stop ourselves
  from making mistakes.

* Turn on the `Restrict who can push to matching branches`. Only Google team
  members should be pushing to release branches.

[git-docs]: https://git-scm.com/doc
[github-guides]: https://guides.github.com/
[github-branch-settings]: https://github.com/googleapis/google-cloud-cpp-spanner/settings/branches
