#!/bin/bash

BASE_FOLDER=$(pwd)

cd "$BASE_FOLDER"
mkdir -p build/release
cd build/release
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS_RELEASE=-O2 ../..
if [ "$1" = "developer" ]; then
	cmake -Dspice_build_benchmarks:BOOL=TRUE -Dspice_build_tests:BOOL=TRUE ../..
fi

cd "$BASE_FOLDER"
mkdir -p build/debug
cd build/debug
cmake -DCMAKE_BUILD_TYPE=Debug ../..
if [ "$1" = "developer" ]; then
	cmake -Dspice_build_benchmarks:BOOL=TRUE -Dspice_build_tests:BOOL=TRUE ../..
fi

if [ "$1" = "developer" ]; then
	cd "$BASE_FOLDER"
	mkdir -p build/perf
	cd build/perf
	cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS_RELEASE="-O2 -ggdb" -Dspice_build_benchmarks:BOOL=TRUE -Dspice_build_tests:BOOL=FALSE -Dspice_assert_preconditions:BOOL=FALSE -Dspice_assert_invariants:BOOL=FALSE ../..
fi