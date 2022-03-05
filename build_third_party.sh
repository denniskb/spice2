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