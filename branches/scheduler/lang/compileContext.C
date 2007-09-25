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

#include "compileContext.h"
#include "plumber.h"
#include "systemTable.h"
#include "val_list.h"
#include "val_str.h"
#include "val_int64.h"
#include "val_tuple.h"

namespace compile {

  TuplePtr 
  Context::simple_action(TuplePtr p)
  {
    CommonTable::ManagerPtr catalog = Plumber::catalog();
    if ((*p)[catalog->attribute(PROGRAM, "STATUS")]->toString() == name())
      try {
        p = program(catalog, p);
      }
      catch (compile::Exception e) {
        TELL_ERROR << "Compile Exception: Program "
                   << (*p)[catalog->attribute(PROGRAM, "NAME")]->toString() << std::endl
                   << "\t" << e.toString() << std::endl;
        p = p->clone();
        p->set(catalog->attribute(PROGRAM, "STATUS"), Val_Str::mk("error"));
        p->freeze();
      }
      catch (Element::Exception e) {
        TELL_ERROR << "Element Exception: Program " 
                   << (*p)[catalog->attribute(PROGRAM, "NAME")]->toString() << std::endl
                   << "\t" << e.toString() << std::endl;
        p = p->clone();
        p->set(catalog->attribute(PROGRAM, "STATUS"), Val_Str::mk("error"));
        p->freeze();
      }
    return p;
  }

  TuplePtr 
  Context::program(CommonTable::ManagerPtr catalog, TuplePtr program) 
  {
    TELL_INFO << "PROGRAM TUPLE ID: " << (*program)[TUPLE_ID]->toString() << std::endl;
    TuplePtr ruleTp;
    try {
      CommonTablePtr ruleTbl = catalog->table(RULE);
      CommonTable::Iterator rIter;
      for(rIter = ruleTbl->lookup(CommonTable::theKey(CommonTable::KEY2), 
                                  CommonTable::theKey(CommonTable::KEY3), program);
          !rIter->done(); ) {
        ruleTp = rIter->next();
        rule(catalog, ruleTp);
      }
    }
    catch (compile::Exception e) {
      TELL_ERROR << "Compile Exception: Program " 
                 << (*program)[catalog->attribute(PROGRAM, "NAME")]->toString() << std::endl
                 << "\t" << e.toString() << std::endl
                 << "Rule " << (*ruleTp)[catalog->attribute(RULE, "NAME")]->toString() << std::endl; 
    }
    catch (Element::Exception e) {
      TELL_ERROR << "Element Exception: Program " 
                 << (*program)[catalog->attribute(PROGRAM, "NAME")]->toString() << std::endl
                 << "\t" << e.toString() << std::endl
                 << "Rule " << (*ruleTp)[catalog->attribute(RULE, "NAME")]->toString() << std::endl; 
   
    }
    program = program->clone(PROGRAM);
    program->freeze();
    return program;
  }

  void 
  Context::rule(CommonTable::ManagerPtr catalog, TuplePtr rule)
  {
    throw compile::Exception("Unknown rule rewrite.");
  }

};
