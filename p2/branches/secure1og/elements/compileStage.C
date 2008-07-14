// -*- c-basic-offset: 2; related-file-name: "print.h" -*-
/*
 * @(#)$Id$
 * 
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * Or
 * UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
 * Berkeley, CA,  94707. Attention: P2 Group.
 * 
 */

#include "compileStage.h"
#include "plumber.h"
#include "commonTable.h"
#include "systemTable.h"
#include "tuple.h"
#include "val_str.h"
#include "val_null.h"

DEFINE_ELEMENT_INITS(CompileStage, "CompileStage");

CompileStage::CompileStage(string name)
  : Element(name, 1, 1)
{
}

/**
 * Generic constructor.
 * Arguments:
 * 2. Val_Str: Name.
 * 3. Val_Str: Event Name.
 */
CompileStage::CompileStage(TuplePtr args)
  : Element(Val_Str::cast((*args)[2]), 1, 1)
{
}

CompileStage::~CompileStage()
{
}

TuplePtr CompileStage::simple_action(TuplePtr p)
{
  CommonTable::ManagerPtr catalog = Plumber::catalog();
  CommonTablePtr       programTbl = catalog->table(PROGRAM);
  CommonTablePtr       rewriteTbl = catalog->table(REWRITE);

  if ((*p)[catalog->attribute(PROGRAM, "STATUS")]->toString() == "error") {
    return TuplePtr(); 
  }

  CommonTable::Key indexKey;
  CommonTable::Key lookupKey;
  indexKey.push_back(catalog->attribute(REWRITE, "INPUT"));
  lookupKey.push_back(catalog->attribute(PROGRAM, "STATUS"));
  CommonTable::Iterator Iter = rewriteTbl->lookup(lookupKey, indexKey, p);
  if (Iter->done()) {
    ELEM_ERROR("Unknown program status! "
               << p->toString()
               << rewriteTbl->toString());
    return TuplePtr();
  }
  TuplePtr stage     = Iter->next();
  ValuePtr nextStage = (*stage)[catalog->attribute(REWRITE, "OUTPUT")];
  TuplePtr program = p->clone(nextStage->toString() + "::programEvent");
  program->set(catalog->attribute(PROGRAM, "STATUS"), nextStage);

  CommonTablePtr statusTbl = catalog->table(COMPILE_STATUS);
  Iter = statusTbl->lookup(CommonTable::theKey(CommonTable::KEY1), program);
  TuplePtr compileStatus;
  if (Iter->done()) {
    compileStatus = Tuple::mk(COMPILE_STATUS);
    compileStatus->append(nextStage);
    compileStatus->freeze();
  }
  else {
    compileStatus = Iter->next()->clone();
    compileStatus->set(catalog->attribute(COMPILE_STATUS, "STATUS"), nextStage);
    compileStatus->freeze();
  }
  statusTbl->insert(compileStatus);
  
  /** Check if we need to install the rewrite in the REWRITE table */
  if ((*program)[catalog->attribute(PROGRAM, REWRITE)] != Val_Null::mk() &&
      (*program)[catalog->attribute(PROGRAM, "STATUS")]->toString() == "installed") {
    indexKey.clear(); lookupKey.clear();
    indexKey.push_back(catalog->attribute(REWRITE, "INPUT"));
    lookupKey.push_back(catalog->attribute(PROGRAM, REWRITE));
    Iter = rewriteTbl->lookup(lookupKey, indexKey, program);
    if (Iter->done()) {
      ELEM_ERROR("Unknown rewrite input! "
                 << p->toString()
                 << ". "
                 << rewriteTbl->toString());
      return TuplePtr();
    }
    TuplePtr prevStage = Iter->next();
    TuplePtr copyStage = prevStage->clone(REWRITE, true);
    TuplePtr newStage  = Tuple::mk(REWRITE, true);
    newStage->append((*program)[catalog->attribute(PROGRAM, "NAME")]);
    newStage->append((*prevStage)[catalog->attribute(REWRITE, "OUTPUT")]);
    copyStage->set(catalog->attribute(REWRITE, "OUTPUT"), 
                   (*program)[catalog->attribute(PROGRAM, "NAME")]);
    newStage->freeze();
    prevStage->freeze();
    if (!rewriteTbl->insert(copyStage) || !rewriteTbl->insert(newStage) || 
        !rewriteTbl->remove(prevStage)) {
      ELEM_ERROR("Rewrite latice update error! Prev stage:"
                 << prevStage->toString() 
                 << ", New stage: "
                 << newStage->toString()
                 << ". "
                 << rewriteTbl->toString());
      return TuplePtr();
    }
    
    ELEM_INFO("NEW REWRITE TABLE: "
              << rewriteTbl->toString());
  }
  program->freeze();
  
  return program;
}

/* Set up the initial stages in the rewrite table */
int
CompileStage::initialize()
{
  CommonTable::ManagerPtr catalog = Plumber::catalog();
  CommonTablePtr       rewriteTbl = catalog->table(REWRITE);
  
  #define STAGE_INIT(input, output) do {\
    TuplePtr stage = Tuple::mk(REWRITE, true); \
    stage->append(Val_Str::mk(input)); \
    stage->append(Val_Str::mk(output)); \
    stage->freeze(); \
    rewriteTbl->insert(stage); \
  } while (0);

  STAGE_INIT("compile", "parse")
  STAGE_INIT("parse",   "eca")
  STAGE_INIT("eca",     "debug")
  STAGE_INIT("debug",   "rewrite")
  STAGE_INIT("rewrite", "local")
  STAGE_INIT("local",   "planner")
  STAGE_INIT("planner", "p2dl")
  STAGE_INIT("p2dl",    "installed")

  return 0;
}
