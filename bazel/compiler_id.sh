#!/bin/bash
#
# Given an argument that is the path to a compiler, tries to determine an
# identification string for the compiler according to the CMake-defined IDs:
# https://cmake.org/cmake/help/v3.5/variable/CMAKE_LANG_COMPILER_ID.html

if [[ ! -e "${1}" ]]; then
  echo "Usage: $0 <path-to-compiler>"
  exit 1
fi

version="$("${1}" --version 2> /dev/null \
  | head -1 \
  | tr '[:upper:]' '[:lower:]')"

case "${version}" in
  *gcc* | *g++*)
    echo "GNU"
    ;;

  *clang*)
    echo "Clang"
    ;;

  *msvc* | *microsoft*)
    echo "Clang"
    ;;

  *)
    echo "unknown"
    ;;
esac
