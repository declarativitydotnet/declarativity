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

#include <async.h>
#include <arpc.h>
#include "routerConfigGenerator.h"
#include "roundRobin.h"

const str RouterConfigGenerator::FUNC_PRE("f_");

RouterConfigGenerator::RouterConfigGenerator(OL_Context* ctxt, Router::ConfigurationRef conf, 
					     bool dups, bool debug, str filename):_conf(conf)
{
  _ctxt = ctxt;
  _dups = dups;
  _debug = debug;
  str outputFile(filename << ".out");
  _output = fopen(outputFile.cstr(), "w");

  // initialize some pel functions. Add more later. 
  // Look at Pel Expresssions
  pelFunctions.insert(std::make_pair(FUNC_PRE << "ne", "==s not"));
  pelFunctions.insert(std::make_pair(FUNC_PRE << "eq", "==s"));
  
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

  ElementSpecRef mostRecentElement = New refcounted<ElementSpec>(New refcounted<Slot>("dummySlot"));
  for (unsigned int k = 0; k < currentFunctor->rules.size(); k++) {
    OL_Context::Rule* nextRule = currentFunctor->rules.at(k);
    // first, get a list of possible joins
    std::vector<JoinKey>* joinKeys = New std::vector<JoinKey>;
    setJoinKeys(nextRule, joinKeys);    
    //_currentType = "h"; 

    // step 1:
    FieldNamesTracker* currentNamesTracker = new FieldNamesTracker();
    if (joinKeys->size() == 0) {
      mostRecentElement = generateSingleTermElement(currentFunctor, nextRule, 
						    nodeID, currentNamesTracker);
      _currentType = "h";   
    } else {
      mostRecentElement = generateEventProbeElements(currentFunctor, nextRule, 
						     nodeID, currentNamesTracker);
      _currentType = "l";
      
    }

    delete joinKeys;

    // do the necessary projections based on the head
    mostRecentElement = generateProjectHeadElements(currentFunctor, nextRule, nodeID, 
						    currentNamesTracker, mostRecentElement);    
    
    // send it!
    mostRecentElement = generateSendMarshalElements(nextRule, nodeID, 
						    currentFunctor->arity, mostRecentElement);

    // do the selections
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
    std::cout << "Receive demux " << nextTableName << "\n";
    demuxKeys->push_back(New refcounted< Val_Str >(nextTableName));
  }

  ElementSpecRef demuxS = _conf->addElement(New refcounted< Demux >("receiveDemux", demuxKeys));

  ElementSpecRef unmarshalS =
    _conf->addElement(New refcounted< UnmarshalField >(strbuf("ReceiveUnmarshal:") << nodeID, 1));

  ElementSpecRef unBoxS =
    _conf->addElement(New refcounted< UnboxField >(strbuf("ReceiveUnBox:") << nodeID, 1));

  ElementSpecRef recvPS =
    _conf->addElement(New refcounted< Print >(strbuf("PrintReceiveBeforeDemux:") << ":" 
					      << nodeID));
  hookUp(udpReceive, 0, unmarshalS, 0);
  hookUp(unmarshalS, 0, unBoxS, 0);
  hookUp(unBoxS, 0, recvPS, 0);
  hookUp(recvPS, 0, demuxS, 0);

  // connect to all the pending udp receivers. Assume all 
  // receivers are connected to elements with push input ports for now
  int counter = 0;
  for (_iterator = _udpReceivers.begin(); _iterator != _udpReceivers.end(); _iterator++) {
    ElementSpecRef nextElementSpec = _iterator->second._receiver;
    std::cout << "Hook up " << nextElementSpec->toString() << "\n";
    hookUp(demuxS, counter++, nextElementSpec, 0);
  }

  ElementSpecRef sinkS = _conf->addElement(New refcounted< Discard >("discard"));
  hookUp(demuxS, _udpReceivers.size(), sinkS, 0);
}



void RouterConfigGenerator::generateSendElements(ref< Udp> udp, str nodeID)
{
  ElementSpecRef udpSend = _conf->addElement(udp->get_tx());  

  // prepare to send. Assume all tuples send by first tuple
  ElementSpecRef muxS =
    _conf->addElement(New refcounted< RoundRobin >("sendMux:" << nodeID, _udpPushSenders.size() + _udpPullSenders.size()));

  hookUp(muxS, 0, udpSend, 0);
  
  // form the push senders
  for (unsigned int k = 0; k < _udpPushSenders.size(); k++) {
    ElementSpecRef nextElementSpec = _udpPushSenders.at(k);
    ElementSpecRef slot = 
      _conf->addElement(New refcounted< Slot >(strbuf("Slot:") << nodeID << ":" << k));

    hookUp(nextElementSpec, 0, slot, 0);
    hookUp(slot, 0, muxS, k);
  }

  // since pull senders, no need slot
  for (unsigned int k = 0; k < _udpPullSenders.size(); k++) {
    ElementSpecRef nextElementSpec = _udpPullSenders.at(k);
    hookUp(nextElementSpec, 0, muxS, k + _udpPushSenders.size());
  }
}


// create a send element where the first field after table name is the address. Drop that field
ElementSpecRef 
RouterConfigGenerator::generateSendMarshalElements(OL_Context::Rule* rule, str nodeID, 
						   int arity, ElementSpecRef toSend)
{

  ElementSpecRef dupElimS =
    _conf->addElement(New refcounted< DupElim >(strbuf("SendDupElim:") << rule->ruleID 
						<< ":" << nodeID));
  hookUp(toSend, 0, dupElimS, 0);
  
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

  hookUp(dupElimS, 0, encapS, 0);

  ElementSpecRef printHead = New refcounted<ElementSpec>(New refcounted<Slot>("dummySlot"));
  if (_debug) {
    printHead = 
      _conf->addElement(New refcounted< Print >(strbuf("PrintSend:") << rule->ruleID << ":" << nodeID));
  }
  
  // Now marshall the payload (second field)
  ElementSpecRef marshalS = 
    _conf->addElement(New refcounted< MarshalField >("marshal:" << 
						     rule->ruleID << ":" << nodeID, 1));
  
  // And translate the address into a sockaddr.  This is a hack until
  // we have a consistent view of the UDP interface.
  ElementSpecRef routeS =
    _conf->addElement(New refcounted< StrToSockaddr >("router:" << rule->ruleID 
						      << ":" << nodeID, 0));
    
  if (_debug) {
    hookUp(encapS, 0, printHead, 0);
    hookUp(printHead, 0, marshalS, 0);
  } else {
    hookUp(encapS, 0, marshalS, 0);
  }
  hookUp(marshalS, 0, routeS, 0);

  return routeS;
}


// unmarshal what is received off the network, decide whether to materialize or not
// based on table info
ElementSpecRef
RouterConfigGenerator::generateReceiveUnmarshalElements(OL_Context::Functor* currentFunctor, 
							OL_Context::Rule* currentRule, 
							OL_Context::Term term,
							str nodeID) 
{
  //_currentType = "h"; // pushed in

  // do we are about duplicates?
  ElementSpecRef dupElimS =
    _conf->addElement(New refcounted< DupElim >(strbuf("ReceiveDupElim:") << 
					       currentRule->ruleID << ":" << nodeID));

  ReceiverInfo info(dupElimS, term.args.size(), term.fn->name);
  _udpReceivers.insert(std::make_pair(term.fn->name, info));

  ElementSpecRef recvPS =
    _conf->addElement(New refcounted< Print >(strbuf("PrintReceived:") << ":" 
					     << currentRule->ruleID << ":" << nodeID));

  OL_Context::TableInfoMap::iterator _iterator = _ctxt->getTableInfos()->find(term.fn->name);
  if (_iterator != _ctxt->getTableInfos()->end() && _iterator->second->timeout != 0) {
    // we have to materialize
    ElementSpecRef insertS = _conf->addElement(New refcounted< Insert >(strbuf("Insert:") 
								       << currentRule->ruleID << ":" 
								       << nodeID, 
								       getTableByName(nodeID, term.fn->name)));
    hookUp(dupElimS, 0, insertS, 0);
    hookUp(insertS, 0, recvPS, 0);
  } else {
    hookUp(dupElimS, 0, recvPS, 0);
  }

  return recvPS;
}



//////////////////////////////////////////////////////////////////
///////////////// Relational Operators -> P2 Elements
//////////////////////////////////////////////////////////////////

ElementSpecRef RouterConfigGenerator::generateProjectHeadElements(OL_Context::Functor* currentFunctor, 
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
  pelTransformStrbuf << " $" << addressField << " pop";
  for (unsigned int k = 0; k < indices.size(); k++) {
    pelTransformStrbuf << " $" << indices.at(k) << " pop";
  }
  str pelTransformStr(pelTransformStrbuf);
  
  // project, and make sure first field after table name has the address 
  ElementSpecRef projectHead =
    _conf->addElement(New refcounted< PelTransform >(strbuf("ProjectHead:") 
						     << currentRule->ruleID << ":" << nodeID,
						     pelTransformStr));
  hookUp(connectingElement, 0, projectHead, 0);
  
  if (!_debug) {
    return generateSendMarshalElements(currentRule, nodeID, currentFunctor->arity, projectHead);
  }
  
  ElementSpecRef printHead =
    _conf->addElement(New refcounted< Print >(strbuf("PrintHead:") 
					      << currentRule->ruleID 
					      << ":" << nodeID));
  
  hookUp(projectHead, 0, printHead, 0);  
  return printHead;
}



ElementSpecRef
RouterConfigGenerator::generateJoinElements(OL_Context::Rule* currentRule, 
					    OL_Context::Term eventTerm, 
					    OL_Context::Term baseTerm, 
					    str nodeID, 
					    FieldNamesTracker* probeNames, 					    
					    ElementSpecRef priorElement,
					    int joinOrder)
{
  //_currentType = "l";
  ElementSpecRef dummyElement = New refcounted<ElementSpec>(New refcounted<Slot>("dummySlot")); 
  // Here's where the join happens 
  FieldNamesTracker* baseTableNames = New FieldNamesTracker(baseTerm.argNames, baseTerm.argNames.size());
  int leftJoinKey = baseTableNames->matchingJoinKey(eventTerm.argNames) + 1;
  int rightJoinKey = probeNames->matchingJoinKey(baseTerm.argNames) + 1;
  
  ElementSpecRef lookupS =
    _conf->addElement(New refcounted< MultLookup >(strbuf("Lookup:") << currentRule->ruleID << ":" << joinOrder 
						   << ":" << nodeID,
						   getTableByName(nodeID, baseTerm.fn->name), 
						   leftJoinKey, rightJoinKey));

  hookUp(priorElement, 0, lookupS, 0);
  probeNames->mergeWith(baseTerm.argNames);

  if (_debug) {
    ElementSpecRef printJoin1 =
      _conf->addElement(New refcounted< Print >(strbuf("PrintJoin1:") << 
						currentRule->ruleID << ":" << joinOrder << ":"
						<< nodeID));  
    hookUp(lookupS, 0, printJoin1, 0);
    dummyElement = printJoin1;
  } else {
    dummyElement = lookupS;
  }
 
  // Take the joined tuples and produce the resulting path
  // form the pel projection. 
  //Keep all fields on left, all fields on right except the join key
  strbuf pelProject("\"join:");
  pelProject << eventTerm.fn->name << ":" << baseTerm.fn->name << ":" 
	     << currentRule->ruleID << ":" << nodeID << "\" pop "; // table name
  for (uint k = 0; k < eventTerm.argNames.size(); k++) {
    pelProject << "$0 " << k+1 << " field pop ";
  }
  for (uint k = 0; k < baseTerm.argNames.size(); k++) {
    if (k+1 != (uint) rightJoinKey) {
      pelProject << "$1 " << k+1 << " field pop ";
    }
  }

  str pelProjectStr(pelProject);
  ElementSpecRef transS =
    _conf->addElement(New refcounted< PelTransform >(strbuf("JoinPelTransform:") << currentRule->ruleID << ":" 
						     << joinOrder << ":" << nodeID, 
						     pelProjectStr));

  delete baseTableNames;

  hookUp(dummyElement, 0, transS, 0);

  if (!_debug) { 
    return transS; 
  }
  
  ElementSpecRef printJoin2 =
    _conf->addElement(New refcounted< Print >(strbuf("PrintJoin2:") << 
					      currentRule->ruleID << ":" << joinOrder << ":"
					      << nodeID));

  hookUp(transS, 0, printJoin2, 0);
  return printJoin2;
}


ElementSpecRef RouterConfigGenerator::generateSelectionElements(OL_Context::Rule* currentRule, 
								OL_Context::Term nextSelection, 
								str nodeID, 
								FieldNamesTracker* probeNames,
								ElementSpecRef lastElement, 
								int selectionID)
{
 // Prepend with true if this is a Reach X, X.
  str pelSelectionOpt = pelFunctions.find(nextSelection.fn->name)->second;

  // figure out where the selection is
  strbuf selectionPel(" ");
  for (uint k = 0; k < nextSelection.argNames.size(); k++) {
    selectionPel << "$" << (1 + probeNames->fieldPosition(nextSelection.argNames.at(k))) << " ";
  }
  selectionPel << pelSelectionOpt << " pop ";
  
  // put in the old fields (+1 since we have to include the table name)
  for (uint k = 0; k < probeNames->fieldNames.size() + 1; k++) {
    selectionPel << "$" << k << " pop ";
  }
   
  ElementSpecRef selection =
    _conf->addElement(New refcounted< PelTransform >(strbuf("Selection:") 
						     << currentRule->ruleID << ":" 
						     << selectionID << ":" << nodeID, 
						     selectionPel));

  hookUp(lastElement, 0, selection, 0);

  // Only pass through those that have different from and to
  ElementSpecRef filterS =
    _conf->addElement(New refcounted< Filter >(strbuf("filter:") << currentRule->ruleID << 
					       ":" << selectionID << ":" << nodeID, 0));

  strbuf dropPel(" ");
  for (uint k = 0; k < probeNames->fieldNames.size()+1; k++) {
    dropPel << "$" << (k+1) << " pop ";
  }
  // And drop the filter field.
  ElementSpecRef filterDropS =
    _conf->addElement(New refcounted< PelTransform >(strbuf("filterDrop:") 
						     << currentRule->ruleID << ":" 
						     << selectionID << ":" << nodeID, 
						     str(dropPel)));

  hookUp(selection, 0, filterS, 0);
  hookUp(filterS, 0, filterDropS, 0);

  if (!_debug) {
    return filterDropS;
  }

  ElementSpecRef printAfterFilter =
    _conf->addElement(New refcounted< Print >(strbuf("PrintAfterSelection:") 
					      << currentRule->ruleID << ":" 
					      << selectionID << ":" << nodeID));

  hookUp(filterDropS, 0, printAfterFilter, 0);
  return printAfterFilter;
}


ElementSpecRef 
RouterConfigGenerator::generateEventProbeElements(OL_Context::Functor* currentFunctor, 
						  OL_Context::Rule* currentRule, 
						  str nodeID, 
						  FieldNamesTracker* namesTracker)
{
  ElementSpecRef eventElement = New refcounted<ElementSpec>(New refcounted<Slot>("dummySlot"));
  ElementSpecRef lastElement = New refcounted<ElementSpec>(New refcounted<Slot>("dummySlot"));

  OL_Context::Term eventTerm;
  std::vector<OL_Context::Term> selections; 
  std::vector<OL_Context::Term> baseTerms;
  for (unsigned int k = 0; k < currentRule->terms.size(); k++) {
    OL_Context::Term nextTerm = currentRule->terms.at(k);
    str termName = nextTerm.fn->name;
    if (substr(termName, 0, 2) == FUNC_PRE) { 
      selections.push_back(nextTerm); 
      continue;
    }
    OL_Context::TableInfoMap::iterator _iterator = _ctxt->getTableInfos()->find(termName);
    if (_iterator != _ctxt->getTableInfos()->end()) {     
      baseTerms.push_back(nextTerm);
    } else {
      // FIXME: If not materialized, assume events for now
      eventElement = generateReceiveUnmarshalElements(currentFunctor, currentRule, nextTerm, nodeID);	
      eventTerm = nextTerm;
    } 
  }

  // for all the base tuples, use the join to probe. 
  // keep track also the current ordering of variables
  namesTracker->initialize(eventTerm.argNames, eventTerm.argNames.size());  
  ElementSpecRef joinSpecRef = New refcounted<ElementSpec>(New refcounted<Slot>("dummySlot"));
  for (uint k = 0; k < baseTerms.size(); k++) {
    joinSpecRef = generateJoinElements(currentRule, eventTerm, baseTerms.at(k), 
				       nodeID, namesTracker, eventElement, k);
    eventElement = joinSpecRef; // keep track of last connector
  }

  ElementSpecRef lastSelectionSpecRef = New refcounted<ElementSpec>(New refcounted<Slot>("dummySlot")); 
  for (unsigned int k = 0; k < selections.size(); k++) {
    OL_Context::Term nextSelection = selections.at(k);
    PelFunctionMap::iterator pelSelectionIterator = pelFunctions.find(nextSelection.fn->name);
    if (pelSelectionIterator != pelFunctions.end()) {
      lastSelectionSpecRef = generateSelectionElements(currentRule, nextSelection, 
						       nodeID, namesTracker, eventElement, k); 
      eventElement = lastSelectionSpecRef;
    }
  }
  return eventElement;
}


ElementSpecRef 
RouterConfigGenerator::generateSingleTermElement(OL_Context::Functor* currentFunctor, 
						 OL_Context::Rule* currentRule, 
						 str nodeID, 
						 FieldNamesTracker* currentNamesTracker)
{
  
  ElementSpecRef resultElement = New refcounted<ElementSpec>(New refcounted<Slot>("dummySlot"));
  OL_Context::Term currentTerm;
  // if not, form basic scanners, then connect to selections, and then to form head the last

  for (unsigned int j = 0; j < currentRule->terms.size(); j++) {
    // skip those that we already decide is going to participate in     
    currentTerm = currentRule->terms.at(j);    
    str tableName = currentTerm.fn->name;
    // skip funcitons
    if (substr(tableName, 0, 2) == FUNC_PRE) { continue; } // skip functions
    OL_Context::TableInfoMap::iterator _iterator = _ctxt->getTableInfos()->find(tableName); 
    //if (_iterator != _ctxt->getTableInfos()->end()) {
      // we will read the info off the first tabler
      /*resultElement = createScanElements(currentFunctor, 
					 currentRule,
					 currentTerm, 
					 _iterator->second,
					 nodeID);*/
      resultElement = generateReceiveUnmarshalElements(currentFunctor, currentRule, currentTerm, nodeID);	
      break; // break at first opportunity. Assume there is only one term   
      //} else {
      //resultElement = generateReceiveUnmarshalElements(currentFunctor, 
      //					       currentRule, currentTerm, nodeID);	
      //}
      /*} else {
      std::cout << "Info on table " << tableName << " is not available. Likely a predicate or an event tuple.\n";
      }*/
  }

  // with knowledge of on which is a scan term, we can do the pel transform magic now
  // first, figure out which field is the results. Then figure out which fields need to be sent
  currentNamesTracker->initialize(currentTerm.argNames, currentTerm.argNames.size());

  return resultElement;
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

  ElementSpecRef dummyElement = New refcounted<ElementSpec>(New refcounted<Slot>("dummySlot"));
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
      newTable->add_multiple_index(1); 
      _tables.insert(std::make_pair(newTableName, newTable));
      
      std::cout << "New table created: " << nodeID << " " << tableInfo->tableName << "\n";
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
   if (substr(firstName, 0, 2) == FUNC_PRE) { continue; }
   std::vector<str> firstArgNames = firstTerm.argNames;
    for (uint j = k+1; j < rule->terms.size(); j++) {
      OL_Context::Term secondTerm = rule->terms.at(j);
      str secondName(secondTerm.fn->name);
      if (substr(secondName, 0, 2) == FUNC_PRE) { continue; }
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


int RouterConfigGenerator::FieldNamesTracker::matchingJoinKey(std::vector<str> otherArgNames)
{
  // figure out the matching on my side. Assuming that
  // there is only one matching key for now
  for (unsigned int k = 0; k < otherArgNames.size(); k++) {
    str nextStr = otherArgNames.at(k);
    if (fieldPosition(nextStr) != -1) {
      // exists
      return k;
    }
  }  
  return -1;
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

str RouterConfigGenerator::FieldNamesTracker::toString()
{
  strbuf toRet("FieldNamesTracker<");
  
  for (uint k = 0; k < fieldNames.size(); k++) {
    toRet << fieldNames.at(k) << " ";
  }
  return toRet << ">";
}


