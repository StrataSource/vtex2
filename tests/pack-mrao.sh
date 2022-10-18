#!/usr/bin/env bash
set -e
cd "$(dirname "$0")"

echo "Convert M, R, AO to MRAO"
../build/vtex2 pack --mrao --metalness-map m.png --roughness-map r.png --ao-map ao.png --outfile mrao.vtf
