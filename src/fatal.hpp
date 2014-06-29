// Copyright 2012 Ethan Burns. All rights reserved.
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file.
#ifndef _FATAL_HPP_
#define _FATAL_HPP_

struct Fatal {
	Fatal(const char*, ...);
	char msg[128];
};

#endif	// _FATAL_HPP_
