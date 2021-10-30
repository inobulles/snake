#!/bin/bash
set -e

# GIMP: why use standard integer types when you could use shitty glibc alternatives?

sed -i 's/guint8/uint8_t/g' img/$1.c
sed -i 's/guint/unsigned/g' img/$1.c

sed -i "s/gimp_image/img_$1/g" img/$1.c

mv img/$1.c img/$1.h
