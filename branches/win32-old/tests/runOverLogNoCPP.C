// -*- c-basic-offset: 2; related-file-name: "" -*-
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
 * DESCRIPTION: A runner of arbitrary OverLog code.  It only
 * fills in the env table with the local host.
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */
#include <iostream>
#include <stdlib.h>
#include <sys/types.h>
// #include <sys/wait.h>

#include "tuple.h"
#include "plumber.h"
#include "val_int32.h"
#include "val_uint32.h"
#include "val_str.h"
#include "val_id.h"

#include "ol_lexer.h"
#include "ol_context.h"
#include "eca_context.h"
#include "localize_context.h"
#include "tableStore.h"
#include "udp.h"
#include "planner.h"
#include "ruleStrand.h"
#include "stageStrand.h"


#include "table2.h"
#include "commonTable.h"
#include "dot.h"

#include "stageLoader.h"

#include "getopt.h"

bool DEBUG = false;
bool CC = false;





/** Load any loadable modules */
void
loadAllModules()
{
  // Stages
  StageLoader::loadStages();
}



/**
   Put together the dataflow.
*/
void
startOverLogDataflow(boost::shared_ptr< OL_Context> ctxt,
                     string overLogFile, 
                     string derivativeFile,
                     string localAddress,
                     bool outputStages,
                     bool outputDot,
                     bool run,
                     int port,
                     double delay)
{
  eventLoopInitialize();
  // create dataflow for translated OverLog

  PlumberPtr plumber(new Plumber());
  Plumber::DataflowPtr conf(new Plumber::Dataflow("test"));
  boost::shared_ptr< TableStore > tableStore(new TableStore(ctxt.get()));  
  tableStore->initTables(); 
  
  boost::shared_ptr< Localize_Context > lctxt(new Localize_Context()); 
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
  
  boost::shared_ptr< Planner > planner(new Planner(conf,
                                                   tableStore.get(),
                                                   false, 
						   localAddress));
  boost::shared_ptr< Udp > udp(new Udp("Udp", port));

  std::vector<StageStrand*> stageStrands =
    planner->generateStageStrands(ctxt.get());

  std::vector<RuleStrand*> ruleStrands =
    planner->generateRuleStrands(ectxt);
  if (outputStages) {
    std::ofstream ruleStrandStream((derivativeFile + ".ruleStrand").c_str());
    for (unsigned k = 0; k < ruleStrands.size(); k++) {
      ruleStrandStream << ruleStrands.at(k)->toString();
    }
    ruleStrandStream.close();
  }
  
  for (unsigned k = 0; k < ruleStrands.size(); k++) {
    TELL_INFO << ruleStrands.at(k)->toString();
  }
  planner->setupNetwork(udp);
  planner->registerAllRuleStrands(ruleStrands, stageStrands);
  TELL_INFO << planner->getNetPlanner()->toString() << "\n";

  if (plumber->install(conf) == 0) {
    TELL_INFO << "Correctly initialized dataflow.\n";
    if (outputDot) {
      plumber->toDot(derivativeFile + ".dot");
    }
  } else {
    TELL_ERROR << "** Failed to initialize correct spec\n";
    return;
  }

  // Run the plumber
  if (run) {
    eventLoop();
  }
}


string
readScript(string overlog,
           string derivative,
           std::vector< std::string > definitions)
{
  string processed = derivative + ".cpp";
  

  // Turn definitions vector into a cpp argument array.
  int defSize = definitions.size();
  char ** args = new (char * [defSize
             + 1                // for cpp
             + 2                // for flags -C and -P
             + 2                // for filenames
             + 1]);              // for the null pointer at the end

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



  /** Ignore CPP. This version of the program is only to be run with
      pre-processed files 
      
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
  */


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
	delete args;

    return script;
  }
}

/**
 * Builds a dataflow graph from an OverLog specification
 */
void testOverLog(string myAddress,
                 int port,    // extracted from myAddress for convenience
                 string filename,
                 string derivativeFile,
                 double delay,
                 std::string program,
                 bool outputCanonicalForm,
                 bool outputStages,
                 bool outputDot,
                 bool run)
{
  boost::shared_ptr< OL_Context > ctxt(new OL_Context());

  // Parse the preprocessed file
  std::istringstream istr(program);
  ctxt->parse_stream(&istr);
  if (ctxt->gotErrors()) {
    TELL_ERROR << "Parse Errors Found\n";
    ctxt->dumpErrors();
    TELL_ERROR << "Compilation aborted\n";
    exit (-1);
  }

  if (outputCanonicalForm) {
    TELL_OUTPUT << "Canonical Form:\n";
    TELL_OUTPUT << ctxt->toString() << "\n";
  } 

  loadAllModules();

  startOverLogDataflow(ctxt, filename, derivativeFile,
                       myAddress,
                       outputStages, outputDot, run,
                       port, delay);
}






/**
   My usage string
*/
static char* USAGE = "Usage:\n\t runOverLog\n"
                     "\t\t[-o <overLogFile> (default: standard input)]\n"
                     "\t\t[-r <loggingLevel> (default: ERROR)]\n"
                     "\t\t[-s <seed> (default: 0)]\n"
                     "\t\t[-n <myipaddr> (default: localhost)]\n"
                     "\t\t[-p <port> (default: 10000)]\n"
                     "\t\t[-d <startDelay> (default: 0)]\n"
                     "\t\t[-g (produce a DOT graph)]\n"
                     "\t\t[-c (output canonical form)]\n"
                     "\t\t[-v (show stages of planning)]\n"
                     "\t\t[-x (dry run, don't start dataflow)]\n"
                     "\t\t[-D<key>=<value>]*\n"
                     "\t\t[-h (gets usage help)]\n";


int
main(int argc,
     char** argv)
{
  string overLogFile("-");
  string derivativeFile("stdin");
  int seed = 0;
  string myHostname = "localhost";
  int port = 10000;
  double delay = 0.0;
  std::vector< std::string > definitions;
  bool outputDot = false;
  bool run = true;
  bool outputCanonicalForm = false;
  bool outputStages = false;

  // Parse command line options
  int c;
  while ((c = getopt(argc, argv, "o:r:s:n:p:d:D:hgcvx")) != -1) {
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

    case 'x':
      run = false;
      break;

    case 'v':
      outputStages = true;
      break;

    case 's':
      seed = atoi(optarg);
      break;

    case 'n':
      myHostname = optarg;
      break;

    case 'p':
      port = atoi(optarg);
      break;

    case 'd':
      delay = atof(optarg);
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
  
  TELL_INFO << "Running from translated file \"" << overLogFile << "\"\n";

  srandom(seed);
  TELL_INFO << "Seed is \"" << seed << "\"\n";

  std::ostringstream myAddressBuf;
  myAddressBuf <<  myHostname << ":" << port;
  std::string myAddress = myAddressBuf.str();
  TELL_INFO << "My address is \"" << myAddress << "\"\n";
  
  TELL_INFO << "My start delay is " << delay << "\n";

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

  testOverLog(myAddress,
              port,
              overLogFile, 
              derivativeFile,
              delay,
              program,
              outputCanonicalForm,
              outputStages,
              outputDot,
              run);
  return 0;
}

/**
 * End of File
 */
