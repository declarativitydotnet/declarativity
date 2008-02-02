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
#ifndef __COMPILEUTIL_H__
#define __COMPILEUTIL_H__

#include <ostream>
#include "list.h"
#include "value.h"
#include "tuple.h"
#include "commonTable.h"

namespace compile {

  class Exception {
    public:
      Exception(string d="Compile Exception") : desc_(d) {};
      virtual string toString() { return desc_; };
      virtual ~Exception() {};
    private:
      string desc_;
  };


  namespace namestracker {
    
    ListPtr getMask(const ValuePtr v);
    
    void calculateMaskRecur(ListPtr &mask, const ValuePtr v);

    ListPtr applyMask(ListPtr original, ListPtr mask, unsigned oldPos);

    string exprString(TuplePtr expr);

    /**
     * Utility function that locates the variable name
     * within the argument list.  */
    int position(const ListPtr args, const ValuePtr var);

    int subset(const ListPtr schema1, const ListPtr schema2);

    /** Converts the argument (variable, location, or aggregation)
        to a regular variable. */
    ValuePtr toVar(ValuePtr var);

    bool isTheta(ValuePtr boolv);

    /** Find the location attribute in the schema */
    ValuePtr location(const ListPtr args);

    /**
     * Determine the position of an aggregation arguement 
     * if one exists. */
    int aggregation(const ListPtr args);

    ListPtr groupby(const ListPtr args);

    ListPtr flatten(const ListPtr args);
  
    /**
     * Utility function that forms a new argument list
     * out of two passed in argument lists.  The newly
     * formed list represents the tuple schema formed by
     * a join.  */
    ListPtr merge(const ListPtr outer, const ListPtr inner);

    bool equivalent(const ListPtr plan1, const ListPtr plan1);

    bool prefix(const ListPtr prefix, const ListPtr schema);

    ValuePtr sortAttr(const ListPtr outer, const ValuePtr outerOrder,
                      const ListPtr inner, const ValuePtr innerOrder);

    ListPtr adornment(const ListPtr bound, const ListPtr schema);

    ListPtr project(const ListPtr positions, const ListPtr schema);

    ListPtr assignSchema(const ListPtr outer, const ValuePtr var);

    bool filter(const ListPtr schema, const ValuePtr var);

    ListPtr variables(const ValuePtr var);

    void joinKeys(const ListPtr outer, const ListPtr inner,
                  CommonTable::Key& joinKey, 
                  CommonTable::Key& indexKey, 
                  CommonTable::Key& baseKey); 

    ValuePtr castassign(const ListPtr outer, 
                        const ListPtr inner, 
                        const TuplePtr select);
  };

  namespace pel {

    string 
    gen(const ListPtr schema, TuplePtr expr);

  };

};

#endif
