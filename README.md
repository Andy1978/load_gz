# load_gz

Loads numerical 2D matrices from CSV files into GNU Octave.

key points:

* it's fast (currently >10 times faster than `load` from core)
* works on uncompressed and gzip compressed files
* supports incremental load of increasing files
* returns line comments starting with '#' into Octave
* robustness: works with different mixed column separators and line endings (LF, CR, CR+LF mixed)

## Getting started

### Dependencies:

* zlib
* GNU Octave >= 3.8
* mkoctfile

On Debian based systems and if you've installed GNU Octave via `apt`, `aptitude` or `apt-get`:

    $ apt install zlib1g-dev liboctave-dev

### Build

    $ git clone https://github.com/Andy1978/load_gz.git
    $ cd load_gz && make

### Install

Add the directory to your GNU Octave load path, for example:

    $ echo "addpath (\"$(pwd)\");" >> ~/.octaverc

### How parsing works

Data read from a gzip compressed or uncompressed CSV file is read in BUFFER_SIZE chunks and fed into `parse_csv`.

The C function `strtod` reads until it hits a non-convertible character. The returned double is written into the result matrix if there conversion was successful and the column pointer is incremented.

The internal Matrix is resized when needed. A row resize is done with a `GROWTH_FACTOR` of typically 1.5 where the `INITIAL_ROWS` are 100.

If the next char is a newline (CR or LF), the next row is addressed and the column index is set to 0. Subsequent newline chars are ignored.

With this it's possible to mix single column delimiters, for example

    "4 5.6;7.8,9" returns [4 5.6 7.8 9]

If there are two non-convertible chars, the empty val (NA) is used.

    "4;;5;6" returns [4 NA 5 6]

The CSV can have line comments starting with # and are returned with `get`


### Usage

    octave:> x = load_gz ("./tests/sample1.csv");
    octave:> y = mget(x);  # returns a 2D double matrix

Get internals:

	octave:> get(x)
	ans =

	  scalar structure containing the fields:

	    fn = ./tests/sample1.csv
	    rows =  10
	    allocated_rows =  100
	    columns =  4
	    comments =
	    {
	      [1,1] = # testing mixed delimiter and linebreaks
	      [2,1] = #
	      [3,1] = # CR linebreak (classic apple)
	      [4,1] = # CR+LF linebreak (windoze)
	    }

### Bugs

Zarro Boogs Found

### Fuzzing

The underlying workhorse function `parse_csv` has been extensively tested and fuzzed with zzuf and afl, see ./tests
