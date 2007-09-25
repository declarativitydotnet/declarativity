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

#ifndef __PLANNER_CONTEXT_H__
#define __PLANNER_CONTEXT_H__

#include <iostream>
#include <ostream>
#include <map>
#include "compileContext.h"
#include "value.h"
#include "commonTable.h"
#include "tuple.h"
#include "val_str.h"
#include "val_int64.h"
#include "element.h"
#include "elementRegistry.h"


namespace compile {
  namespace planner {
    class Exception : public compile::Context::Exception {
    public:
      Exception(string msg) : compile::Context::Exception(msg) {};
    };

    class Context : public compile::Context {
    public:
      /** Context Constructor:
       ** Arg 1: Name of the element
       ** Arg 2: Name of the main dataflow
       ** Arg 3: Name of the internal strand input element
       ** Arg 4: Name of the internal strand output element
       ** Arg 5: Name of the external strand output element */
      Context(string, string, string, string, string);
      Context(TuplePtr args);

      virtual ~Context();
  
      const char *class_name() const { return "planner::Context";}
  
      DECLARE_PUBLIC_ELEMENT_INITS

    private:
      string _mainDataflowName;
      string _internalStrandInputElement;
      string _internalStrandOutputElement;
      string _externalStrandOutputElement;

      long _nameCounter; // Used to create unique graph names

      std::map<string, ValuePtr> programEvents;

      int initialize();
 
      /** Helper data structure used to describe the ports
        * characteristics of subgraphs created by the
        * event, condition, and action portions of a rule */
      typedef struct PortDesc {
        int   inputs;
        char* inProc;
        char* inFlow;
        int   outputs;
        char* outProc;
        char* outFlow;
      } PortDesc;
  
      TuplePtr program(CommonTable::ManagerPtr catalog, TuplePtr rule);

      void rule(CommonTable::ManagerPtr catalog, TuplePtr rule);
  
      void ruleTerms(CommonTable::ManagerPtr catalog, 
                     TuplePtr rule, TuplePtrList& terms);
  
      PortDesc event(ostringstream& oss, string indent, TuplePtr rule,
                     CommonTable::ManagerPtr catalog, 
                     TuplePtrList& terms);
  
      PortDesc condition(ostringstream& oss, string indent, TuplePtr rule,
                         CommonTable::ManagerPtr catalog, 
                         TuplePtrList& terms);
  
      PortDesc action(ostringstream& oss, string indent, TuplePtr rule,
                      CommonTable::ManagerPtr catalog, 
                      TuplePtrList& terms);
  
      PortDesc insertEvent(ostringstream& oss, string indent, TuplePtr rule,
                           CommonTable::ManagerPtr catalog,
                           TuplePtr head, TuplePtr event);
  
      string probe(ostringstream& oss, string indent,
                   CommonTable::ManagerPtr catalog, 
                   TuplePtr probe, ListPtr tupleSchema, bool filter);
  
      string assign(ostringstream& oss, string indent,
                    CommonTable::ManagerPtr catalog, 
                    TuplePtr assign, ListPtr tupleSchema);
  
      string select(ostringstream& oss, string indent,
                    CommonTable::ManagerPtr catalog, 
                    TuplePtr select, ListPtr tupleSchema);

      /** Given a predicate name and modifier, returns true
        * if the predicate is to be watched and false otherwise.
        * An empty string modifier refers to a regular watch statement. */
      bool watched(string name, string mod);

      DECLARE_PRIVATE_ELEMENT_INITS
    };
  }
}

#endif /* __PLANNER_CONTEXT_H__ */
