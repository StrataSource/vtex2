#!/usr/bin/env bash
set -e
cd "$(dirname "$0")"

echo "Convert JPEG -> RGBA8888 VTF"
../build/vtex2 convert -f rgba8888 -o convert-test.vtf funny-cat-2.jpg
../build/vtfview convert-test.vtf
