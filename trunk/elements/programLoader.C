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
  string source = P2_SOURCE_DIR;
  LOAD("gevent",     source + "/lang/olg/gevent.olg",     "eca",     NULL);
  LOAD("stageGuard", source + "/lang/olg/stageGuard.olg", "eca",     NULL);
  LOAD("error",      source + "/lang/olg/error.olg",      "parse",   NULL);
  LOAD("seffect",    source + "/lang/olg/seffect.olg",    "eca",     NULL);
  LOAD("aggview1",   source + "/lang/olg/aggview1.olg",   "error",   NULL);
  LOAD("aggview2",   source + "/lang/olg/aggview2.olg",   "aggview1",NULL);
  LOAD("aggview3",   source + "/lang/olg/aggview3.olg",   "aggview2",NULL);
  LOAD("mview",      source + "/lang/olg/mview.olg",      "aggview3",NULL);
  LOAD("delta",      source + "/lang/olg/delta.olg",      "mview",   NULL);
  LOAD("localize",   source + "/lang/olg/localize.olg",   "aggview3",NULL);

/*
  LOAD("magic", source + "/lang/olg/magic.olg", "parse", NULL);
  LOAD("stats", source + "/lang/olg/stats.olg", "eca", NULL);
  LOAD("systemr", source + "/lang/olg/systemr.olg", "stats", NULL);
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
      TELL_OUTPUT << "Program " << name << " installed.\n";
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
  program->append(catalog->nodeid());        // Source address
  program->freeze();
  output(0)->push(program, NULL);
}

/* Set up the initial stages in the rewrite table */
int
ProgramLoader::initialize()
{
  CommonTablePtr programTbl = Plumber::catalog()->table(PROGRAM);
  programTbl->updateListener(boost::bind(&ProgramLoader::programUpdate, this, _1));
  delayCB(0, boost::bind(&ProgramLoader::loader, this), this);

  programIter = programs.begin();
  return 0;
}
