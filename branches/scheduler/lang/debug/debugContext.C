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

#include "debugContext.h"
#include "plumber.h"
#include "systemTable.h"
#include "value.h"
#include "val_str.h"
#include "val_null.h"
#include "val_int64.h"
#include "val_list.h"
#include "val_tuple.h"
#include "oper.h"

namespace compile {
  namespace debug {
  using namespace opr;

    DEFINE_ELEMENT_INITS_NS(Context, "DebugContext", compile::debug)

    Context::Context(string name)
    : compile::Context(name) { }

    Context::Context(TuplePtr args)
    : compile::Context((*args)[2]->toString()) { }

    void
    Context::rule(CommonTable::ManagerPtr catalog, TuplePtr rule)
    {
      CommonTablePtr functorTbl = catalog->table(FUNCTOR);
      CommonTablePtr assignTbl  = catalog->table(ASSIGN);
      CommonTablePtr selectTbl  = catalog->table(SELECT);
  
      CommonTable::Key functorKey;
      CommonTable::Key assignKey;
      CommonTable::Key selectKey;

      functorKey.push_back(catalog->attribute(FUNCTOR, "RID"));
      functorKey.push_back(catalog->attribute(FUNCTOR, "POSITION"));

      assignKey.push_back(catalog->attribute(ASSIGN, "RID"));
      assignKey.push_back(catalog->attribute(ASSIGN, "POSITION"));

      selectKey.push_back(catalog->attribute(SELECT, "RID"));
      selectKey.push_back(catalog->attribute(SELECT, "POSITION"));

      ostringstream oss;
      if ((*rule)[catalog->attribute(RULE, "NAME")] != Val_Null::mk())
        oss << (*rule)[catalog->attribute(RULE, "NAME")]->toString() << " ";

      CommonTable::Iterator iter;
      for (int pos = 0; true; pos++) {
        TuplePtr lookup = Tuple::mk();
        lookup->append((*rule)[TUPLE_ID]);
        lookup->append(Val_Int64::mk(pos));
        lookup->freeze();
        TuplePtr lookup2 = Tuple::mk();
        lookup2->append((*rule)[TUPLE_ID]);
        lookup2->append(Val_Int64::mk(pos));
        lookup2->freeze();

        iter = functorTbl->lookup(CommonTable::theKey(CommonTable::KEY01), functorKey, lookup);
        if (iter->done())
          iter = functorTbl->lookup(CommonTable::theKey(CommonTable::KEY01), functorKey, lookup2);
        if (!iter->done()) {
          if (pos > 1) oss << ",\n\t";

          TuplePtr functor = iter->next();
          ListPtr  schema  = Val_List::cast((*functor)[catalog->attribute(FUNCTOR, "ATTRIBUTES")]);
          oss << (*functor)[catalog->attribute(FUNCTOR, "NAME")]->toString() << "(";
          for (ValPtrList::const_iterator siter = schema->begin(); 
               siter != schema->end(); ) {
            TuplePtr arg = Val_Tuple::cast(*siter);
            namestracker::exprString(&oss, arg);
            siter++;
            if (siter != schema->end()) oss << ", ";
          }
          oss << ")";
          if (pos == 0) {
            oss << " :-\n\t";
          }
          continue;
        }
        iter = assignTbl->lookup(CommonTable::theKey(CommonTable::KEY01), assignKey, lookup);
        if (iter->done())
          iter = assignTbl->lookup(CommonTable::theKey(CommonTable::KEY01), assignKey, lookup2);
        if (!iter->done()) {
          if (pos > 1) oss << ",\n\t";
          TuplePtr assign = iter->next();
          namestracker::exprString(&oss, Val_Tuple::cast((*assign)[catalog->attribute(ASSIGN, "VAR")])); 
          oss << " := ";
          namestracker::exprString(&oss, Val_Tuple::cast((*assign)[catalog->attribute(ASSIGN, "VALUE")])); 
          continue;
        }

        iter = selectTbl->lookup(CommonTable::theKey(CommonTable::KEY01), selectKey, lookup);
        if (iter->done())
          iter = selectTbl->lookup(CommonTable::theKey(CommonTable::KEY01), selectKey, lookup2);
        if (!iter->done()) {
          if (pos > 1) oss << ",\n\t";
          TuplePtr select = iter->next();
          namestracker::exprString(&oss, Val_Tuple::cast((*select)[catalog->attribute(SELECT, "BOOL")])); 

          continue;
        }

        oss << ".\n";
        TELL_OUTPUT << oss.str() << std::endl << std::endl;
        return;  // Done.
      }
    }
  }
}
