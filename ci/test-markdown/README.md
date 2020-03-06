# Helper tools to generate certain markdown files.

## HOWTO: Generate the doc/packaging.md file.

This file is generated from the Dockerfiles in `ci/kokoro/install` just run:

```bash
cd google-cloud-cpp-spanner
./ci/test-readme/generate-packaging.sh > doc/packaging.md
```

and then create a pull request to merge your changes.

## HOWTO: Generate the README.md file.

To generate the README.md file just run:

```bash
cd google-cloud-cpp-spanner
./ci/test-readme/generate-readme.sh > README.md
```

## HOWTO: Manually verify the instructions.

If you need to change the instructions then change the relevant Dockerfile and
execute (for example):

```bash
export NCPU=$(nproc)
cd google-cloud-cpp-spanner
./ci/kokoro/install/build.sh fedora
```

Note that any change to the project contents, including changes to the README,
doc/packaging.md, or any other documentation, starts the docker build almost
from the beginning. This is working as intended, we want these scripts to
simulate the experience of a user downloading the project on their workstation.
