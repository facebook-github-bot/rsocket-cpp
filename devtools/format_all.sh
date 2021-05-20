#!/usr/bin/env bash
#
# Copyright 2004-present Facebook. All Rights Reserved.
#
set -xue

cd "$(dirname "$0")/.."
find src/ test/ tck-test/ experimental/ -type f '(' -name '*.cpp' -o -name '*.h' ')' -exec clang-format -style=file -i {} \;

# EOF
