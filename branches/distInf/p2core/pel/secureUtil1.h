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

    ValuePtr serializeTuple1(TuplePtr tuple, string name, std::set<uint32_t> linkSet, std::set<uint32_t> skipFields, uint32_t linkSetOffset);

    ValuePtr generateLocSpec(bool strong = false);

    ValuePtr generateVersion(bool strong = false);

    bool isLocSpec(ValuePtr v);
    
    bool isVersion(ValuePtr v);

    /**
     * given an opaque, return whether the opaque is says opaque or not?
     */
    bool isSaysHint(ValuePtr v);

    ValuePtr sha1(ValuePtr);

    ValuePtr loadFile(ValuePtr);

    ValuePtr getCert(ValuePtr);

    ValuePtr processGen(ValuePtr, ValuePtr);

    ValuePtr processExtract(ValuePtr, ValuePtr);

    ValuePtr serialize(CommonTable::ManagerPtr catalog, TuplePtr parent, string parentRootName, ListPtr buf, uint32_t myRefType, bool certify);

  };
  
};

#endif //SECUREUTIL1
