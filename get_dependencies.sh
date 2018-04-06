#!/usr/bin/env bash

cd "$(dirname "${BASH_SOURCE[0]}")"

# Install Google Test
rm -rf googletest
curl -L https://github.com/google/googletest/archive/release-1.8.0.tar.gz | tar zx
mkdir -p lib
mv googletest-release-1.8.0 lib/googletest
