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

#include "rtr_confgen.h"
#include "trace.h"
const str Rtr_ConfGen::SEL_PRE("select_");
const str Rtr_ConfGen::AGG_PRE("agg_");
const str Rtr_ConfGen::ASSIGN_PRE("assign_");
const str Rtr_ConfGen::TABLESIZE("TABLESIZE");


Rtr_ConfGen::Rtr_ConfGen(OL_Context* ctxt, Router::ConfigurationRef conf, 
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
  pelFunctions.insert(std::make_pair(ASSIGN_PRE << "minusOneID", "--id"));
  pelFunctions.insert(std::make_pair(ASSIGN_PRE << "addID", "+id"));
  pelFunctions.insert(std::make_pair(ASSIGN_PRE << "addI", "+i"));
  pelFunctions.insert(std::make_pair(ASSIGN_PRE << "distID", "distance"));
  pelFunctions.insert(std::make_pair(ASSIGN_PRE << "random", "rand"));
  pelFunctions.insert(std::make_pair(ASSIGN_PRE << "idOne", "1 ->u32 ->id"));
  pelFunctions.insert(std::make_pair(ASSIGN_PRE << "leftShiftID", "<<id"));
  pelFunctions.insert(std::make_pair(ASSIGN_PRE << "varAssign", ""));
  pelFunctions.insert(std::make_pair(ASSIGN_PRE << "modI", "%"));
  pelFunctions.insert(std::make_pair(ASSIGN_PRE << "rangeID3", "(]id"));
  pelFunctions.insert(std::make_pair(ASSIGN_PRE << "iZero", "0"));
  pelFunctions.insert(std::make_pair(ASSIGN_PRE << "concatS", "strcat"));

  _pendingRegisterReceiver = false;
}


Rtr_ConfGen::~Rtr_ConfGen()
{
  fclose(_output);
}


// call this for each udp element that we wish to hook up the same dataflow
// if running only one, nodeID is the local host name,
void Rtr_ConfGen::configureRouter(ref< Udp > udp, str nodeID)
{
  // first create the tables if they are not created yet.
  // iterate through all the table info
  
  // iterate through all the functors (heads, unique by name, arity, location)
  OL_Context::FunctorMap::iterator _iterator;
  for (_iterator = _ctxt->getFunctors()->begin(); 
       _iterator != _ctxt->getFunctors()->end(); _iterator++) {
    OL_Context::Functor* nextFunctor = _iterator->second;
    processFunctor(nextFunctor, nodeID);
  }

  if (_udpPushSenders.size()) { 
    genSendElements(udp, nodeID);
  }

  _currentElementChain.clear();
  if (_udpReceivers.size() > 0) {
    genReceiveElements(udp, nodeID);
  }
}

void Rtr_ConfGen::clear()
{
  _udpReceivers.clear();
  _udpPushSenders.clear();
}


bool Rtr_ConfGen::hasSingleAggTerm(OL_Context::Rule* curRule)
{
  int numAggs = 0;
  for (uint k = 0; k < curRule->terms.size(); k++) {
    if (isAggregation(curRule->terms.at(k))) {
      numAggs ++;
    }
  }
  return (numAggs == 1);
}

bool Rtr_ConfGen::hasPeriodicTerm(OL_Context::Rule* curRule)
{
  for (uint k = 0; k < curRule->terms.size(); k++) {
    if (curRule->terms.at(k).fn->name == "periodic") {
      return true;
    }
  }
  return false;
}

void Rtr_ConfGen::processFunctor(OL_Context::Functor* fn, str nodeID)
{
  // The cur translation proceeds as follows. 
  // For each functor, we enumerate through all the rules.
  // For each rule, 
  // 1) First, identify all terms where we need to create listeners. They are either
  //    a) Single Tail Terms
  //    b) Event terms used for probing other tables
  // 2) If joins are needed, identify the event term that is used to probe the others.
  // 3) Apply selections
  // 4) Apply projections, address to send (based on head)
  // 5) Stick muxers and demuxers accordingly
  
  for (unsigned int k = 0; k < fn->rules.size(); k++) {
    OL_Context::Rule* r = fn->rules.at(k);

    processRule(r, fn, nodeID);
  }
}

void Rtr_ConfGen::processRule(OL_Context::Rule *r, 
			      OL_Context::Functor *fn,
			      str nodeID)
{
  std::cout << "Process rule " << r->ruleID << "\n";
  // first, get a list of possible unifications needed
  std::vector<JoinKey> joinKeys;
  setJoinKeys(r, &joinKeys);    
  FieldNamesTracker curNamesTracker;
  ptr<Aggwrap> agg_el = NULL;

  _pendingReceiverSpec = NULL;
  // Do we need an aggregation element to wrap around this?
  // support multiple aggregates next time
  if (r->aggField >= 0) {
    // add 1 for position, add one for table name
    agg_el = New refcounted<Aggwrap>(r->aggFn, r->aggField + 2);
  }
  
  _currentElementChain.clear();
  
  if (hasSingleAggTerm(r)) { 
    // has a single agg on a single materialized table. 
    // Merge this with the other kind of agg
    genSingleAggregateElements(fn, r, nodeID, &curNamesTracker);
    return;
  }
  
  if (hasPeriodicTerm(r)) {
    // generate a functor source at at every period
    genFunctorSource(fn, r, nodeID);
  } else {
    if (joinKeys.size() == 0) {
      // if there are no possible unifications, we process only the first term
      genSingleTermElement(fn, r, nodeID, &curNamesTracker);
    } else {
      genJoinElements(fn, r, nodeID, &curNamesTracker, agg_el);
    }
    
    // do the selections and assignment, followed by projection
    genAllSelectionElements(r, nodeID, &curNamesTracker);    
    genAllAssignmentElements(r, nodeID, &curNamesTracker);
    std::cout << "Pending register receiver " << _pendingRegisterReceiver << "\n";
    genProjectHeadElements(fn, r, nodeID, &curNamesTracker);
  }

  
  // generate the elements for the output
  if (r->deleteFlag == true) {
    debugRule(r, str(strbuf() << "Delete " << fn->name << " for rule \n"));
    // And send it for deletion. Assume deletion happens here. 
    // It may happen on another node in practice. FIXME in future.
    TableRef tableToDelete = getTableByName(nodeID, fn->name);
    OL_Context::TableInfo* ti = _ctxt->getTableInfos()->find(fn->name)->second;
    
    genPrintElement(strbuf("PrintBeforeDelete:") << r->ruleID << nodeID);
    genPrintWatchElement(strbuf("PrintWatchDelete:") << r->ruleID << nodeID);

    ElementSpecRef deleteElement =
      _conf->addElement(New refcounted< Delete >(strbuf("Delete:") << r->ruleID << nodeID,
						 tableToDelete, 
						 ti->primaryKeys.at(0), 						   
						 ti->primaryKeys.at(0)));    
    // chain up
    hookUp(deleteElement, 0);

    if (_pendingReceiverSpec) {
      registerReceiver(_pendingReceiverTable, _pendingReceiverSpec);
    }
    return; // discard. deleted tuples not sent anywhere
  } else {    
    if (agg_el) { 
      ElementSpecRef agg_spec = _conf->addElement(agg_el);
      hookUp(agg_spec, 1); // hook up the internal output
      hookUp(agg_spec, 1, _pendingReceiverSpec, 0); // hookup the internal input
      _pendingReceiverSpec = agg_spec; // hook the agg_spect to the front later by receivers
      _currentElementChain.push_back(agg_spec);
    } 
    // send it!
    genSendMarshalElements(r, nodeID, fn->arity);         
    
    // at the receiver side:
    str headTableName = fn->name;
    registerReceiverTable(headTableName);
    ElementSpecRef sinkS = _conf->addElement(New refcounted< Discard >("discard"));    
    registerReceiver(headTableName, sinkS);
  }

  if (_pendingReceiverSpec) {
    registerReceiver(_pendingReceiverTable, _pendingReceiverSpec);
  }

  // anything at this point needs to be hookup with senders
  _udpPushSenders.push_back(_currentElementChain.back()); 
}



//////////////////// Transport layer //////////////////////////////////////////
void Rtr_ConfGen::genReceiveElements(ref< Udp> udp, str nodeID)
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
 
  genPrintElement(strbuf("PrintReceivedBeforeDemux:") << nodeID);
  genDupElimElement(strbuf("ReceiveDupElimBeforeDemux:") << nodeID); 
  genPrintWatchElement(strbuf("PrintWatchReceiveBeforeDemux:") << nodeID);

  ElementSpecRef bufferQueue = 
    _conf->addElement(New refcounted< Queue >(strbuf("ReceiveQueue:") << 
					      nodeID, 1000));
  ElementSpecRef pullPush = 
    _conf->addElement(New refcounted<TimedPullPush>("ReceiveQueuePullPush", 0));

  hookUp(bufferQueue, 0);
  hookUp(pullPush, 0);
  hookUp(demuxS, 0);

  // connect to all the pending udp receivers. Assume all 
  // receivers are connected to elements with push input ports for now
  int counter = 0;
  for (_iterator = _udpReceivers.begin(); _iterator != _udpReceivers.end(); _iterator++) {
    ReceiverInfo ri = _iterator->second;
    int numElementsToReceive = ri._receivers.size(); // figure out the number of receivers
    str tableName = ri._name;

    std::cout << "Generate Receive: Demuxing " << tableName << " for " << numElementsToReceive << " elements\n";

    // DupElim -> DemuxS -> Insert -> Duplicator -> Fork

    ElementSpecRef bufferQueue = 
      _conf->addElement(New refcounted< Queue >(strbuf("DemuxQueue:") << 
						nodeID << ":" << tableName, 1000));
    ElementSpecRef pullPush = 
      _conf->addElement(New refcounted<TimedPullPush>("DemuxQueuePullPush" << nodeID << ":" << tableName, 0));
    hookUp(demuxS, counter++, bufferQueue, 0);
    hookUp(bufferQueue, 0, pullPush, 0);
    

    // duplicator
    ElementSpecRef duplicator = 
      _conf->addElement(New refcounted< DuplicateConservative >(strbuf("DuplicateConservative:") 
								<< tableName << ":" << nodeID, 
								numElementsToReceive));    
    // materialize table only if it is declared and has lifetime>0
    OL_Context::TableInfoMap::iterator _iterator = _ctxt->getTableInfos()->find(tableName);
    if (_iterator != _ctxt->getTableInfos()->end() && _iterator->second->timeout != 0) {
      ElementSpecRef insertS 
	= _conf->addElement(New refcounted< Insert >(strbuf("Insert:") << 
						     tableName << ":" << nodeID,  
						     getTableByName(nodeID, tableName)));

      hookUp(pullPush, 0, insertS, 0);
      genPrintWatchElement(strbuf("PrintWatchInsert:") << nodeID);

      hookUp(duplicator, 0);
    } else {
      hookUp(pullPush, 0, duplicator, 0);
    }

    // connect the duplicator to elements for this name
    for (uint k = 0; k < ri._receivers.size(); k++) {
      ElementSpecRef nextElementSpec = ri._receivers.at(k);

      if (_debug) {
	ElementSpecRef printDuplicator = 
	  _conf->addElement(New refcounted< PrintTime >(strbuf("PrintAfterDuplicator:"
							       << tableName << ":" << nodeID)));
	hookUp(duplicator, k, printDuplicator, 0);
	hookUp(printDuplicator, 0, nextElementSpec, 0);
	continue;
      }
      hookUp(duplicator, k, nextElementSpec, 0);
    }
  }

  ElementSpecRef sinkS = _conf->addElement(New refcounted< Discard >("discard"));
  hookUp(demuxS, _udpReceivers.size(), sinkS, 0); // if we don't know where this should go
}


void Rtr_ConfGen::registerUDPPushSenders(ElementSpecRef elementSpecRef)
{
  _udpPushSenders.push_back(elementSpecRef);
}


void Rtr_ConfGen::genSendElements(ref< Udp> udp, str nodeID)
{
  ElementSpecRef udpSend = _conf->addElement(udp->get_tx());  

  // prepare to send. Assume all tuples send by first tuple
  ElementSpecRef muxS =
    _conf->addElement(New refcounted< RoundRobin >("sendMux:" << nodeID, 
						   _udpPushSenders.size())); 

  hookUp(muxS, 0, udpSend, 0);

  // form the push senders
  for (unsigned int k = 0; k < _udpPushSenders.size(); k++) {
    ElementSpecRef nextElementSpec = _udpPushSenders.at(k);

    ElementSpecRef pushToPull = 
      _conf->addElement(New refcounted< Queue >(strbuf("SendQueue:") << 
					       nodeID << ":" << k, 1000));
    hookUp(nextElementSpec, 0, pushToPull, 0);
    hookUp(pushToPull, 0, muxS, k);
  }
}

void Rtr_ConfGen::genFunctorSource(OL_Context::Functor* functor, OL_Context::Rule* rule, str nodeID)
{
  TupleRef functorTuple = Tuple::mk();
  functorTuple->append(Val_Str::mk(functor->name));
  functorTuple->append(Val_Str::mk(nodeID)); // address for marshalling
  functorTuple->append(Val_Str::mk(nodeID)); 
  functorTuple->freeze();

  ElementSpecRef source =
    _conf->addElement(New refcounted< TupleSource >(str("FunctorSource:") << rule->ruleID << nodeID,
                                                   functorTuple));
  _currentElementChain.push_back(source);

  genPrintElement(strbuf("PrintFunctorSource:") << rule->ruleID << ":" << nodeID);
  str firstVar = rule->terms.at(0).argNames.at(0);
  // The timed pusher
  ElementSpecRef pushFunctor =
    _conf->addElement(New refcounted< TimedPullPush >(strbuf("FunctorPush:") << rule->ruleID << ":" << nodeID,
						      atof(firstVar.cstr())));

  hookUp(pushFunctor, 0);
}


// create a send element where the first field after table name is the address. Drop that field
void Rtr_ConfGen::genSendMarshalElements(OL_Context::Rule* rule, str nodeID, int arity)
{
  genDupElimElement(strbuf("SendDupElim:") << rule->ruleID << ":" << nodeID);

  genPrintWatchElement(strbuf("PrintWatchSend:") << 
		       rule->ruleID << ":" << nodeID);

   
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
						     "$1 pop " << marshalPelStr)); // the rest
  hookUp(encapS, 0);

  genPrintElement(strbuf("PrintSend:") << 
		  rule->ruleID << ":" << nodeID);

  // Now marshall the payload (second field)
  ElementSpecRef marshalS = 
    _conf->addElement(New refcounted< MarshalField >("marshal:" << 
						     rule->ruleID << ":" << nodeID, 1));
  
  // And translate the address into a sockaddr.  This is a hack until
  // we have a consistent view of the UDP interface.
  ElementSpecRef routeS =
    _conf->addElement(New refcounted< StrToSockaddr >("router:" << rule->ruleID 
						      << ":" << nodeID, 0));

  hookUp(marshalS, 0);
  hookUp(routeS, 0);
}

// for a particular table name that we are receiving, register an elementSpec that needs 
// that data
void Rtr_ConfGen::registerReceiver(str tableName, ElementSpecRef elementSpecRef)
{
  // add to the right receiver
  ReceiverInfoMap::iterator _iterator = _udpReceivers.find(tableName);
  if (_iterator != _udpReceivers.end()) {
    _iterator->second.addReceiver(elementSpecRef);
  }
}



// regiser a new receiver for a particular table name
// use to later hook up the demuxer
void Rtr_ConfGen::registerReceiverTable(str tableName)
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

bool Rtr_ConfGen::isSelection(OL_Context::Term term)
{
  str termName1 = term.fn->name;
  str termName2 = term.fn->name;
  return substr(termName1, 0, SEL_PRE.len()) == SEL_PRE || 
    substr(termName2, 1, SEL_PRE.len()) == SEL_PRE;  
}

bool Rtr_ConfGen::isAssignment(OL_Context::Term term)
{
  str termName = term.fn->name;
  return substr(termName, 0, ASSIGN_PRE.len()) == ASSIGN_PRE;
}


bool Rtr_ConfGen::isAggregation(OL_Context::Term term)
{
  str termName = term.fn->name;
  return substr(termName, 0, AGG_PRE.len()) == AGG_PRE;
}

void Rtr_ConfGen::genSelectionElements(OL_Context::Rule* curRule, 
				       OL_Context::Term nextSelection, 
				       str nodeID, 
				       FieldNamesTracker* probeNames,
				       int selectionID)
{
  // Prepend with true if this is a Reach X, X.
  str termName = nextSelection.fn->name;
  bool negation = false;
  if (substr(termName, 0, 1) == "!") {
    negation = true;
    termName = substr(termName, 1, termName.len()-1);
  } 

  if (pelFunctions.find(termName) == pelFunctions.end()) { return; } // skip this selection
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

  debugRule(curRule, str(strbuf() << "Generate selection functions for " << str(selectionPel) 
			 << " " << probeNames->toString() << "\n"));
 
  ElementSpecRef selection =
    _conf->addElement(New refcounted< PelTransform >(strbuf("Selection:") 
						     << curRule->ruleID << ":" 
						     << selectionID << ":" << nodeID, 
						     selectionPel));

  if (_pendingRegisterReceiver) {
    _pendingReceiverSpec = selection;
    _pendingRegisterReceiver = false;
  } else {
    // connecting now
    hookUp(selection, 0);
  }

  genPrintElement(strbuf("PrintAfterSelection:") 
		  << curRule->ruleID << ":" 
		  << selectionID << ":" << nodeID);
}


void Rtr_ConfGen::genAllSelectionElements(OL_Context::Rule* curRule,
					  str nodeID,
					  FieldNamesTracker* curNamesTracker) 
{
  for (unsigned int j = 0; j < curRule->terms.size(); j++) {
    OL_Context::Term nextTerm = curRule->terms.at(j);
    str termName = nextTerm.fn->name;
    if (isSelection(nextTerm)) {
      debugRule(curRule, str(strbuf() << "Selection term " << termName << " " << curRule->ruleID << "\n"));
      genSelectionElements(curRule, nextTerm, nodeID, curNamesTracker, j); 
    }
  }
}



void Rtr_ConfGen::genAssignmentElements(OL_Context::Rule* curRule,
					OL_Context::Term curTerm, 
					str nodeID,
					FieldNamesTracker* curNamesTracker,
					int assignmentID) 
{
  str termName = curTerm.fn->name;
  
  // get the assignment str
  if (pelFunctions.find(termName) == pelFunctions.end()) {
    // cannot find this 
    std::cerr << "Invalid assignment term " << termName << "\n";
    return;
  }
  strbuf pelStr;
  for (uint k = 0; k < curNamesTracker->fieldNames.size()+1; k++) {
    pelStr << "$" << k << " pop ";
  }
  
  // do the pel portion. First add the extra variables in
  
  str pelExpression = pelFunctions.find(termName)->second;;
  
  // go through the arguments in the assignment
  bool fail = false;
  for (uint k = 1; k < curTerm.argNames.size(); k++) {
    str curTermArgName = curTerm.argNames.at(k);
    
    // is this an var or a val?
    if (curTerm.args.at(k).var == -1) {
      pelStr << curTerm.args.at(k).val->toString() << " ";
    }        
    else if (substr(curTermArgName, 0, TABLESIZE.len()) == TABLESIZE) {
      // table size keyword
      str tableName = substr(curTerm.argNames.at(k), TABLESIZE.len()+1, 
			     curTerm.argNames.at(k).len()-TABLESIZE.len()-1);
      if (_ctxt->getTableInfos()->find(tableName) == _ctxt->getTableInfos()->end()) {
	fail = true;
	std::cerr << tableName << " cannot be found " << curRule->ruleID << "\n";
	break;
      }
      OL_Context::TableInfo* tableInfo = _ctxt->getTableInfos()->find(tableName)->second;
      pelStr << tableInfo->size << " ";	  
    } else {
      if (curNamesTracker->fieldPosition(curTerm.argNames.at(k)) == -1) {
	fail = true;
	std::cerr <<curTerm.argNames.at(k) << " is not a valid variable for " << 
	  termName << " in " << curRule->ruleID << "\n";
	break;
      }
      pelStr << "$" << (1 + curNamesTracker->fieldPosition(curTerm.argNames.at(k))) << " ";
    }
  }
  if (fail) { 
    // we cannot assign
    return; 
  }
  
  pelStr << pelExpression << " pop";
  curNamesTracker->fieldNames.push_back(curTerm.argNames.at(0));
  
  debugRule(curRule, str(strbuf() << "Generate assignments for " << termName << " " << curRule->ruleID << 
			 " " << str(pelStr) << " " << curNamesTracker->toString() << "\n"));
  
  ElementSpecRef assignment =
    _conf->addElement(New refcounted< PelTransform >(strbuf("Assignment:") 
						     << curRule->ruleID << ":" 
						     << assignmentID << ":" << nodeID, 
						     str(pelStr)));

  if (_pendingRegisterReceiver) {
    _pendingReceiverSpec = assignment;
    _pendingRegisterReceiver = false;
  } else {
    // connecting now
    hookUp(assignment, 0);
  }

  genPrintElement(strbuf("PrintAfterAssignment:") << curRule->ruleID << ":" 
		  << assignmentID << ":" << nodeID);
}


void Rtr_ConfGen::genAllAssignmentElements(OL_Context::Rule* curRule,
					   str nodeID,
					   FieldNamesTracker* curNamesTracker) 
{
  for (unsigned int j = 0; j < curRule->terms.size(); j++) {
    OL_Context::Term nextTerm = curRule->terms.at(j);
    if (isAssignment(nextTerm)) {
      genAssignmentElements(curRule, nextTerm, nodeID, curNamesTracker, j);
    }
  }
}


void Rtr_ConfGen::genProjectHeadElements(OL_Context::Functor* curFunctor, 
					 OL_Context::Rule* curRule,
					 str nodeID,
					 FieldNamesTracker* curNamesTracker)
{
  // determine the projection fields, and the first address to return. Add 1 for table name     
  int addressField = curNamesTracker->fieldPosition(curFunctor->loc) + 1;
  std::vector<unsigned int> indices;  
  
  // iterate through all functor's output
  for (unsigned int k = 0; k < curFunctor->arity; k++) {
    indices.push_back(curNamesTracker->fieldPosition(curRule->args.at(k)) + 1);
  }
  
  strbuf pelTransformStrbuf("\"" << curFunctor->name << "\" pop");
  if (curRule->deleteFlag == false) {
    pelTransformStrbuf << " $" << addressField << " pop"; // we are not deleting
  }
  for (unsigned int k = 0; k < indices.size(); k++) {
    pelTransformStrbuf << " $" << indices.at(k) << " pop";
  }
  str pelTransformStr(pelTransformStrbuf);
  
  // project, and make sure first field after table name has the address 
  ElementSpecRef projectHead =
    _conf->addElement(New refcounted< PelTransform >(strbuf("ProjectHead:") 
						     << curRule->ruleID << ":" << nodeID,
						     pelTransformStr));
  if (_pendingRegisterReceiver) {
    _pendingReceiverSpec = projectHead;
    _pendingRegisterReceiver = false;
  } else {
    // connecting now
    hookUp(projectHead, 0);
  }
  
  genPrintElement(strbuf("PrintHead:") << curRule->ruleID << ":" << nodeID);  
}



void 
Rtr_ConfGen::genProbeElements(OL_Context::Rule* curRule, 
			      OL_Context::Term eventTerm, 
			      OL_Context::Term baseTerm, 
			      str nodeID, 
			      FieldNamesTracker* probeNames, 
			      int joinOrder,
			      cbv comp_cb)
{
  // probe the right hand side
  // Here's where the join happens 
  FieldNamesTracker* baseTableNames = New FieldNamesTracker(baseTerm.argNames, baseTerm.argNames.size());
  std::vector<int> leftJoinKeys = baseTableNames->matchingJoinKeys(probeNames->fieldNames);
  std::vector<int> rightJoinKeys =  probeNames->matchingJoinKeys(baseTerm.argNames);

  if (leftJoinKeys.size() == 0 || rightJoinKeys.size() == 0) {
    std::cerr << "No matching join keys " << eventTerm.fn->name << " " << 
      baseTerm.fn->name << " " << curRule->ruleID << "\n";
  }

  // add one to offset for table name
  int leftJoinKey = leftJoinKeys.at(0) + 1;
  int rightJoinKey = rightJoinKeys.at(0) + 1;

  TableRef probeTable = getTableByName(nodeID, baseTerm.fn->name);

  // first, figure out which are the matching keys on the probed table

  // should we use a uniqLookup or a multlookup? Check that the rightJoinKey is the primary key
  OL_Context::TableInfo* tableInfo = _ctxt->getTableInfos()->find(baseTerm.fn->name)->second;
  ElementSpecRef noNull = _conf->addElement(New refcounted< NoNullField >(strbuf("NoNull:") << curRule->ruleID << ":" 
									  << joinOrder << ":" << nodeID, 1));

  ElementSpecRef last_el = New refcounted<ElementSpec>(New refcounted<Slot>("dummySlotProbeElements"));
 
  if (tableInfo->primaryKeys.size() == 1 && tableInfo->primaryKeys.at(0) == rightJoinKey) {
    // use a unique index
    last_el =
      _conf->addElement(New refcounted< UniqueLookup >(strbuf("UniqueLookup:") << curRule->ruleID << ":" << joinOrder 
						       << ":" << nodeID, probeTable,
						       leftJoinKey, rightJoinKey, comp_cb));
    debugRule(curRule, str(strbuf() << "Unique lookup " << " " 
			   << eventTerm.fn->name << " " << baseTerm.fn->name << " " 
			   << leftJoinKey << " " << rightJoinKey << "\n"));
    
  } else {
    last_el =
      _conf->addElement(New refcounted< MultLookup >(strbuf("MultLookup:") << curRule->ruleID << ":" << joinOrder 
						     << ":" << nodeID, probeTable,
						     leftJoinKey, rightJoinKey, comp_cb));
    
    addMultTableIndex(probeTable, rightJoinKey, nodeID);
    debugRule(curRule, str(strbuf() << "Mult lookup " << curRule->ruleID << " " << 
			   eventTerm.fn->name << " " << baseTerm.fn->name << " " 
			   << leftJoinKey << " " << rightJoinKey << "\n"));
  }
  
 
  int numFieldsProbe = probeNames->fieldNames.size();
  debugRule(curRule, str(strbuf() << "Probe before merge " << probeNames->toString() << "\n"));
  probeNames->mergeWith(baseTerm.argNames); 
  debugRule(curRule, str(strbuf() << "Probe after merge " << probeNames->toString() << "\n"));

  if (_pendingRegisterReceiver) {
    // connecting to udp receiver later
    _pendingReceiverSpec = last_el;
    _pendingRegisterReceiver = false;
  } else {
    // connecting now to prior element
    hookUp(last_el, 0);  
  }

  hookUp(last_el, 0, noNull, 0);

  // in case there are other join keys, we have to compare them using selections since we can join only 1 field
  // since we don't know the types, do a string compare
  debugRule(curRule, str(strbuf() << "Number of join selections " << leftJoinKeys.size()-1 << "\n"));
  for (uint k = 1; k < leftJoinKeys.size(); k++) {
    int leftField = leftJoinKeys.at(k);
    int rightField = rightJoinKeys.at(k);
    strbuf selectionPel;
    selectionPel << "$0 " << (leftField+1) << " field " << " $1 " << rightField+1 
		 << " field ==s not ifstop $0 pop $1 pop";

    debugRule(curRule, str(strbuf() << "Join selections " << str(selectionPel) << "\n"));
    ElementSpecRef joinSelections =
      _conf->addElement(New refcounted< PelTransform >(strbuf("JoinSelections:") << curRule->ruleID << ":" 
						       << joinOrder << ":" << k << ":" << nodeID, 
						       str(selectionPel)));    
    hookUp(joinSelections, 0);
  }

  genPrintElement(strbuf("PrintProbe1:") << 
		  curRule->ruleID << ":" << joinOrder << ":"
		  << nodeID);
 
  // Take the joined tuples and produce the resulting path
  // form the pel projection. 
  //Keep all fields on left, all fields on right except the join keys
  strbuf pelProject("\"join:");
  pelProject << eventTerm.fn->name << ":" << baseTerm.fn->name << ":" 
	     << curRule->ruleID << ":" << nodeID << "\" pop "; // table name
  for (int k = 0; k < numFieldsProbe; k++) {
    pelProject << "$0 " << k+1 << " field pop ";
  }
  for (uint k = 0; k < baseTerm.argNames.size(); k++) {
    bool joinKey = false;
    for (uint j = 0; j < rightJoinKeys.size(); j++) {
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
  ElementSpecRef transS =
    _conf->addElement(New refcounted< PelTransform >(strbuf("JoinPelTransform:") << curRule->ruleID << ":" 
						     << joinOrder << ":" << nodeID, 
						     pelProjectStr));

  delete baseTableNames;

  hookUp(transS, 0);

  genPrintElement(strbuf("PrintProbe2:") << 
		  curRule->ruleID << ":" << joinOrder << ":"
		  << nodeID);
}


void Rtr_ConfGen::genJoinElements(OL_Context::Functor* curFunctor, 
				  OL_Context::Rule* curRule, 
				  str nodeID, 
				  FieldNamesTracker* namesTracker,
				  ptr<Aggwrap> agg_el )
{
  // identify the events, use that to probe the other matching tables
  OL_Context::Term eventTerm;
  std::vector<OL_Context::Term> baseTerms;
  bool eventFound = false;
  std::map<str, OL_Context::Term> staticTerms;
  for (unsigned int k = 0; k < curRule->terms.size(); k++) {
    OL_Context::Term nextTerm = curRule->terms.at(k);
    str termName = nextTerm.fn->name;
    if (isSelection(nextTerm) || isAssignment(nextTerm) || isAggregation(nextTerm)) {
      continue;
    }

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
    for (unsigned int k = 0; k < curRule->terms.size(); k++) {
      OL_Context::Term nextTerm = curRule->terms.at(k);
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
  // keep track also the cur ordering of variables
  namesTracker->initialize(eventTerm.argNames, eventTerm.argNames.size());  
  for (uint k = 0; k < baseTerms.size(); k++) {    
    if (baseTerms.at(k).fn->name == eventTerm.fn->name) { continue; }
    debugRule(curRule, str(strbuf() << "Probing " << eventTerm.fn->name << " " 
			   << baseTerms.at(k).fn->name << "\n"));
    cbv comp_cb = cbv_null;
    if (agg_el) {
      comp_cb = agg_el->get_comp_cb();
    }

    genProbeElements(curRule, eventTerm, baseTerms.at(k), 
		     nodeID, namesTracker, k, comp_cb);

    // Change from pull to push
    ElementSpecRef pullPush =
      _conf->addElement(New refcounted< TimedPullPush >(strbuf("JoinPullPush:") 
							<< curRule->ruleID << ":" << nodeID << ":" << k,
							0));
    hookUp(pullPush, 0);
  }
}


void Rtr_ConfGen::addMultTableIndex(TableRef table, int fn, str nodeID)
{
  strbuf uniqStr = str(table->name);
  uniqStr << ":" << fn << ":" << nodeID;
  if (_multTableIndices.find(str(uniqStr)) == _multTableIndices.end()) {
    // not there yet
    table->add_multiple_index(fn);
    _multTableIndices.insert(std::make_pair(str(uniqStr), str(uniqStr)));
    std::cout << "AddMultTableIndex: Mult index added " << str(uniqStr) << "\n";
  } else {
    std::cout << "AddMultTableIndex: Mult index already exists " << str(uniqStr) << "\n";
  }
}

void Rtr_ConfGen::genSingleAggregateElements(OL_Context::Functor* currentFunctor, 
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
  
  addMultTableIndex(aggTable, groupByFields.at(0), nodeID);
  
  Table::MultAggregate tableAgg 
    = aggTable->add_mult_groupBy_agg(groupByFields.at(0), // groupby field
				     groupByFields,
				     aggField, // the agg field
				     af);
  ElementSpecRef aggElement =
    _conf->addElement(New refcounted< Aggregate >(strbuf("Agg:") << currentRule->ruleID << ":" << nodeID,
						  tableAgg));
   
  // send back
  newFieldNamesTracker->fieldNames.push_back(aggTerm.argNames.at(0));

  strbuf pelTransformStr;
  pelTransformStr << "\"" << "aggResult:" << currentRule->ruleID << "\" pop";
  for (uint k = 0; k < newFieldNamesTracker->fieldNames.size(); k++) {
    pelTransformStr << " $" << k << " pop";
  }
  // apply PEL to add a table name
  ElementSpecRef addTableName =
    _conf->addElement(New refcounted< PelTransform >(strbuf("Aggregation:") 
						     << currentRule->ruleID << ":" 
						     << nodeID, 
						     pelTransformStr));

  hookUp(aggElement, 0, addTableName, 0);

  genPrintElement("PrintAgg:" << currentRule->ruleID << ":" << nodeID);

  ElementSpecRef timedPullPush = 
    _conf->addElement(New refcounted<TimedPullPush>("timedPullPush:" << 
						    currentRule->ruleID 
						    << ":" << nodeID, 0));  

  hookUp(timedPullPush, 0);
  genProjectHeadElements(currentFunctor, currentRule, nodeID, newFieldNamesTracker);
  genSendMarshalElements(currentRule, nodeID, currentFunctor->arity);
  _udpPushSenders.push_back(_currentElementChain.back());

  // FIXME: Selections, assignments, better integration with the rest

  // not done yet, we also need to register a table receiver for the head
  str headTableName = currentFunctor->name;
  registerReceiverTable(headTableName);
  // link that to a discard. If there are other flows that need it, they will
  // get the tuples via another fork in the duplicator.
  ElementSpecRef sinkS = _conf->addElement(New refcounted< Discard >("discard"));    
  registerReceiver(headTableName, sinkS);
}


void Rtr_ConfGen::genSingleTermElement(OL_Context::Functor* curFunctor, 
					OL_Context::Rule* curRule, 
					str nodeID, 
					FieldNamesTracker* curNamesTracker)
{
  
  ElementSpecRef resultElement = New refcounted<ElementSpec>(New refcounted<Slot>("dummySlotSingleTerm"));
  OL_Context::Term curTerm;
  // if not, form basic scanners, then connect to selections, and then to form head the last

  for (unsigned int j = 0; j < curRule->terms.size(); j++) {
    // skip those that we already decide is going to participate in     
    curTerm = curRule->terms.at(j);    
    str tableName = curTerm.fn->name;
    // skip the following
    if (isSelection(curTerm) || isAssignment(curTerm) || isAggregation(curTerm)) { continue; } // skip functions
    registerReceiverTable(curTerm.fn->name); 
    _pendingRegisterReceiver = true;
    _pendingReceiverTable = curTerm.fn->name;
    break; // break at first opportunity. Assume there is only one term   
  }

  // with knowledge of on which is a scan term, we can do the pel transform magic now
  // first, figure out which field is the results. Then figure out which fields need to be sent
  curNamesTracker->initialize(curTerm.argNames, curTerm.argNames.size());
}


void Rtr_ConfGen::genDupElimElement(str header)
{
   if (_dups) {
     ElementSpecRef dupElim = _conf->addElement(New refcounted< DupElim >(header));
     hookUp(dupElim, 0);
  }
}


void Rtr_ConfGen::genPrintElement(str header)
{
  if (_debug) {
    ElementSpecRef print = 
      _conf->addElement(New refcounted< PrintTime >(header));
    hookUp(print, 0);
  }
}

void Rtr_ConfGen::genPrintWatchElement(str header)
{
  ElementSpecRef printWatchElement = 
      _conf->addElement(New refcounted< PrintWatch >(header, _ctxt->getWatchTables()));
  hookUp(printWatchElement, 0);
}






/////////////////////////////////////////////////////////////////////////////


////////////////////////// Table Creation ////////////////////////////////////////

// Get a handle to the table. Typically used by the driver program to 
// preload some data.
TableRef Rtr_ConfGen::getTableByName(str nodeID, str tableName)
{
  TableMap::iterator _iterator = _tables.find(nodeID << ":" << tableName);
  return _iterator->second;
}

void Rtr_ConfGen::createTables(str nodeID)
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
	std::cout << "Create Tables: Add unique index " << newTableName << " " << primaryKeys.at(k) << "\n";
      }

      _tables.insert(std::make_pair(newTableName, newTable));      
    }
  }
}

/////////////////////////////////////////////

///////////// Helper functions

str Rtr_ConfGen::getRuleStr(OL_Context::Functor* curFunctor, OL_Context::Rule* curRule)
{
  strbuf ret("<Rule: ");
  ret << curFunctor->name << "@" << curFunctor->loc << "( ";

  for (unsigned int k = 0; k < curFunctor->arity; k++) {
    ret << curRule->args.at(k) << " ";
  }
  return ret << ") " << curRule->toString() << ">";  
}


void Rtr_ConfGen::hookUp(ElementSpecRef firstElement, int firstPort,
			 ElementSpecRef secondElement, int secondPort)
{
  fprintf(_output, "Connect: \n");
  fprintf(_output, "  %s %s %d\n", firstElement->toString().cstr(), 
	  firstElement->element()->_name.cstr(), firstPort);
  fprintf(_output, "  %s %s %d\n", secondElement->toString().cstr(), 
	  secondElement->element()->_name.cstr(), secondPort);
  fflush(_output);
  
  _conf->hookUp(firstElement, firstPort, secondElement, secondPort);

  if (_currentElementChain.size() == 0) {
    // first time
    _currentElementChain.push_back(firstElement);
  } 
  _currentElementChain.push_back(secondElement);

}

void Rtr_ConfGen::hookUp(ElementSpecRef secondElement, int secondPort)
{
  if (_currentElementChain.size() == 0) {
    _currentElementChain.push_back(secondElement);
    assert(secondPort == 0);
  }
  hookUp(_currentElementChain.back(), 0, secondElement, secondPort);
}

void Rtr_ConfGen::setJoinKeys(OL_Context::Rule* rule, std::vector<JoinKey>* joinKeys)
{

  // enumerate through all the terms, ignore functions 
  // ok, this is terribly inefficient for now.
  for (uint k = 0; k < rule->terms.size(); k++) {
   OL_Context::Term firstTerm = rule->terms.at(k);
   str firstName(firstTerm.fn->name);
   if (isSelection(firstTerm) || isAssignment(firstTerm) || isAggregation(firstTerm)) { continue; }
   std::vector<str> firstArgNames = firstTerm.argNames;
    for (uint j = k+1; j < rule->terms.size(); j++) {
      OL_Context::Term secondTerm = rule->terms.at(j);
      str secondName(secondTerm.fn->name);
      if (isSelection(secondTerm) || isAssignment(secondTerm) 
	  || isAggregation(secondTerm)) { continue; }
      std::vector<str> secondArgNames = secondTerm.argNames;
      
      for (uint x = 0; x < firstArgNames.size(); x++) {	
	for (uint y = 0; y < secondArgNames.size(); y++) {
	  if (firstArgNames.at(x) == secondArgNames.at(y)) {
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

Rtr_ConfGen::FieldNamesTracker::FieldNamesTracker() { }

Rtr_ConfGen::FieldNamesTracker::FieldNamesTracker(std::vector<str> argNames, int arity)
{
  for (int k = 0; k < arity; k++) {
    fieldNames.push_back(argNames.at(k));

  }
}

void Rtr_ConfGen::FieldNamesTracker::initialize(std::vector<str> argNames, int arity)
{
  for (int k = 0; k < arity; k++) {
    fieldNames.push_back(argNames.at(k));
  }
}


std::vector<int> Rtr_ConfGen::FieldNamesTracker::matchingJoinKeys(std::vector<str> otherArgNames)
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

int Rtr_ConfGen::FieldNamesTracker::fieldPosition(str var)
{
  for (uint k = 0; k < fieldNames.size(); k++) {
    if (fieldNames.at(k) == var) {
      return k;
    }
  }
  return -1;
}

void Rtr_ConfGen::FieldNamesTracker::mergeWith(std::vector<str> names)
{
  for (uint k = 0; k < names.size(); k++) {
    str nextStr = names.at(k);
    if (fieldPosition(nextStr) == -1) {
      fieldNames.push_back(nextStr);
    }
  }
}

void Rtr_ConfGen::FieldNamesTracker::mergeWith(std::vector<str> names, int numJoinKeys)
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

str Rtr_ConfGen::FieldNamesTracker::toString()
{
  strbuf toRet("FieldNamesTracker<");
  
  for (uint k = 0; k < fieldNames.size(); k++) {
    toRet << fieldNames.at(k) << " ";
  }
  return toRet << ">";
}
