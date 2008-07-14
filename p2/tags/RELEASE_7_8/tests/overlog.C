// -*- c-basic-offset: 2; related-file-name: "" -*-
/*
 * @(#)$Id$
 *
 *
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * Or
 * UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
 * Berkeley, CA,  94707. Attention: P2 Group.
 * 
 * DESCRIPTION: Overlog based on new planner
 *
 */

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "ol_lexer.h"
#include "ol_context.h"
#include "eca_context.h"
#include "localize_context.h"
#include "tableStore.h"
#include "udp.h"
#include "planner.h"
#include "ruleStrand.h"
#include "reporting.h"

#include <sys/wait.h>

#include "dot.h"

string
readScript(string overlog,
           string derivative,
           std::vector< std::string > definitions)
{
  string processed = derivative + ".cpp";
  

  // Turn definitions vector into a cpp argument array.
  int defSize = definitions.size();
  char* args[defSize
             + 1                // for cpp
             + 2                // for flags -C and -P
             + 2                // for filenames
             + 1];              // for the null pointer at the end

  int count = 0;

  args[count++] = "cpp";
  args[count++] = "-P";
  args[count++] = "-C";

  for (std::vector< std::string>::iterator i =
         definitions.begin();
       i != definitions.end();
       i++) {
    args[count] = (char*) (*i).c_str();
    count++;
  }

  args[count++] = (char*) overlog.c_str();
  args[count++] = (char*) processed.c_str();
  args[count++] = NULL;


  // Invoke the preprocessor
  pid_t pid = fork();
  if (pid == -1) {
    TELL_ERROR << "Cannot fork a preprocessor\n";
    exit(1);
  } else if (pid == 0) {
    if (execvp("cpp", args) < 0) {
      TELL_ERROR << "CPP ERROR" << std::endl;
    }
    exit(1);
  } else {
    wait(NULL);
  }


  // Read processed script.
  std::ifstream file;
  file.open(processed.c_str());

  if (!file.is_open()) {
    TELL_ERROR << "Cannot open processed Overlog file \""
               << processed << "\"!\n";
    return std::string();
  } else {

    std::ostringstream scriptStream;
    std::string line;
    
    while(std::getline(file, line)) {
      scriptStream << line << "\n";
    }

    file.close();
    std::string script = scriptStream.str();


    return script;
  }
}


/**
   My usage string
*/
static char* USAGE = "Usage:\n\t overlog\n"
                     "\t\t[-o <overLogFile> (default: standard input)]\n"
                     "\t\t[-r <loggingLevel> (default: ERROR)]\n"
                     "\t\t[-g (produce a DOT graph)]\n"
                     "\t\t[-c (output canonical form)]\n"
                     "\t\t[-v (show stages of planning)]\n"
                     "\t\t[-D<key>=<value>]*\n"
                     "\t\t[-h (gets usage help)]\n";

int
main(int argc, char** argv)
{
  string overLogFile("-");
  string derivativeFile("stdin");
  bool outputDot = false;
  bool outputCanonicalForm = false;
  bool outputStages = false;
  std::vector< std::string > definitions;



  // Parse command line options
  int c;
  while ((c = getopt(argc, argv, "o:r:gcD:hv")) != -1) {
    switch (c) {
    case 'o':
      overLogFile = optarg;
      if (overLogFile == "-") {
        derivativeFile = "stdin";
      } else {
        derivativeFile = overLogFile;
      }
      break;

    case 'r':
      {
        // My minimum reporting level is optarg
        std::string levelName(optarg);
        Reporting::Level level =
          Reporting::levelFromName()[levelName];
        Reporting::setLevel(level);
      }
      break;


    case 'g':
      outputDot = true;
      break;

    case 'c':
      outputCanonicalForm = true;
      break;

    case 'v':
      outputStages = true;
      break;

    case 'D':
      definitions.push_back(std::string("-D") + optarg);
      break;

    case 'h':
    default:
      TELL_ERROR << USAGE;
      exit(-1);
    }
  }      

  if (overLogFile == "-") {
    derivativeFile = "stdin";
  } else {
    derivativeFile = overLogFile;
  }
  TELL_INFO << "Running from translated file \""
            << overLogFile << "\"\n";

  TELL_INFO << "My environment is ";
  for (std::vector< std::string>::iterator i =
         definitions.begin();
       i != definitions.end();
       i++) {
    TELL_INFO << (*i) << " ";
  }
  TELL_INFO << "\n";

  // Preprocess and read in the program
  string program(readScript(overLogFile,
                            derivativeFile,
                            definitions));

  // Parse it
  boost::shared_ptr< OL_Context > ctxt(new OL_Context());
  std::istringstream istr(program);
  ctxt->parse_stream(&istr);
  if (ctxt->gotErrors()) {
    TELL_ERROR << "Parse Errors Found\n";
    ctxt->dumpErrors();
    TELL_ERROR << "Compilation aborted\n";
    exit (-1);
  }
  

  TELL_INFO << "Finished parsing. Functors: "
	    << ctxt->getRules()->size() 
            << ". TableRecords: "
            << ctxt->getTableInfos()->size()
            << "\n";

  if (outputCanonicalForm) {
    TELL_OUTPUT << "Canonical Form:\n";
    TELL_OUTPUT << ctxt->toString() << "\n";
  } 





  // Start the planning
  PlumberPtr plumber(new Plumber());

  Plumber::DataflowPtr conf(new Plumber::Dataflow("overlog"));
  boost::shared_ptr< TableStore >
    tableStore(new TableStore(ctxt.get()));
  tableStore->initTables();
  
  boost::shared_ptr< Localize_Context >
    lctxt(new Localize_Context());
  lctxt->rewrite(ctxt.get(), tableStore.get());
  if (outputStages) {
    std::ofstream localizedStream((derivativeFile + ".localized").c_str());
    localizedStream << lctxt->toString();
    localizedStream.close();
  }
  
  boost::shared_ptr< ECA_Context > ectxt(new ECA_Context());
  ectxt->rewrite(lctxt.get(), tableStore.get());
  if (outputStages) {
    std::ofstream ecaStream((derivativeFile + ".eca").c_str());
    ecaStream << ectxt->toString();
    ecaStream.close();
  }

  boost::shared_ptr< Planner >
    planner(new Planner(conf,
                        tableStore.get(),
                        false,
                        "127.0.0.1:10000"));
  
  
  
  boost::shared_ptr< Udp > udp(new Udp("Udp", 12345));
  std::vector< RuleStrand* > ruleStrands =
    planner->generateRuleStrands(ectxt);
  if (outputStages) {
    std::ofstream ruleStrandStream((derivativeFile + ".ruleStrand").c_str());
    for (unsigned k = 0; k < ruleStrands.size(); k++) {
      ruleStrandStream << ruleStrands.at(k)->toString();
    }
    ruleStrandStream.close();
  }
  
  planner->setupNetwork(udp);
  planner->registerAllRuleStrands(ruleStrands);
  
  if (outputDot) {
    if (plumber->install(conf) == 0) {
      TELL_INFO
        << "Correctly initialized network of reachability flows.\n";
      plumber->toDot(derivativeFile + ".dot");
    } else {
      TELL_ERROR << "** Failed to initialize correct spec\n";
    }
  }

  return 0;
}
