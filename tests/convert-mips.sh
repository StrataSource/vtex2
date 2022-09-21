#!/usr/bin/env bash
set -e
cd "$(dirname "$0")"

echo "Convert JPEG -> DXT1, 10 mips -> BC7, 512x512, 5 mips"
../build/vtex2 convert -f dxt1 -o convert-vtf-src.vtf funny-cat-2.jpg
../build/vtex2 convert -f bc7 --width 512 --height 512 -m 5 -o convert-vtf-src.vtf convert-vtf-src.vtf
../build/vtfview convert-vtf-src.vtf
