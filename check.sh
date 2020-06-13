#!/bin/bash
set -e

./build_simulator_improved.sh
echo "IMPROVED TIME:"
time <final_tests/test_5.txt ./simulator_improved >output_improved.txt

./build_simulator.sh
echo "ORIGINAL TIME:"
time <final_tests/test_5.txt ./simulator >output.txt

diff output_improved.txt output.txt
echo "OK"
