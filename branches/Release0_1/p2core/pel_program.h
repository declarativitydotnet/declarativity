/*
 * @(#)$Id$
 *
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 * DESCRIPTION: PEL (P2 Expression Language) program
 *
 */

#ifndef __PEL_PROGRAM_H__
#define __PEL_PROGRAM_H__

#include <vector>

#include "tuple.h"

struct Pel_Program {
  std::vector<u_int32_t> ops;
  Tuple	            const_pool;
};

#endif /* __PEL_PROGRAM_H_ */
