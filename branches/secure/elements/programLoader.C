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

#define LOAD(name, file, prev) do {\
  ProgramPtr program(new Program((name), (file), (prev))); \
  programs.push_back(program); \
} while (0);

DEFINE_ELEMENT_INITS(ProgramLoader, "ProgramLoader");

ProgramLoader::ProgramLoader(string name)
  : Element(name, 0, 1), terminal(true)
{
  // LOAD("stats", "/Users/tcondie/workspace/secure/doc/stats.olg", "eca");
  // LOAD("systemr", "/Users/tcondie/workspace/secure/doc/systemr.olg", "stats");
  // LOAD("localize", "/Users/tcondie/workspace/secure/doc/localize.olg", "systemr");
}

/**
 * Generic constructor.
 * Arguments:
 * 2. Val_Str: Name.
 * 3. Val_Str: Event Name.
 */
ProgramLoader::ProgramLoader(TuplePtr args)
  : Element(Val_Str::cast((*args)[2]), 0, 1), terminal(true)
{
/*
  LOAD("magic", "/Users/tcondie/workspace/secure/doc/magic.olg", "parse");
  LOAD("stats", "/Users/tcondie/workspace/secure/doc/stats.olg", "eca");
  LOAD("systemr", "/Users/tcondie/workspace/secure/doc/systemr.olg", "stats");
*/
  LOAD("localize", "/Users/tcondie/workspace/secure/doc/localize.olg", "eca");
}

ProgramLoader::~ProgramLoader()
{
}

void
ProgramLoader::program(string name, string file, string stage) 
{
  LOAD(name, file, stage);
  programIter = programs.begin();
}

void 
ProgramLoader::programUpdate(TuplePtr program)
{
  ELEM_INFO("Current program status: " 
            << (*program)[Plumber::catalog()->
                          attribute(PROGRAM, "STATUS")]->toString());

  if ((*program)[Plumber::catalog()->attribute(PROGRAM, "STATUS")]->toString() == "installed") {
    Plumber::toDot("compileTerminal.dot");
    ELEM_OUTPUT("Program successfully installed. "
                << "See compileTerminal.dot for dataflow description.");
    loader();
  }
}

void
ProgramLoader::loader()
{
  string filename = "";
  ostringstream text;
  string name;
  string rewrite = "n";

  if (programIter != programs.end())
  {
    ProgramPtr program = *programIter++;
    filename = program->file;
    name = program->name;
    rewrite = program->stage;
  }
  else if (terminal)
  {
    string more;

    std::cout << "filename? > ";
    std::cin >> filename;
    if (filename == "") return;
    std::cout << "Program name? > ";
    std::cin >> name;
    std::cout << "Rewrite stage? y/n> ";
    std::cin >> rewrite;
    if (rewrite.size() > 0 && (rewrite[0] == 'y' || rewrite[0] == 'Y')) {
      std::cout << "Rewrite stage predecessor name? > ";
      std::cin >> rewrite;
    }
    else {
      rewrite = "";
    }
    std::cout << "More inputs(y/n) ? > ";
    std::cin >> more;
    std::cout << std::endl;
    terminal = more == "Y" || more == "y";
  }
  else return;

  string processed(filename+".processed");

  // Run the OverLog through the preprocessor
  pid_t pid = fork();
  if (pid == -1) {
    std::cerr << "Cannot fork a preprocessor\n";
    exit(-1);
  } else if (pid == 0) {
    // I am the preprocessor
    execlp("cpp", "cpp", "-P", filename.c_str(), processed.c_str(),
           (char*) NULL);
    // If I'm here, I failed
    std::cerr << "Preprocessor execution failed" << std::endl;;
    exit(-1);
  } else {
    // I am the child
    wait(NULL);
  }
  // Parse the preprocessed file
  std::ifstream pstream(processed.c_str());
  text << pstream.rdbuf();
  unlink(processed.c_str());


  CommonTable::ManagerPtr catalog = Plumber::catalog();
  string message = "";
  TuplePtr program = Tuple::mk(PROGRAM, true);
  program->append(Val_Str::mk(name));        // Program name
  if (rewrite == "")
    program->append(Val_Null::mk());         // No predecessor stage
  else
    program->append(Val_Str::mk(rewrite));   // Predecessor stage
  program->append(Val_Str::mk("compile"));   // Program status
  program->append(Val_Str::mk(text.str()));  // Program text
  program->append(Val_Null::mk());           // Program message
  program->append(Val_Null::mk());           // P2DL for program installation
  program->freeze();
  output(0)->push(program, boost::bind(&ProgramLoader::terminal, this));
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
