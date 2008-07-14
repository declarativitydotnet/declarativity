/*
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * Or
 * UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
 * Berkeley, CA,  94707. Attention: P2 Group.
 * 
 * DESCRIPTION: All rewrites extend this abstract class. This class
 * provides some basic functionality common to a rewrite. Its primary
 * duty is to deal with the dataflow tuples that trigger the actual
 * rewrite. A rewrite tuple is a programEvent tuple containing the
 * attributes listed in the PROGRAM table. Upon receipt of a 
 * programEvent in simple_action, we call the program function which
 * will iterate over all rule tuples in the program, calling the
 * rule function for each one. It is assumed that the rewrite extending
 * this class will override the rule function in order to do its rewrite
 * of the rule. Nothing prohibits a rewrite from overriding the program
 * function as well. See the parse rewrite for such an example.
 */

#ifndef __COMPILE_CONTEXT_H__
#define __COMPILE_CONTEXT_H__

#include <deque>
#include <map>
#include "compileUtil.h"
#include "commonTable.h"
#include "element.h"
#include "value.h"
#include "tuple.h"
#include "table2.h"
#include "list.h"

namespace compile {

  class Context : public Element {
  public:
    Context(string name) 
    : Element(name, 1, 1) {};

    virtual ~Context() {};

    virtual const char *class_name() const = 0;
    const char *processing() const { return "a/a"; }
    const char *flow_code()  const { return "x/x"; }

    TuplePtr simple_action(TuplePtr p);

  protected:
    /** Performs rewrite stage on the given program. */ 
    virtual TuplePtr program(CommonTable::ManagerPtr catalog, TuplePtr program);
    /** Performs rewrite stage on the given rule. */
    virtual void rule(CommonTable::ManagerPtr catalog, TuplePtr rule);
  };
};

#endif /* __COMPILE_CONTEXT_H__ */
