#!/bin/bash -e

# Sweep BUFFER_SIZE from 16..230

for k in `seq 16 230`; do
  echo "set BUFFER_SIZE to $k... "
  sed -i "s/#define BUFFER_SIZE.*$/#define BUFFER_SIZE $k/" parse.c
  make -s
  ./parse sample1.csv > sample1.out
  ./parse sample2.csv > sample2.out
  md5sum -c md5sums.ref
done

