#!/usr/bin/env bash
set -e
cd "$(dirname "$0")"

echo "Convert JPEG -> DXT1, 512x1024 -> DXT5, 512x512"
../build/vtex2 convert -f dxt1 --width 512 --height 1024 -o convert-vtf-src.vtf funny-cat-2.jpg
../build/vtex2 convert -f dxt5 --width 512 --height 512 -o convert-vtf-src.vtf convert-vtf-src.vtf
../build/vtfview convert-vtf-src.vtf
