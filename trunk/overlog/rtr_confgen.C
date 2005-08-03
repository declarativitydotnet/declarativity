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

Rtr_ConfGen::Rtr_ConfGen(OL_Context* ctxt, 
			 Router::ConfigurationRef conf, 
			 bool dups, 
			 bool debug, 
			 bool cc,
			 str filename) :_conf(conf)
{
  _ctxt = ctxt;
  _dups = dups;
  _debug = debug;
  _cc = cc;
  str outputFile(filename << ".out");
  _output = fopen(outputFile.cstr(), "w");
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
void Rtr_ConfGen::configureRouter(ref< Udp > udp, str nodeID)
{

  if (!_cc) {
    _ccTx = NULL;
    _ccRx = NULL;
  } else {
    _ccTx 
      = _conf->addElement(New refcounted< CCTx >("Transmit CC" << nodeID, 1, 2048, 0, 1, 1, 2));
    _ccRx 
      = _conf->addElement(New refcounted< CCRx >("CC Receive" << nodeID, 2048, 1, 2));
  }


  // iterate through all the rules and process them
  for (unsigned int k = 0; k < _ctxt->getRules()->size(); k++) {
    _currentRule = _ctxt->getRules()->at(k);    
    processRule(_currentRule, nodeID);
  }

  ElementSpecRef receiveMux = genSendElements(udp, nodeID); 
  _currentElementChain.clear();
  genReceiveElements(udp, nodeID, receiveMux);
}

void Rtr_ConfGen::clear()
{
  _udpReceivers.clear();
  _udpPushSenders.clear();
}


void Rtr_ConfGen::processRule(OL_Context::Rule *r, 
			      str nodeID)
{
  debugRule(r, str(strbuf() << "Process rule " << r->toString() << "\n"));  
  std::vector<JoinKey> joinKeys;
  FieldNamesTracker curNamesTracker;
  ptr<Aggwrap> agg_el = NULL;

  _pendingReceiverSpec = NULL;

  // AGGREGATES
  int aggField = r->head->aggregate();
  if (aggField >= 0) {
    if (hasEventTerm(r)) {
      // there is an aggregate and involves an event, we need an agg wrap      
      Parse_Agg* aggExpr = dynamic_cast<Parse_Agg*>(r->head->arg(aggField));
      debugRule(r, str(strbuf() << "Agg wrap " << aggExpr->aggName() 
		       << " " << aggField << "\n"));
      agg_el = New refcounted<Aggwrap>(aggExpr->aggName(), aggField + 1);
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
      debugRule(r, str(strbuf() << "Periodic join\n"));
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
    std::cout << "Pending register receiver " 
	      << _pendingRegisterReceiver << "\n";
    genProjectHeadElements(r, nodeID, &curNamesTracker);
  }
    
  if (r->deleteFlag == true) {
    debugRule(r, str(strbuf() << "Delete " 
		     << r->head->fn->name << " for rule \n"));
    TableRef tableToDelete = getTableByName(nodeID, r->head->fn->name);
    OL_Context::TableInfo* ti 
      = _ctxt->getTableInfos()->find(r->head->fn->name)->second;
    
    genPrintElement(strbuf("PrintBeforeDelete:") << r->ruleID << nodeID);
    genPrintWatchElement(strbuf("PrintWatchDelete:") << r->ruleID << nodeID);
    
    ElementSpecRef pullPush = 
      _conf->addElement(New refcounted<TimedPullPush>("DeletePullPush", 0));
    hookUp(pullPush, 0);
    
    ElementSpecRef deleteElement =
      _conf->addElement(New refcounted< Delete >(strbuf("Delete:") 
						 << r->ruleID << nodeID,
						 tableToDelete, 
						 ti->primaryKeys.at(0), 
						 ti->primaryKeys.at(0)));    
    hookUp(deleteElement, 0);
    
    if (_isPeriodic == false && _pendingReceiverSpec) {
      registerReceiver(_pendingReceiverTable, _pendingReceiverSpec);
    }
    return; // discard. deleted tuples not sent anywhere
  } else {    
    if (agg_el) { 
      ElementSpecRef aggWrapSlot 
	= _conf->addElement(New refcounted<Slot>("aggWrapSlot"));      
      ElementSpecRef agg_spec = _conf->addElement(agg_el);
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
    str headTableName = r->head->fn->name;
    registerReceiverTable(r, headTableName);
  }

  if (_isPeriodic == false && _pendingReceiverSpec) {
    registerReceiver(_pendingReceiverTable, _pendingReceiverSpec);
  }

  // anything at this point needs to be hookup with senders
  _udpPushSenders.push_back(_currentElementChain.back()); 
  _udpPushSendersPos.push_back(_currentPositionIndex); 
}



//////////////////// Transport layer //////////////////////////////
void 
Rtr_ConfGen::genReceiveElements(ref< Udp> udp, 
				str nodeID, ElementSpecRef wrapAroundDemux)
{

  // network in
  ElementSpecRef udpReceive = _conf->addElement(udp->get_rx());  
  ElementSpecRef unmarshalS =
    _conf->addElement(New refcounted< 
		      UnmarshalField >(strbuf("ReceiveUnmarshal:") 
				       << nodeID, 1));
  ElementSpecRef unBoxS =
    _conf->addElement(New refcounted< 
		      UnboxField >(strbuf("ReceiveUnBox:") << nodeID, 1));

  hookUp(udpReceive, 0, unmarshalS, 0);
  hookUp(unmarshalS, 0, unBoxS, 0);

  ElementSpecPtr wrapAroundMux = NULL;
  if (_cc) {
    wrapAroundMux = _conf->addElement(New refcounted< Mux >(strbuf("wrapAroundSendMux:") 
							    << nodeID, 3));

    ref< vec< ValueRef > > demuxKeysCC = New refcounted< vec< ValueRef > >;
    demuxKeysCC->push_back(New refcounted< Val_Str > ("ack"));
    demuxKeysCC->push_back(New refcounted< Val_Str > ("ccdata"));

    ElementSpecRef demuxRxCC 
      = _conf->addElement(New refcounted< Demux >("receiveDemuxCC", demuxKeysCC));

    genPrintElement(strbuf("PrintBeforeReceiveDemuxCC:") << nodeID);
    hookUp(demuxRxCC, 0);
    hookUp(demuxRxCC, 0, _ccTx, 1);  // send acknowledgements to cc transmit
    hookUp(demuxRxCC, 2, wrapAroundMux, 2); // regular non-CC data

    // handle CC data. <ccdata, seq, src, <t>>
    ElementSpecRef unpackCC =  
      _conf->addElement(New refcounted< 
			UnboxField >(strbuf("ReceiveUnBoxCC:") << nodeID, 3));
    hookUp(demuxRxCC, 1, _ccRx, 0);  // regular CC data    
    hookUp(unpackCC, 0);
    genPrintElement(strbuf("PrintReceiveUnpackCC:") << nodeID);
    hookUp(wrapAroundMux, 0); // connect data to wraparound mux   

  } else {
    wrapAroundMux = _conf->addElement(New refcounted< Mux >(strbuf("wrapAroundSendMux:") 
							    << nodeID, 2));
    hookUp(unBoxS, 0, wrapAroundMux, 0);
  }

  // demuxer
  ref< vec< ValueRef > > demuxKeys = New refcounted< vec< ValueRef > >;
  ReceiverInfoMap::iterator _iterator;
  for (_iterator = _udpReceivers.begin(); 
       _iterator != _udpReceivers.end(); 
       _iterator++) {
    str nextTableName = _iterator->second._name;
    demuxKeys->push_back(New refcounted< Val_Str >(nextTableName));
  }

  ElementSpecRef demuxS 
    = _conf->addElement(New refcounted< Demux >("receiveDemux", demuxKeys));
 
  genPrintElement(strbuf("PrintReceivedBeforeDemux:") << nodeID);
  genDupElimElement(strbuf("ReceiveDupElimBeforeDemux:") << nodeID); 
  genPrintWatchElement(strbuf("PrintWatchReceiveBeforeDemux:") << nodeID);

  ElementSpecRef bufferQueue = 
    _conf->addElement(New refcounted< Queue >(strbuf("ReceiveQueue:") << 
					      nodeID, 1000));
  ElementSpecRef pullPush = 
    _conf->addElement(New refcounted<
		      TimedPullPush>("ReceiveQueuePullPush", 0));

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
    str tableName = ri._name;

    std::cout << "Generate Receive: Demuxing " << tableName << " for " 
	      << numElementsToReceive << " elements\n";

    // DupElim -> DemuxS -> Insert -> Duplicator -> Fork
    ElementSpecRef bufferQueue = 
      _conf->addElement(New refcounted< Queue >(strbuf("DemuxQueue:") << 
						nodeID << ":" << tableName, 
						1000));
    ElementSpecRef pullPush = 
      _conf->addElement(New refcounted<
			TimedPullPush>("DemuxQueuePullPush" << nodeID 
				       << ":" << tableName, 0));
    
    hookUp(demuxS, counter++, bufferQueue, 0);
    hookUp(bufferQueue, 0, pullPush, 0);
    
    // duplicator
    ElementSpecRef duplicator = 
      _conf->addElement(New refcounted< 
			DuplicateConservative 
			>(strbuf("DuplicateConservative:") 
			  << tableName << ":" << nodeID, 
			  numElementsToReceive));    
    // materialize table only if it is declared and has lifetime>0
    OL_Context::TableInfoMap::iterator _iterator 
      = _ctxt->getTableInfos()->find(tableName);
    if (_iterator != _ctxt->getTableInfos()->end() 
	&& _iterator->second->timeout != 0) {
      ElementSpecRef insertS 
	= _conf->addElement(New refcounted< 
			    Insert >(strbuf("Insert:") << 
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
	  _conf->addElement(New refcounted< 
			    PrintTime >(strbuf("PrintAfterDuplicator:"
					       << tableName << ":" 
					       << nodeID)));
	hookUp(duplicator, k, printDuplicator, 0);
	hookUp(printDuplicator, 0, nextElementSpec, 0);
	continue;
      }
      hookUp(duplicator, k, nextElementSpec, 0);
    }
  }

  // connect the acknowledgement port to ccTx
  ElementSpecRef sinkS 
    = _conf->addElement(New refcounted< Discard >("discard"));
  hookUp(demuxS, _udpReceivers.size(), sinkS, 0); 
  

  _currentElementChain.push_back(wrapAroundDemux);
  genPrintElement(strbuf("PrintWrapAround:") << nodeID);
  genPrintWatchElement(strbuf("PrintWrapAround:") << nodeID);
  
  // connect the orignal wrap around
  hookUp(wrapAroundMux, 1);
  
}


void 
Rtr_ConfGen::registerUDPPushSenders(ElementSpecRef elementSpecRef)
{
  _udpPushSenders.push_back(elementSpecRef);
  _udpPushSendersPos.push_back(1);
}


ElementSpecRef 
Rtr_ConfGen::genSendElements(ref< Udp> udp, str nodeID)
{
  ElementSpecRef udpSend = _conf->addElement(udp->get_tx());  

  // prepare to send. Assume all tuples send by first tuple
  ElementSpecRef roundRobin =
    _conf->addElement(New refcounted< RoundRobin >("roundRobinSender:" 
						   << nodeID, 
						   _udpPushSenders.size())); 

  ElementSpecRef pullPush =
      _conf->addElement(New refcounted< 
			TimedPullPush >(strbuf("SendPullPush:") 
					<< nodeID, 0));
  hookUp(roundRobin, 0, pullPush, 0);

  // check here for the wrap around
  ref< vec< ValueRef > > wrapAroundDemuxKeys = 
    New refcounted< vec< ValueRef > >;  
  wrapAroundDemuxKeys->push_back(New refcounted< Val_Str >(str(strbuf() 
							       << nodeID)));
  ElementSpecRef wrapAroundDemux 
    = _conf->addElement(New refcounted< Demux >("wrapAroundSendDemux", 
						wrapAroundDemuxKeys, 1));  

  hookUp(wrapAroundDemux, 0); // connect to the wrap around
  ElementSpecRef sendQueue = 
    _conf->addElement(New refcounted< Queue >(strbuf("SendQueue:") << nodeID, 
					      1000));
  hookUp(wrapAroundDemux, 1, sendQueue, 0); 

  // connect to send queue
  genPrintElement(strbuf("PrintRemoteSend:") << nodeID);
  genPrintWatchElement(strbuf("PrintWatchRemoteSend:") << nodeID);

  ///////// Network Out ///////////////
  if (_cc) {
    ElementSpecRef srcAddress  = 
      _conf->addElement(New refcounted< PelTransform >(strbuf("AddSrcAddressCC:" << nodeID), 
							      "\"" << nodeID 
							      << "\" pop swallow pop"));
    ElementSpecRef seq 
      = _conf->addElement(New refcounted< Sequence >("SequenceCC" << nodeID));
    hookUp(srcAddress, 0);
    hookUp(seq, 0);

    // <data, seq, src, <t>>
    ElementSpecRef tagData  = 
      _conf->addElement(New refcounted< PelTransform >(strbuf("TagData:" << nodeID), 
						       "\"ccdata\" pop $0 pop $1 pop $2 pop"));
    hookUp(tagData, 0);

    genPrintElement(strbuf("PrintRemoteSendCCOne:") << nodeID);

    ElementSpecRef pullPushCC =
      _conf->addElement(New refcounted< 
			TimedPullPush >(strbuf("SendPullPushCC:") 
					<< nodeID, 0));

    hookUp(pullPushCC, 0);
    hookUp(_ccTx, 0); // <seq, addr, <t>>

    // <dst, <seq, addr, <t>>
    ElementSpecRef encapSendCC =
      _conf->addElement(New refcounted< PelTransform >(strbuf("encapSendCC:") 
						       << nodeID, 
						       "$3 1 field pop swallow pop")); 
    hookUp(_ccTx, 0, encapSendCC, 0);

    genPrintElement(strbuf("PrintRemoteSendCCTwo:") << nodeID);
    
    _roundRobinCC =
       _conf->addElement(New refcounted< RoundRobin >("roundRobinSenderCC:" 
						      << nodeID, 2)); 
     hookUp(_roundRobinCC, 0);

    // acknowledgements. <dst, <ack, seq, windowsize>>
    ElementSpecRef ackPelTransform
      = _conf->addElement(New refcounted< PelTransform >("ackPelTransformCC" << nodeID,
							"$0 pop \"ack\" ->t $1 append $2 append pop"));
    
    hookUp(_ccRx, 1, ackPelTransform, 0);
    genPrintElement(strbuf("PrintSendAck:") << nodeID);

    hookUp(_currentElementChain.back(), 0, _roundRobinCC, 1);

     // Now marshall the payload (second field)
     // <dst, marshalled>
     ElementSpecRef marshalSendCC = 
       _conf->addElement(New refcounted< MarshalField >("marshalCC:" << 
							nodeID, 1));  
     genPrintElement(strbuf("PrintRemoteSendCCMarshal:") << nodeID);
     hookUp(marshalSendCC, 0); 

  } else {

    ElementSpecRef encapSend =
      _conf->addElement(New refcounted< PelTransform >(strbuf("encapSend:") 
						       << ":" << nodeID, 
						       //strbuf("$1")// << _udpPushSendersPos.at(k)
						       "$1 pop swallow pop")); 
    hookUp(encapSend, 0);

    // Now marshall the payload (second field)
    ElementSpecRef marshalSend = 
      _conf->addElement(New refcounted< MarshalField >("marshal:" << 
						       nodeID, 1));  
    hookUp(marshalSend, 0);  
  }
   
  ElementSpecRef routeSend =
    _conf->addElement(New refcounted< StrToSockaddr >("router:" << nodeID, 0));

  hookUp(routeSend, 0);
  hookUp(udpSend, 0);

  // form the push senders
  for (unsigned int k = 0; k < _udpPushSenders.size(); k++) {
    ElementSpecRef nextElementSpec = _udpPushSenders.at(k);

    //std::cout << "Encap send " << _udpPushSendersPos.at(k) << "\n";

    /*ElementSpecRef encapSend =
      _conf->addElement(New refcounted< PelTransform >(strbuf("encapSend:") 
						       << ":" << nodeID, 
						       //strbuf("$1")// << _udpPushSendersPos.at(k)
						       "$1 pop swallow pop")); */
    
    // for now, assume addr field is the put here
    hookUp(nextElementSpec, 0, roundRobin, k);

    //hookUp(nextElementSpec, 0, encapSend, 0);
    //hookUp(encapSend, 0, roundRobin, k);
  }

  return wrapAroundDemux;
}


// for a particular table name that we are receiving, 
// register an elementSpec that needs that data
void 
Rtr_ConfGen::registerReceiver(str tableName, 
			      ElementSpecRef elementSpecRef)
{
  // add to the right receiver
  ReceiverInfoMap::iterator _iterator = _udpReceivers.find(tableName);
  if (_iterator != _udpReceivers.end()) {
    _iterator->second.addReceiver(elementSpecRef);
  }
}



// regiser a new receiver for a particular table name
// use to later hook up the demuxer
void Rtr_ConfGen::registerReceiverTable(OL_Context::Rule* rule, 
					str tableName)
{  
  ReceiverInfoMap::iterator _iterator = _udpReceivers.find(tableName);
  if (_iterator == _udpReceivers.end()) {
    // not there, we register
    _udpReceivers.insert(std::make_pair(tableName, 
					ReceiverInfo(tableName, 
						     rule->head->args())));
  }  
  debugRule(rule, str(strbuf() << "Register table " << tableName << "\n"));
}
					     



//////////////////////////////////////////////////////////////////
///////////////// Relational Operators -> P2 Elements
//////////////////////////////////////////////////////////////////

str Rtr_ConfGen::pelMath(FieldNamesTracker* names, Parse_Math *expr) {
  Parse_Var*  var;
  Parse_Val*  val;
  Parse_Math* math;
  Parse_Function* fn  = NULL;
  strbuf      pel;  


  if (expr->id && expr->oper == Parse_Math::MINUS) {
    Parse_Expr *tmp = expr->lhs;
    expr->lhs = expr->rhs;
    expr->rhs = tmp;
  }

  if ((var = dynamic_cast<Parse_Var*>(expr->lhs)) != NULL) {
    int pos = names->fieldPosition(var->toString());
    pel << "$" << (pos+1) << " ";
  }
  else if ((val = dynamic_cast<Parse_Val*>(expr->lhs)) != NULL) {
    pel << val->toString() << " "; 

    if (val->id()) pel << "->u32 ->id "; 
    else if (expr->id) pel << "->u32 ->id "; 
  }
  else if ((math = dynamic_cast<Parse_Math*>(expr->lhs)) != NULL) {
    pel << pelMath(names, math); 
  }
  else if ((fn = dynamic_cast<Parse_Function*>(expr->lhs)) != NULL) {
    pel << pelFunction(names, fn); 
  }
  else {
    // TODO: throw/signal some kind of error
    return "MATH ERROR";
  }

  if ((var = dynamic_cast<Parse_Var*>(expr->rhs)) != NULL) {
    int pos = names->fieldPosition(var->toString());
    pel << "$" << (pos+1) << " ";
  }
  else if ((val = dynamic_cast<Parse_Val*>(expr->rhs)) != NULL) {    
    pel << val->toString() << " "; 

    if (val->id()) pel << "->u32 ->id "; 
    else if (expr->id) pel << "->u32 ->id "; 
  }
  else if ((math = dynamic_cast<Parse_Math*>(expr->rhs)) != NULL) {
    pel << pelMath(names, math); 
  }
  else {
    // TODO: throw/signal some kind of error
    return "MATH ERROR";
  }

  switch (expr->oper) {
    case Parse_Math::LSHIFT:  pel << (expr->id ? "<<id "      : "<< "); break;
    case Parse_Math::RSHIFT:  pel << ">> "; break;
    case Parse_Math::PLUS:    pel << "+ "; break;
    case Parse_Math::MINUS:   pel << "- "; break;
    case Parse_Math::TIMES:   pel << "* "; break;
    case Parse_Math::DIVIDE:  pel << "/ "; break;
    case Parse_Math::MODULUS: pel << "\% "; break;
    default: return "ERROR";
  }

  return pel;
}

str Rtr_ConfGen::pelRange(FieldNamesTracker* names, Parse_Bool *expr) {
  Parse_Var*   var       = NULL;
  Parse_Val*   val       = NULL;
  Parse_Math*  math      = NULL;
  Parse_Var*   range_var = dynamic_cast<Parse_Var*>(expr->lhs);
  Parse_Range* range     = dynamic_cast<Parse_Range*>(expr->rhs);
  strbuf       pel;
  int          pos;

  if (!range || !range_var) return "ERROR";

  pos = names->fieldPosition(range_var->toString());
  if (pos < 0) return "ERROR";
  pel << "$" << (pos + 1) << " ";

  if ((var = dynamic_cast<Parse_Var*>(range->lhs)) != NULL) {
    pos = names->fieldPosition(var->toString());
    if (pos < 0) return "ERROR";
    pel << "$" << (pos + 1) << " ";
  }
  else if ((val = dynamic_cast<Parse_Val*>(range->lhs)) != NULL) {
    pel << val->toString() << " ";
  }
  else if ((math = dynamic_cast<Parse_Math*>(range->lhs)) != NULL) {
   pel << pelMath(names, math);
  }
  else return "ERROR";

  if ((var = dynamic_cast<Parse_Var*>(range->rhs)) != NULL) {
    pos = names->fieldPosition(var->toString());
    if (pos < 0) return "ERROR";
    pel << "$" << (pos + 1) << " ";
  }
  else if ((val = dynamic_cast<Parse_Val*>(range->rhs)) != NULL) {
    pel << val->toString() << " ";
  }
  else if ((math = dynamic_cast<Parse_Math*>(range->rhs)) != NULL) {
   pel << pelMath(names, math);
  }
  else return "ERROR";

  switch (range->type) {
    case Parse_Range::RANGEOO: pel << "() "; break;
    case Parse_Range::RANGEOC: pel << "(] "; break;
    case Parse_Range::RANGECO: pel << "[) "; break;
    case Parse_Range::RANGECC: pel << "[] "; break;
    }

  return pel;
}

str Rtr_ConfGen::pelFunction(FieldNamesTracker* names, Parse_Function *expr) {
  strbuf pel;

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
  else return "ERROR: unknown function name.";

  return pel;
}

str Rtr_ConfGen::pelBool(FieldNamesTracker* names, Parse_Bool *expr) {
  Parse_Var*      var = NULL;
  Parse_Val*      val = NULL;
  Parse_Function* fn  = NULL;
  Parse_Math*     m   = NULL;
  Parse_Bool*     b   = NULL;
  strbuf          pel;  

  if (expr->oper == Parse_Bool::RANGE) return pelRange(names, expr);

  bool strCompare = false;
  if ((var = dynamic_cast<Parse_Var*>(expr->lhs)) != NULL) {
    int pos = names->fieldPosition(var->toString());
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
    pel << pelBool(names, b); 
  }
  else if ((m = dynamic_cast<Parse_Math*>(expr->lhs)) != NULL) {
    pel << pelMath(names, m); 
  }
  else if ((fn = dynamic_cast<Parse_Function*>(expr->lhs)) != NULL) {
      pel << pelFunction(names, fn); 
  }
  else {
    // TODO: throw/signal some kind of error
    return "UNKNOWN BOOL OPERAND ERROR";
  }

  if (expr->rhs != NULL) {
    if ((var = dynamic_cast<Parse_Var*>(expr->rhs)) != NULL) {
      int pos = names->fieldPosition(var->toString());
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
      pel << pelBool(names, b); 
    }
    else if ((m = dynamic_cast<Parse_Math*>(expr->rhs)) != NULL) {
      pel << pelMath(names, m); 
    }
    else if ((fn = dynamic_cast<Parse_Function*>(expr->rhs)) != NULL) {
      pel << pelFunction(names, fn); 
    }
    else {
      // TODO: throw/signal some kind of error
      return "UNKNOWN BOOL OPERAND ERROR";
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
    default: return "ERROR";
    }
  return pel;
}

void 
Rtr_ConfGen::pelSelect(OL_Context::Rule* rule, FieldNamesTracker* names, 
		       Parse_Select *expr,
		       str nodeID, int selectionID)
{
  strbuf sPel = pelBool(names, expr->select) << "not ifstop ";
  
  // put in the old fields (+1 since we have to include the table name)
  for (uint k = 0; k < names->fieldNames.size() + 1; k++) {
    sPel << "$" << k << " pop ";
  }

  debugRule(rule, str(strbuf() << "Generate selection functions for " << sPel 
                  << " " << names->toString() << "\n"));
 
  ElementSpecRef sPelTrans =
    _conf->addElement(New refcounted< PelTransform >(strbuf("Selection:") 
						     << selectionID << ":" 
						     << nodeID, sPel));

  if (_isPeriodic == false &&_pendingRegisterReceiver) {
    _pendingReceiverSpec = sPelTrans;
    _currentElementChain.push_back(sPelTrans); // first element in chain
    _pendingRegisterReceiver = false;
  } else {
    // connecting now
    hookUp(sPelTrans, 0);
  }

  genPrintElement(strbuf("PrintAfterSelection:") << selectionID << ":" 
		  << nodeID);
}

void 
Rtr_ConfGen::genAllSelectionAssignmentElements(OL_Context::Rule* curRule,
					       str nodeID,
					       FieldNamesTracker* 
					       curNamesTracker) 
{
  for (unsigned int j = 0; j < curRule->terms.size(); j++) {
    Parse_Select* parse_select 
      = dynamic_cast<Parse_Select *>(curRule->terms.at(j));
    if (parse_select != NULL) {
      debugRule(curRule, str(strbuf() << "Selection term " << 
			     parse_select->toString() << " " 
			     << curRule->ruleID << "\n"));
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
			    str nodeID, 
			    int assignID) 
{
  strbuf pel;
  strbuf pelAssign;
  Parse_Var      *a   = dynamic_cast<Parse_Var*>(expr->var);
  Parse_Var      *var = NULL;
  Parse_Val      *val = NULL;
  Parse_Bool     *b   = NULL;
  Parse_Math     *m   = NULL;
  Parse_Function *f   = NULL; 

  if (expr->assign == Parse_Expr::Now)
    pelAssign << "now "; 
  else if ((b = dynamic_cast<Parse_Bool*>(expr->assign)) != NULL)
    pelAssign << pelBool(names, b);
  else if ((m = dynamic_cast<Parse_Math*>(expr->assign)) != NULL)
    pelAssign << pelMath(names, m);
  else if ((f = dynamic_cast<Parse_Function*>(expr->assign)) != NULL)
    pelAssign << pelFunction(names, f);
  else if ((var=dynamic_cast<Parse_Var*>(expr->assign)) != NULL && 
           names->fieldPosition(var->toString()) >= 0)                              
    pelAssign << "$" << (names->fieldPosition(var->toString())+1) << " ";
  else if ((val=dynamic_cast<Parse_Val*>(expr->assign)) != NULL) {
    if (val->v->typeCode() == Value::STR) { 
      pelAssign << "\"" << val->toString() << "\" ";
    } else {
      pelAssign << val->toString() << " ";
    }
  } else {
    std::cerr << "Rtr_ConfGen ASSIGN ERROR!\n";
    assert(0);
  }
   
  int pos = names->fieldPosition(a->toString());
  for (int k = 0; k < int(names->fieldNames.size()+1); k++) {
    if (k == pos) pel << pelAssign << "pop ";
    else pel << "$" << k << " pop ";
  }
  if (pos < 0) { 
    pel << pelAssign << "pop ";
    names->fieldNames.push_back(a->toString()); // the variable name
  } 

  debugRule(rule, strbuf() << "Generate assignments for " 
	    << a->toString() << " " 
	    << rule->ruleID << " " << pel << " " 
	    << names->toString() << "\n");
  
  ElementSpecRef assignPelTrans =
    _conf->addElement(New refcounted< PelTransform >(strbuf("Assignment:") 
						     << rule->ruleID << ":" 
						     << assignID << ":" 
						     << nodeID, 
						     pel));

  if (_isPeriodic == false &&_pendingRegisterReceiver) {
    _pendingReceiverSpec = assignPelTrans;
    _currentElementChain.push_back(assignPelTrans); // first element in chain
    _pendingRegisterReceiver = false;
  } else {
    // connecting now
    hookUp(assignPelTrans, 0);
  }

  genPrintElement(strbuf("PrintAfterAssignment:") << rule->ruleID << ":" 
		  << assignID << ":" << nodeID);
}


void Rtr_ConfGen::genProjectHeadElements(OL_Context::Rule* curRule,
					 str nodeID,
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
      if (parse_var->toString() == pf->fn->loc) {
	locationIndex = k;
      }
      // care only about vars    
      pos = curNamesTracker->fieldPosition(parse_var->toString());    
    }
    if (k == pf->aggregate()) {
      // as input into aggwrap
      Parse_Agg* aggExpr = dynamic_cast<Parse_Agg*>(curRule->head->arg(k));
      if (aggExpr->aggName() != "count") {
	pos = curNamesTracker->fieldPosition(aggExpr->v->toString());
      }
    }
    if (pos == -1) { continue; }    
    indices.push_back(pos + 1);
  }
  if (locationIndex == -1) { locationIndex = 0; } // default

  strbuf pelTransformStrbuf("\"" << pf->fn->name << "\" pop");

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

  str pelTransformStr(pelTransformStrbuf);
  debugRule(curRule, str(strbuf() << "Project head " 
			 << curNamesTracker->toString() 
			 << " " << pelTransformStr << " " << 
			 pf->fn->loc << " " << locationIndex 
			 << "\n"));
 
  _currentPositionIndex = locationIndex + 1;
  // project, and make sure first field after table name has the address 
  ElementSpecRef projectHeadPelTransform =
    _conf->addElement(New refcounted< PelTransform >(strbuf("ProjectHead:") 
						     << curRule->ruleID 
						     << ":" << nodeID,
						     pelTransformStr));
  if (_isPeriodic == false && _pendingRegisterReceiver) {
    _pendingReceiverSpec = projectHeadPelTransform;
    _currentElementChain.push_back(projectHeadPelTransform); 
    _pendingRegisterReceiver = false;
  } else {
    // connecting now
    hookUp(projectHeadPelTransform, 0);
  }
  
  genPrintElement(strbuf("PrintHead:") << curRule->ruleID << ":" << nodeID);  
}


void Rtr_ConfGen::genProbeElements(OL_Context::Rule* curRule, 
				   Parse_Functor* eventFunctor, 
				   Parse_Term* baseTableTerm, 
				   str nodeID, 	     
				   FieldNamesTracker* probeNames, 
				   FieldNamesTracker* baseProbeNames, 
				   int joinOrder,
				   cbv comp_cb)
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

  str baseTableName;
  if (pf != NULL) {
    baseTableName = pf->fn->name;
  }
  if (pr != NULL) {
    baseTableName = str("range" << curRule->ruleID);
  }
  

  if (leftJoinKeys.size() == 0 || rightJoinKeys.size() == 0) {
    std::cerr << "No matching join keys " << eventFunctor->fn->name << " " << 
      baseTableName << " " << curRule->ruleID << "\n";
  }

  // add one to offset for table name. Join the first matching key
  int leftJoinKey = leftJoinKeys.at(0) + 1;
  int rightJoinKey = rightJoinKeys.at(0) + 1;

  TableRef probeTable = getTableByName(nodeID, baseTableName);

  // should we use a uniqLookup or a multlookup? 
  // Check that the rightJoinKey is the primary key
  OL_Context::TableInfo* tableInfo 
    = _ctxt->getTableInfos()->find(baseTableName)->second;
  ElementSpecRef noNull 
    = _conf->addElement(New refcounted< NoNullField >(strbuf("NoNull:") 
						      << curRule->ruleID 
						      << ":" << joinOrder 
						      << ":" << nodeID, 1));

  ElementSpecRef last_el 
    = New refcounted<ElementSpec>(New refcounted<
				  Slot>("dummySlotProbeElements"));
 
  if (tableInfo->primaryKeys.size() == 1 && 
      tableInfo->primaryKeys.at(0) == rightJoinKey) {
    // use a unique index
    last_el =
      _conf->addElement(New refcounted< 
			UniqueLookup >(strbuf("UniqueLookup:") 
				       << curRule->ruleID 
				       << ":" << joinOrder 
				       << ":" << nodeID, 
				       probeTable,
				       leftJoinKey, 
				       rightJoinKey, 
				       comp_cb));
    debugRule(curRule, str(strbuf() << "Unique lookup " << " " 
			   << eventFunctor->fn->name << " " 
			   << baseTableName << " " 
			   << leftJoinKey << " " 
			   << rightJoinKey << "\n"));
    
  } else {
    last_el =
      _conf->addElement(New refcounted< MultLookup >(strbuf("MultLookup:") 
						     << curRule->ruleID 
						     << ":" << joinOrder 
						     << ":" << nodeID, 
						     probeTable,
						     leftJoinKey, 
						     rightJoinKey, comp_cb));
    
    addMultTableIndex(probeTable, rightJoinKey, nodeID);
    debugRule(curRule, str(strbuf() << "Mult lookup " << curRule->ruleID 
			   << " " << eventFunctor->fn->name << " " 
			   << baseTableName << " " 
			   << leftJoinKey << " " << rightJoinKey << "\n"));
  }
  
 
  int numFieldsProbe = probeNames->fieldNames.size();
  debugRule(curRule, str(strbuf() << "Probe before merge " 
			 << probeNames->toString() << "\n"));
  probeNames->mergeWith(baseProbeNames->fieldNames); 
  debugRule(curRule, str(strbuf() << "Probe after merge " 
			 << probeNames->toString() << "\n"));

  if (_isPeriodic == false && _pendingRegisterReceiver) {
    // connecting to udp receiver later
    _pendingReceiverSpec = last_el;
    _pendingRegisterReceiver = false;
  } else {
    // connecting now to prior element
    hookUp(last_el, 0);  
  }

  hookUp(last_el, 0, noNull, 0);

  debugRule(curRule, str(strbuf() << "Number of join selections " 
			 << leftJoinKeys.size()-1 << "\n"));
  for (uint k = 1; k < leftJoinKeys.size(); k++) {
    int leftField = leftJoinKeys.at(k);
    int rightField = rightJoinKeys.at(k);
    strbuf selectionPel;
    selectionPel << "$0 " << (leftField+1) << " field " << " $1 " 
		 << rightField+1 << " field ==s not ifstop $0 pop $1 pop";

    debugRule(curRule, str(strbuf() << "Join selections " 
			   << str(selectionPel) << "\n"));
    ElementSpecRef joinSelections =
      _conf->addElement(New refcounted< 
			PelTransform >(strbuf("JoinSelections:") 
				       << curRule->ruleID << ":" 
				       << joinOrder << ":" 
				       << k << ":" << nodeID, 
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
  pelProject << eventFunctor->fn->name << ":" << baseTableName << ":" 
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

  str pelProjectStr(pelProject);
  ElementSpecRef transS 
    = _conf->addElement(New refcounted< 
			PelTransform >(strbuf("JoinPelTransform:") 
				       << curRule->ruleID << ":" 
				       << joinOrder << ":" << nodeID, 
				       pelProjectStr));

  delete baseProbeNames;

  hookUp(transS, 0);

  genPrintElement(strbuf("PrintProbe2:") << 
		  curRule->ruleID << ":" << joinOrder << ":"
		  << nodeID);
}



void Rtr_ConfGen::genJoinElements(OL_Context::Rule* curRule, 
				  str nodeID, 
				  FieldNamesTracker* namesTracker,
				  ptr<Aggwrap> agg_el)
{
  // identify the events, use that to probe the other matching tables
  Parse_Functor* eventFunctor;
  std::vector<Parse_Term*> baseFunctors;
  bool eventFound = false;
  for (unsigned int k = 0; k < curRule->terms.size(); k++) {
    Parse_Functor* pf = dynamic_cast<Parse_Functor*>(curRule->terms.at(k));
    if (pf != NULL) {
      str functorName = pf->fn->name;    
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
      tableInfo->tableName = "range" << curRule->ruleID;
      tableInfo->timeout = -1; // never expire
      tableInfo->size = high - low + 1;
      tableInfo->primaryKeys.push_back(2);
      _ctxt->getTableInfos()->insert(std::make_pair(tableInfo->tableName, 
						    tableInfo));
      
      str newRangeTableName = nodeID << ":" << tableInfo->tableName;
      TableRef rangeTable = New refcounted< Table> (tableInfo->tableName, 
						    (high - low + 1));
      rangeTable->add_unique_index(2);
      addMultTableIndex(rangeTable, 1, nodeID);
      _tables.insert(std::make_pair(newRangeTableName, rangeTable));         
      debugRule(curRule, "Create Range Tables: Add unique index " 
		<< newRangeTableName << " " << low << " " << high << "\n");
 
      for (int x = low; x <= high; x++) {
	TupleRef tuple = Tuple::mk();
	tuple->append(Val_Str::mk(tableInfo->tableName));
	tuple->append(Val_Str::mk(nodeID));
	tuple->append(Val_Int32::mk(x));
	tuple->freeze();
	rangeTable->insert(tuple);
      }
    }
  }
  if (_isPeriodic == false) {
    std:: cout << "Event term " << eventFunctor->fn->name << "\n";
    // for all the base tuples, use the join to probe. 
    // keep track also the cur ordering of variables
    namesTracker->initialize(eventFunctor);
  } else {
    debugRule(curRule, str(strbuf() << 
			   "Periodic joins " << namesTracker->toString() << "\n"));
  }
  for (uint k = 0; k < baseFunctors.size(); k++) {    
    Parse_Functor* pf = dynamic_cast<Parse_Functor*>(baseFunctors.at(k));
    
    if (pf != NULL && pf->fn->name == eventFunctor->fn->name) { continue; } 
    debugRule(curRule, str(strbuf() << "Probing " << eventFunctor->fn->name 
			   << " " << baseFunctors.at(k)->toString() << "\n"));
    cbv comp_cb = cbv_null;
    if (agg_el) {
      comp_cb = agg_el->get_comp_cb();
    }
    
    FieldNamesTracker* baseProbeNames 
      = New FieldNamesTracker(baseFunctors.at(k));
    genProbeElements(curRule, eventFunctor, baseFunctors.at(k), 
		     nodeID, namesTracker, baseProbeNames, k, comp_cb);

    if (agg_el || baseFunctors.size() - 1 != k) {
      // Change from pull to push
      ElementSpecRef pullPush =
	_conf->addElement(New refcounted< 
			  TimedPullPush >(strbuf("JoinPullPush:") 
					  << curRule->ruleID << ":" 
					  << nodeID << ":" << k,
					  0));
      hookUp(pullPush, 0);
    }
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
    std::cout << "AddMultTableIndex: Mult index added " << str(uniqStr) 
	      << "\n";
  } else {
    std::cout << "AddMultTableIndex: Mult index already exists " 
	      << str(uniqStr) << "\n";
  }
}



void 
Rtr_ConfGen::genSingleAggregateElements(OL_Context::Rule* currentRule, 
					str nodeID, 
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
  }

  baseNamesTracker->initialize(baseFunctor);

  std::vector<unsigned int> groupByFields;      
  
  FieldNamesTracker* aggregateNamesTracker = new FieldNamesTracker();
  Parse_Functor* pf = currentRule->head;
  str headTableName = pf->fn->name;

  for (int k = 0; k < pf->args(); k++) {
    // go through the functor head, but skip the aggField itself    
    Parse_Var* pv = dynamic_cast<Parse_Var* > (pf->arg(k));
    if (pv == NULL) { continue; }
    int pos = baseNamesTracker->fieldPosition(pv->toString());
    if (k != -1 && k != pf->aggregate()) {
      debugRule(currentRule, 
		str(strbuf() << pos << " " << 
		    currentRule->head->aggregate() << " " << 
		    baseNamesTracker->fieldNames.at(pos) << "\n"));
      groupByFields.push_back((uint) pos + 1);
      aggregateNamesTracker->fieldNames.push_back(baseNamesTracker->fieldNames.at(pos));
    }
  }
  Parse_Agg* pa = dynamic_cast<Parse_Agg* > (pf->arg(pf->aggregate()));

  str aggVarname = pa->v->toString();

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

  debugRule(currentRule, str(strbuf() << aggregateNamesTracker->toString() 
			     << " " << baseFunctor->fn->name << 
			     " " << aggFieldBaseTable << " " 
			     << pf->aggregate() << " " << aggVarname << "\n"));
  
  // get the table, create the index
  TableRef aggTable = getTableByName(nodeID, baseFunctor->fn->name);  
  addMultTableIndex(aggTable, groupByFields.at(0), nodeID);  
  Table::MultAggregate tableAgg 
    = aggTable->add_mult_groupBy_agg(groupByFields.at(0), // groupby field
				     groupByFields,
				     aggFieldBaseTable, // the agg field
				     af);
  ElementSpecRef aggElement =
    _conf->addElement(New refcounted< Aggregate >(strbuf("Agg:") 
						  << currentRule->ruleID 
						  << ":" << nodeID,
						  tableAgg));
   
  strbuf pelTransformStr;
  pelTransformStr << "\"" << "aggResult:" << currentRule->ruleID << "\" pop";
  for (uint k = 0; k < aggregateNamesTracker->fieldNames.size(); k++) {
    pelTransformStr << " $" << k << " pop";
  }
  debugRule(currentRule, str(strbuf() << "Agg Pel Expr " 
			     << pelTransformStr <<"\n"));
  // apply PEL to add a table name
  ElementSpecRef addTableName =
    _conf->addElement(New refcounted< PelTransform >(strbuf("Aggregation:") 
						     << currentRule->ruleID 
						     << ":" << nodeID, 
						     pelTransformStr));

  hookUp(aggElement, 0, addTableName, 0);

  genPrintElement("PrintAgg:" << currentRule->ruleID << ":" << nodeID);
  genPrintWatchElement("PrintWatchAgg:" << currentRule->ruleID << ":" 
		       << nodeID);

  genProjectHeadElements(currentRule, nodeID, aggregateNamesTracker);
  _udpPushSenders.push_back(_currentElementChain.back());
  _udpPushSendersPos.push_back(1);

  registerReceiverTable(currentRule, headTableName);
}



void Rtr_ConfGen::genSingleTermElement(OL_Context::Rule* curRule, 
				       str nodeID, 
				       FieldNamesTracker* curNamesTracker)
{  
  ElementSpecRef slotElement 
    = _conf->addElement(New refcounted<Slot>("singleTermSlot:" 
					     << curRule->ruleID << ":" 
					     << nodeID));

  std::cout << "Number of terms " << curRule->terms.size() << "\n";
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
				   str nodeID, 
				   FieldNamesTracker* namesTracker)
{
  TupleRef functorTuple = Tuple::mk();
  functorTuple->append(Val_Str::mk(rule->head->fn->name));
  functorTuple->append(Val_Str::mk(nodeID)); 
  functorTuple->freeze();

  ElementSpecRef source =
    _conf->addElement(New refcounted< TupleSource >(str("FunctorSource:") 
						    << rule->ruleID << nodeID,
                                                   functorTuple));
  _currentElementChain.push_back(source);

  genPrintElement(strbuf("PrintFunctorSource:") << rule->ruleID << ":" 
		  << nodeID);
  Parse_Functor* pf = dynamic_cast<Parse_Functor* > (rule->terms.at(0));
  if (pf == NULL) { return; }
  
  str period = pf->arg(2)->toString();

  namesTracker->fieldNames.push_back(pf->arg(0)->toString());
  namesTracker->fieldNames.push_back("E");
  
  debugRule(rule, str(strbuf() << "Functor source " 
		      << pf->toString()) << " " << period << "\n");

  // a pel transform that puts in the periodic stuff
  ElementSpecRef pelRand = 
    _conf->addElement(New refcounted< PelTransform >(strbuf("FunctorSourcePel:" << 
							    rule->ruleID << ":" << nodeID), 
						     "$0 pop $1 pop rand pop"));
 
  hookUp(pelRand, 0);

  // The timed pusher
  ElementSpecRef pushFunctor =
    _conf->addElement(New refcounted< TimedPullPush >(strbuf("FunctorPush:") 
						      << rule->ruleID << 
						      ":" << nodeID,
						      atof(period.cstr())));

  hookUp(pushFunctor, 0);

  //  if (rule->terms.size() <= 1) {
  if (numFunctors(rule) <= 1) {
    ElementSpecRef functorSlot 
      = _conf->addElement(New refcounted<Slot>("functorSlot"));      
    hookUp(functorSlot, 0);
  }
}


void Rtr_ConfGen::genDupElimElement(str header)
{
   if (_dups) {
     ElementSpecRef dupElim 
       = _conf->addElement(New refcounted< DupElim >(header));
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
      _conf->addElement(New refcounted< PrintWatch >(header, 
						     _ctxt->getWatchTables()));
  hookUp(printWatchElement, 0);
}






///////////////////////////////////////////////////////////////////


////////////////////////// Table Creation ///////////////////////////

// Get a handle to the table. Typically used by the driver program to 
// preload some data.
TableRef Rtr_ConfGen::getTableByName(str nodeID, str tableName)
{
  std::cout << "Get table " << nodeID << ":" << tableName << "\n";
  TableMap::iterator _iterator = _tables.find(nodeID << ":" << tableName);
  if (_iterator == _tables.end()) { 
    std::cerr << "Table " << nodeID << ":" << tableName << " not found\n";
  }
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
      TableRef newTable = New refcounted< Table> (tableInfo->tableName, 
						  tableInfo->size);
      if (tableInfo->timeout != -1) {
	timespec* expiration = new timespec();
	expiration->tv_sec = tableInfo->timeout;
	expiration->tv_nsec = 0;
	newTable = New refcounted< Table> (tableInfo->tableName, 
					   tableInfo->size, expiration);
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
    TupleRef tr = _ctxt->getFacts().at(k);
    ValueRef vr = (*tr)[0];
    std::cout << "Insert tuple " << tr->toString() << " into table " 
	      << vr->toString() << " " << tr->size() << "\n";
    TableRef tableToInsert = getTableByName(nodeID, vr->toString());     
    tableToInsert->insert(tr);
    std::cout << "Tuple inserted: " << tr->toString() 
	      << " into table " << vr->toString() 
	      << " " << tr->size() << "\n";
  }
}

/////////////////////////////////////////////

///////////// Helper functions

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
    str termName = pf->fn->name;
    OL_Context::TableInfoMap::iterator _iterator 
      = _ctxt->getTableInfos()->find(termName);
    if (_iterator == _ctxt->getTableInfos()->end()) {     
      debugRule(curRule, str(strbuf() << "Found event term " << termName));
      // an event
      return true;
    }
  }
  return false;
}


bool Rtr_ConfGen::hasPeriodicTerm(OL_Context::Rule* curRule)
{
  for (uint k = 0; k < curRule->terms.size(); k++) {
    Parse_Functor* pf = dynamic_cast<Parse_Functor*>(curRule->terms.at(k));
    if (pf == NULL) { continue;}
    str termName = pf->fn->name;
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
    fieldNames.push_back(str("NI"));
    fieldNames.push_back(str(pr->var->toString()));
  }
}


std::vector<int> 
Rtr_ConfGen::FieldNamesTracker::matchingJoinKeys(std::vector<str> 
						 otherArgNames)
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

int 
Rtr_ConfGen::FieldNamesTracker::fieldPosition(str var)
{
  for (uint k = 0; k < fieldNames.size(); k++) {
    if (fieldNames.at(k) == var) {
      return k;
    }
  }
  return -1;
}

void 
Rtr_ConfGen::FieldNamesTracker::mergeWith(std::vector<str> names)
{
  for (uint k = 0; k < names.size(); k++) {
    str nextStr = names.at(k);
    if (fieldPosition(nextStr) == -1) {
      fieldNames.push_back(nextStr);
    }
  }
}

void 
Rtr_ConfGen::FieldNamesTracker::mergeWith(std::vector<str> names, 
					  int numJoinKeys)
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
