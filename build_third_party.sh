#!/bin/bash

# Build 3rd-party libraries/dependencies. To be run (once) from the projects root folder.

BASE_FOLDER=$(pwd)

# Build googletest
cd "$BASE_FOLDER"
mkdir -p third_party/googletest/build
cd third_party/googletest/build
cmake -DCMAKE_BUILD_TYPE=Release -DINSTALL_GTEST:BOOL=OFF -DBUILD_GMOCK:BOOL=OFF -Dgtest_disable_pthreads:BOOL=ON ..
make -j 8

# Build google benchmark
cd "$BASE_FOLDER"
mkdir -p third_party/benchmark/build
cd third_party/benchmark/build
cmake -DCMAKE_BUILD_TYPE=Release -DBENCHMARK_ENABLE_INSTALL:BOOL=OFF -DBENCHMARK_INSTALL_DOCS:BOOL=OFF -DBENCHMARK_ENABLE_TESTING:BOOL=OFF -DBENCHMARK_ENABLE_GTEST_TESTS:BOOL=OFF -DBENCHMARK_ENABLE_WERROR:BOOL=OFF ..
make -j 8

# Build matplot viz library
cd "$BASE_FOLDER"
mkdir -p third_party/matplot/build
cd third_party/matplot/build
cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_EXAMPLES:BOOL=OFF -DBUILD_HIGH_RESOLUTION_WORLD_MAP:BOOL=OFF -DBUILD_INSTALLER:BOOL=OFF -DBUILD_PACKAGE:BOOL=OFF -DBUILD_TESTING:BOOL=OFF -DBUILD_TESTS:BOOL=OFF -DBUILD_WITH_MSVC_HACKS:BOOL=OFF ..
make -j 8