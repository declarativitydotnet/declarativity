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
#include "val_tuple.h"
#include "val_int64.h"

namespace compile {
  SetPtr Context::materializedTables(new Set());
  LocSpecMap* Context::ruleLocSpecMap = new LocSpecMap();

  Context::Context(string name) 
    : Element(name, 1, 1) 
  {
  }

  TuplePtr 
  Context::simple_action(TuplePtr p)
  {
    CommonTable::ManagerPtr catalog = Plumber::catalog();
    if ((*p)[catalog->attribute(PROGRAM, "STATUS")]->toString() == name())
      try {
        p = program(catalog, p);
      }
      catch (compile::Exception e) {
        this->error(catalog, p, 0, e.toString());
        return TuplePtr();
      }
      catch (Element::Exception e) {
        this->error(catalog, p, 0, e.toString());
        return TuplePtr();
      }
    return p;
  }

  TuplePtr 
  Context::program(CommonTable::ManagerPtr catalog, TuplePtr program) 
  {
    TELL_INFO << "PROGRAM TUPLE ID: " << (*program)[TUPLE_ID]->toString() << std::endl;
    TuplePtr ruleTp;
    CommonTablePtr ruleTbl = catalog->table(RULE);
    CommonTable::Iterator rIter;
    for(rIter = ruleTbl->lookup(CommonTable::theKey(CommonTable::KEY2), 
                                CommonTable::theKey(CommonTable::KEY3), program);
        !rIter->done(); ) {
      ruleTp = rIter->next();
      rule(catalog, ruleTp);
    }
    program = program->clone(PROGRAM_STREAM);
    program->freeze();
    return program;
  }

  void 
  Context::rule(CommonTable::ManagerPtr catalog, TuplePtr rule)
  {
    throw compile::Exception("Unknown rule rewrite.");
  }

  int Context::initialize()
  {
    TELL_INFO << "Default compile stage: " << name() << " loaded." << std::endl;
    return true;
  }

  void
  Context::error(CommonTable::ManagerPtr catalog, TuplePtr program, int code, string desc) 
  {
    TuplePtr errorTp = Tuple::mk();
    errorTp->append(Val_Str::mk(ERROR_STREAM));
    errorTp->append((*program)[catalog->attribute(PROGRAM, "SOURCE")]);
    errorTp->append((*program)[catalog->attribute(PROGRAM, "PID")]);
    errorTp->append((*program)[catalog->attribute(PROGRAM, "NAME")]);
    errorTp->append(catalog->nodeid());
    errorTp->append(Val_Int64::mk(code));
    errorTp->append(Val_Str::mk(desc));
    errorTp->freeze();
    catalog->table(PERROR)->insert(errorTp);
  } 

};
