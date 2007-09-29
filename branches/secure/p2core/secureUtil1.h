/*
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * Or
 * UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
 * Berkeley, CA,  94707. Attention: P2 Group.
 * 
 * DESCRIPTION: Utility classes for compile process
 *
 */

#ifndef __SECUREUTIL1_H__
#define __SECUREUTIL1_H__

#include <ostream>
#include <set>
#include "value.h"
#include "compileUtil.h"

namespace compile {

  
  namespace secure {

    ValuePtr generateLocSpec(bool strong = false);

    ValuePtr generateVersion(bool strong = false);

    bool isLocSpec(ValuePtr v);

    ValuePtr processGen(ValuePtr, ValuePtr);

    ValuePtr processExtract(ValuePtr, ValuePtr);

    void serialize(CommonTable::ManagerPtr catalog, TuplePtr parent, string parentRootName, ListPtr buf, uint32_t myRefType);

  };
  
};

#endif //SECUREUTIL1
