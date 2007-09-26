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

#include "rewriteContext.h"
#include "plumber.h"
#include "systemTable.h"
#include "value.h"
#include "val_str.h"
#include "val_null.h"
#include "val_uint32.h"
#include "val_int32.h"
#include "val_list.h"
#include "val_tuple.h"
#include "oper.h"

namespace compile {
  namespace rewrite {
  using namespace opr;

    DEFINE_ELEMENT_INITS_NS(Context, "RewriteContext", compile::rewrite)

    Context::Context(string name)
    : compile::Context(name) { }

    Context::Context(TuplePtr args)
    : compile::Context((*args)[2]->toString()) {}

    int 
    Context::initialize()
    {
      CommonTable::ManagerPtr catalog = Plumber::catalog();

      CommonTablePtr programTbl = catalog->table(PROGRAM);  
      CommonTable::Iterator iter = programTbl->scan();
      while (!iter->done()) {
        compile::Context::program(catalog, iter->next());
      }

      return 0;
    }

    TuplePtr 
    Context::program(CommonTable::ManagerPtr catalog, TuplePtr program) 
    {
      if ((*program)[catalog->attribute(PROGRAM, REWRITE)] == Val_Null::mk()) {
        /* Override the parent program method... In otherwords do nothing. */
        program = program->clone(PROGRAM);
        program->freeze();
        return program;
      }
      _stageName = (*program)[catalog->attribute(PROGRAM, "NAME")]->toString();
      /* Calling my parent program method will cause my rule method to
         be called for each rule in the program. */
      return this->compile::Context::program(catalog, program);
    }
  
    void
    Context::rule(CommonTable::ManagerPtr catalog, TuplePtr rule)
    {
      CommonTablePtr functorTbl = catalog->table(FUNCTOR);
      CommonTablePtr assignTbl  = catalog->table(ASSIGN);
      CommonTablePtr selectTbl  = catalog->table(SELECT);

      TELL_INFO << "REWRITE STATE PROCESS RULE: " << rule->toString() << std::endl;
      CommonTable::Key functorKey;
      functorKey.push_back(catalog->attribute(FUNCTOR, "RID"));

      CommonTable::Key assignKey;
      assignKey.push_back(catalog->attribute(ASSIGN, "RID"));

      CommonTable::Key selectKey;
      selectKey.push_back(catalog->attribute(SELECT, "RID"));

      ValuePtr locVariable;
      unsigned pos = 0;
      CommonTable::Iterator iter;
      for (iter = functorTbl->lookup(CommonTable::theKey(CommonTable::KEY2), 
                                     functorKey, rule); !iter->done(); pos++) { 
        TuplePtr term = iter->next()->clone();
        if (pos == 1) {
          ListPtr schema = Val_List::cast((*term)[catalog->attribute(FUNCTOR, "ATTRIBUTES")]);
          locVariable = namestracker::location(schema);
        }
        else if (pos > 1) {
          int oldPos = Val_Int32::cast((*term)[catalog->attribute(FUNCTOR, "POSITION")]);
          term->set(catalog->attribute(FUNCTOR, "POSITION"), Val_UInt32::mk(oldPos+2));
          term->freeze();
          functorTbl->insert(term);
        }
      }
      for (iter = assignTbl->lookup(CommonTable::theKey(CommonTable::KEY2), 
                                     assignKey, rule); !iter->done(); pos++) { 
        TuplePtr term = iter->next()->clone();
        int oldPos = Val_Int32::cast((*term)[catalog->attribute(ASSIGN, "POSITION")]);
        term->set(catalog->attribute(ASSIGN, "POSITION"), Val_UInt32::mk(oldPos+2));
        term->freeze();
        assignTbl->insert(term);
      }
      for (iter = selectTbl->lookup(CommonTable::theKey(CommonTable::KEY2), 
                                     selectKey, rule); !iter->done(); pos++) { 
        TuplePtr term = iter->next()->clone();
        int oldPos = Val_Int32::cast((*term)[catalog->attribute(SELECT, "POSITION")]);
        term->set(catalog->attribute(SELECT, "POSITION"), Val_UInt32::mk(oldPos+2));
        term->freeze();
        selectTbl->insert(term);
      }

      /* Create the status variable */
      TuplePtr locationVariable = Tuple::mk(LOC);
      locationVariable->append(locVariable);
      locationVariable->freeze();

      /* Create the status variable */
      TuplePtr statusVariable = Tuple::mk(VAR);
      statusVariable->append(Val_Str::mk(string("Status_") + COMPILE_STATUS));
      statusVariable->freeze();
   
      /* Create the status predicate schema */
      ListPtr schema = List::mk();
      schema->append(Val_Tuple::mk(locationVariable));
      schema->append(Val_Tuple::mk(statusVariable));   

      /* Create the status table predicate */
      TuplePtr status = Tuple::mk(FUNCTOR, true);
      status->append((*rule)[TUPLE_ID]);             // The "trigger" rule identifier
      status->append(Val_Str::mk(COMPILE_STATUS)); // Status tablename
      status->append(Val_Null::mk());                // Set TID below
      status->append(Val_Str::mk("PROBE"));          // ECA action type
      status->append(Val_List::mk(schema));          // Schema
      status->append(Val_UInt32::mk(2));             // Position right after event
      status->append(Val_Null::mk());                // Acess method

      /* Locate the table id associated with the name COMPILE_STATUS.
         Use the table id to set the TID field of the status table predicate */
      CommonTable::Key indexKey;
      indexKey.push_back(catalog->attribute(TABLE, "TABLENAME"));
      iter = catalog->table(TABLE)->lookup(CommonTable::theKey(CommonTable::KEY4), 
                                           indexKey, status);
      if (iter->done()) 
        throw Exception(string("Rewrite status table does not exist: ") + COMPILE_STATUS);
      status->set(catalog->attribute(FUNCTOR, "TID"), (*iter->next())[TUPLE_ID]);
      status->freeze();
      functorTbl->insert(status); // Commit the status predicate to the rule.
      TELL_INFO << "\tFUNCTOR ADDED: " << status->toString() << std::endl;

      /* Create the status value, which is the string name of the compile stage. */
      TuplePtr statusValue = Tuple::mk(VAL);
      statusValue->append(Val_Str::mk(_stageName));
      statusValue->freeze();

      /* Create the boolean expression 'statusVariable == _stageName' */
      TuplePtr boolExpr = Tuple::mk(BOOL);
      boolExpr->append(Val_Str::mk("=="));
      boolExpr->append(Val_Tuple::mk(statusVariable));
      boolExpr->append(Val_Tuple::mk(statusValue));

      /* Create the selection that will be applied after the status table probe*/
      TuplePtr select = Tuple::mk(SELECT, true);
      select->append((*rule)[TUPLE_ID]);
      select->append(Val_Tuple::mk(boolExpr)); // Boolean expression
      select->append(Val_UInt32::mk(3));       // Position right after status table
      select->append(Val_Null::mk());          // Access method
      select->freeze();
      selectTbl->insert(select); 
      TELL_INFO << "\tSELECTION ADDED: " << select->toString() << std::endl;

      rule = rule->clone();
      unsigned termCount = Val_UInt32::cast((*rule)[catalog->attribute(RULE, "TERM_COUNT")]);
      rule->set(catalog->attribute(RULE, "TERM_COUNT"), Val_UInt32::mk(termCount + 2));
      rule->freeze();
      catalog->table(RULE)->insert(rule);      
    }
  }
}
