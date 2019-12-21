#!/bin/bash -e

make parse

# Sweep BUFFER_SIZE from 16..230

for k in `seq 20 290`; do
  echo "Run parse with buffersize = $k..."
  ./parse sample1.csv -b $k > sample1.out
  ./parse sample2.csv -b $k > sample2.out
  diff sample1.out sample1.ref
  diff sample2.out sample2.ref
  #md5sum -c md5sums.ref
done

