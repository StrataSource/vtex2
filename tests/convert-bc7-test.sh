#!/usr/bin/env bash
set -e
cd "$(dirname "$0")"

echo "Convert JPEG -> BC7"
../build/vtex2 convert -f bc7 -c 9 -o convert-test-bc7.vtf funny-cat-2.jpg
../build/vtfview convert-test-bc7.vtf
