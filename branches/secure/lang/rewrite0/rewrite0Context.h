/*
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * Or
 * UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
 * Berkeley, CA,  94707. Attention: P2 Group.
 * 
 * DESCRIPTION: 
 *
 */

#ifndef __REWRITE0_CONTEXT_H__
#define __REWRITE0_CONTEXT_H__

#include <iostream>
#include <set>
#include "element.h"
#include "elementRegistry.h"
#include "compileContext.h"
#include "val_str.h"
#include "set.h"

namespace compile {
  namespace rewrite0 {
    const string STAGEVARPREFIX = "RW0_";

    class Exception : public compile::Exception {
    public:
      Exception(string msg) : compile::Exception(msg) {};
      virtual ~Exception() {};
    };

    class Context : public compile::Context {
    public:
      Context(string name); 
      Context(TuplePtr args); 
  
      virtual ~Context() {
      }; 
  
      const char *class_name() const { return "rewrite0::Context";}
  
      DECLARE_PUBLIC_ELEMENT_INITS

    private:

      /* Process the current rule in the program */
      void rule(CommonTable::ManagerPtr catalog, TuplePtr rule);

      void serializeNewFunctor(CommonTable::ManagerPtr catalog, TuplePtr& functor);
      DECLARE_PRIVATE_ELEMENT_INITS
    };
  }
}

#endif /* __REWRITE0_CONTEXT_H__ */
