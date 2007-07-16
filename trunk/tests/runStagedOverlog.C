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

enum TransportConf {NONE=0, RELIABLE=1, ORDERED=2, CC=4, RCC=5, TERMINAL=6};

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

string stub(string hostname, string port, TransportConf conf)
{
  ostringstream stub;

  stub << "graph main(0, 0, \"/\", \"/\"){\n";

  netPlan(stub,hostname,port,conf);
  seaPlan(stub,hostname,port);

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
       << "\tnetOut -> udp;\n"
       << "\tnetIn -> [+]extDRR;\n"
       << "\tUpdate(\"programUpdate\",\"" << PROGRAM << "\") -> "
       << "PelTransform(\"packageUpdate\", \"$1 pop swallow pop\") -> [+]extDRR;\n"
          //hook extDRR to seaInput, then to intQMux
       << "\textDRR->seaInput;\n"
          //start internal event processing strand
	  //in theory, internal queue must be made infinite to avoid deadlock
 	  //to be absolutely sure.
       << "\tseaInput -> [0]intQMux -> PelTransform(\"unpackage\", \"$1 unboxPop\") -> "
       << "\tQueue(\"intQ\", 100,\"internal\") -> PullPush(\"IntQPP\",0) -> intDemux[0] -> "
       << "Print(\"Unrecognized Message\") -> Discard(\"discard\");\n"
          //start the rule output process
       << "\tintDRR -> PullPush(\"SEAOutputPP\",0) -> intExtDemux;\n"
	  //External events to commitbuf, to netOut
       << "\tintExtDemux[1] -> CommitBuf(\"NetCommitBuf\") -> netOut;\n"
          //internal events to internal mux
       << "\tintExtDemux[0] -> [1]intQMux;\n";

  /* Connect the default compiler stages */
  stub << "\tintDemux[+ \"" << PROGRAM << "\"] -> CompileStage(\"compileStage\") -> "
       << "\tPelTransform(\"package\", \"\\\"" << hostname << ":" << port << "\\\" pop swallow pop\") -> " 
       << "\tQueue(\"csQ\", 10) -> [+]extDRR;\n";

  stub << "\tintDemux[+ \"parse::programEvent\"] -> ParseContext(\"parse\") -> "
       << "\tInsert2(\"parseInsert\", \"" << PROGRAM << "\");\n";

  stub << "\tintDemux[+ \"eca::programEvent\"]   -> EcaContext(\"eca\") -> "
       << "\tInsert2(\"ecaInsert\", \"" << PROGRAM << "\");\n";

  stub << "\tintDemux[+ \"local::programEvent\"] -> LocalContext(\"local\") -> "
       << "\tInsert2(\"localInsert\", \"" << PROGRAM << "\");\n";

  stub << "\tintDemux[+ \"rewrite::programEvent\"] -> RewriteContext(\"rewrite\") -> "
       << "\tInsert2(\"rewriteInsert\", \"" << PROGRAM << "\");\n";

  stub << "\tintDemux[+ \"debug::programEvent\"] -> DebugContext(\"debug\") -> "
       << "\tInsert2(\"debugInsert\", \"" << PROGRAM << "\");\n";

  stub << "\tintDemux[+ \"planner::programEvent\"] -> "
       << "PlannerContext(\"planner\", \"main\", \"intDemux\", \"intDRR\", \"extDRR\") -> "
       << "\tInsert2(\"plannerInsert\", \"" << PROGRAM << "\");\n";

  stub << "\tintDemux[+ \"p2dl::programEvent\"] -> P2DLContext(\"p2dl\") -> "
       << "\tInsert2(\"p2dlInsert\", \"" << PROGRAM << "\");\n";

  stub << "\tintDemux[+ \"installed::programEvent\"] -> "
       << "\tInsert2(\"installInsert\", \"" << PROGRAM << "\");\n";

  if(conf & TERMINAL) {
    stub << "\t\tCompileTerminal(\"ct\") -> Insert2(\"ctInsert\", \"" << PROGRAM << "\");\n";
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
main(int argc, char **argv)
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

  std::ostringstream myAddressBuf, myPortBuf;
  myAddressBuf <<  myHostname << ":" << port;
  myPortBuf <<port;
  std::string myAddress = myAddressBuf.str();
  std::string myPort = myPortBuf.str();
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






  try {
    loadAllModules();

    Reporting::setLevel(Reporting::ERROR);
    CommonTablePtr nodeIDTbl = Plumber::catalog()->table(NODEID);
    TuplePtr nodeIDTp = Tuple::mk(NODEID);
    nodeIDTp->append(Val_Str::mk(myAddress));
    nodeIDTp->append(Val_Str::mk(myAddress));
    nodeIDTp->freeze();
    assert(nodeIDTbl->insert(nodeIDTp));
    assert(Plumber::catalog()->nodeid());

    string dfdesc = stub(myHostname,myPort,TERMINAL);

    eventLoopInitialize();
    compile::Context *context = new compile::p2dl::Context("p2dl", dfdesc);
    Plumber::toDot("runOverlog.dot");
    delete context;
    eventLoop(); 
  } catch (TableManager::Exception& e) {
    std::cerr << e.toString() << std::endl;
  } catch (compile::Exception& e) {
    std::cerr << e.toString() << std::endl;
  }
}

