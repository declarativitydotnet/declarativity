// -*- c-basic-offset: 2; related-file-name: "element.C" -*-
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
 */

#include "rtr_confgen.h"
#include "trace.h"

Rtr_ConfGen::Rtr_ConfGen(OL_Context* ctxt, 
			 Router::ConfigurationPtr conf, 
			 bool dups, 
			 bool debug, 
			 bool cc,
			 string filename) :_conf(conf)
{
  _ctxt = ctxt;
  _dups = dups;
  _debug = debug;
  _cc = cc;
  string outputFile(filename + ".out");
  _output = fopen(outputFile.c_str(), "w");
  _pendingRegisterReceiver = false;  
  _isPeriodic = false;
  _currentPositionIndex = -1;
}


Rtr_ConfGen::~Rtr_ConfGen()
{
  fclose(_output);
}


// call this for each udp element that we wish to hook up the same dataflow
// if running only one, nodeID is the local host name,
void Rtr_ConfGen::configureRouter(boost::shared_ptr< Udp > udp, string nodeID)
{

  if (!_cc) {
    _ccTx.reset();
    _ccRx.reset();
  } else {
    _ccTx 
      = _conf->addElement(ElementPtr(new CCTx("Transmit CC" + nodeID, 1, 2048, 0, 1, 1, 2)));
    _ccRx 
      = _conf->addElement(ElementPtr(new CCRx("CC Receive" + nodeID, 2048, 1, 2)));
  }

  // iterate through all the rules and process them
  // check to see if there are any errors in parsing
  if (_ctxt->errors.size() > 0) {
    ostringstream oss;
    oss << "There are " << _ctxt->errors.size() 
	<< " error(s) accumulated in the parser.\n";
    for (unsigned int k = 0; k < _ctxt->errors.size(); k++) {
      OL_Context::Error* error = _ctxt->errors.at(k);
      oss << " => Parser error at line " << error->line_num 
	  << " with error message \"" << error->msg << "\".\n";
    }
    error(oss.str());
    exit(-1);
  }

  if (_ctxt->getRules()->size() == 0) {
    error("There are no rules to plan.\n");
  }

  for (unsigned int k = 0; k < _ctxt->getRules()->size(); k++) {
    _currentRule = _ctxt->getRules()->at(k);    
    processRule(_currentRule, nodeID);
  }

  ElementSpecPtr receiveMux = genSendElements(udp, nodeID); 
  _currentElementChain.clear();
  genReceiveElements(udp, nodeID, receiveMux);
}

void Rtr_ConfGen::clear()
{
  _udpReceivers.clear();
  _udpSenders.clear();
}


void Rtr_ConfGen::processRule(OL_Context::Rule *r, 
			      string nodeID)
{
  debugRule(r, "Process rule " + r->toString() + "\n");  
  std::vector<JoinKey> joinKeys;
  FieldNamesTracker curNamesTracker;
  boost::shared_ptr<Aggwrap> agg_el;

  _pendingReceiverSpec.reset();

  // AGGREGATES
  int aggField = r->head->aggregate(); 
  if (aggField >= 0) {
    if (hasEventTerm(r)) {
      ostringstream oss;
      
      // get the event term
      Parse_Functor* eventTerm = getEventTerm(r);
      
      if (eventTerm == NULL) {
	error("Cannot find event term", r);
      }
      checkFunctor(eventTerm, r);

      if (numFunctors(r) <= 1) {
	ostringstream oss;
	oss << "Check that " 
	    << eventTerm->fn->name << " is materialized";
	error(oss.str(), r);
      }

      // there is an aggregate and involves an event, we need an agg wrap      
      Parse_Agg* aggExpr = dynamic_cast<Parse_Agg*>(r->head->arg(aggField));
      if (aggExpr == NULL) {
	ostringstream oss;
	oss << "Invalid aggregate field " << aggField << " for rule " << r->ruleID; 
	error(oss.str());
      }

      oss << "Aggwrap:" << r->ruleID << ":" << nodeID;
      agg_el.reset(new Aggwrap(oss.str(), aggExpr->aggName(), 
			       aggField + 1, r->head->fn->name));
      for (int k = 0; k < r->head->args(); k++) {
	if (k != aggField) {
	  // for each groupby value, figure out it's 
	  // location in the initial event tuple, 
	  // if not present, throw an error
	  for (int j = 0; j < eventTerm->args(); j++) {
	    if (r->head->arg(k)->toString() == eventTerm->arg(j)->toString()) {
	      agg_el->registerGroupbyField(j);
	    }
	  }
	}
      }
    } else {
      // an agg that involves only base tables. 
      genSingleAggregateElements(r, nodeID, &curNamesTracker);
      return;    
    }
  }
  
  _currentElementChain.clear();
  _isPeriodic = false;

  // PERIODIC
  if (hasPeriodicTerm(r)) {
    genFunctorSource(r, nodeID, &curNamesTracker);
    // if there are more terms, we have to perform the genJoinElements, 
    // followed by selection/assignments
    _isPeriodic = true;
    if (numFunctors(r) > 1) {
      debugRule(r, "Periodic join\n");
      genJoinElements(r, nodeID, &curNamesTracker, agg_el);
    }
    // do the selections and assignment, followed by projection
    genAllSelectionAssignmentElements(r, nodeID, &curNamesTracker);
    genProjectHeadElements(r, nodeID, &curNamesTracker);    
  } else {
    if (numFunctors(r) == 1) {
      // SINGLE_TERM
      genSingleTermElement(r, nodeID, &curNamesTracker);
    } else {
      // MULTIPLE TERMS WITH JOINS
      genJoinElements(r, nodeID, &curNamesTracker, agg_el);
    }
  
    // do the selections and assignment, followed by projection
    genAllSelectionAssignmentElements(r, nodeID, &curNamesTracker);    
    //std::cout << "NetPlanner: Register receiver at demux " 
    //	      << _pendingRegisterReceiver << "\n";
    genProjectHeadElements(r, nodeID, &curNamesTracker);
  }
    
  if (r->deleteFlag == true) {
    debugRule(r, "Delete " + r->head->fn->name + " for rule \n");
    TablePtr tableToDelete = getTableByName(nodeID, r->head->fn->name);
    OL_Context::TableInfo* ti 
      = _ctxt->getTableInfos()->find(r->head->fn->name)->second;
    
    genPrintElement("PrintBeforeDelete:" + r->ruleID + ":" +nodeID);
    genPrintWatchElement("PrintWatchDelete:" + r->ruleID + ":" +nodeID);
    
    ElementSpecPtr pullPush = 
      _conf->addElement(ElementPtr(new TimedPullPush("DeletePullPush", 0)));
    hookUp(pullPush, 0);
    
    ElementSpecPtr deleteElement =
      _conf->addElement(ElementPtr(new Delete("Delete:" + r->ruleID + ":" + nodeID,
						 tableToDelete, 
						 ti->primaryKeys.at(0), 
						 ti->primaryKeys.at(0))));
    hookUp(deleteElement, 0);
    
    if (_isPeriodic == false && _pendingReceiverSpec) {
      registerReceiver(_pendingReceiverTable, _pendingReceiverSpec);
    }
    return; // discard. deleted tuples not sent anywhere
  } else {    
    if (agg_el) { 
      ElementSpecPtr aggWrapSlot 
	= _conf->addElement(ElementPtr(new Slot("aggWrapSlot:" + r->ruleID 
						 + ":" + nodeID)));
      ElementSpecPtr agg_spec = _conf->addElement(agg_el);
      // hook up the internal output to most recent element 
      hookUp(agg_spec, 1); 
      // hookup the internal input      
      hookUp(agg_spec, 1, _pendingReceiverSpec, 0); 
      
      hookUp(agg_spec, 0, aggWrapSlot, 0);
      // hook the agg_spect to the front later by receivers
      _pendingReceiverSpec = agg_spec; 
      // for hooking up with senders later
      _currentElementChain.push_back(aggWrapSlot); 
    } 
     
    // at the receiver side, generate a dummy receive  
    string headTableName = r->head->fn->name;
    registerReceiverTable(r, headTableName);
  }

  if (_isPeriodic == false && _pendingReceiverSpec) {
    registerReceiver(_pendingReceiverTable, _pendingReceiverSpec);
  }

  // anything at this point needs to be hookup with senders
  _udpSenders.push_back(_currentElementChain.back()); 
  _udpSendersPos.push_back(_currentPositionIndex); 
}



//////////////////// Transport layer //////////////////////////////
void 
Rtr_ConfGen::genReceiveElements(boost::shared_ptr< Udp> udp, 
				string nodeID, ElementSpecPtr wrapAroundDemux)
{

  // network in
  ElementSpecPtr udpReceive = _conf->addElement(udp->get_rx());  
  ElementSpecPtr unmarshalS =
    _conf->addElement(ElementPtr(new UnmarshalField ("ReceiveUnmarshal:" + nodeID, 1)));
  ElementSpecPtr unBoxS =
    _conf->addElement(ElementPtr(new  UnboxField ("ReceiveUnBox:" + nodeID, 1)));

  hookUp(udpReceive, 0, unmarshalS, 0);
  hookUp(unmarshalS, 0, unBoxS, 0);

  ElementSpecPtr wrapAroundMux;
  if (_cc) {
    wrapAroundMux = _conf->addElement(ElementPtr(new Mux ("wrapAroundSendMux:"+ nodeID, 3)));

    boost::shared_ptr< std::vector< ValuePtr > > demuxKeysCC(new std::vector< ValuePtr > );
    demuxKeysCC->push_back(ValuePtr(new Val_Str ("ack")));
    demuxKeysCC->push_back(ValuePtr(new Val_Str ("ccdata")));

    ElementSpecPtr demuxRxCC 
      = _conf->addElement(ElementPtr(new Demux ("receiveDemuxCC", demuxKeysCC)));

    genPrintElement("PrintBeforeReceiveDemuxCC:" + nodeID);
    hookUp(demuxRxCC, 0);
    hookUp(demuxRxCC, 0, _ccTx, 1);  // send acknowledgements to cc transmit
    hookUp(demuxRxCC, 2, wrapAroundMux, 2); // regular non-CC data

    // handle CC data. <ccdata, seq, src, <t>>
    ElementSpecPtr unpackCC =  
      _conf->addElement(ElementPtr(new UnboxField ("ReceiveUnBoxCC:" + nodeID, 3)));
    hookUp(demuxRxCC, 1, _ccRx, 0);  // regular CC data    
    hookUp(unpackCC, 0);
    genPrintElement("PrintReceiveUnpackCC:"+ nodeID);
    hookUp(wrapAroundMux, 0); // connect data to wraparound mux   

  } else {
    wrapAroundMux = _conf->addElement(ElementPtr(new Mux("wrapAroundSendMux:"+ nodeID, 2)));
    hookUp(unBoxS, 0, wrapAroundMux, 0);
  }

  // demuxer
  boost::shared_ptr< std::vector< ValuePtr > > demuxKeys(new std::vector< ValuePtr >);
  ReceiverInfoMap::iterator _iterator;
  for (_iterator = _udpReceivers.begin(); 
       _iterator != _udpReceivers.end(); 
       _iterator++) {
    string nextTableName = _iterator->second._name;
    demuxKeys->push_back(ValuePtr(new Val_Str (nextTableName)));
  }

  ElementSpecPtr demuxS 
    = _conf->addElement(ElementPtr(new Demux("receiveDemux", demuxKeys)));
 
  genPrintElement("PrintReceivedBeforeDemux:"+nodeID);
  genDupElimElement("ReceiveDupElimBeforeDemux:"+ nodeID); 
  genPrintWatchElement("PrintWatchReceiveBeforeDemux:"+ nodeID);

  ElementSpecPtr bufferQueue = 
    _conf->addElement(ElementPtr(new Queue("ReceiveQueue:"+nodeID, 1000)));
  ElementSpecPtr pullPush = 
    _conf->addElement(ElementPtr(new TimedPullPush("ReceiveQueuePullPush", 0)));

  hookUp(bufferQueue, 0);
  hookUp(pullPush, 0);
  hookUp(demuxS, 0);

  // connect to all the pending udp receivers. Assume all 
  // receivers are connected to elements with push input ports for now 
  int counter = 0;
  for (_iterator = _udpReceivers.begin(); 
       _iterator != _udpReceivers.end(); 
       _iterator++) {
    ReceiverInfo ri = _iterator->second;
    int numElementsToReceive = ri._receivers.size(); 
    string tableName = ri._name;

    std::cout << "NetPlanner Receive: add demux port for " << tableName << " for " 
	      << numElementsToReceive << " elements\n";

    // DupElim -> DemuxS -> Insert -> Duplicator -> Fork
    ElementSpecPtr bufferQueue = 
      _conf->addElement(ElementPtr(new Queue("DemuxQueue:"+ nodeID + ":" + tableName, 
						1000)));
    ElementSpecPtr pullPush = 
      _conf->addElement(ElementPtr(new TimedPullPush("DemuxQueuePullPush" + nodeID 
				       + ":" + tableName, 0)));
    
    hookUp(demuxS, counter++, bufferQueue, 0);
    hookUp(bufferQueue, 0, pullPush, 0);
    
    // duplicator
    ElementSpecPtr duplicator = 
      _conf->addElement(ElementPtr(new DuplicateConservative("DuplicateConservative:"
			  + tableName + ":" + nodeID, numElementsToReceive)));    
    // materialize table only if it is declared and has lifetime>0
    OL_Context::TableInfoMap::iterator _iterator 
      = _ctxt->getTableInfos()->find(tableName);
    if (_iterator != _ctxt->getTableInfos()->end() 
	&& _iterator->second->timeout != 0) {
      ElementSpecPtr insertS 
	= _conf->addElement(ElementPtr(new Insert("Insert:"+ tableName + ":" + nodeID,  
				     getTableByName(nodeID, tableName))));
      
      hookUp(pullPush, 0, insertS, 0);
      genPrintWatchElement("PrintWatchInsert:"+nodeID);

      hookUp(duplicator, 0);
    } else {
      hookUp(pullPush, 0, duplicator, 0);
    }

    // connect the duplicator to elements for this name
    for (uint k = 0; k < ri._receivers.size(); k++) {
      ElementSpecPtr nextElementSpec = ri._receivers.at(k);

      if (_debug) {
	ElementSpecPtr printDuplicator = 
	  _conf->addElement(ElementPtr(new PrintTime("PrintAfterDuplicator:"
					       + tableName + ":" + nodeID)));
	hookUp(duplicator, k, printDuplicator, 0);
	hookUp(printDuplicator, 0, nextElementSpec, 0);
	continue;
      }
      hookUp(duplicator, k, nextElementSpec, 0);
    }
  }

  // connect the acknowledgement port to ccTx
  ElementSpecPtr sinkS 
    = _conf->addElement(ElementPtr(new Discard("discard")));
  hookUp(demuxS, _udpReceivers.size(), sinkS, 0); 
  

  _currentElementChain.push_back(wrapAroundDemux);
  genPrintElement("PrintWrapAround:"+nodeID);
  genPrintWatchElement("PrintWrapAround:"+nodeID);
  
  // connect the orignal wrap around
  hookUp(wrapAroundMux, 1);
  
}


void 
Rtr_ConfGen::registerUDPPushSenders(ElementSpecPtr elementSpecPtr)
{
  _udpSenders.push_back(elementSpecPtr);
  _udpSendersPos.push_back(1);
}


ElementSpecPtr 
Rtr_ConfGen::genSendElements(boost::shared_ptr< Udp> udp, string nodeID)
{
  ElementSpecPtr udpSend = _conf->addElement(udp->get_tx());  

  // prepare to send. Assume all tuples send by first tuple
  
  assert(_udpSenders.size() > 0);
  ElementSpecPtr roundRobin =
    _conf->addElement(ElementPtr(new RoundRobin("roundRobinSender:" + nodeID, 
						   _udpSenders.size()))); 

  ElementSpecPtr pullPush =
      _conf->addElement(ElementPtr(new TimedPullPush("SendPullPush:"+nodeID, 0)));
  hookUp(roundRobin, 0, pullPush, 0);

  // check here for the wrap around
  boost::shared_ptr< std::vector< ValuePtr > > wrapAroundDemuxKeys(new std::vector< ValuePtr >);  
  wrapAroundDemuxKeys->push_back(ValuePtr(new Val_Str(nodeID)));
  ElementSpecPtr wrapAroundDemux 
    = _conf->addElement(ElementPtr(new Demux("wrapAroundSendDemux", wrapAroundDemuxKeys, 0)));  

  hookUp(wrapAroundDemux, 0); // connect to the wrap around

  ElementSpecPtr unBoxWrapAround =
    _conf->addElement(ElementPtr(new UnboxField("UnBoxWrapAround:"+nodeID, 1)));

  hookUp(wrapAroundDemux, 0, unBoxWrapAround, 0);

  ElementSpecPtr sendQueue = 
    _conf->addElement(ElementPtr(new Queue("SendQueue:"+nodeID, 1000)));
  hookUp(wrapAroundDemux, 1, sendQueue, 0); 

  // connect to send queue
  genPrintElement("PrintRemoteSend:"+nodeID);
  genPrintWatchElement("PrintWatchRemoteSend:"+nodeID);

  ///////// Network Out ///////////////
  if (_cc) {
    ostringstream oss;
    oss << "\"" << nodeID << "\" pop swallow pop";
    ElementSpecPtr srcAddress  = 
      _conf->addElement(ElementPtr(new PelTransform("AddSrcAddressCC:"+nodeID, oss.str())));
    ElementSpecPtr seq 
      = _conf->addElement(ElementPtr(new Sequence("SequenceCC" + nodeID)));
    hookUp(srcAddress, 0);
    hookUp(seq, 0);

    // <data, seq, src, <t>>
    ElementSpecPtr tagData  = 
      _conf->addElement(ElementPtr(new PelTransform("TagData:" + nodeID, 
						       "\"ccdata\" pop $0 pop $1 pop $2 pop")));
    hookUp(tagData, 0);

    genPrintElement("PrintRemoteSendCCOne:"+nodeID);

    ElementSpecPtr pullPushCC =
      _conf->addElement(ElementPtr(new TimedPullPush("SendPullPushCC:"+nodeID, 0)));

    hookUp(pullPushCC, 0);
    hookUp(_ccTx, 0); // <seq, addr, <t>>

    // <dst, <seq, addr, <t>>
    ElementSpecPtr encapSendCC =
      _conf->addElement(ElementPtr(new PelTransform("encapSendCC:"+nodeID, 
						    "$3 1 field pop swallow pop"))); 
    hookUp(_ccTx, 0, encapSendCC, 0);

    genPrintElement("PrintRemoteSendCCTwo:"+nodeID);
    
    _roundRobinCC =
       _conf->addElement(ElementPtr(new RoundRobin("roundRobinSenderCC:" + nodeID, 2))); 
     hookUp(_roundRobinCC, 0);

    // acknowledgements. <dst, <ack, seq, windowsize>>
    ElementSpecPtr ackPelTransform
      = _conf->addElement(ElementPtr(new PelTransform("ackPelTransformCC" + nodeID,
							"$0 pop \"ack\" ->t $1 append $2 append pop")));
    
    hookUp(_ccRx, 1, ackPelTransform, 0);
    genPrintElement("PrintSendAck:"+nodeID);

    hookUp(_currentElementChain.back(), 0, _roundRobinCC, 1);

     // Now marshall the payload (second field)
     // <dst, marshalled>
     ElementSpecPtr marshalSendCC = 
       _conf->addElement(ElementPtr(new MarshalField("marshalCC:" + nodeID, 1)));
     genPrintElement("PrintRemoteSendCCMarshal:"+nodeID);
     hookUp(marshalSendCC, 0); 

  } else {

    // Now marshall the payload (second field)
    ElementSpecPtr marshalSend = 
      _conf->addElement(ElementPtr(new MarshalField("marshal:" + nodeID, 1)));  
    hookUp(marshalSend, 0);  
  }
   
  ElementSpecPtr routeSend =
    _conf->addElement(ElementPtr(new StrToSockaddr("router:" + nodeID, 0)));

  hookUp(routeSend, 0);
  hookUp(udpSend, 0);

  // form the push senders
  for (unsigned int k = 0; k < _udpSenders.size(); k++) {
    ElementSpecPtr nextElementSpec = _udpSenders.at(k);

    //std::cout << "Encapsulation " << k << " pop " << _udpSendersPos.at(k) << "\n";

    ostringstream oss;
    oss << "$"<< _udpSendersPos.at(k) << " pop swallow pop";
    ElementSpecPtr encapSend =
      _conf->addElement(ElementPtr(new PelTransform("encapSend:" + nodeID, oss.str())));
    
    // for now, assume addr field is the put here
    //hookUp(nextElementSpec, 0, roundRobin, k);

    hookUp(nextElementSpec, 0, encapSend, 0);
    hookUp(encapSend, 0, roundRobin, k);
  }

  return unBoxWrapAround;
}


// for a particular table name that we are receiving, 
// register an elementSpec that needs that data
void 
Rtr_ConfGen::registerReceiver(string tableName, 
			      ElementSpecPtr elementSpecPtr)
{
  // add to the right receiver
  ReceiverInfoMap::iterator _iterator = _udpReceivers.find(tableName);
  if (_iterator != _udpReceivers.end()) {
    _iterator->second.addReceiver(elementSpecPtr);
  }
}



// regiser a new receiver for a particular table name
// use to later hook up the demuxer
void Rtr_ConfGen::registerReceiverTable(OL_Context::Rule* rule, 
					string tableName)
{  
  ReceiverInfoMap::iterator _iterator = _udpReceivers.find(tableName);
  if (_iterator == _udpReceivers.end()) {
    // not there, we register
    _udpReceivers.insert(std::make_pair(tableName, 
					ReceiverInfo(tableName, 
						     rule->head->args())));
  }  
  debugRule(rule, "Register table " + tableName + "\n");
}
					     



//////////////////////////////////////////////////////////////////
///////////////// Relational Operators -> P2 Elements
//////////////////////////////////////////////////////////////////

string Rtr_ConfGen::pelMath(FieldNamesTracker* names, Parse_Math *expr, 
			    OL_Context::Rule* rule) {
  Parse_Var*  var;
  Parse_Val*  val;
  Parse_Math* math;
  Parse_Function* fn  = NULL;
  ostringstream  pel;  


  if (expr->id && expr->oper == Parse_Math::MINUS) {
    Parse_Expr *tmp = expr->lhs;
    expr->lhs = expr->rhs;
    expr->rhs = tmp;
  }

  if ((var = dynamic_cast<Parse_Var*>(expr->lhs)) != NULL) {
    int pos = names->fieldPosition(var->toString());
    if (pos < 0) {
      error("Pel math error " + expr->toString(), rule);
    }
    pel << "$" << (pos+1) << " ";
  }
  else if ((val = dynamic_cast<Parse_Val*>(expr->lhs)) != NULL) {
    if (val->v->typeCode() == Value::STR)
      pel << "\"" << val->toString() << "\"" << " "; 
    else pel << val->toString() << " "; 

    if (val->id()) pel << "->u32 ->id "; 
    else if (expr->id) pel << "->u32 ->id "; 
  }
  else if ((math = dynamic_cast<Parse_Math*>(expr->lhs)) != NULL) {
    pel << pelMath(names, math, rule); 
  }
  else if ((fn = dynamic_cast<Parse_Function*>(expr->lhs)) != NULL) {
    pel << pelFunction(names, fn, rule); 
  }
  else {    
    // TODO: throw/signal some kind of error
    error("Pel Math error " + expr->toString(), rule);
  }

  if ((var = dynamic_cast<Parse_Var*>(expr->rhs)) != NULL) {
    int pos = names->fieldPosition(var->toString());
    if (pos < 0) {
      error("Pel Math error " + expr->toString(), rule);
    }
    pel << "$" << (pos+1) << " ";
  }
  else if ((val = dynamic_cast<Parse_Val*>(expr->rhs)) != NULL) {    
    if (val->v->typeCode() == Value::STR)
      pel << "\"" << val->toString() << "\"" << " "; 
    else pel << val->toString() << " "; 

    if (val->id()) pel << "->u32 ->id "; 
    else if (expr->id) pel << "->u32 ->id "; 
  }
  else if ((math = dynamic_cast<Parse_Math*>(expr->rhs)) != NULL) {
    pel << pelMath(names, math, rule); 
  }
  else {
    // TODO: throw/signal some kind of error
    error("Math error " + expr->toString(), rule);
  }

  switch (expr->oper) {
    case Parse_Math::LSHIFT:  pel << (expr->id ? "<<id "      : "<< "); break;
    case Parse_Math::RSHIFT:  pel << ">> "; break;
    case Parse_Math::PLUS:    pel << "+ "; break;
    case Parse_Math::MINUS:   pel << "- "; break;
    case Parse_Math::TIMES:   pel << "* "; break;
    case Parse_Math::DIVIDE:  pel << "/ "; break;
    case Parse_Math::MODULUS: pel << "\% "; break;
  default: error("Pel Math error" + expr->toString(), rule);
  }

  return pel.str();
}

string Rtr_ConfGen::pelRange(FieldNamesTracker* names, Parse_Bool *expr,
			     OL_Context::Rule* rule) {			   
  Parse_Var*   var       = NULL;
  Parse_Val*   val       = NULL;
  Parse_Math*  math      = NULL;
  Parse_Var*   range_var = dynamic_cast<Parse_Var*>(expr->lhs);
  Parse_Range* range     = dynamic_cast<Parse_Range*>(expr->rhs);
  ostringstream pel;
  int          pos;

  if (!range || !range_var) {
    error("Math range error " + expr->toString(), rule);
  }

  pos = names->fieldPosition(range_var->toString());
  if (pos < 0) {
    error("Math range error " + expr->toString(), rule);
  }
  pel << "$" << (pos + 1) << " ";

  if ((var = dynamic_cast<Parse_Var*>(range->lhs)) != NULL) {
    pos = names->fieldPosition(var->toString());
    if (pos < 0) {
      error("Math range error " + expr->toString(), rule);
    }
    pel << "$" << (pos + 1) << " ";
  }
  else if ((val = dynamic_cast<Parse_Val*>(range->lhs)) != NULL) {
    pel << val->toString() << " ";
  }
  else if ((math = dynamic_cast<Parse_Math*>(range->lhs)) != NULL) {
   pel << pelMath(names, math, rule);
  }
  else {
    error("Math range error " + expr->toString(), rule);
  }

  if ((var = dynamic_cast<Parse_Var*>(range->rhs)) != NULL) {
    pos = names->fieldPosition(var->toString());
    if (pos < 0) {
      error("Math range error " + expr->toString(), rule);
    }
    pel << "$" << (pos + 1) << " ";
  }
  else if ((val = dynamic_cast<Parse_Val*>(range->rhs)) != NULL) {
    pel << val->toString() << " ";
  }
  else if ((math = dynamic_cast<Parse_Math*>(range->rhs)) != NULL) {
   pel << pelMath(names, math, rule);
  }
  else {
    error("Math range error " + expr->toString(), rule);
  }

  switch (range->type) {
    case Parse_Range::RANGEOO: pel << "() "; break;
    case Parse_Range::RANGEOC: pel << "(] "; break;
    case Parse_Range::RANGECO: pel << "[) "; break;
    case Parse_Range::RANGECC: pel << "[] "; break;
    }

  return pel.str();
}

string Rtr_ConfGen::pelFunction(FieldNamesTracker* names, Parse_Function *expr, 
				OL_Context::Rule* rule) {
  ostringstream pel;

  if (expr->name() == "f_coinFlip") {
    Val_Double &val = dynamic_cast<Val_Double&>(*expr->arg(0)->v);
    pel << val.toString() << " coin "; 
  }
  else if (expr->name() == "f_rand") {
    pel << "rand "; 
  } 
  else if (expr->name() == "f_now") {
    pel << "now "; 
  }
  else {
    error("Pel function error " + expr->toString(), rule);
  }
  return pel.str();
}

string Rtr_ConfGen::pelBool(FieldNamesTracker* names, Parse_Bool *expr,
			    OL_Context::Rule* rule) {
  Parse_Var*      var = NULL;
  Parse_Val*      val = NULL;
  Parse_Function* fn  = NULL;
  Parse_Math*     m   = NULL;
  Parse_Bool*     b   = NULL;
  ostringstream   pel;  

  if (expr->oper == Parse_Bool::RANGE) return pelRange(names, expr, rule);

  bool strCompare = false;
  if ((var = dynamic_cast<Parse_Var*>(expr->lhs)) != NULL) {
    int pos = names->fieldPosition(var->toString());
    if (pos < 0) {
      error("Pel bool error " + expr->toString(), rule);
    }
    pel << "$" << (pos+1) << " ";
  }
  else if ((val = dynamic_cast<Parse_Val*>(expr->lhs)) != NULL) {
    if (val->v->typeCode() == Value::STR) { 
      strCompare = true; 
      pel << "\"" << val->toString() << "\" "; 
    } else {
      strCompare = false;
      pel << val->toString() << " "; 
    }
  }
  else if ((b = dynamic_cast<Parse_Bool*>(expr->lhs)) != NULL) {
    pel << pelBool(names, b, rule); 
  }
  else if ((m = dynamic_cast<Parse_Math*>(expr->lhs)) != NULL) {
    pel << pelMath(names, m, rule); 
  }
  else if ((fn = dynamic_cast<Parse_Function*>(expr->lhs)) != NULL) {
      pel << pelFunction(names, fn, rule); 
  }
  else {
    // TODO: throw/signal some kind of error
    error("Unknown bool operand error " + expr->toString(), rule);
  }

  if (expr->rhs != NULL) {
    if ((var = dynamic_cast<Parse_Var*>(expr->rhs)) != NULL) {
      int pos = names->fieldPosition(var->toString());
      if (pos < 0) {
	error("Pel bool error " + expr->toString(), rule);
      }
      pel << "$" << (pos+1) << " ";
    }
    else if ((val = dynamic_cast<Parse_Val*>(expr->rhs)) != NULL) {      
      if (val->v->typeCode() == Value::STR) { 
	strCompare = true; 
	pel << "\"" << val->toString() << "\" "; 
      } else {
	strCompare = false;
	pel << val->toString() << " "; 
      }
    }
    else if ((b = dynamic_cast<Parse_Bool*>(expr->rhs)) != NULL) {
      pel << pelBool(names, b, rule); 
    }
    else if ((m = dynamic_cast<Parse_Math*>(expr->rhs)) != NULL) {
      pel << pelMath(names, m, rule); 
    }
    else if ((fn = dynamic_cast<Parse_Function*>(expr->rhs)) != NULL) {
      pel << pelFunction(names, fn, rule); 
    }
    else {
      // TODO: throw/signal some kind of error
      error("Unknown bool operand error " + expr->toString(), rule);
    }
  }

  switch (expr->oper) {
    case Parse_Bool::NOT: pel << "not "; break;
    case Parse_Bool::AND: pel << "and "; break;
    case Parse_Bool::OR:  pel << "or "; break;
    case Parse_Bool::EQ:  pel << "== "; break;
    case Parse_Bool::NEQ: pel << "== not "; break;
    case Parse_Bool::GT:  pel << "> "; break;
    case Parse_Bool::LT:  pel << "< "; break;
    case Parse_Bool::LTE: pel << "<= "; break;
    case Parse_Bool::GTE: pel << ">= "; break;
    default: error("Unknown bool operand error " + expr->toString(), rule);
    }
  return pel.str();
}

void 
Rtr_ConfGen::pelSelect(OL_Context::Rule* rule, FieldNamesTracker* names, 
		       Parse_Select *expr,
		       string nodeID, int selectionID)
{
  ostringstream sPel;
  sPel << pelBool(names, expr->select, rule) << "not ifstop ";
  
  // put in the old fields (+1 since we have to include the table name)
  for (uint k = 0; k < names->fieldNames.size() + 1; k++) {
    sPel << "$" << k << " pop ";
  }

  debugRule(rule, "Generate selection functions for " + sPel.str() +
                  " " + names->toString() + "\n");
 
  ostringstream oss;
  oss << "Selection:" << rule->ruleID << ":" << selectionID << ":"
      << nodeID;

  ElementSpecPtr sPelTrans =
    _conf->addElement(ElementPtr(new PelTransform(oss.str(), sPel.str())));
  
  if (_isPeriodic == false &&_pendingRegisterReceiver) {
    _pendingReceiverSpec = sPelTrans;
    _currentElementChain.push_back(sPelTrans); // first element in chain
    _pendingRegisterReceiver = false;
  } else {
    // connecting now
    hookUp(sPelTrans, 0);
  }

  genPrintElement(string("PrintAfterSelection:")+":"+nodeID);
}

void 
Rtr_ConfGen::genAllSelectionAssignmentElements(OL_Context::Rule* curRule,
					       string nodeID,
					       FieldNamesTracker* 
					       curNamesTracker) 
{
  for (unsigned int j = 0; j < curRule->terms.size(); j++) {
    Parse_Select* parse_select 
      = dynamic_cast<Parse_Select *>(curRule->terms.at(j));
    if (parse_select != NULL) {
      debugRule(curRule, "Selection term " + parse_select->toString() + " " 
			     + curRule->ruleID + "\n");
      pelSelect(curRule, curNamesTracker, parse_select, nodeID, j); 
    }
    Parse_Assign* parse_assign 
      = dynamic_cast<Parse_Assign *>(curRule->terms.at(j));
    if (parse_assign != NULL) {
      pelAssign(curRule, curNamesTracker, parse_assign, nodeID, j);
    }
  }
}

void Rtr_ConfGen::pelAssign(OL_Context::Rule* rule, 
			    FieldNamesTracker* names,
                            Parse_Assign* expr, 
			    string nodeID, 
			    int assignID) 
{
  ostringstream pel;
  ostringstream pelAssign;
  Parse_Var      *a   = dynamic_cast<Parse_Var*>(expr->var);
  Parse_Var      *var = NULL;
  Parse_Val      *val = NULL;
  Parse_Bool     *b   = NULL;
  Parse_Math     *m   = NULL;
  Parse_Function *f   = NULL; 

  if (expr->assign == Parse_Expr::Now) {
    pelAssign << "now "; 
  }
  else if ((b = dynamic_cast<Parse_Bool*>(expr->assign)) != NULL) {
    pelAssign << pelBool(names, b, rule);
  }
  else if ((m = dynamic_cast<Parse_Math*>(expr->assign)) != NULL) {
    string pelMathStr = pelMath(names, m, rule); 
    pelAssign << pelMathStr;
  }
  else if ((f = dynamic_cast<Parse_Function*>(expr->assign)) != NULL) {
    pelAssign << pelFunction(names, f, rule);
  }
  else if ((var=dynamic_cast<Parse_Var*>(expr->assign)) != NULL && 
           names->fieldPosition(var->toString()) >= 0) {
    pelAssign << "$" << (names->fieldPosition(var->toString())+1) << " ";
  }
  else if ((val=dynamic_cast<Parse_Val*>(expr->assign)) != NULL) {
    if (val->v->typeCode() == Value::STR) { 
      pelAssign << "\"" << val->toString() << "\" ";
    } else {
      pelAssign << val->toString() << " ";
    }
  } else {
    error("Pel Assign error " + expr->toString(), rule);
    assert(0);
  }
   
  int pos = names->fieldPosition(a->toString());
  for (int k = 0; k < int(names->fieldNames.size()+1); k++) {
    if (k == pos) { 
      pel << pelAssign << "pop ";
    } 
    else {
      pel << "$" << k << " pop ";
    }
  }
  if (pos < 0) { 
    pel << pelAssign.str() << "pop ";
    names->fieldNames.push_back(a->toString()); // the variable name
  } 

  debugRule(rule, "Generate assignments for " + a->toString() + " " 
	    + rule->ruleID + " " + pel.str() + " " 
	    + names->toString() + "\n");

  ostringstream oss1;
  oss1 << "Assignment:" << rule->ruleID << ":" << assignID << ":" << nodeID;
  ElementSpecPtr assignPelTrans =
    _conf->addElement(ElementPtr(new PelTransform(oss1.str(),
						  pel.str())));

  if (_isPeriodic == false &&_pendingRegisterReceiver) {
    _pendingReceiverSpec = assignPelTrans;
    _currentElementChain.push_back(assignPelTrans); // first element in chain
    _pendingRegisterReceiver = false;
  } else {
    // connecting now
    hookUp(assignPelTrans, 0);
  }

  ostringstream oss;
  oss << "PrintAfterAssignment:" << rule->ruleID<<":"<<assignID<<":"<<nodeID;

  genPrintElement(oss.str());
}


void Rtr_ConfGen::genProjectHeadElements(OL_Context::Rule* curRule,
					 string nodeID,
					 FieldNamesTracker* curNamesTracker)
{
  Parse_Functor* pf = curRule->head;
  // determine the projection fields, and the first address to return. 
  // Add 1 for table name     
  std::vector<unsigned int> indices;  
  int locationIndex = -1;
  // iterate through all functor's output
  for (int k = 0; k < pf->args(); k++) {
    Parse_Var* parse_var = dynamic_cast<Parse_Var*>(pf->arg(k));
    int pos = -1;
    if (parse_var != NULL) {
      //warn << "Check " << parse_var->toString() << " " << pf->fn->loc << "\n";
      if (parse_var->toString() == pf->fn->loc) {	
	locationIndex = k;
      }
      // care only about vars    
      pos = curNamesTracker->fieldPosition(parse_var->toString());    
      if (pos == -1) {
	error("Head predicate \"" + pf->fn->name 
	      + "\" has invalid variable " + parse_var->toString(), curRule);
      } 
    }
    if (k == pf->aggregate()) {
      // as input into aggwrap
      Parse_Agg* aggExpr = dynamic_cast<Parse_Agg*>(curRule->head->arg(k));
      //warn << "Check " << aggExpr->v->toString() << " " << pf->fn->loc << "\n";
      if (aggExpr->v->toString() == pf->fn->loc) {	
	locationIndex = k;
      }
      if (aggExpr->aggName() != "count") {
	pos = curNamesTracker->fieldPosition(aggExpr->v->toString());
	if (pos == -1) {
	  error("Head predicate \"" + pf->fn->name 
		+ "\" has invalid variable " + aggExpr->v->toString(), curRule);
	} 
      }
    }
    if (pos == -1) { 
      continue; 
    }    
    indices.push_back(pos + 1);
  }

  if (locationIndex == -1 && pf->fn->loc != "") {
    error("Head predicate \"" + pf->fn->name + "\" has invalid location specifier " 
	  + pf->fn->loc, curRule);
  }
						  
  if (locationIndex == -1) { 
    locationIndex = 0;     
  } // default

  ostringstream pelTransformStrbuf;
  pelTransformStrbuf << "\"" << pf->fn->name << "\" pop";

  if (pf->aggregate() != -1 && pf->fn->loc == "") {
    pelTransformStrbuf << " \"" << nodeID << "\" pop";
  }

  for (unsigned int k = 0; k < indices.size(); k++) {
    pelTransformStrbuf << " $" << indices.at(k) << " pop";
  }

  int aggField = curRule->head->aggregate();
  if (aggField != -1) {
    Parse_Agg* aggExpr 
      = dynamic_cast<Parse_Agg*>(curRule->head->arg(aggField));
    if (aggExpr->aggName() == "count") {
      // output the count
      pelTransformStrbuf << " $" << (aggField + 1) << " pop"; 
    } 
  }

  string pelTransformStr = pelTransformStrbuf.str();
  ostringstream oss;
  oss << "Project head " << curNamesTracker->toString() 
      << " " << pelTransformStr + " " << locationIndex << " " <<
    pf->fn->loc << "\n";
  debugRule(curRule, oss.str());
 
  _currentPositionIndex = locationIndex + 1;
  // project, and make sure first field after table name has the address 
  ElementSpecPtr projectHeadPelTransform =
    _conf->addElement(ElementPtr(new PelTransform("ProjectHead:"+ curRule->ruleID + ":" + nodeID,
						     pelTransformStr)));
  if (_isPeriodic == false && _pendingRegisterReceiver) {
    _pendingReceiverSpec = projectHeadPelTransform;
    _currentElementChain.push_back(projectHeadPelTransform); 
    _pendingRegisterReceiver = false;
  } else {
    // connecting now
    hookUp(projectHeadPelTransform, 0);
  }
  
  genPrintElement("PrintHead:"+ curRule->ruleID + ":" + nodeID);  
}


void Rtr_ConfGen::genProbeElements(OL_Context::Rule* curRule, 
				   Parse_Functor* eventFunctor, 
				   Parse_Term* baseTableTerm, 
				   string nodeID, 	     
				   FieldNamesTracker* probeNames, 
				   FieldNamesTracker* baseProbeNames, 
				   int joinOrder,
				   b_cbv comp_cb)
{
  // probe the right hand side
  // Here's where the join happens 
  std::vector<int> leftJoinKeys 
    = baseProbeNames->matchingJoinKeys(probeNames->fieldNames);
  std::vector<int> rightJoinKeys 
    =  probeNames->matchingJoinKeys(baseProbeNames->fieldNames);

  Parse_Functor* pf = dynamic_cast<Parse_Functor*>(baseTableTerm);
  Parse_RangeFunction* pr 
    = dynamic_cast<Parse_RangeFunction*>(baseTableTerm);    

  string baseTableName;
  if (pf != NULL) {
    baseTableName = pf->fn->name;
    checkFunctor(pf, curRule);
  }
  if (pr != NULL) {
    baseTableName = "range" + curRule->ruleID;
  }

  

  if (leftJoinKeys.size() == 0 || rightJoinKeys.size() == 0) {
    error("No matching join keys " + eventFunctor->fn->name + " " + 
	  baseTableName + " ", curRule);
  }

  // add one to offset for table name. Join the first matching key
  int leftJoinKey = leftJoinKeys.at(0) + 1;
  int rightJoinKey = rightJoinKeys.at(0) + 1;

  TablePtr probeTable = getTableByName(nodeID, baseTableName);

  // should we use a uniqLookup or a multlookup? 
  // Check that the rightJoinKey is the primary key
  OL_Context::TableInfo* tableInfo 
    = _ctxt->getTableInfos()->find(baseTableName)->second;
  ostringstream oss;
  oss << "NoNull:" << curRule->ruleID << ":" << joinOrder << ":" << nodeID;

  ElementSpecPtr noNull 
    = _conf->addElement(ElementPtr(new NoNullField(oss.str(), 1)));

  ElementSpecPtr last_el(new ElementSpec(ElementPtr(new Slot("dummySlotProbeElements"))));
 
  if (tableInfo->primaryKeys.size() == 1 && 
      tableInfo->primaryKeys.at(0) == rightJoinKey) {
    // use a unique index
    ostringstream oss;
    oss << "UniqueLookup:" << curRule->ruleID << ":" << joinOrder << ":" << nodeID; 
    last_el =
      _conf->addElement(ElementPtr(new UniqueLookup(oss.str(),
						    probeTable,
						    leftJoinKey, 
						    rightJoinKey, 
						    comp_cb)));
  } else {
    ostringstream oss;
    oss << "MultLookup:" << curRule->ruleID << ":" << joinOrder << ":" << nodeID; 
    last_el =
      _conf->addElement(ElementPtr(new MultLookup(oss.str(),
						  probeTable,
						  leftJoinKey, 
						  rightJoinKey, comp_cb)));
    
    addMultTableIndex(probeTable, rightJoinKey, nodeID);
  }
  
 
  int numFieldsProbe = probeNames->fieldNames.size();
  debugRule(curRule, "Probe before merge " + probeNames->toString() + "\n");
  probeNames->mergeWith(baseProbeNames->fieldNames); 
  debugRule(curRule, "Probe after merge " + probeNames->toString() + "\n");

  if (_isPeriodic == false && _pendingRegisterReceiver) {
    // connecting to udp receiver later
    _pendingReceiverSpec = last_el;
    _pendingRegisterReceiver = false;
  } else {
    // connecting now to prior element
    hookUp(last_el, 0);  
  }

  hookUp(last_el, 0, noNull, 0);

  for (uint k = 1; k < leftJoinKeys.size(); k++) {
    int leftField = leftJoinKeys.at(k);
    int rightField = rightJoinKeys.at(k);
    ostringstream selectionPel;
    selectionPel << "$0 " << (leftField+1) << " field " << " $1 " 
		 << rightField+1 << " field ==s not ifstop $0 pop $1 pop";

    debugRule(curRule, "Join selections " + selectionPel.str() + "\n");

    ostringstream oss;
    oss << "JoinSelections:" << curRule->ruleID << ":"
	<< joinOrder << ":"
	<< k << ":" << nodeID;
    
    ElementSpecPtr joinSelections =
      _conf->addElement(ElementPtr(new PelTransform(oss.str(), 
				                    selectionPel.str())));    
    hookUp(joinSelections, 0);
  }

  // Take the joined tuples and produce the resulting path
  // form the pel projection. 
  //Keep all fields on left, all fields on right except the join keys
  ostringstream pelProject;
  pelProject << "\"join:" << eventFunctor->fn->name << ":" << baseTableName << ":" 
	     << curRule->ruleID << ":" << nodeID << "\" pop "; // table name
  for (int k = 0; k < numFieldsProbe; k++) {
    pelProject << "$0 " << k+1 << " field pop ";
  }
  for (uint k = 0; k < baseProbeNames->fieldNames.size(); k++) {
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

  string pelProjectStr = pelProject.str();
  ostringstream oss1; 
  oss1 << "JoinPelTransform:"
      << curRule->ruleID << ":"
      << joinOrder << ":" << nodeID;

  ElementSpecPtr transS 
    = _conf->addElement(ElementPtr(new PelTransform(oss1.str(), pelProjectStr)));

  delete baseProbeNames;

  hookUp(transS, 0);
}



void Rtr_ConfGen::genJoinElements(OL_Context::Rule* curRule, 
				  string nodeID, 
				  FieldNamesTracker* namesTracker,
				  boost::shared_ptr<Aggwrap> agg_el)
{
  // identify the events, use that to probe the other matching tables
  Parse_Functor* eventFunctor;
  std::vector<Parse_Term*> baseFunctors;
  bool eventFound = false;

  for (unsigned int k = 0; k < curRule->terms.size(); k++) {
    Parse_Functor* pf = dynamic_cast<Parse_Functor*>(curRule->terms.at(k));
    if (pf != NULL) {
      checkFunctor(pf, curRule);
      string functorName = pf->fn->name;    
      OL_Context::TableInfoMap::iterator _iterator 
	= _ctxt->getTableInfos()->find(functorName);
      if (_iterator != _ctxt->getTableInfos()->end()) {     
	baseFunctors.push_back(pf);
      } else {
	// assume one event per not.
	// event probes local base tables
	if (_isPeriodic == false) {
	  registerReceiverTable(curRule, functorName); 
	  _pendingRegisterReceiver = true;
	  _pendingReceiverTable = functorName;
	}
	if (eventFound == true) {
	  error("There can be only one event predicate in a rule. Check all the predicates ", curRule);
	}
	eventFunctor = pf;
	eventFound = true;
      } 
    }

    // check if range, if so, create a table, add to baseFunctors
    Parse_RangeFunction* pr 
      = dynamic_cast<Parse_RangeFunction*>(curRule->terms.at(k));    
    if (pr != NULL) {
      // create table for range. Initialize with values      
      baseFunctors.push_back(pr);

      int32_t low = 0, high = 0;
      low = Val_Int32::cast(pr->start->v);
      high = Val_Int32::cast(pr->end->v);

      OL_Context::TableInfo  *tableInfo = new OL_Context::TableInfo();
      tableInfo->tableName = "range" + curRule->ruleID;
      tableInfo->timeout = -1; // never expire
      tableInfo->size = high - low + 1;
      tableInfo->primaryKeys.push_back(2);
      _ctxt->getTableInfos()->insert(std::make_pair(tableInfo->tableName, 
						    tableInfo));
      
      string newRangeTableName = nodeID + ":" + tableInfo->tableName;
      TablePtr rangeTable(new Table(tableInfo->tableName, (high - low + 1)));
      rangeTable->add_unique_index(2);
      addMultTableIndex(rangeTable, 1, nodeID);
      _tables.insert(std::make_pair(newRangeTableName, rangeTable));         
 
      for (int x = low; x <= high; x++) {
	TuplePtr tuple = Tuple::mk();
	tuple->append(Val_Str::mk(tableInfo->tableName));
	tuple->append(Val_Str::mk(nodeID));
	tuple->append(Val_Int32::mk(x));
	tuple->freeze();
	rangeTable->insert(tuple);
      }
    }
  }
  if (_isPeriodic == false) {
    debugRule(curRule, "Event term " + eventFunctor->fn->name + "\n");
    // for all the base tuples, use the join to probe. 
    // keep track also the cur ordering of variables
    namesTracker->initialize(eventFunctor);
  } else {
    debugRule(curRule, "Periodic joins " + namesTracker->toString() + "\n");
  }
  for (uint k = 0; k < baseFunctors.size(); k++) {    
    Parse_Functor* pf = dynamic_cast<Parse_Functor*>(baseFunctors.at(k));
    
    if (pf != NULL && pf->fn->name == eventFunctor->fn->name) { continue; } 
    debugRule(curRule, "Probing " + eventFunctor->fn->name + " " + 
                       baseFunctors.at(k)->toString() + "\n");
    b_cbv comp_cb = 0;
    if (agg_el) {
      comp_cb = agg_el->get_comp_cb();
    }
    
    FieldNamesTracker* baseProbeNames 
      = new FieldNamesTracker(baseFunctors.at(k));
    
    if (eventFunctor->fn->loc != pf->fn->loc) {
      error("Event " + eventFunctor->fn->name + "@" + eventFunctor->fn->loc + 
	    " and predicate " + pf->fn->name + "@" + pf->fn->loc  
	    + " should have the same location specifier", curRule);
    }

    genProbeElements(curRule, eventFunctor, baseFunctors.at(k), 
		     nodeID, namesTracker, baseProbeNames, k, comp_cb);

    if (agg_el || baseFunctors.size() - 1 != k) {
      // Change from pull to push
      ostringstream oss;
      oss << "JoinPullPush:" << curRule->ruleID << ":"
	  << nodeID << ":" << k;
      ElementSpecPtr pullPush =
	_conf->addElement(ElementPtr(new TimedPullPush(oss.str(), 0)));
      hookUp(pullPush, 0);
    }
  }
}


void Rtr_ConfGen::addMultTableIndex(TablePtr table, int fn, string nodeID)
{
  ostringstream uniqStr;
  uniqStr << table->name << ":" << fn << ":" << nodeID;
  if (_multTableIndices.find(uniqStr.str()) == _multTableIndices.end()) {
    // not there yet
    table->add_multiple_index(fn);
    _multTableIndices.insert(std::make_pair(uniqStr.str(), uniqStr.str()));
    std::cout << "AddMultTableIndex: Mult index added " << uniqStr.str() 
	      << "\n";
  } else {
    std::cout << "AddMultTableIndex: Mult index already exists " 
	      << uniqStr.str() << "\n";
  }
}



void 
Rtr_ConfGen::genSingleAggregateElements(OL_Context::Rule* currentRule, 
					string nodeID, 
					FieldNamesTracker* baseNamesTracker)
{

  Parse_Functor* baseFunctor;
  // figure first, which term is the base term. 
  // Assume there is only one for now. Support more in future.
  for (unsigned int j = 0; j < currentRule->terms.size(); j++) {    
    Parse_Functor* currentFunctor 
      = dynamic_cast<Parse_Functor* > (currentRule->terms.at(j));    
    if (currentFunctor == NULL) { continue; }
    baseFunctor = currentFunctor;
    checkFunctor(baseFunctor, currentRule);
  }

  baseNamesTracker->initialize(baseFunctor);

  std::vector<unsigned int> groupByFields;      
  
  FieldNamesTracker* aggregateNamesTracker = new FieldNamesTracker();
  Parse_Functor* pf = currentRule->head;
  string headTableName = pf->fn->name;

  for (int k = 0; k < pf->args(); k++) {
    // go through the functor head, but skip the aggField itself    
    Parse_Var* pv = dynamic_cast<Parse_Var* > (pf->arg(k));
    if (pv == NULL) { continue; }
    int pos = baseNamesTracker->fieldPosition(pv->toString());
    if (k != -1 && k != pf->aggregate()) {
      groupByFields.push_back((uint) pos + 1);
      aggregateNamesTracker->fieldNames.push_back(baseNamesTracker->fieldNames.at(pos));
    }
  }
  Parse_Agg* pa = dynamic_cast<Parse_Agg* > (pf->arg(pf->aggregate()));

  string aggVarname = pa->v->toString();

  aggregateNamesTracker->fieldNames.push_back(aggVarname);      
  int aggFieldBaseTable = -1;
  Table::AggregateFunction* af = 0;
  
  if (pa->oper == Parse_Agg::MIN) {
    // aggregate for min
    aggFieldBaseTable = baseNamesTracker->fieldPosition(aggVarname) + 1;
    af = &Table::AGG_MIN;
  } 
  if (pa->oper == Parse_Agg::MAX) {
    // aggregate for min
    aggFieldBaseTable = baseNamesTracker->fieldPosition(aggVarname) + 1;
    af = &Table::AGG_MAX;
  } 

  if (pa->oper == Parse_Agg::COUNT) {
    // aggregate for min
    aggFieldBaseTable = groupByFields.at(0);
    af = &Table::AGG_COUNT;
  } 
  
  // get the table, create the index
  TablePtr aggTable = getTableByName(nodeID, baseFunctor->fn->name);  
  addMultTableIndex(aggTable, groupByFields.at(0), nodeID);  
  Table::MultAggregate tableAgg 
    = aggTable->add_mult_groupBy_agg(groupByFields.at(0), // groupby field
				     groupByFields,
				     aggFieldBaseTable, // the agg field
				     af);
  ElementSpecPtr aggElement =
    _conf->addElement(ElementPtr(new Aggregate("Agg:"+currentRule->ruleID +
					       ":" + nodeID, tableAgg)));
   
  ostringstream pelTransformStr;
  pelTransformStr << "\"" << "aggResult:" << currentRule->ruleID << "\" pop";
  for (uint k = 0; k < aggregateNamesTracker->fieldNames.size(); k++) {
    pelTransformStr << " $" << k << " pop";
  }
  debugRule(currentRule, "Agg Pel Expr " + pelTransformStr.str()+ "\n");
  // apply PEL to add a table name
  ElementSpecPtr addTableName =
    _conf->addElement(ElementPtr(new PelTransform("Aggregation:"+currentRule->ruleID 
						     + ":" + nodeID, 
						     pelTransformStr.str())));

  hookUp(aggElement, 0, addTableName, 0);

  genPrintElement("PrintAgg:" + currentRule->ruleID + ":" + nodeID);
  genPrintWatchElement("PrintWatchAgg:" + currentRule->ruleID + ":" + nodeID);

  genProjectHeadElements(currentRule, nodeID, aggregateNamesTracker);
  _udpSenders.push_back(_currentElementChain.back());
  _udpSendersPos.push_back(1);

  registerReceiverTable(currentRule, headTableName);
}



void Rtr_ConfGen::genSingleTermElement(OL_Context::Rule* curRule, 
				       string nodeID, 
				       FieldNamesTracker* curNamesTracker)
{  
  ElementSpecPtr slotElement 
    = _conf->addElement(ElementPtr(new Slot("singleTermSlot:" 
					     + curRule->ruleID + ":" 
					     + nodeID)));

  //std::cout << "Number of terms " << curRule->terms.size() << "\n";
  for (unsigned int j = 0; j < curRule->terms.size(); j++) {
    // skip those that we already decide is going to participate in     
    Parse_Term* curTerm = curRule->terms.at(j);    
    // skip the following
    Parse_Functor* pf = dynamic_cast<Parse_Functor*>(curTerm);
    if (pf == NULL) { continue; }
    registerReceiverTable(curRule, pf->fn->name); 
    registerReceiver(pf->fn->name, slotElement);
    _currentElementChain.push_back(slotElement);

    curNamesTracker->initialize(pf);    
    return; 
  }
}


void Rtr_ConfGen::genFunctorSource(OL_Context::Rule* rule, 
				   string nodeID, 
				   FieldNamesTracker* namesTracker)
{
  TuplePtr functorTuple = Tuple::mk();
  functorTuple->append(Val_Str::mk(rule->head->fn->name));
  functorTuple->append(Val_Str::mk(nodeID)); 
  functorTuple->freeze();

  ElementSpecPtr source =
    _conf->addElement(ElementPtr(new TupleSource("FunctorSource:"+rule->ruleID+nodeID,
                                                   functorTuple)));
  _currentElementChain.push_back(source);

  genPrintElement("PrintFunctorSource:"+rule->ruleID+":"+nodeID);
  Parse_Functor* pf = dynamic_cast<Parse_Functor* > (rule->terms.at(0));
  if (pf == NULL) { return; }
  
  string period = pf->arg(2)->toString();
  int count = 0;
  if (pf->args() > 3) {
    count = atoi(pf->arg(3)->toString().c_str());
  }

  namesTracker->fieldNames.push_back(pf->arg(0)->toString());
  namesTracker->fieldNames.push_back("E");
  
  // a pel transform that puts in the periodic stuff
  ElementSpecPtr pelRand = 
    _conf->addElement(ElementPtr(new PelTransform("FunctorSourcePel:" + rule->ruleID +
                                                  ":" + nodeID, "$0 pop $1 pop rand pop")));
 
  hookUp(pelRand, 0);

  // The timed pusher
  ElementSpecPtr pushFunctor =
    _conf->addElement(ElementPtr(new TimedPullPush("FunctorPush:" +rule->ruleID+ ":"+ nodeID,
						      atof(period.c_str()), count)));

  hookUp(pushFunctor, 0);

  //  if (rule->terms.size() <= 1) {
  if (numFunctors(rule) <= 1) {
    ElementSpecPtr functorSlot 
      = _conf->addElement(ElementPtr(new Slot("functorSlot:" + rule->ruleID + ":" + nodeID)));      
    hookUp(functorSlot, 0);
  }
}


void Rtr_ConfGen::genDupElimElement(string header)
{
   if (_dups) {
     ElementSpecPtr dupElim 
       = _conf->addElement(ElementPtr(new DupElim(header)));
     hookUp(dupElim, 0);
  }
}


void Rtr_ConfGen::genPrintElement(string header)
{
  if (_debug) {
    ElementSpecPtr print = 
      _conf->addElement(ElementPtr(new PrintTime(header)));
    hookUp(print, 0);
  }
}

void Rtr_ConfGen::genPrintWatchElement(string header)
{
  ElementSpecPtr printWatchElement = 
      _conf->addElement(ElementPtr(new PrintWatch(header, _ctxt->getWatchTables())));
  hookUp(printWatchElement, 0);
}






///////////////////////////////////////////////////////////////////


////////////////////////// Table Creation ///////////////////////////

// Get a handle to the table. Typically used by the driver program to 
// preload some data.
TablePtr Rtr_ConfGen::getTableByName(string nodeID, string tableName)
{
  //std::cout << "Get table " << nodeID << ":" << tableName << "\n";
  TableMap::iterator _iterator = _tables.find(nodeID + ":" + tableName);
  if (_iterator == _tables.end()) { 
    error("Table " + nodeID + ":" + tableName + " not found\n");
  }
  return _iterator->second;
}

void Rtr_ConfGen::createTables(string nodeID)
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
      string newTableName = nodeID + ":" + tableInfo->tableName;
      TablePtr newTable(new Table(tableInfo->tableName, tableInfo->size));
      if (tableInfo->timeout != -1) {
	timespec expiration;
	expiration.tv_sec = tableInfo->timeout;
	expiration.tv_nsec = 0;
	newTable.reset(new Table(tableInfo->tableName, tableInfo->size, expiration));
      }

      // first create unique indexes
      std::vector<int> primaryKeys = tableInfo->primaryKeys;
      for (uint k = 0; k < primaryKeys.size(); k++) {
	newTable->add_unique_index(primaryKeys.at(k));
	std::cout << "Create Tables: Add unique index " 
		  << newTableName << " " << 
	  primaryKeys.at(k) << " " << tableInfo->timeout << "\n";
      }

      _tables.insert(std::make_pair(newTableName, newTable));      
    }
  }

  for (unsigned int k = 0; k < _ctxt->getFacts().size(); k++) {
    TuplePtr tr = _ctxt->getFacts().at(k);
    ValuePtr vr = (*tr)[0];
    std::cout << "Insert tuple " << tr->toString() << " into table " 
	      << vr->toString() << " " << tr->size() << "\n";
    TablePtr tableToInsert = getTableByName(nodeID, vr->toString());     
    
    tableToInsert->insert(tr);
    std::cout << "Tuple inserted: " << tr->toString() 
	      << " into table " << vr->toString() 
	      << " " << tr->size() << "\n";
  }
}

/////////////////////////////////////////////

///////////// Helper functions

void Rtr_ConfGen::hookUp(ElementSpecPtr firstElement, int firstPort,
			 ElementSpecPtr secondElement, int secondPort)
{
  fprintf(_output, "Connect: \n");
  fprintf(_output, "  %s %s %d\n", firstElement->toString().c_str(), 
	  firstElement->element()->_name.c_str(), firstPort);
  fprintf(_output, "  %s %s %d\n", secondElement->toString().c_str(), 
	  secondElement->element()->_name.c_str(), secondPort);
  fflush(_output);
  
  _conf->hookUp(firstElement, firstPort, secondElement, secondPort);

  if (_currentElementChain.size() == 0) {
    // first time
    _currentElementChain.push_back(firstElement);
  } 
  _currentElementChain.push_back(secondElement);

}

void Rtr_ConfGen::hookUp(ElementSpecPtr secondElement, int secondPort)
{
  if (_currentElementChain.size() == 0) {
    _currentElementChain.push_back(secondElement);
    assert(secondPort == 0);
    return;
  }
  hookUp(_currentElementChain.back(), 0, secondElement, secondPort);
}


int Rtr_ConfGen::numFunctors(OL_Context::Rule* rule)
{
  int count = 0;

  for (unsigned int k = 0; k < rule->terms.size(); k++) {
    Parse_Functor* pf = dynamic_cast<Parse_Functor*>(rule->terms.at(k));
    if (pf != NULL) { count ++; continue; }

    Parse_RangeFunction* pr 
      = dynamic_cast<Parse_RangeFunction*>(rule->terms.at(k));
    if (pr != NULL) { count ++; continue; }
    
  }

  return count;

}


bool Rtr_ConfGen::hasEventTerm(OL_Context::Rule* curRule)
{
  for (unsigned int k = 0; k < curRule->terms.size(); k++) {
    Parse_Functor* pf = dynamic_cast<Parse_Functor*>(curRule->terms.at(k));
    if (pf == NULL) { continue;}
    string termName = pf->fn->name;
    OL_Context::TableInfoMap::iterator _iterator 
      = _ctxt->getTableInfos()->find(termName);
    if (_iterator == _ctxt->getTableInfos()->end()) {     
      debugRule(curRule, "Found event term " + termName);
      // an event
      return true;
    }
  }
  return false;
}


Parse_Functor* Rtr_ConfGen::getEventTerm(OL_Context::Rule* curRule)
{
  for (unsigned int k = 0; k < curRule->terms.size(); k++) {
    Parse_Functor* pf = dynamic_cast<Parse_Functor*>(curRule->terms.at(k));
    if (pf == NULL) { continue;}
    string termName = pf->fn->name;
    OL_Context::TableInfoMap::iterator _iterator 
      = _ctxt->getTableInfos()->find(termName);
    if (_iterator == _ctxt->getTableInfos()->end()) {     
      return pf;
    }
  }
  return NULL;
}



bool Rtr_ConfGen::hasPeriodicTerm(OL_Context::Rule* curRule)
{
  for (uint k = 0; k < curRule->terms.size(); k++) {
    Parse_Functor* pf = dynamic_cast<Parse_Functor*>(curRule->terms.at(k));
    if (pf == NULL) { continue;}
    checkFunctor(pf, curRule);
    string termName = pf->fn->name;
    if (termName == "periodic") {
      return true;
    }
  }
  return false;
}



///////////////////////////////////////////////////////////////////////////

Rtr_ConfGen::FieldNamesTracker::FieldNamesTracker() { }

Rtr_ConfGen::FieldNamesTracker::FieldNamesTracker(Parse_Term* pf)
{
  initialize(pf);
}


void 
Rtr_ConfGen::FieldNamesTracker::initialize(Parse_Term* term)
{
  Parse_Functor* pf = dynamic_cast<Parse_Functor* > (term);    
 
  if (pf != NULL) {
    for (int k = 0; k < pf->args(); k++) {
      Parse_Var* parse_var = dynamic_cast<Parse_Var*>(pf->arg(k));
      fieldNames.push_back(parse_var->toString());
    }  
  }

  Parse_RangeFunction* pr = dynamic_cast<Parse_RangeFunction* > (term);    
  if (pr != NULL) {
    fieldNames.push_back(string("NI"));
    fieldNames.push_back(string(pr->var->toString()));
  }
}


std::vector<int> 
Rtr_ConfGen::FieldNamesTracker::matchingJoinKeys(std::vector<string> 
						 otherArgNames)
{
  // figure out the matching on other side. Assuming that
  // there is only one matching key for now
  std::vector<int> toRet;
  for (unsigned int k = 0; k < otherArgNames.size(); k++) {
    string nextStr = otherArgNames.at(k);
    if (fieldPosition(nextStr) != -1) {
      // exists
      toRet.push_back(k);
    }
  }  
  return toRet;
}

int 
Rtr_ConfGen::FieldNamesTracker::fieldPosition(string var)
{
  for (uint k = 0; k < fieldNames.size(); k++) {
    if (fieldNames.at(k) == var) {
      return k;
    }
  }
  return -1;
}

void 
Rtr_ConfGen::FieldNamesTracker::mergeWith(std::vector<string> names)
{
  for (uint k = 0; k < names.size(); k++) {
    string nextStr = names.at(k);
    if (fieldPosition(nextStr) == -1) {
      fieldNames.push_back(nextStr);
    }
  }
}

void 
Rtr_ConfGen::FieldNamesTracker::mergeWith(std::vector<string> names, 
					  int numJoinKeys)
{
  int count = 0;
  for (uint k = 0; k < names.size(); k++) {
    string nextStr = names.at(k);
    if (count == numJoinKeys || fieldPosition(nextStr) == -1) {
      fieldNames.push_back(nextStr);
      count++;
    }
  }
}

string Rtr_ConfGen::FieldNamesTracker::toString()
{
  ostringstream toRet;

  toRet << "FieldNamesTracker<";
  
  for (uint k = 0; k < fieldNames.size(); k++) {
    toRet << fieldNames.at(k) << " ";
  }
  toRet << ">";
  return toRet.str();
}

void Rtr_ConfGen::error(string msg)
{
  std::cerr << "PLANNER ERROR: " << msg << "\n";
  exit(-1);
}

void Rtr_ConfGen::error(string msg, 
			OL_Context::Rule* rule)
{
  std::cerr << "PLANNER ERROR: " << msg 
	    << " for rule " << rule->ruleID << ". Planner exits.\n";
  exit(-1);
}

void Rtr_ConfGen::checkFunctor(Parse_Functor* functor, OL_Context::Rule* rule)
{
  if (functor->fn->loc == "") {
    error("\"" + functor->fn->name + "\" lacks a location specifier", rule);
  }
  if (functor->fn->name == "periodic") { 
    if (functor->args() < 3) {
      error("Make sure periodic predicate has three fields (NI,E,duration)", rule);
    }
    return; 
  }
  bool validLoc = false;
  for (int k = 0; k < functor->args(); k++) {
    if (functor->arg(k)->toString() == functor->fn->loc) {
      validLoc = true;
    }
  }
  if (validLoc == false) {
    error("Invalid location specifier in predicate " + functor->toString(), rule);
  }
}
