# load_gz
Playground to improve loading large .gz into GNU Octave

zfstream.cc/.h are taken from zlibs tarball:
zlib-1.2.11/contrib/iostream3/zfstream.cc,.h

# Tests with GNU Octave on AMD Ryzen 5 2600 Six-Core Processor

$ time gunzip -k foo.gz

real	0m0,511s
user	0m0,436s
sys	0m0,072s

$ ls -hla
-rw-r--r--  1 andy andy 140M Feb 22 10:41 foo
-rw-r--r--  1 andy andy 6,2M Feb 22 10:41 foo.gz

$ time octave --eval 'd=load("foo.gz");'

1 CPU, 100%, up to 1,7% mem usage

real	0m33,239s
user	0m33,256s
sys	0m0,656s

$ time octave --eval 'd=load("foo");'

real	0m32,073s
user	0m31,984s
sys	0m0,748s
