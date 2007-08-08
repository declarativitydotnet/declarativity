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

#include "compileTerminal.h"
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

DEFINE_ELEMENT_INITS(CompileTerminal, "CompileTerminal");

CompileTerminal::CompileTerminal(string name)
  : Element(name, 0, 1)
{
  initDefaultStages();
}

/**
 * Generic constructor.
 * Arguments:
 * 2. Val_Str: Name.
 * 3. Val_Str: Event Name.
 */
CompileTerminal::CompileTerminal(TuplePtr args)
  : Element(Val_Str::cast((*args)[2]), 0, 1)
{
  initDefaultStages();
}

CompileTerminal::~CompileTerminal()
{
  delete[] defaultStages;
}


void 
CompileTerminal::programUpdate(TuplePtr program)
{
  if ((*program)[Plumber::catalog()->attribute(PROGRAM, "STATUS")]->toString() == "installed") {
    Plumber::toDot("compileTerminal.dot");
    ELEM_OUTPUT("Program successfully installed. "
                << "See compileTerminal.dot for dataflow description.");

  if (more.size() > 0 && (more[0] == 'y' || more[0] == 'Y')) {
    terminal();
    //    delayCB(1, boost::bind(&CompileTerminal::terminal, this), this);
  }
  else {
    // do nothing: no more call backs needed 
  }
    
    //    if (!counter) delayCB(1, boost::bind(&CompileTerminal::terminal, this), this);
    //    counter++;
  }
  else {
    ELEM_INFO("Current program status: " 
              << (*program)[Plumber::catalog()->
                            attribute(PROGRAM, "STATUS")]->toString());
  }
}

void
CompileTerminal::terminal()
{
  static int counter = 0;
  string filename;
  ostringstream text;
  string name;
  string rewrite = "n";

  if(numDefaultStages <= counter)
  {
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
  }
  else
  {
    filename = defaultStages[counter].file;
    name = defaultStages[counter].name;
    rewrite = defaultStages[counter].prevStageName;
    more = "y";
  }
  counter++;

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
  output(0)->push(program, boost::bind(&CompileTerminal::terminal, this));

}

/* Set up the initial stages in the rewrite table */
int
CompileTerminal::initialize()
{
  CommonTablePtr programTbl = Plumber::catalog()->table(PROGRAM);
  programTbl->updateListener(boost::bind(&CompileTerminal::programUpdate, this, _1));
  delayCB(0, boost::bind(&CompileTerminal::terminal, this), this);

  return 0;
}
