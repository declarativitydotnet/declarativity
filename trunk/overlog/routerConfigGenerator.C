// -*- c-basic-offset: 2; related-file-name: "element.C" -*-
/*
 * @(#)$Id$
 * 
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 */

//#include <async.h>
#//include <arpc.h>
#include "routerConfigGenerator.h"
#include "aggregate.h"

const str RouterConfigGenerator::SEL_PRE("select_");
const str RouterConfigGenerator::AGG_PRE("agg_");
const str RouterConfigGenerator::ASSIGN_PRE("assign_");
const str RouterConfigGenerator::TABLESIZE("TABLESIZE");


RouterConfigGenerator::RouterConfigGenerator(OL_Context* ctxt, Router::ConfigurationRef conf, 
					     bool dups, bool debug, str filename) :_conf(conf)
{
  _ctxt = ctxt;
  _dups = dups;
  _debug = debug;
  str outputFile(filename << ".out");
  _output = fopen(outputFile.cstr(), "w");

  // initialize some pel functions. Add more later. 
  // Look at Pel Expresssions
  pelFunctions.insert(std::make_pair(SEL_PRE << "neS", "==s not"));
  pelFunctions.insert(std::make_pair(SEL_PRE << "eqS", "==s"));
  pelFunctions.insert(std::make_pair(SEL_PRE << "geI", ">=i"));
  pelFunctions.insert(std::make_pair(SEL_PRE << "gtI", ">i"));
  pelFunctions.insert(std::make_pair(SEL_PRE << "eqI", "==i"));
  pelFunctions.insert(std::make_pair(SEL_PRE << "rangeID1", "()id"));
  pelFunctions.insert(std::make_pair(SEL_PRE << "rangeID2", "[]id"));
  pelFunctions.insert(std::make_pair(SEL_PRE << "rangeID3", "(]id"));
  pelFunctions.insert(std::make_pair(SEL_PRE << "rangeID4", "[)id"));
  pelFunctions.insert(std::make_pair(ASSIGN_PRE << "minusI", "-i"));
  pelFunctions.insert(std::make_pair(ASSIGN_PRE << "addID", "+id"));
  pelFunctions.insert(std::make_pair(ASSIGN_PRE << "addI", "+i"));
  pelFunctions.insert(std::make_pair(ASSIGN_PRE << "distID", "distance"));
  pelFunctions.insert(std::make_pair(ASSIGN_PRE << "random", "rand"));
  pelFunctions.insert(std::make_pair(ASSIGN_PRE << "idOne", "1 ->u32 ->id"));
  pelFunctions.insert(std::make_pair(ASSIGN_PRE << "leftShiftID", "<<id"));
  pelFunctions.insert(std::make_pair(ASSIGN_PRE << "varAssign", ""));
  pelFunctions.insert(std::make_pair(ASSIGN_PRE << "modI", "%"));

  _pendingRegisterReceiver = false;
}


RouterConfigGenerator::~RouterConfigGenerator()
{
  fclose(_output);
}


// call this for each udp element that we wish to hook up the same dataflow
// if running only one, nodeID is the local host name,
void RouterConfigGenerator::configureRouter(ref< Udp > udp, str nodeID)
{
  // first create the tables if they are not created yet.
  // iterate through all the table info
  _udpReceivers.clear();
  _udpPullSenders.clear();
  _udpPushSenders.clear();
  
  // iterate through all the functors (heads, unique by name, arity, location)
  OL_Context::FunctorMap::iterator _iterator;
  for (_iterator = _ctxt->getFunctors()->begin(); 
       _iterator != _ctxt->getFunctors()->end(); _iterator++) {
    OL_Context::Functor* nextFunctor = _iterator->second;
    processFunctor(nextFunctor, nodeID);
  }

  if (_udpPushSenders.size() + _udpPullSenders.size() > 0) {
    generateSendElements(udp, nodeID);
  }

  if (_udpReceivers.size() > 0) {
    generateReceiveElements(udp, nodeID);
  }
}


bool RouterConfigGenerator::hasSingleAggTerm(OL_Context::Rule* currentRule)
{
  int numAggs = 0;
  for (uint k = 0; k < currentRule->terms.size(); k++) {
    if (isAggregation(currentRule->terms.at(k))) {
      numAggs ++;
    }
  }
  return (numAggs == 1);
}

bool RouterConfigGenerator::hasPeriodicTerm(OL_Context::Rule* currentRule)
{
  for (uint k = 0; k < currentRule->terms.size(); k++) {
    if (currentRule->terms.at(k).fn->name == "periodic") {
      return true;
    }
  }
  return false;
}

void RouterConfigGenerator::processFunctor(OL_Context::Functor* currentFunctor, str nodeID)
{
  // The current translation proceeds as follows. 
  // For each functor, we enumerate through all the rules.
  // For each rule, 
  // 1) First, identify all terms where we need to create listeners. They are either
  //    a) Single Tail Terms
  //    b) Event terms used for probing other tables
  // 2) If joins are needed, identify the event term that is used to probe the others.
  // 3) Apply selections
  // 4) Apply projections, address to send (based on head)
  // 5) Stick muxers and demuxers accordingly

  ElementSpecRef mostRecentElement = New refcounted<ElementSpec>(New refcounted<Slot>("dummySlotProcessFunctor"));

  for (unsigned int k = 0; k < currentFunctor->rules.size(); k++) {
    OL_Context::Rule* nextRule = currentFunctor->rules.at(k);
    // first, get a list of possible unifications needed
    std::vector<JoinKey>* joinKeys = New std::vector<JoinKey>;
    setJoinKeys(nextRule, joinKeys);    

    // step 1:
    FieldNamesTracker* currentNamesTracker = new FieldNamesTracker();

    // special case for aggregates
    if (hasSingleAggTerm(nextRule)) { // has a single agg on a single materialized table
      generateSingleAggregateElements(currentFunctor, nextRule, 
				      nodeID, currentNamesTracker);      
      
      delete joinKeys;
      delete currentNamesTracker;
      continue;
    }

    // another special case for event generation
    if (hasPeriodicTerm(nextRule)) {
      mostRecentElement = generateFunctorSource(currentFunctor, nextRule, nodeID);
      goto sendData;
    }

    if (joinKeys->size() == 0) {
      // if there are no possible unifications, we process only the first term
      generateSingleTermElement(currentFunctor, nextRule, 
				nodeID, currentNamesTracker);
      _currentType = "h";   
    } else {
      // probe
      // FIXME for future:
      // 1) In future, have to detect that if there are no events fall back to 
      //    traditional symmetric hash joins
      // 2) Probes may be on different nodes. So may have to rehash.
      mostRecentElement = generateJoinElements(currentFunctor, nextRule, 
					       nodeID, currentNamesTracker);
      _currentType = "h";      
    }
    
    // do any selections. 
    mostRecentElement = generateAllSelectionElements(nextRule, nodeID, currentNamesTracker, mostRecentElement);

    mostRecentElement = generateAllAssignmentElements(nextRule, nodeID, currentNamesTracker, mostRecentElement);


    // do the necessary projections based on the head
    mostRecentElement = generateProjectHeadElements(currentFunctor, nextRule, nodeID, 
						    currentNamesTracker, mostRecentElement);    

  sendData:    
    if (nextRule->deleteFlag == true) {
      std::cout << "Delete " << currentFunctor->name << " for rule " << 
	nextRule->toString() << "\n";
      // And send it for deletion. Assume deletion happens here. FIXME in future.
      TableRef tableToDelete = getTableByName(nodeID, currentFunctor->name);
      OL_Context::TableInfo* ti = _ctxt->getTableInfos()->find(currentFunctor->name)->second;

      /*ElementSpecRef pullPushDelete =
	_conf->addElement(New refcounted< TimedPullPush >(strbuf("PullPushDelete:") << nextRule->ruleID,
							 0));
      */ 
      mostRecentElement = generatePrintElement(strbuf("PrintBeforeDelete:") << nextRule->ruleID << nodeID, 
					       mostRecentElement);
      ElementSpecRef deleteElement =
	_conf->addElement(New refcounted< Delete >(strbuf("Delete:") << nextRule->ruleID << nodeID,
						   tableToDelete, 
						   ti->primaryKeys.at(0), 						   
						   ti->primaryKeys.at(0)));

      // Link the two joins together
      hookUp(mostRecentElement, 0, deleteElement, 0);
      //hookUp(pullPushDelete, 0, deleteElement, 0);
      delete joinKeys;
      delete currentNamesTracker;
      continue;
    } else {
      // send it!
      mostRecentElement = generateSendMarshalElements(nextRule, nodeID, 
						      currentFunctor->arity, mostRecentElement);     
      // send back
      // not done yet, we also need to register a table receiver for the head
      str headTableName = currentFunctor->name;
      registerReceiverTable(headTableName);
      // link that to a discard. If there are other flows that need it, they will
      // get the tuples via another fork in the duplicator.
      ElementSpecRef sinkS = _conf->addElement(New refcounted< Discard >("discard"));    
      registerReceiver(headTableName, sinkS);
    }

    // do the selections
    delete joinKeys;
    delete currentNamesTracker;


    if (_currentType == "h") {
      _udpPushSenders.push_back(mostRecentElement); // need a slot
    } else {
      _udpPullSenders.push_back(mostRecentElement); // no need a slot
    }      
  }
}



//////////////////// Transport layer //////////////////////////////////////////
void RouterConfigGenerator::generateReceiveElements(ref< Udp> udp, str nodeID)
{

  // My store of reach tuples
  ElementSpecRef udpReceive = _conf->addElement(udp->get_rx());  

  // demuxer
  ref< vec< ValueRef > > demuxKeys = New refcounted< vec< ValueRef > >;
  ReceiverInfoMap::iterator _iterator;
  for (_iterator = _udpReceivers.begin(); _iterator != _udpReceivers.end(); _iterator++) {
    str nextTableName = _iterator->second._name;
    demuxKeys->push_back(New refcounted< Val_Str >(nextTableName));
  }

  ElementSpecRef demuxS = _conf->addElement(New refcounted< Demux >("receiveDemux", demuxKeys));

  ElementSpecRef unmarshalS =
    _conf->addElement(New refcounted< UnmarshalField >(strbuf("ReceiveUnmarshal:") << nodeID, 1));

  ElementSpecRef unBoxS =
    _conf->addElement(New refcounted< UnboxField >(strbuf("ReceiveUnBox:") << nodeID, 1));

  hookUp(udpReceive, 0, unmarshalS, 0);
  hookUp(unmarshalS, 0, unBoxS, 0);
 
  ElementSpecRef mostRecentElement = generatePrintElement(strbuf("PrintReceivedBeforeDemux:") << nodeID, unBoxS);
  mostRecentElement = generateDupElimElement(strbuf("ReceiveDupElim:") << nodeID, mostRecentElement); 
  //mostRecentElement = generatePrintElement(strbuf("PrintReceivedAfterDupElim:") << nodeID, mostRecentElement);  

  hookUp(mostRecentElement, 0, demuxS, 0);

  // connect to all the pending udp receivers. Assume all 
  // receivers are connected to elements with push input ports for now
  int counter = 0;
  for (_iterator = _udpReceivers.begin(); _iterator != _udpReceivers.end(); _iterator++) {
    ReceiverInfo ri = _iterator->second;
    int numElementsToReceive = ri._receivers.size(); // figure out the number of receivers
    str tableName = ri._name;

    std::cout << "Demuxing " << tableName << " for " << numElementsToReceive << " elements\n";

    // DupElim -> DemuxS -> Insert -> Duplicator -> Fork
    
    // duplicator
    ElementSpecRef duplicator = _conf->addElement(New refcounted< Duplicate >(strbuf("ReceiveDup:") 
									      << tableName << ":" << nodeID, 
									      numElementsToReceive));    
    // materialize table only if it is declared and has lifetime>0
    OL_Context::TableInfoMap::iterator _iterator = _ctxt->getTableInfos()->find(tableName);
    if (_iterator != _ctxt->getTableInfos()->end() && _iterator->second->timeout != 0) {
      ElementSpecRef insertS = _conf->addElement(New refcounted< Insert >(strbuf("Insert:") << 
									  tableName << ":" << nodeID,  
									  getTableByName(nodeID, 
											 tableName)));
      hookUp(demuxS, counter++, insertS, 0);
      hookUp(insertS, 0, duplicator, 0);
    } else {
      hookUp(demuxS, counter++, duplicator, 0);
    }

    // connect the duplicator to elements for this name
    for (uint k = 0; k < ri._receivers.size(); k++) {
      ElementSpecRef nextElementSpec = ri._receivers.at(k);
      //std::cout << "Hook up with duplicator " << tableName 
      //<< " " << k << " " << nextElementSpec->toString() << "\n";
      hookUp(duplicator, k, nextElementSpec, 0);
    }
  }

  ElementSpecRef sinkS = _conf->addElement(New refcounted< Discard >("discard"));
  hookUp(demuxS, _udpReceivers.size(), sinkS, 0); // if we don't know where this should go
}



void RouterConfigGenerator::generateSendElements(ref< Udp> udp, str nodeID)
{
  ElementSpecRef udpSend = _conf->addElement(udp->get_tx());  

  // prepare to send. Assume all tuples send by first tuple
  ElementSpecRef muxS =
    _conf->addElement(New refcounted< RoundRobin >("sendMux:" << nodeID, _udpPushSenders.size() + _udpPullSenders.size()));

  /*ElementSpecRef pullPush =
    _conf->addElement(New refcounted< TimedPullPush >(strbuf("PullPush:") 
						      << nodeID,
						      0));
  ElementSpecRef pushToPullQueue = 
    _conf->addElement(New refcounted< Queue >(strbuf("Queue:") << nodeID, 1000));
  */
  hookUp(muxS, 0, udpSend, 0);
  //hookUp(pullPush, 0, pushToPullQueue, 0);
  //hookUp(pushToPullQueue, 0, udpSend, 0);
  
  // form the push senders
  for (unsigned int k = 0; k < _udpPushSenders.size(); k++) {
    ElementSpecRef nextElementSpec = _udpPushSenders.at(k);

    ElementSpecRef pushToPull = 
      _conf->addElement(New refcounted< Queue >(strbuf("Queue:") << 
					       nodeID << ":" << k, 1000));
    hookUp(nextElementSpec, 0, pushToPull, 0);
    hookUp(pushToPull, 0, muxS, k);
  }

  // since pull senders, no need slot
  for (unsigned int k = 0; k < _udpPullSenders.size(); k++) {
    ElementSpecRef nextElementSpec = _udpPullSenders.at(k);
    hookUp(nextElementSpec, 0, muxS, k + _udpPushSenders.size());
  }
}

ElementSpecRef 
RouterConfigGenerator::generateFunctorSource(OL_Context::Functor* functor, OL_Context::Rule* rule, str nodeID)
{
 // My fix finger tuple
  TupleRef functorTuple = Tuple::mk();
  functorTuple->append(Val_Str::mk(functor->name));
  functorTuple->append(Val_Str::mk(nodeID)); // address for marshalling
  functorTuple->append(Val_Str::mk(nodeID)); 
  functorTuple->freeze();

  ElementSpecRef source =
    _conf->addElement(New refcounted< TupleSource >(str("FunctorSource:") << rule->ruleID << nodeID,
                                                   functorTuple));
  
  ElementSpecRef mostRecentElement = generatePrintElement(strbuf("PrintFunctorSource:") << 
							  rule->ruleID << ":" << nodeID, source);
  str firstVar = rule->terms.at(0).argNames.at(0);
  // The timed pusher
  ElementSpecRef pushFunctor =
    _conf->addElement(New refcounted< TimedPullPush >(strbuf("FunctorPush:") << rule->ruleID << ":" << nodeID,
						      atoi(firstVar.cstr())));

  //hookUp(source, 0, mostRecentElement, 0);
  hookUp(mostRecentElement, 0, pushFunctor, 0);

  return pushFunctor;

}

// create a send element where the first field after table name is the address. Drop that field
ElementSpecRef 
RouterConfigGenerator::generateSendMarshalElements(OL_Context::Rule* rule, str nodeID, 
						   int arity, ElementSpecRef toSend)
{
  ElementSpecRef mostRecentElement = generateDupElimElement(strbuf("SendDupElim:") << rule->ruleID 
							    << ":" << nodeID, toSend);

  // Create the encapsulated version of this tuple, holding the
  // destination address and, encapsulated, the payload containing the
  // reach tuple. Take care to avoid sending the first field address
  strbuf marshalPelStr(" $0 ->t ");
  // fill in depending on arity. Notice we drop the first field, and avoid the 
  // table name which is already inside (+2)
  for (int k = 0; k < arity; k++) {
    marshalPelStr << "$" << k+2 << " append ";
  }
  marshalPelStr << "pop";

  ElementSpecRef encapS =
    _conf->addElement(New refcounted< PelTransform >(strbuf("encapSend:") << rule->ruleID << ":" << nodeID, 
						    "$1 pop" << marshalPelStr)); // the rest
  hookUp(mostRecentElement, 0, encapS, 0);

  mostRecentElement = generatePrintElement(strbuf("PrintSend:") << 
					   rule->ruleID << ":" << nodeID, encapS);

  
  // Now marshall the payload (second field)
  ElementSpecRef marshalS = 
    _conf->addElement(New refcounted< MarshalField >("marshal:" << 
						     rule->ruleID << ":" << nodeID, 1));
  
  // And translate the address into a sockaddr.  This is a hack until
  // we have a consistent view of the UDP interface.
  ElementSpecRef routeS =
    _conf->addElement(New refcounted< StrToSockaddr >("router:" << rule->ruleID 
						      << ":" << nodeID, 0));

  hookUp(mostRecentElement, 0, marshalS, 0);
  hookUp(marshalS, 0, routeS, 0);

  return routeS;
}

// for a particular table name that we are receiving, register an elementSpec that needs 
// that data
void RouterConfigGenerator::registerReceiver(str tableName, ElementSpecRef elementSpecRef)
{
  // add to the right receiver
  ReceiverInfoMap::iterator _iterator = _udpReceivers.find(tableName);
  if (_iterator != _udpReceivers.end()) {
    _iterator->second.addReceiver(elementSpecRef);
  }
}



// regiser a new receiver for a particular table name
// use to later hook up the demuxer
void RouterConfigGenerator::registerReceiverTable(str tableName)
{  
  ReceiverInfoMap::iterator _iterator = _udpReceivers.find(tableName);
  if (_iterator == _udpReceivers.end()) {
    // not there, we register
    _udpReceivers.insert(std::make_pair(tableName, ReceiverInfo(tableName)));
  }  
}
					     



//////////////////////////////////////////////////////////////////
///////////////// Relational Operators -> P2 Elements
//////////////////////////////////////////////////////////////////

bool RouterConfigGenerator::isSelection(OL_Context::Term term)
{
  str termName1 = term.fn->name;
  str termName2 = term.fn->name;
  return substr(termName1, 0, SEL_PRE.len()) == SEL_PRE || 
    substr(termName2, 1, SEL_PRE.len()) == SEL_PRE;  
}

bool RouterConfigGenerator::isAssignment(OL_Context::Term term)
{
  str termName = term.fn->name;
  return substr(termName, 0, ASSIGN_PRE.len()) == ASSIGN_PRE;
}


bool RouterConfigGenerator::isAggregation(OL_Context::Term term)
{
  str termName = term.fn->name;
  return substr(termName, 0, AGG_PRE.len()) == AGG_PRE;
}

ElementSpecRef 
RouterConfigGenerator::generateSelectionElements(OL_Context::Rule* currentRule, 
						 OL_Context::Term nextSelection, 
						 str nodeID, 
						 FieldNamesTracker* probeNames,
						 ElementSpecRef lastElement, 
						 int selectionID)
{
  // Prepend with true if this is a Reach X, X.
  str termName = nextSelection.fn->name;
  bool negation = false;
  if (substr(termName, 0, 1) == "!") {
    negation = true;
    termName = substr(termName, 1, termName.len()-1);
  } 

  if (pelFunctions.find(termName) == pelFunctions.end()) { return lastElement; } // skip this selection
  str pelSelectionOpt = pelFunctions.find(termName)->second;
  

  // figure out where the selection is
  strbuf selectionPel(" ");
  for (uint k = 0; k < nextSelection.argNames.size(); k++) {
    // is this an var or a val?
    if (nextSelection.args.at(k).var == -1) {
      selectionPel << nextSelection.args.at(k).val->toString() << " ";
    } else {
      selectionPel << "$" << (1 + probeNames->fieldPosition(nextSelection.argNames.at(k))) << " ";
    }
  }
  if (negation) {
    selectionPel << pelSelectionOpt << " ifstop "; 
  } else {
    selectionPel << pelSelectionOpt << " not ifstop "; 
  }
  
  // put in the old fields (+1 since we have to include the table name)
  for (uint k = 0; k < probeNames->fieldNames.size() + 1; k++) {
    selectionPel << "$" << k << " pop ";
  }

  std::cout << "Generate selection functions for " << str(selectionPel) << " " << probeNames->toString() << "\n";
 
  ElementSpecRef selection =
    _conf->addElement(New refcounted< PelTransform >(strbuf("Selection:") 
						     << currentRule->ruleID << ":" 
						     << selectionID << ":" << nodeID, 
						     selectionPel));

 if (_pendingRegisterReceiver) {
    registerReceiver(_pendingReceiverTable, selection);
    _pendingRegisterReceiver = false;
  } else {
    // connecting now
    hookUp(lastElement, 0, selection, 0);
  }

  return generatePrintElement(strbuf("PrintAfterSelection:") 
			      << currentRule->ruleID << ":" 
			      << selectionID << ":" << nodeID, selection);
}


ElementSpecRef 
RouterConfigGenerator::generateAllSelectionElements(OL_Context::Rule* currentRule,
						    str nodeID,
						    FieldNamesTracker* currentNamesTracker,
						    ElementSpecRef connectingElement) 
{
  ElementSpecRef lastSelectionSpecRef(connectingElement);
  for (unsigned int j = 0; j < currentRule->terms.size(); j++) {
    OL_Context::Term nextTerm = currentRule->terms.at(j);
    str termName = nextTerm.fn->name;
    if (isSelection(nextTerm)) {
      std::cout << "Selection term " << termName << " " << currentRule->ruleID << "\n";
      lastSelectionSpecRef = generateSelectionElements(currentRule, nextTerm, 
						       nodeID, currentNamesTracker, 
						       lastSelectionSpecRef, j); 
    }
  }
  return lastSelectionSpecRef;
}



ElementSpecRef 
RouterConfigGenerator::generateAssignmentElements(OL_Context::Rule* currentRule,
						  OL_Context::Term currentTerm, 
						  str nodeID,
						  FieldNamesTracker* currentNamesTracker,
						  ElementSpecRef connectingElement,
						  int assignmentID) 
{
  str termName = currentTerm.fn->name;
  
  // get the assignment str
  if (pelFunctions.find(termName) == pelFunctions.end()) {
    // cannot find this 
    std::cerr << "Invalid assignment term " << termName << "\n";
    return connectingElement;
  }
  strbuf pelStr;
  for (uint k = 0; k < currentNamesTracker->fieldNames.size()+1; k++) {
    pelStr << "$" << k << " pop ";
  }
  
  // do the pel portion. First add the extra variables in
  
  str pelExpression = pelFunctions.find(termName)->second;;
  
  // go through the arguments in the assignment
  bool fail = false;
  for (uint k = 1; k < currentTerm.argNames.size(); k++) {
    str currentTermArgName = currentTerm.argNames.at(k);
    
    // is this an var or a val?
    if (currentTerm.args.at(k).var == -1) {
      pelStr << currentTerm.args.at(k).val->toString() << " ";
    }        
    else if (substr(currentTermArgName, 0, TABLESIZE.len()) == TABLESIZE) {
      // table size keyword
      str tableName = substr(currentTerm.argNames.at(k), TABLESIZE.len()+1, 
			     currentTerm.argNames.at(k).len()-TABLESIZE.len()-1);
      std::cout << tableName << "\n";
      if (_ctxt->getTableInfos()->find(tableName) == _ctxt->getTableInfos()->end()) {
	fail = true;
	std::cerr << tableName << " cannot be found " << currentRule->ruleID << "\n";
	break;
      }
      OL_Context::TableInfo* tableInfo = _ctxt->getTableInfos()->find(tableName)->second;
      pelStr << tableInfo->size << " ";	  
    } else {
      if (currentNamesTracker->fieldPosition(currentTerm.argNames.at(k)) == -1) {
	fail = true;
	std::cerr <<currentTerm.argNames.at(k) << " is not a valid variable for " << 
	  termName << " in " << currentRule->ruleID << "\n";
	break;
      }
      pelStr << "$" << (1 + currentNamesTracker->fieldPosition(currentTerm.argNames.at(k))) << " ";
    }
  }
  if (fail) { 
    // we cannot assign
    return connectingElement; 
  }
  
  //if (substr(termName, ASSIGN_PRE.len(), 
  //if (substr(termName, ASSIGN_PRE.len(), termName.len()-ASSIGN_PRE.len()) == "varAssign") {
    //int pos = currentNamesTracker->fieldPosition(currentTerm.argNames.at(1));
    //pelStr << "$" << pos << " pop";
  //} else {
  pelStr << pelExpression << " pop";
  //}
  currentNamesTracker->fieldNames.push_back(currentTerm.argNames.at(0));
  
  std::cout << "Generate assignments for " << termName << " " << currentRule->ruleID << 
    " " << str(pelStr) << " " << currentNamesTracker->toString() << "\n";
  
  ElementSpecRef assignment =
    _conf->addElement(New refcounted< PelTransform >(strbuf("Assignment:") 
						     << currentRule->ruleID << ":" 
						     << assignmentID << ":" << nodeID, 
						     str(pelStr)));

 if (_pendingRegisterReceiver) {
    registerReceiver(_pendingReceiverTable, assignment);
    _pendingRegisterReceiver = false;
  } else {
    // connecting now
    hookUp(connectingElement, 0, assignment, 0);
  }

  connectingElement = generatePrintElement(strbuf("PrintAfterAssignment:") 
					   << currentRule->ruleID << ":" 
					   << assignmentID << ":" << nodeID, assignment);
  return connectingElement;
}


ElementSpecRef 
RouterConfigGenerator::generateAllAssignmentElements(OL_Context::Rule* currentRule,
						     str nodeID,
						     FieldNamesTracker* currentNamesTracker,
						     ElementSpecRef connectingElement) 
{
  for (unsigned int j = 0; j < currentRule->terms.size(); j++) {
    OL_Context::Term nextTerm = currentRule->terms.at(j);
    if (isAssignment(nextTerm)) {
      connectingElement = generateAssignmentElements(currentRule, nextTerm, nodeID, 
						    currentNamesTracker, connectingElement, j);
    }
  }
  return connectingElement;
}



ElementSpecRef 
RouterConfigGenerator::generateProjectHeadElements(OL_Context::Functor* currentFunctor, 
						   OL_Context::Rule* currentRule,
						   str nodeID,
						   FieldNamesTracker* currentNamesTracker,
						   ElementSpecRef connectingElement)
{
  // determine the projection fields, and the first address to return. Add 1 for table name     
  int addressField = currentNamesTracker->fieldPosition(currentFunctor->loc) + 1;
  std::vector<unsigned int> indices;  
  
  // iterate through all functor's output
  for (unsigned int k = 0; k < currentFunctor->arity; k++) {
    indices.push_back(currentNamesTracker->fieldPosition(currentRule->args.at(k)) + 1);
  }
  
  strbuf pelTransformStrbuf("\"" << currentFunctor->name << "\" pop");
  if (currentRule->deleteFlag == false) {
    pelTransformStrbuf << " $" << addressField << " pop"; // we are not deleting
  }
  for (unsigned int k = 0; k < indices.size(); k++) {
    pelTransformStrbuf << " $" << indices.at(k) << " pop";
  }
  str pelTransformStr(pelTransformStrbuf);
  
  // project, and make sure first field after table name has the address 
  ElementSpecRef projectHead =
    _conf->addElement(New refcounted< PelTransform >(strbuf("ProjectHead:") 
						     << currentRule->ruleID << ":" << nodeID,
						     pelTransformStr));
  if (_pendingRegisterReceiver) {
    registerReceiver(_pendingReceiverTable, projectHead);
    _pendingRegisterReceiver = false;
  } else {
    // connecting now
    hookUp(connectingElement, 0, projectHead, 0);
  }
  
  return generatePrintElement(strbuf("PrintHead:") 
			      << currentRule->ruleID 
			      << ":" << nodeID, projectHead);
  
}



ElementSpecRef
RouterConfigGenerator::generateProbeElements(OL_Context::Rule* currentRule, 
					     OL_Context::Term eventTerm, 
					     OL_Context::Term baseTerm, 
					     str nodeID, 
					     FieldNamesTracker* probeNames, 					    
					     ElementSpecRef priorElement,
					     int joinOrder)
{
  // probe the right hand side
  // Here's where the join happens 
  ElementSpecRef mostRecentElement = New refcounted<ElementSpec>(New refcounted<Slot>("dummySlotProbeElements"));
  FieldNamesTracker* baseTableNames = New FieldNamesTracker(baseTerm.argNames, baseTerm.argNames.size());
  std::vector<int> leftJoinKeys = baseTableNames->matchingJoinKeys(probeNames->fieldNames);
  std::vector<int> rightJoinKeys =  probeNames->matchingJoinKeys(baseTerm.argNames);

  /*
  for (uint k = 0; k < rightJoinKeys.size(); k++) {
    std::cout << "Right join keys " << rightJoinKeys.at(k) << " " << baseTableNames->toString() << "\n";
  }
  for (uint k = 0; k < leftJoinKeys.size(); k++) {
    std::cout << "Left join keys " << leftJoinKeys.at(k) << " " << probeNames->toString() << "\n";
    }*/

  if (leftJoinKeys.size() == 0 || rightJoinKeys.size() == 0) {
    std::cerr << "No matching join keys " << eventTerm.fn->name << " " << 
      baseTerm.fn->name << " " << currentRule->ruleID << "\n";
  }

  // add one to offset for table name
  int leftJoinKey = leftJoinKeys.at(0) + 1;
  int rightJoinKey = rightJoinKeys.at(0) + 1;

  TableRef probeTable = getTableByName(nodeID, baseTerm.fn->name);

  // first, figure out which are the matching keys on the probed table

  // should we use a uniqLookup or a multlookup? Check that the rightJoinKey is the primary key
  OL_Context::TableInfo* tableInfo = 
    _ctxt->getTableInfos()->find(baseTerm.fn->name)->second;
  
  if (tableInfo->primaryKeys.size() == 1 && tableInfo->primaryKeys.at(0) == rightJoinKey) {
    // use a unique index
    mostRecentElement =
      _conf->addElement(New refcounted< UniqueLookup >(strbuf("UniqueLookup:") << currentRule->ruleID << ":" << joinOrder 
						       << ":" << nodeID, probeTable,
						       leftJoinKey, rightJoinKey));
    std::cout << "Unique lookup " << currentRule->ruleID << " " << eventTerm.fn->name << " " << baseTerm.fn->name << " " 
	      << leftJoinKey << " " << rightJoinKey << "\n";

  } else {
    mostRecentElement =
      _conf->addElement(New refcounted< MultLookup >(strbuf("MultLookup:") << currentRule->ruleID << ":" << joinOrder 
						     << ":" << nodeID, probeTable,
						     leftJoinKey, rightJoinKey));
    
    addMultTableIndex(probeTable, rightJoinKey, nodeID);
    std::cout << "Mult lookup " << currentRule->ruleID << " " << 
     eventTerm.fn->name << " " << baseTerm.fn->name << " " 
	      << leftJoinKey << " " << rightJoinKey << "\n";
  }
  
  ElementSpecRef noNull = _conf->addElement(New refcounted< NoNullField >(strbuf("NoNull:") << currentRule->ruleID << ":" 
									  << joinOrder << ":" << nodeID, 1));
  
  hookUp(mostRecentElement, 0, noNull, 0);

  int numFieldsProbe = probeNames->fieldNames.size();
  std::cout << "Probe before merge " << probeNames->toString() << "\n";
  probeNames->mergeWith(baseTerm.argNames); 
  std::cout << "Probe after merge " << probeNames->toString() << "\n";

  if (_pendingRegisterReceiver) {
    // connecting to udp receiver later
    registerReceiver(_pendingReceiverTable, mostRecentElement);
    _pendingRegisterReceiver = false;
  } else {
    // connecting now
    hookUp(priorElement, 0, mostRecentElement, 0);  
  }

  mostRecentElement = noNull;

  /*mostRecentElement  = generatePrintElement(strbuf("PrintProbe1:") << 
					    currentRule->ruleID << ":" << joinOrder << ":"
					    << nodeID, mostRecentElement);*/

  // in case there are other join keys, we have to compare them using selections since we can join only 1 field
  // since we don't know the types, do a string compare
  std::cout << "Number of join selections " << leftJoinKeys.size()-1 << "\n";
  for (uint k = 1; k < leftJoinKeys.size(); k++) {
    int leftField = leftJoinKeys.at(k);
    int rightField = rightJoinKeys.at(k);
    strbuf selectionPel;
    selectionPel << "$0 " << (leftField+1) << " field " << " $1 " << rightField+1 
		 << " field ==s not ifstop $0 pop $1 pop";

    std::cout << "Join selections " << str(selectionPel) << "\n";
    ElementSpecRef joinSelections =
      _conf->addElement(New refcounted< PelTransform >(strbuf("JoinSelections:") << currentRule->ruleID << ":" 
						       << joinOrder << ":" << k << ":" << nodeID, 
						       str(selectionPel)));    
    hookUp(mostRecentElement, 0, joinSelections, 0);
    mostRecentElement = joinSelections;
  }

  mostRecentElement = generatePrintElement(strbuf("PrintProbe1:") << 
					   currentRule->ruleID << ":" << joinOrder << ":"
					   << nodeID, mostRecentElement);
 
  // Take the joined tuples and produce the resulting path
  // form the pel projection. 
  //Keep all fields on left, all fields on right except the join keys
  strbuf pelProject("\"join:");
  pelProject << eventTerm.fn->name << ":" << baseTerm.fn->name << ":" 
	     << currentRule->ruleID << ":" << nodeID << "\" pop "; // table name
  for (int k = 0; k < numFieldsProbe; k++) {
    pelProject << "$0 " << k+1 << " field pop ";
  }
  for (uint k = 0; k < baseTerm.argNames.size(); k++) {
    bool joinKey = false;
    for (uint j = 0; j < rightJoinKeys.size(); j++) {
      //std::cout << k << " " << rightJoinKeys.at(j) << "\n";
      if (k == (uint) rightJoinKeys.at(j)) { // add one for table name
	joinKey = true;
	break;
      }
    }
    if (!joinKey) {
      pelProject << "$1 " << k+1 << " field pop ";
    }
  }

  str pelProjectStr(pelProject);
  //std::cout << "Pel transform " << pelProjectStr << "\n";
  ElementSpecRef transS =
    _conf->addElement(New refcounted< PelTransform >(strbuf("JoinPelTransform:") << currentRule->ruleID << ":" 
						     << joinOrder << ":" << nodeID, 
						     pelProjectStr));

  delete baseTableNames;


  hookUp(mostRecentElement, 0, transS, 0);

  return generatePrintElement(strbuf("PrintProbe2:") << 
			      currentRule->ruleID << ":" << joinOrder << ":"
			      << nodeID, transS);
}


ElementSpecRef 
RouterConfigGenerator::generateJoinElements(OL_Context::Functor* currentFunctor, 
					    OL_Context::Rule* currentRule, 
					    str nodeID, 
					    FieldNamesTracker* namesTracker)
{
  // identify the events, use that to probe the other matching tables
  ElementSpecRef mostRecentElement = New refcounted<ElementSpec>(New refcounted<Slot>("dummySlotJoinElements"));

  OL_Context::Term eventTerm;
  std::vector<OL_Context::Term> selections; 
  std::vector<OL_Context::Term> baseTerms;
  bool eventFound = false;
  std::map<str, OL_Context::Term> staticTerms;
  for (unsigned int k = 0; k < currentRule->terms.size(); k++) {
    OL_Context::Term nextTerm = currentRule->terms.at(k);
    str termName = nextTerm.fn->name;
    if (isSelection(nextTerm)) {
      selections.push_back(nextTerm); 
      continue;
    }
    if (isAssignment(nextTerm)) {
      continue;
    }
    if (isAggregation(nextTerm)) {
      continue;
    }
    //baseTerms.push_back(nextTerm);

    if (nextTerm.fn->rules.size() == 0) {
      staticTerms.insert(std::make_pair(termName, nextTerm));
    }

    OL_Context::TableInfoMap::iterator _iterator = _ctxt->getTableInfos()->find(termName);
    if (_iterator != _ctxt->getTableInfos()->end()) {     
      baseTerms.push_back(nextTerm);
    } else {
      // FIXME: If not materialized, assume events for now.
      // WE ASSUME THERE IS ONLY ONE SUCH EVENT PER RULE!!      
      // We also assume event is fired by probing tables on the same node!
      registerReceiverTable(nextTerm.fn->name); 
      _pendingRegisterReceiver = true;
      _pendingReceiverTable = nextTerm.fn->name;
      eventTerm = nextTerm;
      eventFound = true;
    } 
  }

  if (eventFound == false) {
    std::cerr << "Cannot find event tuple. Performing multi-way joins in future!. \n";
    // is there a static table? If so, we can just use the other table(s) to probe
    // FIXME: Write the code for n-way joins after SOSP
    
    for (unsigned int k = 0; k < currentRule->terms.size(); k++) {
      OL_Context::Term nextTerm = currentRule->terms.at(k);
      str termName = nextTerm.fn->name;
      if (staticTerms.find(termName) == staticTerms.end()) {
	// pick the non-static term
	registerReceiverTable(nextTerm.fn->name); 
	_pendingRegisterReceiver = true;
	_pendingReceiverTable = nextTerm.fn->name;
	eventTerm = nextTerm;
	eventFound = true;	
	break;
      }        
    } 
  }
  std:: cout << "Event term " << eventTerm.fn->name << "\n";
  // for all the base tuples, use the join to probe. 
  // keep track also the current ordering of variables
  namesTracker->initialize(eventTerm.argNames, eventTerm.argNames.size());  
  for (uint k = 0; k < baseTerms.size(); k++) {    
    if (baseTerms.at(k).fn->name == eventTerm.fn->name) { continue; }
    std::cout << "Probing " << eventTerm.fn->name << " " << baseTerms.at(k).fn->name << "\n";
    mostRecentElement = generateProbeElements(currentRule, eventTerm, baseTerms.at(k), 
					      nodeID, namesTracker, mostRecentElement, k);
    //std::cout << "Current namesTracker " << namesTracker->toString() << "\n";
    // are there any more probes? If so, we have to change from pull to push
    //if (k != (baseTerms.size() - 1)) {
    ElementSpecRef pullPush =
      _conf->addElement(New refcounted< TimedPullPush >(strbuf("PullPush:") 
							<< currentRule->ruleID << ":" << nodeID,
							0));
    hookUp(mostRecentElement, 0, pullPush, 0);
    mostRecentElement = pullPush;
      //}
  }

  return mostRecentElement;
}


void RouterConfigGenerator::addMultTableIndex(TableRef table, int fn, str nodeID)
{
  strbuf uniqStr = str(table->name);
  uniqStr << ":" << fn << ":" << nodeID;
  if (_multTableIndices.find(str(uniqStr)) == _multTableIndices.end()) {
    // not there yet
    table->add_multiple_index(fn);
    _multTableIndices.insert(std::make_pair(str(uniqStr), str(uniqStr)));
    std::cout << "Mult index already added " << str(uniqStr) << "\n";
  } else {
    std::cerr << "Mult index already exists " << str(uniqStr) << "\n";
  }
}

void
RouterConfigGenerator::generateSingleAggregateElements(OL_Context::Functor* currentFunctor, 
						       OL_Context::Rule* currentRule, 
						       str nodeID, 
						       FieldNamesTracker* currentNamesTracker)
{
  OL_Context::Term baseTerm, aggTerm;
  // figure first, which term is the base term
  for (unsigned int j = 0; j < currentRule->terms.size(); j++) {
    // skip those that we already decide is going to participate in     
    OL_Context::Term currentTerm = currentRule->terms.at(j);    
    str tableName = currentTerm.fn->name;
    
    // skip funcitons
    if (isSelection(currentTerm)) { continue; } // skip functions
    if (isAssignment(currentTerm)) { continue; } // skip assignments
    if (isAggregation(currentTerm)) { 
      // figure out what kind of agg this is
      aggTerm = currentTerm;
    } else {
      baseTerm = currentTerm;
    }
  }

  currentNamesTracker->initialize(baseTerm.argNames, baseTerm.argNames.size());

  std::vector<unsigned int> groupByFields;      
  
  // once we obtain the base term, figure out which are the fields needed for aggregation
  // project only the necessary ones
  // iterate through all functor's output, project all, except the aggregate field itself
  FieldNamesTracker* newFieldNamesTracker = new FieldNamesTracker();
  for (unsigned int k = 0; k < currentFunctor->arity; k++) {
    int pos = currentNamesTracker->fieldPosition(currentRule->args.at(k));
    if (pos != -1) {
      groupByFields.push_back((uint) pos + 1);
      newFieldNamesTracker->fieldNames.push_back(currentNamesTracker->fieldNames.at(pos));
    }
  }
  int aggField = -1;
  Table::AggregateFunction* af = 0;
  
  if (aggTerm.fn->name == "agg_min") {
    // aggregate for min
    aggField = currentNamesTracker->fieldPosition(aggTerm.argNames.at(1)) + 1;
    af = &Table::AGG_MIN;
  } 
  if (aggTerm.fn->name == "agg_max") {
    // aggregate for min
    aggField = currentNamesTracker->fieldPosition(aggTerm.argNames.at(1)) + 1;
    af = &Table::AGG_MAX;
  } 

  if (aggTerm.fn->name == "agg_count") {
    // aggregate for min
    aggField = groupByFields.at(0);
    af = &Table::AGG_COUNT;
  } 
  
  // get the table, create the index
  TableRef aggTable = getTableByName(nodeID, baseTerm.fn->name);
  /*std::cout << "Obtain base term " << baseTerm.fn->name << " " << aggTerm.fn->name << " " 
    << groupByFields.at(0) << " " << aggField << " " << " for aggregation \n";*/
  
  addMultTableIndex(aggTable, groupByFields.at(0), nodeID);
  
  Table::MultAggregate tableAgg 
    = aggTable->add_mult_groupBy_agg(groupByFields.at(0), // groupby field
				     groupByFields,
				     aggField, // the agg field
				     af);
  ElementSpecRef aggElement =
    _conf->addElement(New refcounted< Aggregate >(strbuf("Agg:") << currentRule->ruleID << ":" << nodeID,
						  tableAgg));
  
  ElementSpecRef printAgg = 
    _conf->addElement(New refcounted< Print >("PrintAgg:" << 
					      currentRule->ruleID << ":" << nodeID));
 
  // send back
  newFieldNamesTracker->fieldNames.push_back(aggTerm.argNames.at(0));

  strbuf pelTransformStr;
  pelTransformStr << "\"" << "aggResult:" << currentRule->ruleID << "\" pop";
  for (uint k = 0; k < newFieldNamesTracker->fieldNames.size(); k++) {
    pelTransformStr << " $" << k << " pop";
  }
  //std::cout << "Pel Transform str " << str(pelTransformStr) << "\n";
  // apply PEL to add a table name
  ElementSpecRef addTableName =
    _conf->addElement(New refcounted< PelTransform >(strbuf("Aggregation:") 
						     << currentRule->ruleID << ":" 
						     << nodeID, 
						     pelTransformStr));

  ElementSpecRef timedPullPush = 
    _conf->addElement(New refcounted<TimedPullPush>("timedPullPush:" << 
						    currentRule->ruleID 
						    << ":" << nodeID, 0));
  
  hookUp(aggElement, 0, addTableName, 0);
  hookUp(addTableName, 0, printAgg, 0);
  hookUp(printAgg, 0, timedPullPush, 0);
  ElementSpecRef projectHeadElement = generateProjectHeadElements(currentFunctor, currentRule, 
								  nodeID, newFieldNamesTracker, 
								  timedPullPush);
  ElementSpecRef mostRecentElement = generateSendMarshalElements(currentRule, nodeID, 
								 currentFunctor->arity, projectHeadElement);

  _udpPushSenders.push_back(mostRecentElement);

  // FIXME: Selections, assignments, better integration with the rest

  // not done yet, we also need to register a table receiver for the head
  str headTableName = currentFunctor->name;
  registerReceiverTable(headTableName);
  // link that to a discard. If there are other flows that need it, they will
  // get the tuples via another fork in the duplicator.
  ElementSpecRef sinkS = _conf->addElement(New refcounted< Discard >("discard"));    
  registerReceiver(headTableName, sinkS);
}


void RouterConfigGenerator::generateSingleTermElement(OL_Context::Functor* currentFunctor, 
						      OL_Context::Rule* currentRule, 
						      str nodeID, 
						      FieldNamesTracker* currentNamesTracker)
{
  
  ElementSpecRef resultElement = New refcounted<ElementSpec>(New refcounted<Slot>("dummySlotSingleTerm"));
  OL_Context::Term currentTerm;
  // if not, form basic scanners, then connect to selections, and then to form head the last

  for (unsigned int j = 0; j < currentRule->terms.size(); j++) {
    // skip those that we already decide is going to participate in     
    currentTerm = currentRule->terms.at(j);    
    str tableName = currentTerm.fn->name;
    // skip funcitons
    if (isSelection(currentTerm)) { continue; } // skip functions
    if (isAssignment(currentTerm)) { continue; } // skip functions
    if (isAggregation(currentTerm)) { continue; } // skip functions
    registerReceiverTable(currentTerm.fn->name); //generateReceiveUnmarshalElements(currentFunctor, currentRule, currentTerm, nodeID);	
    _pendingRegisterReceiver = true;
    _pendingReceiverTable = currentTerm.fn->name;
    break; // break at first opportunity. Assume there is only one term   
  }

  // with knowledge of on which is a scan term, we can do the pel transform magic now
  // first, figure out which field is the results. Then figure out which fields need to be sent
  currentNamesTracker->initialize(currentTerm.argNames, currentTerm.argNames.size());
}


ElementSpecRef 
RouterConfigGenerator::generateDupElimElement(str header, ElementSpecRef connectingElement)
{
   if (_dups) {
     ElementSpecRef dupElim = _conf->addElement(New refcounted< DupElim >(header));

    hookUp(connectingElement, 0, dupElim, 0);
    return dupElim;
  }
  return connectingElement;
}


ElementSpecRef 
RouterConfigGenerator::generatePrintElement(str header, ElementSpecRef connectingElement)
{
  if (_debug) {
    ElementSpecRef print = 
      _conf->addElement(New refcounted< PrintTime >(header));
    hookUp(connectingElement, 0, print, 0);
    return print;
  }
  return connectingElement;
}


 // Unused 
ElementSpecRef 
RouterConfigGenerator::createScanElements(OL_Context::Functor* currentFunctor, 
					  OL_Context::Rule* rule,
					  OL_Context::Term term, 
					  OL_Context::TableInfo* tableInfo, 
					  str nodeID)
{
  // if we want to debug, have to create print elements. For now, assume we debug
  // get the table to create the scanner over.
  // in future, this scanner may be replaced by a push-based scanner that outputs
  // whenever there is an incoming tuple that is newly added/updated.
  // in future, keep track of all scanners, and we may be able to combile them

  ElementSpecRef dummyElement = New refcounted<ElementSpec>(New refcounted<Slot>("dummySlotScanElements"));
  TableRef baseTable = getTableByName(nodeID, tableInfo->tableName);

  str ruleStr = rule->ruleID;
  // by default, scan from the first
  ElementSpecRef scanS =
    _conf->addElement(New refcounted< Scan >(strbuf("Scanner:") << ruleStr << ":" << nodeID, baseTable, 1));

  ElementSpecRef timedPullPushS =
    _conf->addElement(New refcounted< TimedPullPush >(strbuf("ScanPush:") << ruleStr << ":" << nodeID, 1));

  // only if we use the duplicate elimination operator
  ElementSpecRef dupRemove =
    _conf->addElement(New refcounted< DupElim >(strbuf("ScanDupElim:") << ruleStr << ":" << nodeID));
 
  if (_debug) {
    ElementSpecRef scanPrint1 =
      _conf->addElement(New refcounted< Print >(strbuf("PrintScan:") << ruleStr << ":" << nodeID));
    hookUp(scanS, 0, scanPrint1, 0);  
    hookUp(scanPrint1, 0, timedPullPushS, 0); 
    hookUp(timedPullPushS, 0, dupRemove, 0); // only necessary if dup elimination
    return dupRemove;
  }

  hookUp(scanS, 0, timedPullPushS, 0);  
  hookUp(timedPullPushS, 0, dupRemove, 0); // only necessary if dup elimination
  return dupRemove;
}





/////////////////////////////////////////////////////////////////////////////


////////////////////////// Table Creation ////////////////////////////////////////

// Get a handle to the table. Typically used by the driver program to 
// preload some data.
TableRef RouterConfigGenerator::getTableByName(str nodeID, str tableName)
{
  TableMap::iterator _iterator = _tables.find(nodeID << ":" << tableName);
  return _iterator->second;
}

void RouterConfigGenerator::createTables(str nodeID)
{
  // have to decide where joins are possibly performed, and on what fields
  // create appropriate indices for them
  OL_Context::TableInfoMap::iterator _iterator;
  for (_iterator = _ctxt->getTableInfos()->begin(); 
       _iterator != _ctxt->getTableInfos()->end(); _iterator++) {
    OL_Context::TableInfo* tableInfo = _iterator->second;
    // create the table, add the unique local name, store in hash table

    if (tableInfo->timeout != 0) { 
      // if timeout is zero, table is never materialized 
      size_t tableSize;
      if (tableInfo->size != -1) {
	tableSize = tableInfo->size;
      } else {
	tableSize = UINT_MAX; // consider this infinity
      }
      str newTableName = nodeID << ":" << tableInfo->tableName;
      TableRef newTable = New refcounted< Table> (tableInfo->tableName, tableInfo->size);

      // FIXME: Create multiple-indexes for just the first field for now.
      // in future, need more sophisticated detection of mult/unique indices.
      
      // first create unique indexes
      std::vector<int> primaryKeys = tableInfo->primaryKeys;
      for (uint k = 0; k < primaryKeys.size(); k++) {
	newTable->add_unique_index(primaryKeys.at(k));
	std::cout << "Add unique index " << newTableName << " " << primaryKeys.at(k) << "\n";
      }

      //newTable->add_multiple_index(1); 
      _tables.insert(std::make_pair(newTableName, newTable));      
    }
  }
}

/////////////////////////////////////////////

///////////// Helper functions

str RouterConfigGenerator::getRuleStr(OL_Context::Functor* currentFunctor, OL_Context::Rule* currentRule)
{
  strbuf ret("<Rule: ");
  ret << currentFunctor->name << "@" << currentFunctor->loc << "( ";

  for (unsigned int k = 0; k < currentFunctor->arity; k++) {
    ret << currentRule->args.at(k) << " ";
  }
  return ret << ") " << currentRule->toString() << ">";  
}


void RouterConfigGenerator::hookUp(ElementSpecRef firstElement, int firstPort,
				   ElementSpecRef secondElement, int secondPort)
{
  fprintf(_output, "Connect: \n");
  fprintf(_output, "  %s %s %d\n", firstElement->toString().cstr(), 
	  firstElement->element()->_name.cstr(), firstPort);
  fprintf(_output, "  %s %s %d\n", secondElement->toString().cstr(), 
	  secondElement->element()->_name.cstr(), secondPort);
  fflush(_output);

  // in future, if push/pull, have to put a slot in between. So we don't have to check

  _conf->hookUp(firstElement, firstPort, secondElement, secondPort);
}

void RouterConfigGenerator::setJoinKeys(OL_Context::Rule* rule, std::vector<JoinKey>* joinKeys)
{

  // enumerate through all the terms, ignore functions 
  // ok, this is terribly inefficient for now.
  for (uint k = 0; k < rule->terms.size(); k++) {
   OL_Context::Term firstTerm = rule->terms.at(k);
   str firstName(firstTerm.fn->name);
   if (isSelection(firstTerm)) { continue; }
   if (isAssignment(firstTerm)) { continue; }
   if (isAggregation(firstTerm)) { continue; }
   std::vector<str> firstArgNames = firstTerm.argNames;
    for (uint j = k+1; j < rule->terms.size(); j++) {
      OL_Context::Term secondTerm = rule->terms.at(j);
      str secondName(secondTerm.fn->name);
      if (isSelection(secondTerm)) { continue; }
      if (isAssignment(secondTerm)) { continue; }
      if (isAggregation(secondTerm)) { continue; }
      std::vector<str> secondArgNames = secondTerm.argNames;
      
      for (uint x = 0; x < firstArgNames.size(); x++) {	
	for (uint y = 0; y < secondArgNames.size(); y++) {
	  if (firstArgNames.at(x) == secondArgNames.at(y)) {
	    //std::cout << "Set join keys " << firstTerm.fn->name << " " << firstArgNames.at(x) << 
	    // " " << secondTerm.fn->name << " " << secondArgNames.at(y) << "\n";
	    JoinKey joinKey(firstName, firstArgNames.at(x), 
			    secondName, secondArgNames.at(y));
	    joinKeys->push_back(joinKey);	    
	  }
	}
      }
    }
  }
}


///////////////////////////////////////////////////////////////////////////

RouterConfigGenerator::FieldNamesTracker::FieldNamesTracker() { }

RouterConfigGenerator::FieldNamesTracker::FieldNamesTracker(std::vector<str> argNames, int arity)
{
  for (int k = 0; k < arity; k++) {
    fieldNames.push_back(argNames.at(k));
  }
}

void RouterConfigGenerator::FieldNamesTracker::initialize(std::vector<str> argNames, int arity)
{
  for (int k = 0; k < arity; k++) {
    fieldNames.push_back(argNames.at(k));
  }
}


std::vector<int> RouterConfigGenerator::FieldNamesTracker::matchingJoinKeys(std::vector<str> otherArgNames)
{
  // figure out the matching on other side. Assuming that
  // there is only one matching key for now
  std::vector<int> toRet;
  for (unsigned int k = 0; k < otherArgNames.size(); k++) {
    str nextStr = otherArgNames.at(k);
    if (fieldPosition(nextStr) != -1) {
      // exists
      toRet.push_back(k);
    }
  }  
  return toRet;
}

int RouterConfigGenerator::FieldNamesTracker::fieldPosition(str var)
{
  for (uint k = 0; k < fieldNames.size(); k++) {
    if (fieldNames.at(k) == var) {
      return k;
    }
  }
  return -1;
}

void RouterConfigGenerator::FieldNamesTracker::mergeWith(std::vector<str> names)
{
  for (uint k = 0; k < names.size(); k++) {
    str nextStr = names.at(k);
    if (fieldPosition(nextStr) == -1) {
      fieldNames.push_back(nextStr);
    }
  }
}

void RouterConfigGenerator::FieldNamesTracker::mergeWith(std::vector<str> names, int numJoinKeys)
{
  int count = 0;
  for (uint k = 0; k < names.size(); k++) {
    str nextStr = names.at(k);
    if (count == numJoinKeys || fieldPosition(nextStr) == -1) {
      fieldNames.push_back(nextStr);
      count++;
    }
  }
}

str RouterConfigGenerator::FieldNamesTracker::toString()
{
  strbuf toRet("FieldNamesTracker<");
  
  for (uint k = 0; k < fieldNames.size(); k++) {
    toRet << fieldNames.at(k) << " ";
  }
  return toRet << ">";
}
