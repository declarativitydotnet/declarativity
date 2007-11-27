/*
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * Or
 * UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
 * Berkeley, CA,  94707. Attention: P2 Group.
 * 
 */

#include "programLoader.h"
#include "p2.h"
#include "plumber.h"
#include "loop.h"
#include "commonTable.h"
#include "systemTable.h"
#include "tuple.h"
#include "val_str.h"
#include "val_null.h"
#include "val_tuple.h"

#include <iostream>
#include <sys/wait.h>
#include <unistd.h>
#include "boost/bind.hpp"

#define LOAD(name, file, prev, defs) do {\
  ProgramPtr program(new Program((name), (file), (prev), (defs))); \
  programs.push_back(program); \
} while (0);

DEFINE_ELEMENT_INITS(ProgramLoader, "ProgramLoader");

ProgramLoader::ProgramLoader(string name)
  : Element(name, 0, 1), dotFile("")
{
}

/**
 * Generic constructor.
 * Arguments:
 * 2. Val_Str: Name.
 * 3. Val_Str: Event Name.
 */
ProgramLoader::ProgramLoader(TuplePtr args)
  : Element(Val_Str::cast((*args)[2]), 0, 1), dotFile("")
{
  // LOAD("magic", "/Users/tcondie/workspace/secure/doc/magic.olg", "parse", NULL);

  LOAD("gevent", "/Users/tcondie/workspace/secure/doc/gevent.olg", "eca", NULL);
  LOAD("saffect", "/Users/tcondie/workspace/secure/doc/saffect.olg", "parse", NULL);
  LOAD("mview", "/Users/tcondie/workspace/secure/doc/mview.olg", "saffect", NULL);
  LOAD("localize", "/Users/tcondie/workspace/secure/doc/localize.olg", "saffect", NULL);

/*
  LOAD("stats", "/Users/tcondie/workspace/secure/doc/stats.olg", "eca", NULL);
  LOAD("systemr", "/Users/tcondie/workspace/secure/doc/systemr.olg", "stats", NULL);
*/
}

ProgramLoader::~ProgramLoader()
{
}

void
ProgramLoader::program(string name, string file, string stage, std::vector<std::string>* defs) 
{
  LOAD(name, file, stage, defs);
  programIter = programs.begin();
}

void
ProgramLoader::dot(string name) 
{
  dotFile = name;
}

void 
ProgramLoader::programUpdate(TuplePtr program)
{
  ELEM_INFO("Current program status: " 
            << (*program)[Plumber::catalog()->
                          attribute(PROGRAM, "STATUS")]->toString());

  if ((*program)[Plumber::catalog()->attribute(PROGRAM, "STATUS")]->toString() == "installed") {
    if ((*program)[Plumber::catalog()->attribute(PROGRAM, REWRITE)] == Val_Null::mk()) {
      string name = (*program)[Plumber::catalog()->attribute(PROGRAM, "NAME")]->toString();
      TELL_OUTPUT << "Program " << name << " installed.";
      if (dotFile != "") {
        Plumber::toDot(dotFile);
        TELL_OUTPUT << "Dot file " << dotFile << " generated.";
      }
    }
    loader();
  }
}

void
ProgramLoader::loader()
{
  string filename;
  string name;
  string rewrite;
  std::vector<std::string>* defs;

  if (programIter != programs.end())
  {
    ProgramPtr program = *programIter++;
    filename = program->file;
    name     = program->name;
    rewrite  = program->stage;
    defs     = program->defs;
  }
  else return;

  // Preprocess and/or read in the program
  string programText;
  if (defs) {
    programText = P2::preprocessReadOverLogProgram(filename,
                                                   filename,
                                                   *defs);
  } else {
    programText = P2::readOverLogProgram(filename);
  }

  CommonTable::ManagerPtr catalog = Plumber::catalog();
  string message = "";
  TuplePtr program = Tuple::mk(PROGRAM, true);
  program->append(Val_Str::mk(name));        // Program name
  if (rewrite == "")
    program->append(Val_Null::mk());         // No predecessor stage
  else
    program->append(Val_Str::mk(rewrite));   // Predecessor stage
  program->append(Val_Str::mk("compile"));   // Program status
  program->append(Val_Str::mk(programText)); // Program text
  program->append(Val_Null::mk());           // Program message
  program->append(Val_Null::mk());           // P2DL for program installation
  program->freeze();
  output(0)->push(program, NULL);
}

/* Set up the initial stages in the rewrite table */
int
ProgramLoader::initialize()
{
  CommonTablePtr programTbl = Plumber::catalog()->table(PROGRAM);
  programTbl->updateListener(boost::bind(&ProgramLoader::programUpdate, this, _1));
  delayCB(1, boost::bind(&ProgramLoader::loader, this), this);

  programIter = programs.begin();
  return 0;
}
