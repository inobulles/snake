#!/bin/sh
set -e

if [ $(which time) ]; then
	TIMEIT=time
fi

$TIMEIT cc main.c -I/usr/local/share/aqua/lib/c/ -shared -fPIC -o .package/native.bin
$TIMEIT iar --pack .package/ --output package.zpk