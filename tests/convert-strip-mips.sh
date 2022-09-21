#!/usr/bin/env bash
set -e
cd "$(dirname "$0")"

echo "Convert JPEG -> DXT1, 10 mips -> BC7, 0 mips"
../build/vtex2 convert -f dxt1 --clampu --clamps --clampt -o convert-vtf-src.vtf funny-cat-2.jpg
../build/vtex2 convert -f bc7 --no-mips -o convert-vtf-src.vtf convert-vtf-src.vtf
../build/vtfview convert-vtf-src.vtf
