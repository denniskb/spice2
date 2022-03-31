#!/bin/bash

if [ "$1" = "debug" ]; then
	cd build/debug
	make -j 8
elif [ "$1" = "perf" ]; then
	cd build/perf
	make -j 8
else
	cd build/release
	make -j 8
fi