#!/usr/bin/env bash
set -e
cd "$(dirname "$0")"

echo "Convert JPEG -> RGB888 512x256"
../build/vtex2 convert -f rgb888 --width 512 --height 256 -o convert-clamp-test.vtf funny-cat-2.jpg
../build/vtfview convert-clamp-test.vtf
