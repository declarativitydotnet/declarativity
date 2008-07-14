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
#include "oper.h"

#include <iostream>
#include <fstream>
#include <sys/wait.h>
#include <unistd.h>
#include "boost/bind.hpp"

#include <sys/stat.h>
#include <unistd.h>
#include <time.h>

using namespace opr;

#define LOAD2(name, prog) do {\
  ProgramPtr program(new Program((name), "", "", NULL, (prog))); \
  programs.push_back(program); \
} while (0);

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
  : Element(Val_Str::cast((*args)[2]), 0, 1), compileOnly(false), dotFile("")
{
  string source = P2_LANG_DIR;
  LOAD("gevent",     source + "/olg/gevent.olg",     "eca",     NULL);
  LOAD("stageGuard", source + "/olg/stageGuard.olg", "gevent",     NULL);
  LOAD("error",      source + "/olg/error.olg",      "parse",   NULL);
  LOAD("localize",   source + "/olg/localize.olg",   "error",   NULL);
  LOAD("seffect",    source + "/olg/seffect.olg",    "eca",     NULL);
  LOAD("aggview1",   source + "/olg/aggview1.olg",   "localize",   NULL);
  LOAD("aggview2",   source + "/olg/aggview2.olg",   "aggview1",NULL);
  LOAD("aggview3",   source + "/olg/aggview3.olg",   "aggview2",NULL);
  LOAD("mview",      source + "/olg/mview.olg",      "aggview3", NULL);
  LOAD("delta",      source + "/olg/delta.olg",      "mview",   NULL);
  LOAD("dummyWatch", source + "/olg/dummyWatch.olg", "gevent", NULL);

/*
  LOAD("sys", source + "/olg/sys.olg", "", NULL);
  LOAD("histogram", source + "/olg/histogram.olg", "parse", NULL);
  LOAD("wireless",  source + "/olg/swireless.olg", "histogram", NULL);

  LOAD("magic", source + "/olg/magic.olg", "parse", NULL);
  LOAD("stats", source + "/olg/stats.olg", "eca", NULL);
  LOAD("systemr", source + "/olg/systemr.olg", "stats", NULL);
*/
}

ProgramLoader::~ProgramLoader()
{
}

void
ProgramLoader::program(string name, string prog)
{
  LOAD2(name, prog);
  programIter = programs.begin();
}

void
ProgramLoader::program(string name,
                       string file,
                       string stage,
                       std::vector<std::string>* defs,
                       bool compileOnly) 
{
  LOAD(name, file, stage, defs);
  programIter = programs.begin();
  this->compileOnly = compileOnly;
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
        TELL_OUTPUT << "Dot file " << dotFile << " generated.\n";
      }
    }
    loader();
  }
  else if ((*program)[Plumber::catalog()->attribute(PROGRAM, "STATUS")]->toString() == "p2dl" &&
           ((*program)[Plumber::catalog()->attribute(PROGRAM, REWRITE)] != Val_Null::mk() ||
            compileOnly)) {
    ProgramPtr pdata = *(programIter-1);
    if (pdata->file == "") return;

    std::ofstream out((pdata->file + ".df").c_str());
    if ((*program)[Plumber::catalog()->attribute(PROGRAM, "P2DL")] != Val_Null::mk()) {
      ValuePtr programP2DL = (*program)[Plumber::catalog()->attribute(PROGRAM, "P2DL")];
      out << programP2DL->toString() << std::endl;
    }
    CommonTablePtr ruleTbl = Plumber::catalog()->table(RULE);
    CommonTable::Iterator Iter;
    for(Iter = ruleTbl->lookup(CommonTable::theKey(CommonTable::KEY2), 
                               CommonTable::theKey(CommonTable::KEY3), program);
        !Iter->done(); ) {
      TuplePtr rule = Iter->next();
      ValuePtr text = (*rule)[Plumber::catalog()->attribute(RULE, "P2DL")];
      out << text->toString() << std::endl;
    }
    out.close();
  }
}

void
ProgramLoader::loader()
{
  string filename;
  string name;
  string rewrite;
  std::vector<std::string>* defs = NULL;
  string programText = "";
  string p2dl = "";
  ProgramPtr program;

  if (programIter != programs.end())
  {
    program = *programIter++;
    filename = program->file;
    name     = program->name;
    rewrite  = program->stage;
    defs     = program->defs;
    programText = program->prog;
  }
  else if (compileOnly) {
    exit(0);
  }
  else return;

  const char* olgFile = program->file.c_str();
  ostringstream tmp;
  tmp << program->file << ".df";
  const char* dfFile  = tmp.str().c_str();

  struct stat olgAttrib;              // create a file attribute structure
  struct stat dfAttrib;               // create a file attribute structure
  /* Get the attributes of files (stat returns 0 on success).
   * The st_mtime field of stat is passed to difftime to return
   * the difference in modification times. */
  if (!stat(olgFile,&olgAttrib) && !stat(dfFile, &dfAttrib) &&
      difftime(dfAttrib.st_mtime, olgAttrib.st_mtime) > 0) {
    std::ifstream df (dfFile);
    if (df.is_open())
    {
      ostringstream text;
      string line;
      while (! df.eof() )
      {
        std::getline (df,line);
        text << line;
      }
      df.close();
      p2dl = text.str();
    }
  }

  // Preprocess and/or read in the program
  if (programText != "") {
    /* Do nothing. */
  }
  else if (defs) {
    programText = P2::preprocessReadOverLogProgram(filename,
                                                   filename,
                                                   *defs);
  } else {
    programText = P2::readOverLogProgram(filename);
  }

  CommonTable::ManagerPtr catalog = Plumber::catalog();
  string message = "";
  TuplePtr ptp = Tuple::mk(PROGRAM, true);
  ptp->append(Val_Str::mk(name));        // Program name
  if (rewrite == "")
    ptp->append(Val_Null::mk());         // No predecessor stage
  else
    ptp->append(Val_Str::mk(rewrite));   // Predecessor stage
  if (p2dl == "") {
    ptp->append(Val_Str::mk("compile")); // Program status
  }
  else {
    ptp->append(Val_Str::mk("planner")); // Program status
  }
  ptp->append(Val_Str::mk(programText)); // Program text
  ptp->append(Val_Null::mk());           // Program message
  if (p2dl == "") {
    ptp->append(Val_Null::mk());         // No P2DL for program installation
  }
  else {
    ptp->append(Val_Str::mk(p2dl));      // P2DL for program installation
  }
  ptp->append(catalog->nodeid());        // Source address
  ptp->freeze();
  
  output(0)->push(ptp, NULL);
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
