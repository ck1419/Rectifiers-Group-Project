#!/bin/bash
set -e

./build_simulator_improved.sh
echo "IMPROVED TIME:"
time <netlist.txt ./simulator_improved >output_improved.txt

./build_simulator.sh
echo "ORIGINAL TIME:"
time <netlist.txt ./simulator >output.txt