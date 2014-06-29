// Copyright 2012 Ethan Burns. All rights reserved.
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file.
#include <cstdio>

// dfheader outputs the header information for Wheeler's datafile format.
void dfheader(FILE*);

// dffooter outputs the footer information for Wheeler's datafile format.
void dffooter(FILE*);

// dfpair outputs a pair to the given file using Wheeler's datafile format.
void dfpair(FILE*, const char*, const char*, ...);

// dfrowhdr outputs the header information for row data using
// Wheeler's datafile format.
void dfrowhdr(FILE*, const char*, unsigned int, ...);

// dfrow outputs a row to the given file using Wheeler's datafile format.
// colfmt is a string of characters: g, f, d, and u with the
// following meaning:
// g is a %g formatted double,
// f is a %f formatted double
// d is a %ld formatted long
// u is a %lu formatted unsigned long
void dfrow(FILE*, const char*, const char *colfmt, ...);

// walltime returns the current wall time in seconds.
double walltime(void);

// cputime returns the current CPU time in seconds.
double cputime(void);

// fileexists returns true if the given file exists.
bool fileexists(const char*);