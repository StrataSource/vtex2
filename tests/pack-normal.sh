#!/usr/bin/env bash
set -e
cd "$(dirname "$0")"

echo "Convert M, R, AO to MRAO"
../build/vtex2 pack --normal --normal-map normal.png --height-map height.png --outfile normal.vtf
