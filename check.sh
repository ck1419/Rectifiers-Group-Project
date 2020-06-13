#!/bin/bash
set -e
g++ -I eigen-3.3.7/ main_code_improved.cpp -o simulator
<final_tests/test_7.txt ./simulator >test.txt
diff test.txt working_test.txt
echo "OK"