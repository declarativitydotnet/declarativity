/*
 * @(#)$Id$
 *
 * Copyright (c) 2005 Intel Corporation. All rights reserved.
 *
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 * DESCRIPTION: Test suite for fdbufs
 *
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */
#include <iostream>
#include <string>

#include "fdbuf.h"

#define TSTERR std::cerr << __FILE__ << ":" << __LINE__ << ": test FAILED: "
#define TST(_e,_l,_s,_c) { _c; \
 if (fb.length() != _l) { TSTERR << "bad length of " << fb.length() << "; expected " << _l << "\n"; } \
 if (fb.last_errno() != _e) { TSTERR << "bad errno of " << fb.last_errno() << "; expected " << _s << "\n"; } \
 if (fb.str() != _s) { TSTERR << "bad value of " << fb.str() << "; expected " << _s << "\n"; } \
}

int main(int argc, char **argv)
{
  std::cout << "FDBUF\n";
  Fdbuf fb(0);

  TST(0,0,"",);
  TST(0,0,"",fb.clear());
  TST(0,1,"1",fb.push_back(1));
  TST(0,2,"12",fb.push_back(2));
  TST(0,3,"122",fb.push_back("2"));
  TST(0,0,"",fb.clear());
  TST(0,12,"Hello, world",fb.push_back("Hello, world"));
  TST(0,30,"Hello, world, and other worlds", std::string s(", and other worlds"); fb.push_back(s));
  TST(0,0,"",fb.clear());
  TST(0,5,"1.234",fb.push_back(1.234));
  TST(0,6,"1.234a",fb.push_back('a'));
  TST(0,0,"",fb.clear());
  TST(0,5,"This ",fb.push_back("This is a string",5));

  return 0;
}
  

/*
 * End of file 
 */
