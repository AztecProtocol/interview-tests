#!/usr/bin/env bash

set -euo pipefail

echo "Compiling."

cd "$(dirname "$0")"/src

clang++ \
  -std=c++20 \
  -Wall -Wextra \
  -O2 \
  -o merkle_test \
  sha256.cpp \
  main.cpp

echo "Running tests."
./merkle_test

echo "All done!"