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
 * DESCRIPTION: PEL (P2 Expression Language) program
 *
 */

#include <vector>

#include "tuple.h"

struct Pel_Program {
  std::vector<u_int32_t> ops;
  Tuple	            const_pool;
};

  

/*
 * End of file 
 */
