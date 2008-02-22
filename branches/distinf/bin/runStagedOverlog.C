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
#include <sys/wait.h>

#include "p2dlContext.h"
#include "p2.h"
#include "commonTable.h"
#include "tableManager.h"
#include "systemTable.h"
#include "loop.h"
#include "reporting.h"
#include "elementLoader.h"
#include "netLoader.h"
#include "stageLoader.h"
#include "aggFuncLoader.h"
#include "langLoader.h"
#include "functionLoader.h"
#include "programLoader.h"
#include "compileContext.h"

void seaPlan(ostringstream& oss, string hostname, string port)
{
  /**input subgraph: starts with a DRR for ext events
   * ends with a PP+Demux towards the inner rule strands
   * */
  oss << "\tgraph seaInput(1,1,\"l/h\",\"/\") { \n"
      << "\t\tinput -> PullPush(\"ExtDRRPP\",100) -> Queue(\"extQ\",1000,\"external\") -> "
      << "Switch(\"ExtQGateSwitch\",1,true) -> output;\n"
      << "\t};\n\n"; 
}

void netPlan(ostringstream& stub, string hostname, 
	       string port, TransportConf conf)
{
  stub << "\tgraph netIn (1,1,\"l/l\",\"/\"){\n"
       << "\t\tinput ->\n"
       << "\t\tUnmarshalField(\"unmarshal\", 1) ->\n"
       << "\t\tPelTransform(\"unRoute\", \"$1 unboxPop\") ->\n"
       << "\t\tDefrag(\"defrag\") -> PullPush(\"defrag_pull\", 0) ->\n"
       << "\t\tPelTransform(\"unPackage\", \"$0 pop $8 pop\") -> Queue(\"netQ\", 1000) -> output;\n"
       << "\t}; /**END OF NETIN SUBGRAPH*/\n\n";

  stub << "\tgraph netOut(1,1,\"h/h\",\"/\") {\n"
       << "\t\tnetOutQ = Queue(\"netout_queue\",0);\n";
  stub << "\t\theader  = PelTransform(\"source\", \"$0 pop \\\""
                                    << hostname <<":" << port
                                    << "\\\" pop swallow unbox drop "
                                    << "0 pop 0 pop 0 pop 0 pop 0 pop 0 pop popall\");\n";

  stub << "\t\tinput -> netOutQ -> header -> Sequence(\"terminal_sequence\", 1) ->\n"
       << "\t\tPullPush(\"ppfrag\", 0) -> Frag(\"fragment\") ->\n"
       << "\t\tPelTransform(\"package\", \"$0 pop swallow pop\") ->\n"
       << "\t\tMarshalField(\"marshalField\", 1) -> StrToSockaddr(\"addr_conv\", 0) -> output;\n"
       << "\t};/**END NETOUT*/\n\n"; 
}

string stub(string hostname, string port, TransportConf conf, bool tupleCountAndBandwidth)
{
  ostringstream stub;

  stub << "graph main(0, 0, \"/\", \"/\"){\n";

  netPlan(stub,hostname,port,conf);
  seaPlan(stub,hostname,port);

  string netOut_udp_string = "\tnetOut -> udp;\n";
  if (tupleCountAndBandwidth) {
	  netOut_udp_string = "\tnetOut -> Bandwidth(\"bandwidth\") -> TupleCounter(\"counter\", \"anytype\") -> udp;\n";
  }
	  
	  //UDP element for netin/netout
  stub << "\tudp = Udp2(\"udp\","<<port<<");\n"
          //ExtDRR for external strands to hookup to
       << "\textDRR = DRoundRobin(\"extDRR\",0);\n"
          //Internal demux for internal strands to hook to
       << "\tintDemux = DDemux(\"intDemux\", {}, 0);\n"
          //Feed to internal from local & external queues
       <<"\tintQMux = Mux(\"IntQMUX\",2);\n"
          //output hookup point
       <<"\tintDRR = DRoundRobin(\"intDRR\",0);\n"
          //internal/external demux
       <<"\tintExtDemux = Demux(\"intExtDemux\", {\""<<hostname<<":"<<port<<"\"}, 0);\n"
	  //get netin/out OK
       << "\tudp -> netIn; /* Connect UDP to net input */\n"
       << netOut_udp_string
       << "\tnetIn -> [+]extDRR;\n"
       << "\tUpdate(\"programUpdate\",\"" << PROGRAM << "\") -> "
       << "PelTransform(\"packageUpdate\", \"$1 pop swallow pop\") -> [+]extDRR;\n"
          //hook extDRR to seaInput, then to intQMux
       << "\textDRR->seaInput;\n"
          //start internal event processing strand
	  //in theory, internal queue must be made infinite to avoid deadlock
 	  //to be absolutely sure.
       << "\tseaInput -> [0]intQMux -> PelTransform(\"unpackage\", \"$1 unboxPop\") -> "
       << "\tQueue(\"intQ\", 100000,\"internal\") -> PullPush(\"IntQPP\",0) -> "
       << "\tintDemux[0] -> "
       << "Discard(\"discard\");\n"
          //start the rule output process
       << "\tintDRR -> PullPush(\"SEAOutputPP\",0) -> intExtDemux;\n"
	  //External events to commitbuf, to netOut
       << "\tintExtDemux[1] -> CommitBuf(\"NetCommitBuf\") -> netOut;\n"
          //internal events to internal mux
       << "\tintExtDemux[0] -> [1]intQMux;\n";

  /* Connect the default compiler stages */
  stub << "\tintDemux[+ \"" << PROGRAM << "\"] -> "
       << "CompileStage(\"compileStage\") -> "
       << "PelTransform(\"package\", \"\\\"" << hostname << ":" << port << "\\\" pop swallow pop\") -> " 
       << "Queue(\"csQ\", 10) -> [+]extDRR;\n";

  stub << "\tintDemux[+ \"" << ERROR_STREAM << "\"] -> "
       << "PelTransform(\"errorStreamPel\",  \"\\\"" << PERROR << "\\\" pop swallow unbox drop popall\") -> " 
       << "\tInsert2(\"errorStreamAdd\", \"" << PERROR << "\");\n";

  stub << "\tintDemux[+ \"" << PROGRAM_STREAM << "\"] -> "
       << "PelTransform(\"programStreamPel\",  \"\\\"" << PROGRAM << "\\\" pop swallow unbox drop popall\") -> " 
       << "\tInsert2(\"programStreamAdd\", \"" << PROGRAM << "\");\n";


  stub << "\tintDemux[+ \"parse::programEvent\"] -> ParseContext(\"parse\") -> "
       << "PelTransform(\"package\", \"\\\"" << hostname << ":" << port << "\\\" pop swallow pop\") -> " 
       << "Queue(\"csQ\", 10) -> [+]extDRR;\n";

  stub << "\tintDemux[+ \"eca::programEvent\"]   -> EcaContext(\"eca\") -> "
       << "PelTransform(\"package\", \"\\\"" << hostname << ":" << port << "\\\" pop swallow pop\") -> " 
       << "Queue(\"csQ\", 10) -> [+]extDRR;\n";

  stub << "\tintDemux[+ \"debug::programEvent\"] -> DebugContext(\"debug\") -> "
       << "PelTransform(\"package\", \"\\\"" << hostname << ":" << port << "\\\" pop swallow pop\") -> " 
       << "Queue(\"csQ\", 10) -> [+]extDRR;\n";

  stub << "\tintDemux[+ \"planner::programEvent\"] -> "
       << "PlannerContext(\"planner\", \"main\", \"intDemux\", \"intDRR\", \"extDRR\") -> "
       << "PelTransform(\"package\", \"\\\"" << hostname << ":" << port << "\\\" pop swallow pop\") -> " 
       << "Queue(\"csQ\", 10) -> [+]extDRR;\n";

  stub << "\tintDemux[+ \"p2dl::programEvent\"] -> P2DLContext(\"p2dl\") -> "
       << "PelTransform(\"package\", \"\\\"" << hostname << ":" << port << "\\\" pop swallow pop\") -> " 
       << "Queue(\"csQ\", 10) -> [+]extDRR;\n";

  stub << "\tintDemux[+ \"installed::programEvent\"] -> "
       << "PelTransform(\"package\", \"\\\"" << hostname << ":" << port << "\\\" pop swallow pop\") -> " 
       << "Queue(\"csQ\", 10) -> [+]extDRR;\n";

  if(conf & TERMINAL) {
    stub << "\t\tProgramLoader(\"loader\") -> Insert2(\"ctInsert\", \"" << PROGRAM << "\");\n";
  }
  stub << "};/**END MAIN*/\n\n";

  return stub.str();
}

/** Load any loadable modules */
void
loadAllModules()
{
  // Stages
  ElementLoader::loadElements();
  NetLoader::loadElements();
  LangLoader::loadElements();
  StageLoader::loadStages();
  AggFuncLoader::loadAggFunctions();
  FunctionLoader::loadFunctions();
}


/**
   My usage string
*/
static char* USAGE = "Usage:\n\t runStagedOverLog\n"
                     "\t\t[-r <loggingLevel> (default: ERROR)]\n"
                     "\t\t[-n <myipaddr> (default: localhost)]\n"
                     "\t\t[-p <port> (default: 10000)]\n"
                     "\t\t[-D<key>=<value>]*\n"
                     "\t\t[-s <seed> (default: 0)]\n"

                     "\t\t[-o <overLogFile> (default: standard input)]\n"
                     "\t\t[-d <startDelay> (default: 0)]\n"
                     "\t\t[-g <filename> (produce a DOT graph)]\n"
                     "\t\t[-c (output canonical form)]\n"
                     "\t\t[-C (pre-compile into dataflow only)]\n"
                     "\t\t[-v (show stages of planning)]\n"
                     "\t\t[-x (dry run, don't start dataflow)]\n"
                     "\t\t[-h (gets usage help)]\n";



int
main(int argc, char **argv)
{
  string myHostname = "localhost";
  int port = 10000;
  std::vector< std::string > definitions;
  int seed = 0;


  string overLogFile("-");
  string dotFile("-");
  double delay = 0.0;
  bool outputDot = false;
  bool run = true;
  bool outputCanonicalForm = false;
  bool outputStages = false;
  bool preprocess = true;
  bool compileOnly = false;
  
  bool tupleCountAndBandwidth = false;

  Reporting::setLevel(Reporting::ERROR);
  // Parse command line options
  int c;
  while ((c = getopt(argc, argv, "r:n:p:D:ho:f:s:d:gcCx:z")) != -1) {
    switch (c) {
    case 'r':
      {
        // My minimum reporting level is optarg
        std::string levelName(optarg);
        Reporting::Level level =
        Reporting::levelFromName()[levelName];
        Reporting::setLevel(level);
      }
      break;

    case 'z':
      tupleCountAndBandwidth = true;
      break;

    case 'n':
      myHostname = optarg;
      break;

    case 'p':
      port = atoi(optarg);
      break;

    case 'D':
      definitions.push_back(optarg);
      break;

    case 's':
      seed = atoi(optarg);
      break;

    case 'o':
      overLogFile = optarg;
      break;

    case 'g':
      outputDot = true;
      break;

    case 'c':
      outputCanonicalForm = true;
      break;

    case 'C':
      compileOnly = true;
      break;

    case 'm':
      preprocess = false;
      break;

    case 'x':
      run = false;
      break;

    case 'v':
      outputStages = true;
      break;

    case 'd':
      delay = atof(optarg);
      break;

    case 'h':
    default:
      TELL_ERROR << USAGE;
      exit(-1);
    }
  }      

  srandom(seed);
  TELL_INFO << "Seed is \"" << seed << "\"\n";

  TELL_INFO << "My environment is ";
  for (std::vector< std::string>::iterator i =
         definitions.begin();
       i != definitions.end();
       i++) {
    TELL_INFO << (*i) << " ";
  }
  TELL_INFO << "\n";

  TELL_INFO << "Running from translated file \"" << overLogFile << "\"\n";



  ////////////////////////////////////////////////////////////
  // Get a temporary file name for the derivatives
  char derivativeFileName[17] = "";
  int fd = -1;
  strncpy(derivativeFileName, "/tmp/p2.XXXXXX", sizeof derivativeFileName);
  if ((fd = mkstemp(derivativeFileName)) == -1) {
    TELL_ERROR << "Could not generate temporary filename '"
               << derivativeFileName
               << "' due to error "
               << strerror(errno)
               << "\n";
    exit(-1);
  }
  TELL_INFO << "Temporary files will have the prefix "
            << derivativeFileName
            << "\n";


  std::ostringstream myAddressBuf, myPortBuf;
  myAddressBuf <<  myHostname << ":" << port;
  myPortBuf <<port;
  std::string myAddress = myAddressBuf.str();
  std::string myPort = myPortBuf.str();
  TELL_INFO << "My address is \"" << myAddress << "\"\n";
  
  TELL_INFO << "My start delay is " << delay << "\n";

  try {
    loadAllModules();

    // Set up my NodeID table
    Plumber::catalog()->nodeid(Val_Str::mk(myAddress), Val_Str::mk(myAddress));
    assert(Plumber::catalog()->nodeid());

    // Set up my arguments table
    CommonTablePtr argumentsTbl = Plumber::catalog()->table(ARGUMENT);
    for (std::vector< std::string>::iterator i =
           definitions.begin();
         i != definitions.end();
         i++) {
      std::string definition = (*i);
      std::string::size_type eqPosition = definition.find('=');
      if (eqPosition == std::string::npos) {
        // Doesn't have an equals. Throw warning and skip
        TELL_WARN << "Argument \"" 
                  << definition
                  << "\" is missing a '='. Skipped."
                  << "\n";
      } else if (eqPosition == 0) {
        // Doesn't have a key. Throw warning and skip
        TELL_WARN << "Argument \"" 
                  << definition
                  << "\" has no key, only value (after the '='). Skipped."
                  << "\n";
      } else if (eqPosition == definition.size() - 1) {
        // Doesn't have a value. Throw warning and skip
        TELL_WARN << "Argument \"" 
                  << definition
                  << "\" has no value, only key (before the '='). Skipped."
                  << "\n";
      } else {
        std::string key = definition.substr(0, eqPosition - 1);
        std::string value = definition.substr(eqPosition + 1);

        TuplePtr argumentsTp = Tuple::mk(ARGUMENT);
        argumentsTp->append(Val_Str::mk(key));
        argumentsTp->append(Val_Str::mk(value));
        argumentsTp->freeze();
        assert(argumentsTbl->insert(argumentsTp));
      }
    }

    string dfdesc = stub(myHostname, myPort, TERMINAL, tupleCountAndBandwidth);

    eventLoopInitialize();
    TELL_INFO << "Stub dataflow is:\n----\n"
              << dfdesc
              << "\n----\n";
    compile::Context *context = new compile::p2dl::Context("p2dl", dfdesc);

    DataflowPtr main = Plumber::dataflow("main");
    ElementPtr loaderElement = main->find("loader")->element();
    ProgramLoader *loader = dynamic_cast<ProgramLoader*>(loaderElement.get());
    if (overLogFile != "-") {
      if (!definitions.empty() && !preprocess) {
        TELL_WARN << "You have suppressed preprocessing (via -m) "
                  << "but have also supplied extra macros (via -D). "
                  << "All macro definitions will be ignored.\n";
      }   
     
      loader->program("commandLine", overLogFile, "",
                      (preprocess ) ? &definitions : NULL, compileOnly);
    }


    // Output the graph if requested using the temporary file name
    if (outputDot) {
      dotFile = string(derivativeFileName) + ".dot";
      loader->dot(dotFile);
    }

    delete context;
    eventLoop(); 
  } catch (TableManager::Exception& e) {
    std::cerr << e.toString() << std::endl;
  } catch (compile::Exception& e) {
    std::cerr << e.toString() << std::endl;
  }
}

