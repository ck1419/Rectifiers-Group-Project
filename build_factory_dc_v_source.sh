#!/bin/bash
set -e
g++ -I eigen-3.3.7/ factory_dc_v_source_new.cpp component.cpp -o factory_dc_voltage_sources
