#!/bin/sh
set -e

if [ $(which time) ]; then
	TIMEIT=time
fi

$TIMEIT cc main.c -I/usr/local/include -liar -I/home/obiwac/aqua-unix/src/lib/c -shared -fPIC -o .package/entry.native # -fprofile-instr-generate
$TIMEIT aqua-manager --layout
$TIMEIT iar --pack .package/ --output package.zpk
