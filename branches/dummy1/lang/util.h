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

#include "list.h"
#include "value.h"
#include "tuple.h"
#include "commonTable.h"

namespace compile {

  namespace namestracker {
    /**
     * Utility function that locates the variable name
     * within the argument list.  */
    int position(const ListPtr args, const ValuePtr var);

    /**
     * Determine the position of an aggregation arguement 
     * if one exists. */
    int aggregation(const ListPtr args);
  
    /**
     * Utility function that forms a new argument list
     * out of two passed in argument lists.  The newly
     * formed list represents the tuple schema formed by
     * a join.  */
    ListPtr merge(const ListPtr outer, const ListPtr inner);

    void joinKeys(const ListPtr outer, const ListPtr inner,
                  CommonTable::Key& joinKey, 
                  CommonTable::Key& indexKey, 
                  CommonTable::Key& baseKey); 
  };

  namespace pel {

    string 
    gen(const ListPtr schema, TuplePtr expr);

  };
};
