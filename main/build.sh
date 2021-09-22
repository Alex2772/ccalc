#!/bin/bash
cd /home/alex2772/esp-idf/CCOS/main
if ! test -f build-number.txt; then echo 0 > build-number.txt; fi
number=$(cat build-number.txt)
number=$((number+1))
echo $number > build-number.txt
echo -e "#include \"Build.h\"\n uint32_t Build::getBuildNumber() { return $number; }" > Build.cpp
