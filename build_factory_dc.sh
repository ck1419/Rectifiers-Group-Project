#!/bin/bash
set -e
g++ -I eigen-3.3.7/ factory_dc.cpp component.cpp -o factory_dc
