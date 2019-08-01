#!/bin/bash
#
# Given an argument that is the path to a compiler, tries to determine
# the compiler's version by looking for the first string that looks like
# X.Y with more optional numbers '.', '-', and '+'.

if [[ ! -e "${1}" ]]; then
  echo "Usage: $0 <path-to-compiler>"
  exit 1
fi

${1} --version 2> /dev/null | grep -Eo "[0-9]+\.[0-9.+-]+" | head -1
