# Helper tools to generate README.md and INSTALL.md

## HOWTO: Generate the INSTALL.md file.

The INSTALL file is generated from the Dockerfiles in `ci/kokoro/install`
just run:

```bash
cd google-cloud-cpp
./ci/test-readme/generate-install.sh >INSTALL.md
```

and then create a pull request to merge your changes.

## HOWTO: Generate the README.md file.

The README file is generated from multiple sources, including the Dockefiles in
`ci/kokoro/readme`, and the list of builds and badges from `ci/kokoro`. To
generate it just run:

```bash
cd google-cloud-cpp
./ci/test-readme/generate-readme.sh >README.md
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
INSTALL, or any other documentation, starts the docker build almost from the
beginning. This is working as intended, we want these scripts to simulate the
experience of a user downloading the project on their workstation.
