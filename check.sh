#!/bin/bash
set -e
./build_simulator.sh
<final_tests/test_7.txt ./simulator >test.txt
diff test.txt working_test.txt
echo "OK"