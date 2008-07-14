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

#ifndef __REWRITE2_CONTEXT_H__
#define __REWRITE2_CONTEXT_H__

#include <iostream>
#include "element.h"
#include "elementRegistry.h"
#include "compileContext.h"
#include "val_str.h"

namespace compile {
  namespace rewrite2 {
    const string STAGEVARPREFIX = "RW2_";
    const string CERTFN = "f_getCert";
    const uint32_t CURVERSION = 0;
    class Exception : public compile::Exception {
    public:
      Exception(string msg) : compile::Exception(msg) {};
    };

    class Context : public compile::Context {
    public:
      Context(string name); 
      Context(TuplePtr args); 
  
      virtual ~Context() {}; 
  
      const char *class_name() const { return "rewrite2::Context";}
  
      DECLARE_PUBLIC_ELEMENT_INITS

    private:
      /* Process the current rule in the program */
      void rule(CommonTable::ManagerPtr catalog, TuplePtr rule);

      /* If location is NULL, then create a new location variable else use the passed location variable*/
      TuplePtr makeVersionedTuple(uint32_t &varSuffix, TuplePtr functor, TuplePtr location, CommonTable::ManagerPtr catalog);

      TuplePtr createLocSpecTuple(ValuePtr ruleId, TuplePtr location, TuplePtr locSpec, TuplePtr refLocation, 
				  TuplePtr ver, CommonTable::ManagerPtr catalog);

      TuplePtr createCurVerAssign(ValuePtr ruleId, TuplePtr ver);

      TuplePtr createCurVerSelect(ValuePtr ruleId, TuplePtr ver);

      TuplePtr createLinkExpanderTuple(ValuePtr ruleId, TuplePtr location, TuplePtr locSpec, TuplePtr linkExpanderSet, CommonTable::ManagerPtr catalog);

      TuplePtr createLinkExpanderCheck(ValuePtr ruleId, TuplePtr linkExpanderSet, TuplePtr ver);

      TuplePtr program(CommonTable::ManagerPtr catalog, TuplePtr rule);

      DECLARE_PRIVATE_ELEMENT_INITS
    };
  }
}

#endif /* __REWRITE2_CONTEXT_H__ */
