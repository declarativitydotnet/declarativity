/*
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * Or
 * UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
 * Berkeley, CA,  94707. Attention: P2 Group.
 * 
 * DESCRIPTION: Element which simply prints any tuple that passes
 * through it.
 */

#include "p2.h"
#include "loop.h"

// For (2)wait
#include <sys/types.h>
#include <sys/wait.h>


#include "udp.h"
#include "dDuplicateConservative.h"
#include "timedPullPush.h"
#include "queue.h"
#include "ol_lexer.h"
#include "ol_context.h"
#include "val_str.h"
#include <iostream>
#include "reporting.h"

#include "ol_context.h"
#include "eca_context.h"
#include "localize_context.h"

#include "planner.h"
#include "ruleStrand.h"
#include "stageStrand.h"

#include "tableStore.h"

#include "elementLoader.h"
#include "netLoader.h"
#include "stageLoader.h"
#include "aggFuncLoader.h"
#include "functionLoader.h"

string
P2::preprocessReadOverLogProgram(std::string overLogFilename,
                                 std::string derivedFilename,
                                 std::vector< std::string > definitions)
{
  string processed = derivedFilename + ".cpp";
  

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

  args[count++] = (char*) overLogFilename.c_str();
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


string
P2::readOverLogProgram(std::string overLogFilename)
{
  // Read processed script.
  std::ifstream file;
  file.open(overLogFilename.c_str());

  if (!file.is_open()) {
    TELL_ERROR << "Cannot open Overlog file \""
               << overLogFilename << "\"!\n";
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


P2::DataflowHandle
P2::createDataflow(string dataflowName,
                   string myAddress,
                   int port,    // extracted from myAddress for
                                // convenience
                   string derivativeFile,
                   std::string program,
                   bool outputCanonicalForm,
                   bool outputStages,
                   bool outputDot)
{
  // Initialize the event loop
  eventLoopInitialize();


  // Load up all loadable modules
  loadAllModules();


  // Set up the P2 dataflow
  PlumberPtr plumber(new Plumber());
  Plumber::DataflowPtr dataflow(new Plumber::Dataflow(dataflowName));


  // Set up the P2 table environment
  boost::shared_ptr< OL_Context > ctxt(new OL_Context());
  boost::shared_ptr< TableStore > tableStore(new TableStore(ctxt.get()));  
  

  ////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////
  // Compilation pipeline
  ////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////

  ////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////
  // PARSE
  ////////////////////////////////////////////////////////////
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
  tableStore->initTables(); 


  ////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////
  // LOCALIZE
  ////////////////////////////////////////////////////////////
  
  boost::shared_ptr< Localize_Context > lctxt(new Localize_Context()); 
  lctxt->rewrite(ctxt.get(), tableStore.get());  
  if (outputStages) {
    std::ofstream localizedStream((derivativeFile + ".localized").c_str());
    localizedStream << lctxt->toString();
    localizedStream.close();
  }


  ////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////
  // Turn to ECA
  ////////////////////////////////////////////////////////////

  boost::shared_ptr< ECA_Context > ectxt(new ECA_Context()); 
  ectxt->rewrite(lctxt.get(), tableStore.get());
  if (outputStages) {
    std::ofstream ecaStream((derivativeFile + ".eca").c_str());
    ecaStream << ectxt->toString();
    ecaStream.close();
  }
  

  ////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////
  // Plan to dataflow
  ////////////////////////////////////////////////////////////

  boost::shared_ptr< Planner > planner(new Planner(dataflow,
                                                   tableStore.get(),
                                                   false, 
						   myAddress));
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

    // Dump table definitions
    OL_Context::TableInfoMap* tablesInfos = tableStore->getTableInfos();
    for (OL_Context::TableInfoMap::iterator i =
           tablesInfos->begin();
         i != tablesInfos->end();
         i++) {
      OL_Context::TableInfo* thisTable =
        (*i).second;
      ruleStrandStream << thisTable->toString()
                       << "\n";
    }
    TableStore::TableMap* tables = tableStore->getTables();
    for (TableStore::TableMap::iterator i =
           tables->begin();
         i != tables->end();
         i++) {
      CommonTablePtr thisTable = (*i).second;
      ruleStrandStream << thisTable->toString()
                       << "\n";
    }
           
      
    ruleStrandStream.close();
  }
  
  for (unsigned k = 0; k < ruleStrands.size(); k++) {
    TELL_INFO << ruleStrands.at(k)->toString();
  }
  planner->setupNetwork(udp);
  planner->registerAllRuleStrands(ruleStrands, stageStrands);
  TELL_INFO << planner->getNetPlanner()->toString() << "\n";


  ////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////
  // Install dataflow
  ////////////////////////////////////////////////////////////

  if (plumber->install(dataflow) == 0) {
    TELL_INFO << "Correctly initialized dataflow.\n";
    if (outputDot) {
      plumber->toDot(derivativeFile + ".dot");
    }
  } else {
    TELL_ERROR << "** Failed to initialize correct spec\n";
    // Throw an exception of some sort
  }


  // Create dataflow handle to return
  P2::DataflowHandle handle(plumber, udp, dataflow);
  return handle;
}


void
P2::run()
{
  eventLoop();
}


void
P2::loadAllModules()
{
  // Stages
  ElementLoader::loadElements();
  NetLoader::loadElements();
  StageLoader::loadStages();
  AggFuncLoader::loadAggFunctions();
  FunctionLoader::loadFunctions();
}











































P2::P2() 
{
}


int
P2::install(string type, string program)
{
  return 0;
}

P2::CallbackHandle
P2::subscribe(DataflowHandle handle,
              string tupleName,
              TupleListener::TupleCallback callback)
{
  int port = -1;	// The port that the listener sits on

  // Create a new edit 
  string dataflowName = handle._dataflow->name();
  Plumber::DataflowEditPtr edit =
    handle._plumber->edit(dataflowName);

  // We will be creating a new tuple listener
  ElementSpecPtr listener = 
    edit->addElement(ElementPtr(new TupleListener("listener_" + tupleName,
                                                  callback)));


  // Do we have a duplicator for this tuple name?
  ElementSpecPtr duplicator = edit->find("DDuplicateConservative!" +
                                         tupleName);

  if (!duplicator) {
    // We don't seem to. Do we even listen for this tuple name? Find the
    // demux and ask it for its output port.
    ElementSpecPtr demux = edit->find("receiveDemux");
    if (!demux) {
      // Couldn't find the main demux. Fail.
      TELL_ERROR << "Could not locate main demultiplexer in "
                 << "dataflow '"
                 << dataflowName
                 << "'.\n";
      exit(-1);
    }

    int tupleOutputPort =
      demux->element()->output(Val_Str::mk(tupleName));
    if (tupleOutputPort == -1) {
      // No such demux output exists

      // We don't currently handle that case
      TELL_ERROR << "We don't currently support subscriptions "
                 << "to tuple that are not being listened to, "
                 << "such as '"
                 << tupleName
                 << "'.\n";
      assert(false);
    } else {
      // We've got the demux output but no duplicator. Insert the
      // duplicator between the demux and the queue on this port.

      // The element name on the demux port
      std::string strandHeadName =
        demux->element()->output(tupleOutputPort)->element()->name();
      ElementSpecPtr strandHeadElement =
        edit->find(strandHeadName);
      if (!strandHeadElement) {
        // Couldn't look up the name. Biig screwup.
        TELL_ERROR << "Could not find the element on the demux output '"
                   << tupleOutputPort
                   << "' by name. Oops!\n";
        exit(-1);
      }

      duplicator =
        edit->
        addElement(ElementPtr(new
                              DDuplicateConservative("DuplicateConservative!"
                                                     + tupleName,
                                                     1)));
      edit->hookUp(duplicator, 0,
                   strandHeadElement, 
                   demux->element()->
                   output(tupleOutputPort)->port());
      edit->hookUp(demux, tupleOutputPort, duplicator, 0);
    }
  } 
  
  // We have the duplicator now. Attach a new output with the listener
  port = duplicator->add_output();
  edit->hookUp(duplicator, port, listener, 0);
  
  // And install the edit
  if (handle._plumber->install(edit) < 0) {
    TELL_ERROR << "ERROR: Couldn't install listener" << std::endl;
    exit(-1);
  }
  
  // The handle is the ID of the listener element
  return P2::CallbackHandle(listener);
}


void
P2::unsubscribe(DataflowHandle dfHandle,
                P2::CallbackHandle handle)
{
  TELL_ERROR << "P2::unsubscribe is not yet implemented!" << std::endl;
  assert(0);
} 


int
P2::injectTuple(DataflowHandle dfHandle,
                TuplePtr tp,
                b_cbv callback)
{
  TELL_WORDY << "P2::injectTuple got "
             << tp->toString()
             << std::endl;
  return dynamic_cast<TupleInjector*>(dfHandle._dataflow->injector().get())
    ->tuple(tp, callback);
}


void
P2::nullCallback()
{
}


P2::DataflowHandle::DataflowHandle(PlumberPtr plumber,
                                   UdpPtr udp,
                                   Plumber::DataflowPtr dataflow)
  : _plumber(plumber),
    _udp(udp),
    _dataflow(dataflow)
{
}

PlumberPtr
P2::DataflowHandle::plumber()
{
  return _plumber;
}


P2::CallbackHandle::CallbackHandle(ElementSpecPtr listener)
  : _listener(listener)
{
}
