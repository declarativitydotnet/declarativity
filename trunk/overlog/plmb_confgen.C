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

#include "plmb_confgen.h"
#include "trace.h"
#include "cct.h"
#include "basicAck.h"
#include "aggFactory.h"
#include "tableTracer.h"

static std::map<void*, string> variables;

////////////////////////////////////////////////////////////
// Function to output into a dataflow graph specification
////////////////////////////////////////////////////////////

static string conf_comment(string comment)
{
  ostringstream oss;
  oss << "\t# " << comment << std::endl;
  return oss.str();
}

static string conf_valueVec(std::vector<ValuePtr>& values)
{
  ostringstream oss;
  oss << "{";
  for (std::vector<ValuePtr>::iterator iter = values.begin(); 
       iter != values.end(); iter++) {
    oss << (*iter)->toConfString();
    if (iter + 1 != values.end())
      oss << ", ";
  }
  if (values.size() == 0) oss << "value";
  oss << "}";
  return oss.str(); 
}

static string conf_StrVec(std::set<string> values)
{
  ostringstream oss;
  oss << "{";
  for (std::set<string>::iterator iter = values.begin(); 
       iter != values.end(); iter++) {
    oss << "\"" << *iter << "\"" ;
    if (++iter  != values.end())
      oss << ", ";
    --iter;
  }
  if (values.size() == 0) oss << "str";
  oss << "}";
  return oss.str(); 
}

static string conf_UIntVec(std::vector<unsigned>& values)
{
  ostringstream oss;
  oss << "{";
  for (std::vector<unsigned>::iterator iter = values.begin(); 
       iter != values.end(); iter++) {
    oss << *iter;
    if (iter + 1  != values.end())
      oss << ", ";
  }
  if (values.size() == 0) oss << "int";
  oss << "}";
  return oss.str(); 
}

template <typename T>
static string conf_var(T *key) {
  std::map<void*, string>::iterator iter = variables.find((void*)key);
  if (iter == variables.end())
    return "unknown";
  return iter->second;
}

template <typename T>
static string conf_assign(T *e, string obj)
{
  ostringstream oss;
  ostringstream var;
  string v;
  var << "variable_" << variables.size();

  // std::cout << "CONF ASSIGN: " << var.str() << " = " << obj << std::endl;
  std::map<void*, string>::iterator iter = variables.find((void*)e);
  if (iter == variables.end()) {
    variables.insert(std::make_pair((void*)e, var.str()));  
    v = var.str();
  } else v = iter->second;

  oss << "\tlet " << v << " = " << obj << ";" << std::endl;
  return oss.str();
}

template <typename T>
static string conf_call(T *obj, string fn, bool element=true) {
  ostringstream oss;
  string obj_var = conf_var((void*) obj);
  
  oss << "\tcall " << obj_var;
  if (element)
    oss << ".element()." << fn;
  else
    oss << "." << fn;
  return oss.str();
}

template <typename P1, typename P2>
static string conf_hookup(string toVar, P1 toPort, string fromVar, P2 fromPort)
{
  ostringstream oss;
  
  oss << "\t" << toVar << "[" << toPort << "] -> [" << fromPort << "]" << fromVar << ";" 
      << std::endl; 
  return oss.str();
}

static string conf_function(string fn)
{
  ostringstream oss;
  // std::cerr << "CONF FUNCTION CREATE: " << fn << std::endl;
  oss << fn << "()";
  return oss.str(); 
}

static bool checkString(string check) {
  if (check.compare(0, 9, "variable_") == 0) {
    return false;	// P2DL variable
  }
  else if (check.compare(0, 1, string("{")) == 0) {
    return false;	// Vector of values, int, or strings
  }
  else if (check.compare(0, 1, string("<")) == 0) {
    return false;	// A tuple of some sort.
 }
  return true;		// This is a string
}

static string switchQuotes(string s) {
  for (string::size_type loc = s.find("\"", 0);
       loc != string::npos; loc = s.find("\"", loc+2))
    s.replace(loc, 1, "\\\"");
  return s;
}

template <typename A>
static string conf_function(string fn, A arg0) 
{
  ostringstream oss;
  ostringstream arg_ss;
  arg_ss << arg0;

  string s = conf_function(fn);
  s.erase(s.end()-1);
  if ((typeid(arg0) == typeid(string) || 
       typeid(arg0) == typeid(const char*)) && 
      checkString(arg_ss.str()))
    oss << s << "\"" << switchQuotes(arg_ss.str()) << "\"" << ")";
  else
    oss << s << arg0 << ")";
  return oss.str();
}
template <typename A, typename B>
static string conf_function(string fn, A arg0, B arg1) 
{
  ostringstream oss;
  ostringstream arg_ss;
  arg_ss << arg1;

  string s = conf_function(fn, arg0);
  s.erase(s.end()-1);
  if ((typeid(arg1) == typeid(string) || 
       typeid(arg1) == typeid(const char*)) && 
      checkString(arg_ss.str()))
    oss << s << ", \"" << switchQuotes(arg_ss.str()) << "\"" << ")";
  else
    oss << s << ", " << arg1 << ")";
  return oss.str();
}
template <typename A, typename B, typename C>
static string conf_function(string fn, A arg0, B arg1, C arg2) 
{
  ostringstream oss;
  ostringstream arg_ss;
  arg_ss << arg2;

  std::cerr << "CONF FUNCTION ARG2: " << arg2 << std::endl;
  string s = conf_function(fn, arg0, arg1);
  s.erase(s.end()-1);
  if ((typeid(arg2) == typeid(string) || 
       typeid(arg2) == typeid(const char*)) && 
      checkString(arg_ss.str()))
    oss << s << ", \"" << switchQuotes(arg_ss.str()) << "\"" << ")";
  else
    oss << s << ", " << arg2 << ")";
  return oss.str();
}
template <typename A, typename B, typename C, typename D>
static string conf_function(string fn, A arg0, B arg1, C arg2, D arg3) 
{
  ostringstream oss;
  ostringstream arg_ss;
  arg_ss << arg3;

  string s = conf_function(fn, arg0, arg1, arg2);
  s.erase(s.end()-1);
  if ((typeid(arg3) == typeid(string) || 
       typeid(arg3) == typeid(const char*)) && 
      checkString(arg_ss.str()))
    oss << s << ", \"" << switchQuotes(arg_ss.str()) << "\"" << ")";
  else
    oss << s << ", " << arg3 << ")";
  return oss.str();
}

template <typename A, typename B, typename C, typename D, typename E>
static string conf_function(string fn, A arg0, B arg1, C arg2, D arg3, E arg4) 
{
  ostringstream oss;
  ostringstream arg_ss;
  arg_ss << arg4;

  string s = conf_function(fn, arg0, arg1, arg2, arg3);
  s.erase(s.end()-1);
  if ((typeid(arg4) == typeid(string) || 
       typeid(arg4) == typeid(const char*)) && 
      checkString(arg_ss.str()))
    oss << s << ", \"" << switchQuotes(arg_ss.str()) << "\"" << ")";
  else
    oss << s << ", " << arg4 << ")";
  return oss.str();
}





Plmb_ConfGen::Plmb_ConfGen(OL_Context* ctxt, 
                           Plumber::DataflowPtr conf, 
                           bool dups, 
                           bool debug, 
                           bool cc,
                           string filename,
                           bool rTracing,
                           std::ostream &s,
                           bool e) 
  :_conf(conf), _p2dl(s), _edit(e)
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

  initTracingState(rTracing);

  if (_edit) {
    _p2dl << "edit " << conf->name() << " {\n";
  }
  else {
    _p2dl << "dataflow " << conf->name() << " {\n";
  }
}


Plmb_ConfGen::~Plmb_ConfGen()
{
  fclose(_output);
}


void
Plmb_ConfGen::initTracingState(bool rTracing)
{
  _ruleTracing = rTracing;
  _needTracingPortAtRR = false;
  _taps_end = 0;
  _taps_beg = 0;
}


/** call this for each udp element that we wish to hook up the same
    dataflow if running only one, nodeID is the local host name */
void
Plmb_ConfGen::configurePlumber(boost::shared_ptr< Udp > udp, string nodeID)
{
  if (!_edit) {
    if (!_cc) {
      _ccTx.reset();
      _ccRx.reset();
    } else {
      _ccTx 
        = _conf->addElement(ElementPtr(new CCT("Transmit CC" + nodeID, 1, 2048)));
      _p2dl << conf_assign(_ccTx.get(), 
                           conf_function("CCT", "transmitCC", 1, 2048));
      _ccRx 
        = _conf->addElement(ElementPtr(new BasicAck("CC Receive" + nodeID)));
      _p2dl << conf_assign(_ccRx.get(), 
                           conf_function("BasicAck", "receiveCC"));
    }
  }

  // iterate through all the rules and process them
  // check to see if there are any errors in parsing
  if (_ctxt->errors.size() > 0) {
    ostringstream oss;
    oss << "There are "
        << _ctxt->errors.size() 
	<< " error(s) accumulated in the parser.\n";
    for (unsigned int k = 0;
         k < _ctxt->errors.size();
         k++) {
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

  for (OL_Context::RuleList::iterator i = _ctxt->getRules()->begin();
       i != _ctxt->getRules()->end();
       i++) {
    _currentRule = (*i);
    processRule(_currentRule, nodeID);
  }

  if (!_edit) {
    _p2dl << conf_assign(udp.get(), udp->toConfString());

    ElementSpecPtr receiveMux = genSendElements(udp, nodeID); 
    _currentElementChain.clear();
    genReceiveElements(udp, nodeID, receiveMux);
  }
  else {
    genEditFinalize(nodeID); 
  }

  _p2dl << "\n} # End of dataflow " << _conf->name() << "\n.";

  if(_ruleTracing){
    genTappedDataFlow(nodeID);
  }
}


void
Plmb_ConfGen::genTappedDataFlow(string nodeID)
{
  // hookup taps at the beginning and end of the rule strand with the tracer
  // for a given rule here..
  int numRuleTracers = 0;
  
  // Assumption: the debugging overlog file provide two materialize
  // for ruleExecTable and tupleTable
  string table_rule = "ruleExecTable";
  string table_tuple = "tupleTable";

  Table2Ptr ruleExecTable = getTableByName(nodeID, table_rule);
  Table2Ptr tupleTable = getTableByName(nodeID, table_tuple);

  for (unsigned int k = 0; k < _ctxt->getRules()->size(); k++) {
    OL_Context::Rule *_cr = _ctxt->getRules()->at(k);
    std::cout << "Handling rule name " << _cr->ruleID << " ruleNum " << 
      _cr->ruleNum << "\n";
    
    ElementSpecPtr tap_beg = find_tap(_cr->ruleNum, 0);
    ElementSpecPtr tap_end = find_tap(_cr->ruleNum, 1);
    int numPrecond;
    PrecondInfoMap::iterator _iterator = _taps_for_precond.find(_cr->ruleNum);
    PreconditionInfo p = _iterator->second;
    if(_iterator == _taps_for_precond.end()){
      std::cout << "There does not exist any precondition for rule " << 
	_cr->ruleID << ", beg " << tap_beg << " end " << tap_end << "\n";
      numPrecond = 0;
    }
    else{
      p = _iterator->second;
      numPrecond = p._preconds.size();
    }
    
    
    /** create a ruleTracer element here for each rule **/
    int numPorts = numPrecond;
    bool tracerNeeded = true;
    
    int port_index = 0;
    
    ElementSpecPtr rt;
    if(tap_beg != NULL && tap_end != NULL)
      numPorts += 2;
    else if(tap_beg != NULL || tap_end != NULL)
      numPorts += 1;
    else
      tracerNeeded = false;
    
    if(tracerNeeded){
      
      if(tap_beg == NULL)
	rt = _conf->addElement(ElementPtr(
					  new RuleTracer("RuleTracer:"+
							 _cr->ruleID, 
							 _cr->ruleID, 
							 nodeID,
							 -1, 
							 numPorts-1, 
							 _cr->ruleNum,
							 ruleExecTable,
							 tupleTable)));
      else
	rt = _conf->addElement(ElementPtr(
					  new RuleTracer("RuleTracer:"+
							 _cr->ruleID, 
							 _cr->ruleID, 
							 nodeID, 
							 0, 
							 numPorts-1, 
							 _cr->ruleNum,
							 ruleExecTable,
							 tupleTable)));

      numRuleTracers ++;
      _ruleTracers.push_back(rt);
    }
    else
      continue;
    
    std::cout << "Rule Tracer " << rt->element()->name() 
	      << ", tap_beg " << tap_beg << ", tap_end " 
	      << tap_end << " preconditions " << numPrecond 
	      << " numPorts " << numPorts << "\n";
    
    for(int i = 0; i < numPrecond && numPrecond > 0; i++){
      ElementSpecPtr t1 = p._preconds.at(i);
      if(tap_beg != NULL)
	hookUp(t1, 1, rt, i+1);
      else
	hookUp(t1, 1, rt, i);
    }
    
    if(tap_beg != NULL){
      _conf->hookUp(tap_beg, 1, rt, 0);
      fprintf(_output, "Connect: \n");
      fprintf(_output, "  %s %s %d\n", tap_beg->toString().c_str(), 
	      tap_beg->element()->_name.c_str(), 1);
      fprintf(_output, "  %s %s %d\n", rt->toString().c_str(), 
	      rt->element()->_name.c_str(), 0);
      fflush(_output);
      port_index ++;
    }
    
    
    
    if(tap_end != NULL){
      _conf->hookUp(tap_end, 1, rt, rt->element()->ninputs() - 1);
      fprintf(_output, "Connect: \n");
      fprintf(_output, "  %s %s %d\n", tap_end->toString().c_str(), 
	      tap_end->element()->_name.c_str(), 1);
      fprintf(_output, "  %s %s %d\n", rt->toString().c_str(), 
		rt->element()->_name.c_str(), rt->element()->ninputs() - 1);
      fflush(_output);
    }
  }

  // now connect the port 1 of all the traceTuple and tableTracer
  // elements to a traceMux
  int inputPortSize = _traceTupleElements.size() +
    _tableTracers.size();
  if(inputPortSize > 0){
    ElementSpecPtr traceMux = 
      _conf->addElement(ElementPtr(new Mux("traceMux", 
					   inputPortSize)));
    uint32_t portCounter = 0;
    for(std::vector< ElementSpecPtr >::iterator i = _traceTupleElements.begin();
        i != _traceTupleElements.end();
        i++, portCounter++){
      ElementSpecPtr t = (*i);
      ostringstream oss;
      oss << "$1 pop swallow pop";
      ElementSpecPtr encapSend =
	_conf->addElement(ElementPtr(new PelTransform("encapSend:" + nodeID, oss.str())));
      _conf->hookUp(t, 1, encapSend, 0);
      _conf->hookUp(encapSend, 0, traceMux, portCounter);
    }

    for(std::vector< ElementSpecPtr >::iterator i = _tableTracers.begin();
        i != _tableTracers.end();
        i++, portCounter++){
      ElementSpecPtr t = (*i);
      ostringstream oss;
      oss << "$1 pop swallow pop";
      ElementSpecPtr encapSend =
	_conf->addElement(ElementPtr(new PelTransform("encapSend:" + nodeID, oss.str())));
      _conf->hookUp(t, 0, encapSend, 0);
      _conf->hookUp(encapSend, 0, traceMux, portCounter);
    }
    
    // now hookup the output of the mux to a queue
    ElementSpecPtr traceQueue = 
      _conf->addElement(ElementPtr(new Queue("traceQueue", 1000)));
    _conf->hookUp(traceMux, 0, traceQueue, 0);
    
    // obtain a pointer to the roundrobin
    ElementSpecPtr rr = _conf->find("roundRobinSender:" + nodeID);
    if(rr == NULL){
      std::cout << "There does not exist a roundRobin, not possible. Exiting..\n";
      std::exit(-1);
    }
    // hookup output of the queue to the roundrobin's last input port
    
    _conf->hookUp(traceQueue, 0, rr, rr->element()->ninputs() - 1);
  }
}


ElementSpecPtr
Plmb_ConfGen::find_tap(int ruleNum, int beg_or_end)
{
  std::map<int, ElementSpecPtr>::iterator result;
  ElementSpecPtr xx;
  if(_ruleTracing){
    if(beg_or_end == 0){
      result = _taps_beg->find(ruleNum);
      if(result == _taps_beg->end()){
	return xx;
      }
      else{
	return result->second;
      }
    }
    else{
      std::cout << "RuleNum " << ruleNum << " taps " 
		<< _taps_end << " beg " << beg_or_end <<"\n";
      result = _taps_end->find(ruleNum);
      if(result == _taps_end->end()){
	return xx;
      }
      else{
	return result->second;
      }
    }
  }
  return xx;
}


void
Plmb_ConfGen::clear()
{
  _udpReceivers.clear();
  _udpSenders.clear();
}


Parse_Functor*
Plmb_ConfGen::eventTerm(OL_Context::Rule* curRule)
{
  for (std::list< Parse_Term* >::iterator i =
         curRule->terms.begin();
       i != curRule->terms.end();
       i++) {
    Parse_Functor* pf =
      dynamic_cast<Parse_Functor*>(*i);
    if (pf == NULL) {
      continue;
    }

    string termName = pf->fn->name;
    OL_Context::TableInfoMap::iterator _iterator =
      _ctxt->getTableInfos()->find(termName);
    if (_iterator == _ctxt->getTableInfos()->end()) {     
      debugRule(curRule, "Found event term " + termName);
      // an event
      return pf;
    }
  }
  return NULL;
}


void
Plmb_ConfGen::processRule(OL_Context::Rule *r, 
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
    // This contains an aggregate

    // get the event term if it exists (i.e., a predicate that is not
    // materialized).
    Parse_Functor* theEventTerm = eventTerm(r);
    if (theEventTerm != NULL) {
      // We got it
      ostringstream oss;
      
      checkFunctor(theEventTerm, r);

      if (numFunctors(r) <= 1) {
	ostringstream oss;
	oss << "Check that " 
	    << theEventTerm->fn->name << " is materialized";
	error(oss.str(), r);
      }

      // there is an aggregate and involves an event, we need an agg wrap      
      Parse_Agg* aggExpr = dynamic_cast< Parse_Agg* >
        (r->head->arg(aggField));
      if (aggExpr == NULL) {
	ostringstream oss;
	oss << "Invalid aggregate field " << aggField
            << " for rule " << r->ruleID; 
	error(oss.str());
      }
      
      oss << "Aggwrap:" << r->ruleID << ":" << nodeID;
      agg_el.reset(new Aggwrap(oss.str(), aggExpr->aggName(), 
			       aggField + 1, r->head->fn->name));
      agg_spec = _conf->addElement(agg_el);
      _p2dl << conf_assign(agg_spec.get(), 
                           conf_function("Aggwrap",
                                         "aggwrap_" + r->ruleID,
                                         aggExpr->aggName(), 
                                         aggField + 1, r->head->fn->name));
      for (int k = 0;
           k < r->head->args();
           k++) {
	if (k != aggField) {
	  // for each groupby value, figure out its location in the
	  // initial event tuple, if not present, throw an error
	  for (int j = 0;
               j < theEventTerm->args();
               j++) {
	    if (fieldNameEq(r->head->arg(k)->toString(),
                            theEventTerm->arg(j)->toString())) {
	      agg_el->registerGroupbyField(j);
              _p2dl << conf_call(agg_spec.get(),
                                 conf_function("registerGroupbyField", j))
                    << ";" << std::endl;
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
    Table2Ptr tableToDelete = getTableByName(nodeID, r->head->fn->name);
    
    genPrintElement("PrintBeforeDelete:" + r->ruleID + ":" +nodeID);
    genPrintWatchElement("PrintWatchDelete:" + r->ruleID + ":" +nodeID);
        
    ElementSpecPtr pullPush = 
      _conf->addElement(ElementPtr(new TimedPullPush("DeletePullPush", 0)));
    _p2dl << conf_assign(pullPush.get(), 
                         conf_function("TimedPullPush", "deletePullPush", 0));
    hookUp(pullPush, 0);
    
    ElementSpecPtr deleteElement =
      _conf->addElement(ElementPtr(new Delete("Delete:" + r->ruleID
                                              + ":" + nodeID,
                                              tableToDelete)));
    _p2dl << conf_assign(deleteElement.get(), 
                         conf_function("Delete", "delete_"+r->ruleID,
                                       conf_var(tableToDelete.get())));
    hookUp(deleteElement, 0);
    
    if (_isPeriodic == false && _pendingReceiverSpec) {
      registerReceiver(_pendingReceiverTable, _pendingReceiverSpec, r);
    }
    return; // discard. deleted tuples not sent anywhere
  } else {    
    if (agg_el) { 
      ElementSpecPtr aggWrapSlot 
	= _conf->addElement(ElementPtr(new Slot("aggWrapSlot:" + r->ruleID 
						 + ":" + nodeID)));
      _p2dl << conf_assign(aggWrapSlot.get(), 
                           conf_function("Slot", "aggWrapSlot_"+r->ruleID));

      // BOON CHECK: MOVED THIS UP TO BE COMPATIBLE WITH THE DATAFLOW LANGUAGE.
      // ElementSpecPtr agg_spec = _conf->addElement(agg_el);

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
    registerReceiver(_pendingReceiverTable, _pendingReceiverSpec, r);
  }

  // anything at this point needs to be hookup with senders
  registerUDPPushSenders(_currentElementChain.back(), r, nodeID);
  //_udpSenders.push_back(_currentElementChain.back()); 
  //_udpSendersPos.push_back(_currentPositionIndex); 
}


void
Plmb_ConfGen::checkFunctor(Parse_Functor* functor,
                           OL_Context::Rule* rule)
{
  if (functor->fn->name == "periodic") { 
    if (functor->args() < 3) {
      error("Make sure periodic predicate has at least "
            "three fields (NI,E,duration)", rule);
    }
    return; 
  }
  else {
    functor->getlocspec();
  }
}


void 
Plmb_ConfGen::genSingleAggregateElements(OL_Context::Rule* currentRule, 
                                         string nodeID, 
                                         FieldNamesTracker* baseNamesTracker)
{
  Parse_Functor* baseFunctor = NULL;
  // figure first, which term is the base term. 
  // Assume there is only one for now. Support more in future.
  for (std::list< Parse_Term* >::iterator j = currentRule->terms.begin();
       j != currentRule->terms.end();
       j++) {    
    Parse_Functor* currentFunctor =
      dynamic_cast<Parse_Functor* > (*j);
    if (currentFunctor == NULL) {
      continue;
    }
    baseFunctor = currentFunctor;
    checkFunctor(baseFunctor, currentRule);
  }

  baseNamesTracker->initialize(baseFunctor);

  Table2::Key groupByFields;      
  
  FieldNamesTracker* aggregateNamesTracker = new FieldNamesTracker();
  Parse_Functor* pf = currentRule->head;
  string headTableName = pf->fn->name;

  for (int k = 0;
       k < pf->args();
       k++) {
    // go through the functor head, but skip the aggField itself    
    Parse_Var* pv = dynamic_cast< Parse_Var* > (pf->arg(k));

    if (pv == NULL) {
      continue;
    }
    
    int pos = baseNamesTracker->fieldPosition(pv->toString());

    if (k != -1 && k != pf->aggregate()) {
      groupByFields.push_back((uint) pos + 1);
      aggregateNamesTracker->fieldNames.
        push_back(baseNamesTracker->fieldNames.at(pos));
    }
  }

  Parse_Agg* pa = dynamic_cast<Parse_Agg* > (pf->arg(pf->aggregate()));

  string aggVarname = pa->v->toString();

  aggregateNamesTracker->fieldNames.push_back(aggVarname);

  int aggFieldBaseTable =
    baseNamesTracker->fieldPosition(aggVarname) + 1;
  // What does this mean for COUNT?
  // aggFieldBaseTable = groupByFields.at(0);

  // get the table, create the index
  Table2Ptr aggTable = getTableByName(nodeID, baseFunctor->fn->name);  
  secondaryIndex(aggTable, groupByFields, nodeID);  

  Table2::Aggregate tableAgg =
    aggTable->aggregate(groupByFields,
                        aggFieldBaseTable, // the agg field
                        pa->oper);
  if (tableAgg == NULL) {
    // Ooops, I couldn't create an aggregate. Complain.
    error("Could not create aggregate \"" + pa->oper
          + "\". I only know aggregates " +
          AggFactory::aggList());
    return;
  }

  _p2dl << conf_assign(tableAgg, 
                       conf_call(aggTable.get(), 
                                 conf_function("aggregate",
                                               conf_UIntVec(groupByFields), 
                                               aggFieldBaseTable,
                                               pa->oper), false));

  ElementSpecPtr aggElement =
    _conf->addElement(ElementPtr(new Aggregate("Agg:"+currentRule->ruleID +
					       ":" + nodeID, tableAgg)));
  _p2dl << conf_assign(aggElement.get(), 
                       conf_function("Aggregate", 
                                     "agg_rule_" +
                                     currentRule->ruleID, 
                                     conf_var(tableAgg)));
   
  ostringstream pelTransformStr;
  pelTransformStr << "\"" << "aggResult:" << currentRule->ruleID << "\" pop";
  for (uint k = 0;
       k < aggregateNamesTracker->fieldNames.size();
       k++) {
    pelTransformStr << " $" << k << " pop";
  }
  debugRule(currentRule, "Agg Pel Expr " +
            pelTransformStr.str()+ "\n");

  // apply PEL to add a table name
  ElementSpecPtr addTableName =
    _conf->addElement(ElementPtr(new PelTransform("Aggregation:"
                                                  + currentRule->ruleID
                                                  + ":" + nodeID,
                                                  pelTransformStr.str())));
  _p2dl << conf_assign(addTableName.get(), 
                       conf_function("PelTransform", 
                                     "agg_rule_" + currentRule->ruleID, 
                                     pelTransformStr.str()));

  hookUp(aggElement, 0, addTableName, 0);

  genPrintElement("PrintAgg:" + currentRule->ruleID + ":" + nodeID);
  genPrintWatchElement("PrintWatchAgg:" + currentRule->ruleID + ":" + nodeID);


  genProjectHeadElements(currentRule, nodeID, aggregateNamesTracker);
  registerUDPPushSenders(_currentElementChain.back(), currentRule, nodeID);
  //_udpSenders.push_back(_currentElementChain.back());
  //_udpSendersPos.push_back(1);

  registerReceiverTable(currentRule, headTableName);
}


void
Plmb_ConfGen::FieldNamesTracker::
joinKeys(Plmb_ConfGen::FieldNamesTracker* probeNames,
         Table2::Key& lookupKey,
         Table2::Key& indexKey,
         Table2::Key& remainingBaseKey)
{
  unsigned myFieldNo = 1;       // start at one to skip the table name
  for (std::vector< string >::iterator i = fieldNames.begin();
       i != fieldNames.end();
       i++, myFieldNo++) {
    string myNextArgument = *i;
    int probePosition = 
      probeNames->fieldPosition(myNextArgument);

    // Does my argument match any probe arguments?
    if (probePosition == -1) {
      // My argument doesn't match. It's a "remaining" base key
      remainingBaseKey.push_back(myFieldNo);
    } else {
      // My argument myNextArgument at field number myFieldNo matches
      // the probe's argument at field number probePosition. The lookup
      // key will project probePosition on the probe tuple onto
      // myFieldNo.
      lookupKey.push_back(probePosition + 1); // add 1 for the table name
      indexKey.push_back(myFieldNo);
    }
  }  
}


void
Plmb_ConfGen::genProbeElements(OL_Context::Rule* curRule, 
                               Parse_Functor* eventFunctor, 
                               Parse_Term* baseTableTerm, 
                               string nodeID, 	     
                               FieldNamesTracker* probeNames, 
                               FieldNamesTracker* baseNames, 
                               int joinOrder,
                               b_cbv *comp_cb)
{
  // We will be joining the probe (an event or intermediate result
  // tuple) with the base table.
  Table2::Key indexKey;         // base matching fields
  Table2::Key lookupKey;        // and the probe fields they match
  Table2::Key remainingBaseKey; // Non-index keys in the base table
  baseNames->joinKeys(probeNames, lookupKey, indexKey, remainingBaseKey);

  // Now we must create a lookup into the base table using these lookup
  // and index keys (as per Table2::lookup()).

  Parse_Functor* pf =
    dynamic_cast<Parse_Functor*>(baseTableTerm);

  string baseTableName;
  if (pf != NULL) {
    baseTableName = pf->fn->name;
    checkFunctor(pf, curRule);
  }
  
  if (lookupKey.size() == 0 || indexKey.size() == 0) {
    error("No join keys " + eventFunctor->fn->name + " " + 
	  baseTableName + " ", curRule);
  }
  
  // Fetch the base table
  Table2Ptr baseTable = getTableByName(nodeID, baseTableName);

  // The NoNull filter for the join sequence
  OL_Context::TableInfo* tableInfo =
    _ctxt->getTableInfos()->find(baseTableName)->second;
  ostringstream nonulloss;
  nonulloss << "NoNull:" << curRule->ruleID << ":" << joinOrder << ":" << nodeID;
  ElementSpecPtr noNull =
    _conf->addElement(ElementPtr(new NoNullField(nonulloss.str(), 1)));
  _p2dl << conf_assign(noNull.get(), 
                       conf_function("NoNullField", "n_noNull", 1));
  
  // The connector slot for the output of my join
  ElementSpecPtr last_el(new ElementSpec
                         (ElementPtr(new Slot("dummySlotProbeElements"))));

  // The lookup
  ostringstream lookuposs;
  lookuposs << "Lookup2:" << curRule->ruleID
            << ":" << joinOrder << ":" << nodeID; 
  last_el =
    _conf->addElement(ElementPtr(new Lookup2(lookuposs.str(),
                                             baseTable,
                                             lookupKey,
                                             indexKey, 
                                             *comp_cb)));
  if (*comp_cb == 0 || conf_var(comp_cb) == "unknown") {
    _p2dl << conf_assign(last_el.get(), 
                         conf_function("Lookup2", lookuposs.str(),
                                       conf_var(baseTable.get()),
                                       conf_UIntVec(lookupKey),
                                       conf_UIntVec(indexKey)));
  }
  else {
    _p2dl << conf_assign(last_el.get(), 
                         conf_function("Lookup2", lookuposs.str(),
                                       conf_var(baseTable.get()),
                                       conf_UIntVec(lookupKey),
                                       conf_UIntVec(indexKey),
                                       conf_var(comp_cb)));
  }
  std::cout << "CALLBACK VARIABLE LOOKUP: " << *comp_cb << std::endl;
  
  
  if (tableInfo->primaryKeys == indexKey) {
    // This is a primary key join so we don't need a secondary index
  } else {
    // Ensure there's a secondary index on the indexKey
    secondaryIndex(baseTable, indexKey, nodeID);
  }
  
 



  int numFieldsProbe = probeNames->fieldNames.size();
  debugRule(curRule, "Probe before merge " + probeNames->toString() + "\n");
  probeNames->mergeWith(baseNames->fieldNames); 
  debugRule(curRule, "Probe after merge " + probeNames->toString() + "\n");

  if (_isPeriodic == false && _pendingRegisterReceiver) {
    // connecting to udp receiver later
    _pendingReceiverSpec = last_el;
    _pendingRegisterReceiver = false;
  } else {
    // connecting now to prior element
    hookUp(last_el, 0);  
  }

  // ruleTracing: add a tap element for each such join element
  // to find out the precondition
  if(_ruleTracing){
    ElementSpecPtr tap = createTapElement(curRule);
    hookUp(last_el, 0, tap, 0);
    hookUp(tap, 0, noNull, 0);
  }
  else {
    hookUp(last_el, 0, noNull, 0);
  }



  // Deal with selections afterwards.  Not necessary since we do
  // multi-field joins!!!!

//   for (uint k = 1; k < lookupKey.size(); k++) {
//     int leftField = lookupKey.at(k);
//     int rightField = indexKey.at(k);
//     ostringstream selectionPel;
//     selectionPel << "$0 " << leftField << " field " << " $1 " 
// 		 << rightField << " field ==s not ifstop $0 pop $1 pop";

//     debugRule(curRule, "Join selections " + selectionPel.str() + "\n");

//     ostringstream oss;
//     oss << "joinSelections_" << curRule->ruleID << "_"
// 	<< joinOrder << "_" << k;
    
//     ElementSpecPtr joinSelections =
//       _conf->addElement(ElementPtr(new PelTransform(oss.str(), 
// 				                    selectionPel.str())));    
//     _p2dl << conf_assign(joinSelections.get(), 
//                 conf_function("PelTransform", "joinSelections", selectionPel.str()));
//     hookUp(joinSelections, 0);
//   }




  // Take the joined tuples and produce the resulting path form the pel
  // projection.  Keep all fields from the probe, and all fields from
  // the base table that were not join keys.
  ostringstream pelProject;
  pelProject << "\"join:"
             << eventFunctor->fn->name
             << ":" << baseTableName
             << ":" 
	     << curRule->ruleID
             << ":"
             << nodeID
             << "\" pop "; // table name
  for (int k = 0; k < numFieldsProbe; k++) {
    pelProject << "$0 " << k + 1 << " field pop ";
  }

  // And also pop all remaining base field numbers (those that did not
  // participate in the join.
  for (Table2::Key::iterator i = remainingBaseKey.begin();
       i != remainingBaseKey.end();
       i++) {
    pelProject << "$1 " << (*i) << " field pop ";
  }

  string pelProjectStr = pelProject.str();
  ostringstream oss1; 
  oss1 << "joinPel_" << curRule->ruleID << "_"
       << joinOrder << "_" << nodeID;

  ElementSpecPtr transS =
    _conf->addElement(ElementPtr(new PelTransform(oss1.str(),
                                                  pelProjectStr)));
  _p2dl << conf_assign(transS.get(), 
                       conf_function("PelTransform",
                                     "joinPel", pelProjectStr));
  delete baseNames;

  hookUp(transS, 0);
}









////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
// Clean up boundary
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////




//////////////////// Dataflow Edit Finalizer /////////////////////
void
Plmb_ConfGen::genEditFinalize(string nodeID) 
{
  ElementSpecPtr watchSpec = _conf->find("printWatch");
  if (watchSpec) {
    std::set<string> watches = _ctxt->getWatchTables();
    PrintWatch *pwatch= dynamic_cast<PrintWatch*>(watchSpec->element().get());
    for (std::set<string>::iterator iter = watches.begin(); 
         iter != watches.end(); iter++) {
      pwatch->watch(*iter);
    } 
  }
  _p2dl << conf_comment("");
  _p2dl << conf_comment("");
  _p2dl << conf_comment("CONNECT THE SEND SIDE TO THE DYNAMIC ROUND ROBIN");
  _p2dl << conf_comment("");
  _p2dl << conf_comment("");
  for (unsigned int k = 0; k < _udpSenders.size(); k++) {
    ElementSpecPtr nextElementSpec = _udpSenders.at(k);

    ostringstream oss;
    ostringstream oss_name;
    oss_name << "encapSend_" << k;
    oss << "$"<< _udpSendersPos.at(k) << " pop swallow pop";
    ElementSpecPtr encapSend =
      _conf->addElement(ElementPtr(new PelTransform(oss_name.str(), oss.str())));
    _p2dl << conf_assign(encapSend.get(), 
                         conf_function("PelTransform", oss_name.str(), oss.str()));
    
/*
    ElementSpecPtr print = 
      _conf->addElement(ElementPtr(new Print("print_confgen")));
    _p2dl << conf_assign(print.get(), conf_function("Print", "print_confgen"));
    hookUp(encapSend, 0, print, 0);
*/

    // Turn off the port checks for the port to the round robin
    // encapSend->output(0)->check(false);	

    // Hookup next element to encapsulator and the encapsulator to the dynamic round robin
    hookUp(nextElementSpec, 0, encapSend, 0);
    _p2dl << conf_hookup(conf_var(encapSend.get()), 0, string(".dRoundRobin"), string("+"));
    // _p2dl << conf_hookup(conf_var(print.get()), 0, string(".dRoundRobin"), string("+"));
  }
  _p2dl << conf_comment("=================================================");


  _p2dl << conf_comment("");
  _p2dl << conf_comment("");
  _p2dl << conf_comment("CONNECT THE RECEIVE SIDE TO THE DYNAMIC DEMUX");
  _p2dl << conf_comment("");
  _p2dl << conf_comment("");

  // connect to all the pending udp receivers. Assume all 
  // receivers are connected to elements with push input ports for now 
  for (ReceiverInfoMap::iterator _iterator = _udpReceivers.begin(); 
       _iterator != _udpReceivers.end(); 
       _iterator++) {
    ostringstream oss_comment;
    ReceiverInfo ri = _iterator->second;
    int numElementsToReceive = ri._receivers.size(); 
    string tableName = ri._name;

    ElementSpecPtr duplicator; 
    string dup_name = "dc_" + tableName;
    bool found_previous_duplicator = false;
    // CHECK FOR PREVIOUS DUPLICATOR
    if (dynamic_cast<Plumber::DataflowEdit*>(_conf.get()) != NULL &&
        (duplicator = dynamic_cast<Plumber::DataflowEdit*>(_conf.get())->find(dup_name)) != 0) {
      oss_comment << "Found Dynamic Duplicator for table " << tableName << " adding " 
                  << numElementsToReceive << " more elements";
      _p2dl << conf_comment("");
      _p2dl << conf_comment(oss_comment.str());
      _p2dl << conf_comment("");
      found_previous_duplicator = true;
    }
    else {
      oss_comment << "Add demux port for " << tableName << " for " 
                  << numElementsToReceive << " elements";
      _p2dl << conf_comment("");
      _p2dl << conf_comment(oss_comment.str());
      _p2dl << conf_comment("");

      ElementSpecPtr bufferQueue = 
        _conf->addElement(ElementPtr(new Queue("demuxQueue", 1000)));
      _p2dl << conf_assign(bufferQueue.get(), 
                conf_function("Queue", "demuxQueue_" + tableName, 1000));
      ElementSpecPtr pullPush = 
        _conf->addElement(ElementPtr(new TimedPullPush("DemuxQueuePullPush", 0)));
      _p2dl << conf_assign(pullPush.get(), 
                conf_function("TimedPullPush", 
                              "demuxQueuePullPush_" + tableName, 0));
    
      bufferQueue->input(0)->check(false);
      _p2dl << conf_hookup(string(".dDemux"), Val_Str(_iterator->second._name).toConfString(), 
                           conf_var(bufferQueue.get()), 0);
      hookUp(bufferQueue, 0, pullPush, 0);
    
      duplicator = _conf->addElement(ElementPtr(
         new DuplicateConservative(dup_name, numElementsToReceive)));    
      _p2dl << conf_assign(duplicator.get(), 
                           conf_function("DDuplicateConservative", 
                                         dup_name,
                                         numElementsToReceive));
      // materialize table only if it is declared
      OL_Context::TableInfoMap::iterator _iterator 
        = _ctxt->getTableInfos()->find(tableName);
      if (_iterator != _ctxt->getTableInfos()->end()) {
        ElementSpecPtr insertS = _conf->addElement(
          ElementPtr(new Insert("insert",  getTableByName(nodeID, tableName))));
        _p2dl << conf_assign(insertS.get(), 
                   conf_function("Insert", "insert", 
                     conf_var(getTableByName(nodeID, tableName).get())));
      
        hookUp(pullPush, 0, insertS, 0);
        genPrintWatchElement("PrintWatchInsert:"+nodeID);

        hookUp(duplicator, 0);
      } else {
        hookUp(pullPush, 0, duplicator, 0);
      }
    }

    // connect the duplicator to elements for this name
    for (uint k = 0; k < ri._receivers.size(); k++) {
      int ruleNum = ri._ruleNums.at(k);
      std::cout << "Rule num " << ruleNum << ", ruleTracing " << _ruleTracing << "\n";
      if(!_ruleTracing){
	ElementSpecPtr nextElementSpec = ri._receivers.at(k);
	
	if (_debug) {
	  ElementSpecPtr printDuplicator = 
	    _conf->addElement(ElementPtr(new PrintTime("PrintAfterDuplicator:"
						       + tableName + ":" + nodeID)));
	  _p2dl << conf_assign(printDuplicator.get(), 
			       conf_function("PrintTime", "printAfterDuplicator"));
	  if (found_previous_duplicator)
	    _p2dl << conf_hookup("." + dup_name, string("+"), 
				 conf_var(printDuplicator.get()), 0);
	  else hookUp(duplicator, k, printDuplicator, 0);
	  hookUp(printDuplicator, 0, nextElementSpec, 0);
	}
	else {
	  if (found_previous_duplicator)
	    _p2dl << conf_hookup("." + dup_name, string("+"), 
				 conf_var(nextElementSpec.get()), 0);
	  else hookUp(duplicator, k, nextElementSpec, 0);
	}
      }
      else {

	// Inserting taps at the beginning
	
	ElementSpecPtr nextElementSpec = ri._receivers.at(k);
	ElementSpecPtr tap_beg = find_tap(ruleNum, 0);

	std::cout << "Receivers " << nextElementSpec->element()->name() << ", taps is " << tap_beg->element()->name() << "\n";
	std::exit(-1);
	if (_debug) {
	  ElementSpecPtr printDuplicator = 
	    _conf->addElement(ElementPtr(new PrintTime("PrintAfterDuplicator:"
						       + tableName + ":" + nodeID)));
	  _p2dl << conf_assign(printDuplicator.get(), 
			       conf_function("PrintTime", "printAfterDuplicator"));
	  if (found_previous_duplicator)
	    _p2dl << conf_hookup("." + dup_name, string("+"), 
				 conf_var(printDuplicator.get()), 0);
	  else {
	    //hookUp(duplicator, k, printDuplicator, 0);
	    hookUp(duplicator, k, tap_beg, 0);
	    hookUp(tap_beg, 0, printDuplicator, 0);
	  }
	  hookUp(printDuplicator, 0, nextElementSpec, 0);
	}
	else {
	  if (found_previous_duplicator)
	    _p2dl << conf_hookup("." + dup_name, string("+"), 
				 conf_var(nextElementSpec.get()), 0);
	  else {
	    //hookUp(duplicator, k, nextElementSpec, 0);
	    hookUp(duplicator, k, tap_beg, 0);
	    hookUp(tap_beg, 0, nextElementSpec, 0);
	  }
	}
	
      }
    }
  }
  _p2dl << conf_comment("=================================================");
}

//////////////////// Transport layer //////////////////////////////
void 
Plmb_ConfGen::genReceiveElements(boost::shared_ptr< Udp> udp, 
				string nodeID, ElementSpecPtr wrapAroundDemux)
{

  // network in
  ElementSpecPtr udpReceive = _conf->addElement(udp->get_rx());  
  _p2dl << conf_assign(udpReceive.get(), conf_call(udp.get(), "get_rx()", false));
  ElementSpecPtr unmarshalS =
    _conf->addElement(ElementPtr(new UnmarshalField ("ReceiveUnmarshal:" + nodeID, 1)));
  _p2dl << conf_assign(unmarshalS.get(), 
                       conf_function("UnmarshalField", "receiveUnmarshal", 1));
  ElementSpecPtr unBoxS =
    _conf->addElement(ElementPtr(new  UnboxField ("ReceiveUnBox:" + nodeID, 1)));
  _p2dl << conf_assign(unBoxS.get(), 
                       conf_function("UnboxField", "receiveUnBox", 1));

  hookUp(udpReceive, 0, unmarshalS, 0);
  hookUp(unmarshalS, 0, unBoxS, 0);

  ElementSpecPtr wrapAroundMux;
  if (_cc) {
    wrapAroundMux = _conf->addElement(ElementPtr(new Mux ("wrapAroundSendMux:"+ nodeID, 3)));
    _p2dl << conf_assign(wrapAroundMux.get(), 
                         conf_function("Mux", "wrapAroundSendMux", 3));

    boost::shared_ptr< std::vector< ValuePtr > > demuxKeysCC(new std::vector< ValuePtr > );
    demuxKeysCC->push_back(ValuePtr(new Val_Str ("ack")));
    demuxKeysCC->push_back(ValuePtr(new Val_Str ("ccdata")));

    ElementSpecPtr demuxRxCC 
      = _conf->addElement(ElementPtr(new Demux ("receiveDemuxCC", demuxKeysCC)));
    _p2dl << conf_assign(demuxRxCC.get(), 
                         conf_function("Demux", "receiveDemuxCC", conf_valueVec(*demuxKeysCC)));

    genPrintElement("PrintBeforeReceiveDemuxCC:" + nodeID);
    hookUp(demuxRxCC, 0);
    hookUp(demuxRxCC, 0, _ccTx, 1);  // send acknowledgements to cc transmit
    hookUp(demuxRxCC, 2, wrapAroundMux, 2); // regular non-CC data

    // handle CC data. <ccdata, seq, src, <t>>
    ElementSpecPtr unpackCC =  
      _conf->addElement(ElementPtr(new UnboxField ("ReceiveUnBoxCC:" + nodeID, 3)));
    _p2dl << conf_assign(unpackCC.get(), 
                         conf_function("UnboxField", "receiveUnBoxCC", 3));
    hookUp(demuxRxCC, 1, _ccRx, 0);  // regular CC data    
    hookUp(unpackCC, 0);
    genPrintElement("PrintReceiveUnpackCC:"+ nodeID);
    hookUp(wrapAroundMux, 0); // connect data to wraparound mux   

  } else {
    wrapAroundMux = _conf->addElement(ElementPtr(new Mux("wrapAroundSendMux:"+ nodeID, 2)));
    _p2dl << conf_assign(wrapAroundMux.get(), 
                         conf_function("Mux", "wrapAroundSendMux", 2));
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
  _p2dl << conf_assign(demuxS.get(), 
                       conf_function("Demux", "receiveDemux", conf_valueVec(*demuxKeys)));
 
  genPrintElement("PrintReceivedBeforeDemux:"+nodeID);
  genDupElimElement("ReceiveDupElimBeforeDemux:"+ nodeID); 
  genPrintWatchElement("PrintWatchReceiveBeforeDemux:"+ nodeID);


  ElementSpecPtr bufferQueue = 
    _conf->addElement(ElementPtr(new Queue("ReceiveQueue:"+nodeID, 1000)));
  _p2dl << conf_assign(bufferQueue.get(), 
                       conf_function("Queue", "receiveQueue", 1000));
  ElementSpecPtr pullPush = 
    _conf->addElement(ElementPtr(new TimedPullPush("ReceiveQueuePullPush", 0)));
  _p2dl << conf_assign(pullPush.get(), 
                       conf_function("TimedPullPush", "receiveQueuePullPush", 0));

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
    _p2dl << conf_assign(bufferQueue.get(), 
              conf_function("Queue", "demuxQueue_" + tableName, 1000));
    ElementSpecPtr pullPush = 
      _conf->addElement(ElementPtr(new TimedPullPush("DemuxQueuePullPush" + nodeID 
				       + ":" + tableName, 0)));
    _p2dl << conf_assign(pullPush.get(), 
              conf_function("TimedPullPush", 
                            "demuxQueuePullPush_" + tableName, 0));
    
    hookUp(demuxS, counter++, bufferQueue, 0);
    hookUp(bufferQueue, 0, pullPush, 0);
    
    // insert a traceTuple element here if this tuple is being traced
    genTraceElement(tableName);
    
    // duplicator
    ElementSpecPtr duplicator = 
      _conf->addElement(ElementPtr(new DuplicateConservative("DuplicateConservative:"
			  + tableName + ":" + nodeID, numElementsToReceive)));    
    _p2dl << conf_assign(duplicator.get(), 
             conf_function("DuplicateConservative", "dupCons", 
                           numElementsToReceive));
    // materialize table only if it is declared
    OL_Context::TableInfoMap::iterator _iterator 
      = _ctxt->getTableInfos()->find(tableName);
    if (_iterator != _ctxt->getTableInfos()->end()) {
      ElementSpecPtr insertS 
	= _conf->addElement(ElementPtr(new Insert("Insert:"+ tableName + ":" + nodeID,  
				     getTableByName(nodeID, tableName))));
      _p2dl << conf_assign(insertS.get(), 
               conf_function("Insert", "insert", 
                             conf_var(getTableByName(nodeID, tableName).get())));
      
      //hookUp(pullPush, 0, insertS, 0);
      hookUp(insertS, 0);
      genPrintWatchElement("PrintWatchInsert:"+nodeID);


      hookUp(duplicator, 0);
    } else {
      //hookUp(pullPush, 0, duplicator, 0);
      hookUp(duplicator, 0);
    }
    std::cout << " Number of duplications needed " << ri._receivers.size() << " for table " << tableName << ", ruleTracing " << _ruleTracing << "\n";
    // connect the duplicator to elements for this name
    for (uint k = 0; k < ri._receivers.size(); k++) {
      int ruleNum = ri._ruleNums.at(k);

      if(!_ruleTracing){
	ElementSpecPtr nextElementSpec = ri._receivers.at(k);
	
	if (_debug) {
	  ElementSpecPtr printDuplicator = 
	    _conf->addElement(ElementPtr(new PrintTime("PrintAfterDuplicator:"
						       + tableName + ":" + nodeID)));
	  _p2dl << conf_assign(printDuplicator.get(), 
			       conf_function("PrintTime", "printAfterDuplicator"));
	  hookUp(duplicator, k, printDuplicator, 0);
	  hookUp(printDuplicator, 0, nextElementSpec, 0);
	  continue;
	}
	hookUp(duplicator, k, nextElementSpec, 0);
      }
      else {
	ElementSpecPtr nextElementSpec = ri._receivers.at(k);
	ElementSpecPtr tap_beg = find_tap(ruleNum, 0);
	std::cout << "RuleNum " << ruleNum << " tap is " << tap_beg->element()->name() << "\n";
	if (_debug) {
	  ElementSpecPtr printDuplicator = 
	    _conf->addElement(ElementPtr(new PrintTime("PrintAfterDuplicator:"
						       + tableName + ":" + nodeID)));
	  _p2dl << conf_assign(printDuplicator.get(), 
			       conf_function("PrintTime", "printAfterDuplicator"));
	  //hookUp(duplicator, k, printDuplicator, 0);
	  //hookUp(printDuplicator, 0, nextElementSpec, 0);
	  hookUp(duplicator, k, tap_beg, 0);
	  hookUp(tap_beg, 0, printDuplicator, 0);
	  hookUp(printDuplicator, 0, nextElementSpec, 0);
	  continue;
	}
	//hookUp(duplicator, k, nextElementSpec, 0);
	hookUp(duplicator, k, tap_beg, 0);
	hookUp(tap_beg, 0, nextElementSpec, 0);
      }
    }
  }

  // connect the acknowledgement port to ccTx
  ElementSpecPtr sinkS 
    = _conf->addElement(ElementPtr(new Discard("discard")));
  _p2dl << conf_assign(sinkS.get(), 
                       conf_function("Discard", "discard"));
  hookUp(demuxS, _udpReceivers.size(), sinkS, 0); 
  

  _currentElementChain.push_back(wrapAroundDemux);
  genPrintElement("PrintWrapAround:"+nodeID);
  genPrintWatchElement("PrintWrapAround:"+nodeID);
  
  // connect the orignal wrap around
  hookUp(wrapAroundMux, 1);
  
}


void 
Plmb_ConfGen::registerUDPPushSenders(ElementSpecPtr elementSpecPtr,
				     OL_Context::Rule * curRule,
				     string nodeid)
{
  _udpSenders.push_back(elementSpecPtr);
  _udpSendersPos.push_back(1);
  if(curRule->ruleID != "$")
    std::cout << "Registering sender for " << curRule->head->fn->name << "\n";

  // if ruleTracing, create a tap element here for a given rule
  if(_ruleTracing){
    std::cout << "Creating tap_end for rule " << curRule->ruleID << "\n";
    if(curRule->ruleID == "$")
      return;
    ElementSpecPtr tap_end = _conf->addElement(ElementPtr(new Tap("Tap_End:"+curRule->ruleID, curRule->ruleNum)));
    
    if(_taps_end == 0)
      _taps_end = new std::map<int, ElementSpecPtr>();
    
    std::cout << "Rule Number " << curRule->ruleNum << ", Rule Name " << curRule->ruleID << "_taps_end " << _taps_end << ", tap " << tap_end << "\n";
    _taps_end->insert(std::make_pair(curRule->ruleNum, tap_end));
    
    _taps_end_vector.push_back(tap_end);
  }
}


ElementSpecPtr 
Plmb_ConfGen::genSendElements(boost::shared_ptr< Udp> udp, string nodeID)
{
  ElementSpecPtr udpSend = _conf->addElement(udp->get_tx());  
  _p2dl << conf_assign(udpSend.get(), conf_call(udp.get(), "get_tx()", false));

  // prepare to send. Assume all tuples send by first tuple
  
  assert(_udpSenders.size() > 0);
  int sizeInputPort = _udpSenders.size();
  
  // reserve a port for traced tuples
  if(_needTracingPortAtRR)
    sizeInputPort ++;

  ElementSpecPtr roundRobin =
    _conf->addElement(ElementPtr(new RoundRobin("roundRobinSender:" + nodeID, 
						   sizeInputPort))); 
  _p2dl << conf_assign(roundRobin.get(), 
                       conf_function("RoundRobin", "roundRobin", 
                                     _udpSenders.size()));
  ElementSpecPtr pullPush =
      _conf->addElement(ElementPtr(new TimedPullPush("SendPullPush:"+nodeID, 0)));
  _p2dl << conf_assign(pullPush.get(), 
                       conf_function("TimedPullPush", "sendPulllPush", 0));

  hookUp(roundRobin, 0, pullPush, 0);

  // check here for the wrap around
  boost::shared_ptr< std::vector< ValuePtr > > wrapAroundDemuxKeys(new std::vector< ValuePtr >);  
  wrapAroundDemuxKeys->push_back(ValuePtr(new Val_Str(nodeID)));
  ElementSpecPtr wrapAroundDemux 
    = _conf->addElement(ElementPtr(new Demux("wrapAroundSendDemux", wrapAroundDemuxKeys, 0)));  
  _p2dl << conf_assign(wrapAroundDemux.get(), 
                       conf_function("Demux", "wrapAroundSendDemux", 
                                     conf_valueVec(*wrapAroundDemuxKeys), 0));

  hookUp(wrapAroundDemux, 0); // connect to the wrap around

  ElementSpecPtr unBoxWrapAround =
    _conf->addElement(ElementPtr(new UnboxField("UnBoxWrapAround:"+nodeID, 1)));
  _p2dl << conf_assign(unBoxWrapAround.get(), 
                       conf_function("UnboxField", "unboxWrapAround", 1));

  hookUp(wrapAroundDemux, 0, unBoxWrapAround, 0);

  ElementSpecPtr sendQueue = 
    _conf->addElement(ElementPtr(new Queue("SendQueue:"+nodeID, 1000)));
  _p2dl << conf_assign(sendQueue.get(), 
                       conf_function("Queue", "sendQueue", 1000));
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
    _p2dl << conf_assign(srcAddress.get(), 
                         conf_function("PelTransform", "addSrcAddrCC", oss.str()));
    ElementSpecPtr seq 
      = _conf->addElement(ElementPtr(new Sequence("SequenceCC" + nodeID)));
    _p2dl << conf_assign(seq.get(), 
                         conf_function("Sequence", "seqCC"));
    hookUp(srcAddress, 0);
    hookUp(seq, 0);

    // <data, seq, src, <t>>
    ElementSpecPtr tagData  = 
      _conf->addElement(ElementPtr(new PelTransform("TagData:" + nodeID, 
						       "\"ccdata\" pop $0 pop $1 pop $2 pop")));
    _p2dl << conf_assign(tagData.get(), 
                         conf_function("PelTransform", "tagData", 
                                       "\"ccdata\" pop $0 pop $1 pop $2 pop"));
    hookUp(tagData, 0);

    genPrintElement("PrintRemoteSendCCOne:"+nodeID);

    ElementSpecPtr pullPushCC =
      _conf->addElement(ElementPtr(new TimedPullPush("SendPullPushCC:"+nodeID, 0)));
    _p2dl << conf_assign(pullPushCC.get(), 
                         conf_function("TimedPullPush", "sendPullPushCC", 0));

    hookUp(pullPushCC, 0);
    hookUp(_ccTx, 0); // <seq, addr, <t>>

    // <dst, <seq, addr, <t>>
    ElementSpecPtr encapSendCC =
      _conf->addElement(ElementPtr(new PelTransform("encapSendCC:"+nodeID, 
						    "$3 1 field pop swallow pop"))); 
    _p2dl << conf_assign(encapSendCC.get(), 
                         conf_function("PelTransform", "encapSendCC",
                                       "$3 1 field pop swallow pop"));
    hookUp(_ccTx, 0, encapSendCC, 0);

    genPrintElement("PrintRemoteSendCCTwo:"+nodeID);
    
    _roundRobinCC =
       _conf->addElement(ElementPtr(new RoundRobin("roundRobinSenderCC:" + nodeID, 2))); 
    _p2dl << conf_assign(_roundRobinCC.get(), 
                         conf_function("RoundRobin",  
                                       "roundRobinSenderCC", 2));
     hookUp(_roundRobinCC, 0);

    // acknowledgements. <dst, <ack, seq, windowsize>>
    ElementSpecPtr ackPelTransform
      = _conf->addElement(ElementPtr(new PelTransform("ackPelTransformCC" + nodeID,
							"$0 pop \"ack\" ->t $1 append $2 append pop")));
    _p2dl << conf_assign(ackPelTransform.get(), 
                       conf_function("PelTransform", "ackPel",
                                     "$0 pop \"ack\" ->t $1 append $2 append pop"));
    
    hookUp(_ccRx, 1, ackPelTransform, 0);
    genPrintElement("PrintSendAck:"+nodeID);

    hookUp(_currentElementChain.back(), 0, _roundRobinCC, 1);

     // Now marshall the payload (second field)
     // <dst, marshalled>
     ElementSpecPtr marshalSendCC = 
       _conf->addElement(ElementPtr(new MarshalField("marshalCC:" + nodeID, 1)));
     _p2dl << conf_assign(marshalSendCC.get(), 
                          conf_function("MarshalField", "marshalCC", 1));
     genPrintElement("PrintRemoteSendCCMarshal:"+nodeID);
     hookUp(marshalSendCC, 0); 

  } else {

    // Now marshall the payload (second field)
    ElementSpecPtr marshalSend = 
      _conf->addElement(ElementPtr(new MarshalField("marshal:" + nodeID, 1)));  
    _p2dl << conf_assign(marshalSend.get(), 
                         conf_function("MarshalField", "marshal", 1));
    hookUp(marshalSend, 0);  
  }
   
  ElementSpecPtr routeSend =
    _conf->addElement(ElementPtr(new StrToSockaddr("plumber:" + nodeID, 0)));
  _p2dl << conf_assign(routeSend.get(), 
                       conf_function("StrToSockaddr", "plumber", 0));

  hookUp(routeSend, 0);
  hookUp(udpSend, 0);

  unsigned int xx = 0;
  // form the push senders
  for (unsigned int k = 0; k < _udpSenders.size(); k++) {
    ElementSpecPtr nextElementSpec = _udpSenders.at(k);

    //std::cout << "Encapsulation " << k << " pop " << _udpSendersPos.at(k) << "\n";

    ostringstream oss;
    oss << "$"<< _udpSendersPos.at(k) << " pop swallow pop";
    ElementSpecPtr encapSend =
      _conf->addElement(ElementPtr(new PelTransform("encapSend:" + nodeID, oss.str())));
    _p2dl << conf_assign(encapSend.get(), 
                       conf_function("PelTransform", "encapSend", oss.str()));
    
    // for now, assume addr field is the put here
    //hookUp(nextElementSpec, 0, roundRobin, k);

    if(!_ruleTracing){
      hookUp(nextElementSpec, 0, encapSend, 0);
      hookUp(encapSend, 0, roundRobin, k);
    }
    else {
      if(k > 0 && xx < _taps_end_vector.size()){
	ElementSpecPtr tap_end = _taps_end_vector.at(xx++);
	hookUp(nextElementSpec, 0, tap_end, 0);
	hookUp(tap_end, 0, encapSend, 0);
	hookUp(encapSend, 0, roundRobin, k);
      }
      else {
	hookUp(nextElementSpec, 0, encapSend, 0);
	hookUp(encapSend, 0, roundRobin, k);
      }
	
    }
  }

  return unBoxWrapAround;
}


// for a particular table name that we are receiving, 
// register an elementSpec that needs that data
void 
Plmb_ConfGen::registerReceiver(string tableName, 
                               ElementSpecPtr elementSpecPtr,
			       OL_Context::Rule * _curRule)
{
  // add to the right receiver
  ReceiverInfoMap::iterator _iterator = _udpReceivers.find(tableName);
  if (_iterator != _udpReceivers.end()) {
    _iterator->second.addReceiver(elementSpecPtr, _curRule->ruleNum);
  }
  std::cout << "RegisterReceiver " << tableName  << "\n";
  // if rule tracing, add a tap at the end of a rule here
  if(_ruleTracing){
    std::set<string> tt = _ctxt->getTuplesToTrace();
    if(tt.find(tableName) != tt.end()){
      std::cout << "Enabling needTracing for tuple " << tableName << "\n";
      _needTracingPortAtRR = true;
    }
    
    if(_taps_beg == 0)
      _taps_beg = new std::map<int, ElementSpecPtr>();
    
    ElementSpecPtr tap_beg = _conf->addElement(ElementPtr(new Tap("Tap_Beg:"+_curRule->ruleID, _curRule->ruleNum)));
    std::cout << "Reciever is " << elementSpecPtr->element()->name() << ", tap is " << tap_beg->element()->name() << "\n";
    _taps_beg->insert(std::make_pair(_curRule->ruleNum, tap_beg));
    _taps_beg_vector.push_back(tap_beg);
  }
  
}



// regiser a new receiver for a particular table name
// use to later hook up the demuxer
void Plmb_ConfGen::registerReceiverTable(OL_Context::Rule* rule, 
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

string Plmb_ConfGen::pelMath(FieldNamesTracker* names, Parse_Math *expr, 
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

string Plmb_ConfGen::pelRange(FieldNamesTracker* names, Parse_Bool *expr,
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

string Plmb_ConfGen::pelFunction(FieldNamesTracker* names, Parse_Function *expr, 
				OL_Context::Rule* rule) {
  ostringstream pel;

  if (expr->name() == "f_coinFlip") {
    Val_Double &val = dynamic_cast<Val_Double&>(*expr->arg(0)->v);
    pel << val.toString() << " coin "; 
  }
  else if (expr->name() == "f_rand") {
    pel << "rand "; 
  } 
  else if (expr->name() == "f_sha1") {
    int pos = names->fieldPosition(expr->arg(0)->toString());
    pel << "$" << (pos+1) << " sha1 "; 
  } 
  else if (expr->name() == "f_now") {
    pel << "now "; 
  }
  else {
    error("Pel function error " + expr->toString(), rule);
  }
  return pel.str();
}

string Plmb_ConfGen::pelBool(FieldNamesTracker* names, Parse_Bool *expr,
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
Plmb_ConfGen::pelSelect(OL_Context::Rule* rule, FieldNamesTracker* names, 
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
  oss << "selection_" << selectionID;

  ElementSpecPtr sPelTrans =
    _conf->addElement(ElementPtr(new PelTransform(oss.str(), sPel.str())));
  _p2dl << conf_assign(sPelTrans.get(), 
                       conf_function("PelTransform", "p_"+oss.str(), sPel.str()));
  
  if (_isPeriodic == false &&_pendingRegisterReceiver) {
    _pendingReceiverSpec = sPelTrans;
    _currentElementChain.push_back(sPelTrans); // first element in chain
    _pendingRegisterReceiver = false;
  } else {
    // connecting now
    hookUp(sPelTrans, 0);
  }

  genPrintElement(string("printSelect")+"_"+nodeID);
}

void 
Plmb_ConfGen::genAllSelectionAssignmentElements(OL_Context::Rule* curRule,
                                                string nodeID,
                                                FieldNamesTracker* 
                                                curNamesTracker) 
{
  unsigned counter = 0;
  for (std::list< Parse_Term* >::iterator j = curRule->terms.begin();
       j != curRule->terms.end();
       j++, counter++) {    
    Parse_Select* parse_select =
      dynamic_cast<Parse_Select *>(*j);
    if (parse_select != NULL) {
      debugRule(curRule, "Selection term "
                + parse_select->toString() + " " 
                + curRule->ruleID + "\n");
      pelSelect(curRule, curNamesTracker, parse_select, nodeID, counter); 
    }
    Parse_Assign* parse_assign =
      dynamic_cast<Parse_Assign *> (*j);
    if (parse_assign != NULL) {
      pelAssign(curRule, curNamesTracker, parse_assign, nodeID, counter);
    }
  }
}

void
Plmb_ConfGen::pelAssign(OL_Context::Rule* rule, 
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
    }
    else {
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
  _p2dl << conf_assign(assignPelTrans.get(), 
                       conf_function("PelTransform", "pel_assignemnt", pel.str()));

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


void
Plmb_ConfGen::genProjectHeadElements(OL_Context::Rule* curRule,
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
      if (fieldNameEq(parse_var->toString(), pf->getlocspec())) {	
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
      //warn << "Check " << aggExpr->v->toString() << " " << pf->getlocspec() << "\n";
      if (fieldNameEq(aggExpr->v->toString(), pf->getlocspec())) {	
		locationIndex = k;
      }
      if (aggExpr->aggName() != "COUNT") {
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

  if (locationIndex == -1 && !pf->getlocspec().empty()) {
    error("Head predicate \"" + pf->fn->name + "\" has invalid location specifier " 
		  + pf->getlocspec());
  }
						  
  if (locationIndex == -1) { 
    locationIndex = 0;     
  } // default

  ostringstream pelTransformStrbuf;
  pelTransformStrbuf << "\"" << pf->fn->name << "\" pop";

  if (pf->aggregate() != -1 && pf->getlocspec() == "") {
    pelTransformStrbuf << " \"" << nodeID << "\" pop";
  }

  for (unsigned int k = 0; k < indices.size(); k++) {
    pelTransformStrbuf << " $" << indices.at(k) << " pop";
  }

  int aggField = curRule->head->aggregate();
  if (aggField != -1) {
    Parse_Agg* aggExpr 
      = dynamic_cast<Parse_Agg*>(curRule->head->arg(aggField));
    if (aggExpr->aggName() == "COUNT") {
      // output the count
      pelTransformStrbuf << " $" << (aggField + 1) << " pop"; 
    } 
  }

  string pelTransformStr = pelTransformStrbuf.str();
  ostringstream oss;
  oss << "Project head " << curNamesTracker->toString() 
      << " " << pelTransformStr + " " << locationIndex << " " <<
    pf->getlocspec() << "\n";
  debugRule(curRule, oss.str());
 
  _currentPositionIndex = locationIndex + 1;
  // project, and make sure first field after table name has the address 
  ElementSpecPtr projectHeadPelTransform =
    _conf->addElement(ElementPtr(new PelTransform("ProjectHead:"+ curRule->ruleID + ":" + nodeID,
						     pelTransformStr)));
  _p2dl << conf_assign(projectHeadPelTransform.get(), 
                       conf_function("PelTransform", 
                                     "projectHead", 
                                     pelTransformStr));
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

ElementSpecPtr Plmb_ConfGen::createTapElement(OL_Context::Rule *curRule)
{
  ElementSpecPtr tap = _conf->addElement(ElementPtr(new Tap("Tap"+curRule->ruleID, curRule->ruleNum)));
  
  PrecondInfoMap::iterator _iterator = _taps_for_precond.find(curRule->ruleNum);
  if(_iterator == _taps_for_precond.end())
    _taps_for_precond.insert(std::make_pair(curRule->ruleNum, PreconditionInfo(curRule->ruleNum, tap)));
  else
    _iterator->second.addPrecondition(tap);
  return tap;
}




void
Plmb_ConfGen::genJoinElements(OL_Context::Rule* curRule, 
                              string nodeID, 
                              FieldNamesTracker* namesTracker,
                              boost::shared_ptr<Aggwrap> agg_el)
{
  // identify the events, use that to probe the other matching tables
  Parse_Functor* eventFunctor = NULL;
  std::vector<Parse_Term*> baseFunctors;
  bool eventFound = false;

  for (std::list< Parse_Term* >::iterator j = curRule->terms.begin();
       j != curRule->terms.end();
       j++) {    
    Parse_Functor* pf = dynamic_cast<Parse_Functor*>(*j);
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
	  error("There can be only one event predicate "
                "in a rule. Check all the predicates ", curRule);
	}
	eventFunctor = pf;
	eventFound = true;
      } 
    }
  }
  if (_isPeriodic == false) {
    // Is there an event functor?
    if (!eventFound) {
      error("I don't seem to have an event. Is this a static view?",
            curRule);
      return;
    } else {
      debugRule(curRule, "Event term " + eventFunctor->fn->name + "\n");
      // for all the base tuples, use the join to probe. 
      // keep track also the cur ordering of variables
      namesTracker->initialize(eventFunctor);
    }
  } else {
    debugRule(curRule, "Periodic joins " + namesTracker->toString() + "\n");
  }
  for (uint k = 0;
       k < baseFunctors.size();
       k++) {    
    Parse_Functor* pf = dynamic_cast<Parse_Functor*>(baseFunctors.at(k));
    
    if (pf != NULL &&
        (pf->fn->name == eventFunctor->fn->name)) {
      continue;
    } 
    debugRule(curRule, "Probing " + eventFunctor->fn->name + " " + 
              baseFunctors.at(k)->toString() + "\n");
    b_cbv comp_cb = 0;
    if (agg_el) {
      comp_cb = agg_el->get_comp_cb();
      _p2dl << conf_assign(&comp_cb, 
                           conf_call(agg_spec.get(), conf_function("get_comp_cb")));
    }
    
    FieldNamesTracker* baseProbeNames 
      = new FieldNamesTracker(baseFunctors.at(k));
    
    if (!fieldNameEq(eventFunctor->getlocspec(), pf->getlocspec())) {
      error("Event " + eventFunctor->fn->name + "@" 
            + eventFunctor->getlocspec() + 
            " and predicate " + pf->fn->name + "@" + pf->getlocspec()  
	    + " should have the same location specifier", curRule);
    }

    genProbeElements(curRule, eventFunctor, baseFunctors.at(k), 
		     nodeID, namesTracker, baseProbeNames, k, &comp_cb);

    if (agg_el || baseFunctors.size() - 1 != k) {
      // Change from pull to push
      ostringstream oss;
      oss << "JoinPullPush:" << curRule->ruleID << ":"
	  << nodeID << ":" << k;
      ElementSpecPtr pullPush =
	_conf->addElement(ElementPtr(new TimedPullPush(oss.str(), 0)));
      _p2dl << conf_assign(pullPush.get(), 
                  conf_function("TimedPullPush", "joinPullPush", 0));
      hookUp(pullPush, 0);
    }
  }
}


void
Plmb_ConfGen::secondaryIndex(Table2Ptr table,
                             Table2::Key key,
                             string nodeID)
{
  ostringstream uniqStr;
  uniqStr << table->name() << ":";
  std::vector< unsigned >::iterator iter = key.begin();
  while (iter != key.end()) {
    uniqStr << (*iter) << "_";
    iter++;
  }
  uniqStr << ":" << nodeID;
  if (_multTableIndices.find(uniqStr.str()) == _multTableIndices.end()) {
    // not there yet
    table->secondaryIndex(key);
    
    _p2dl << conf_call(table.get(),
                       conf_function("secondaryIndex",
                                     conf_UIntVec(key)),
                       false)
          << ";"
          << std::endl;
    _multTableIndices.insert(std::make_pair(uniqStr.str(), uniqStr.str()));
    std::cout << "AddMultTableIndex: Mult index added " << uniqStr.str() 
	      << "\n";
  } else {
    std::cout << "AddMultTableIndex: Mult index already exists " 
	      << uniqStr.str() << "\n";
  }
}



void
Plmb_ConfGen::genSingleTermElement(OL_Context::Rule* curRule, 
				       string nodeID, 
				       FieldNamesTracker* curNamesTracker)
{  
  ElementSpecPtr slotElement 
    = _conf->addElement(ElementPtr(new Slot("singleTermSlot:" 
					     + curRule->ruleID + ":" 
					     + nodeID)));
  _p2dl << conf_assign(slotElement.get(), 
                       conf_function("Slot", "singleTermSlot"));

  //std::cout << "Number of terms " << curRule->terms.size() << "\n";
  for (std::list< Parse_Term* >::iterator j = curRule->terms.begin();
       j != curRule->terms.end();
       j++) {    
    // skip those that we already decide is going to participate in     
    Parse_Term* curTerm = (*j);
    // skip the following
    Parse_Functor* pf = dynamic_cast<Parse_Functor*>(curTerm);
    if (pf == NULL) { continue; }
    registerReceiverTable(curRule, pf->fn->name); 
    registerReceiver(pf->fn->name, slotElement, curRule);
    _currentElementChain.push_back(slotElement);

    curNamesTracker->initialize(pf);    
    return; 
  }
}


void Plmb_ConfGen::genFunctorSource(OL_Context::Rule* rule, 
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
  _p2dl << conf_assign(source.get(), 
                       conf_function("TupleSource", "functorSource", 
                                     functorTuple->toConfString()));
  _currentElementChain.push_back(source);

  genPrintElement("PrintFunctorSource:"+rule->ruleID+":"+nodeID);
  Parse_Functor* pf = dynamic_cast<Parse_Functor* > (rule->terms.front());
  if (pf == NULL) {
    return;
  }
  
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
  _p2dl << conf_assign(pelRand.get(), 
                       conf_function("PelTransform", "functorSourcePel",
                                     "$0 pop $1 pop rand pop"));
 
  hookUp(pelRand, 0);

  // The timed pusher
  ElementSpecPtr pushFunctor =
    _conf->addElement(ElementPtr(new TimedPullPush("FunctorPush:" +rule->ruleID+ ":"+ nodeID,
						      atof(period.c_str()), count)));
  _p2dl << conf_assign(pushFunctor.get(), 
                       conf_function("TimedPullPush", "functorPush", 
                                     atof(period.c_str()), count));

  hookUp(pushFunctor, 0);

  //  if (rule->terms.size() <= 1) {
  if (numFunctors(rule) <= 1) {
    ElementSpecPtr functorSlot 
      = _conf->addElement(ElementPtr(new Slot("functorSlot:" + rule->ruleID + ":" + nodeID)));      
    _p2dl << conf_assign(functorSlot.get(), 
                         conf_function("Slot", "functorSlot"));
    hookUp(functorSlot, 0);
  }
}


void Plmb_ConfGen::genDupElimElement(string header)
{
   if (_dups) {
     ElementSpecPtr dupElim 
       = _conf->addElement(ElementPtr(new DupElim(header)));
     _p2dl << conf_assign(dupElim.get(), conf_function("DupElim", header));
     hookUp(dupElim, 0);
  }
}


void Plmb_ConfGen::genPrintElement(string header)
{
  if (_debug) {
    ElementSpecPtr print = 
      _conf->addElement(ElementPtr(new PrintTime(header)));
    _p2dl << conf_assign(print.get(), conf_function("PrintTime", header));
    hookUp(print, 0);
  }
}

void Plmb_ConfGen::genPrintWatchElement(string header)
{
  if (!_edit) {
    ElementSpecPtr printWatchElement = 
      _conf->addElement(ElementPtr(new PrintWatch(header, _ctxt->getWatchTables())));
    _p2dl << conf_assign(printWatchElement.get(), 
                         conf_function("PrintWatch", header, 
                                       conf_StrVec(_ctxt->getWatchTables())));
    hookUp(printWatchElement, 0);
  }
}


void
Plmb_ConfGen::genTraceElement(string header)
{
  if(_ruleTracing){
    std::set<string> tt = _ctxt->getTuplesToTrace();
    if(tt.find(header) != tt.end()){
      //if(header == "lookup"){
      std::cout << "Adding a traceTuple element for tuple "
                << header
                << ", size "
                << tt.size()
                << "\n";
      ElementSpecPtr traceElement =
	_conf->addElement(ElementPtr(new TraceTuple("inTrace" + header,
                                                    header)));
      
      hookUp(traceElement, 0);
      _traceTupleElements.push_back(traceElement);
    }
  }
}



///////////////////////////////////////////////////////////////////


////////////////////////// Table Creation ///////////////////////////

// Get a handle to the table. Typically used by the driver program to 
// preload some data.
Table2Ptr
Plmb_ConfGen::getTableByName(string nodeID,
                             string tableName)
{
  //std::cout << "Get table " << nodeID << ":" << tableName << "\n";
  TableMap::iterator _iterator = _tables.find(nodeID + ":" + tableName);
  if (_iterator == _tables.end()) { 
    error("Table " + nodeID + ":" + tableName + " not found\n");
  }
  return _iterator->second;
}



void
Plmb_ConfGen::createTables(string nodeID)
{
  _p2dl << conf_comment("CREATING TABLE VARIABLES");
  // have to decide where joins are possibly performed, and on what
  // fields to create appropriate indices for them
  OL_Context::TableInfoMap::iterator _iterator;
  for (_iterator = _ctxt->getTableInfos()->begin(); 
       _iterator != _ctxt->getTableInfos()->end();
       _iterator++) {
    OL_Context::TableInfo* tableInfo = _iterator->second;
    // create the table, add the unique local name, store in hash table
    

    // What's my expiration? -1 in the inputs means no expiration.
    boost::posix_time::time_duration expiration = tableInfo->timeout;

    // What's my size? -1 in the inputs means no size
    uint32_t tableSize = tableInfo->size;

    // What's my primary key?
    Table2::Key key = tableInfo->primaryKeys;

    // What's the table name. This is unique across nodes
    string newTableName = nodeID + ":" + tableInfo->tableName;


    // Create the table. Should this table be traced?
    Table2Ptr newTable;

    std::set< string, std::less< string > > tablesToTrace =
      _ctxt->getTablesToTrace();
    std::set< string, std::less< string > >::iterator i =
      tablesToTrace.find(tableInfo->tableName);
    
    if (_ruleTracing &&
        (i != tablesToTrace.end())) {
      _needTracingPortAtRR = true;
      TableTracer* tracer = new TableTracer(tableInfo->tableName,
                                            key,
                                            tableSize,
                                            expiration);
      newTable.reset(tracer);
      _p2dl << conf_assign(newTable.get(), 
                           conf_function("TableTracer",
                                         tableInfo->tableName,
                                         conf_UIntVec(key), 
                                         tableSize, 
                                         boost::posix_time::
                                         to_simple_string(expiration)));
      
      // Insert the tracer element into the table tracers, so that it
      // can be connected to the demux eventually
      ElementSpecPtr tableTraceElement =
	_conf->addElement(tracer->getElementPtr());
      _tableTracers.push_back(tableTraceElement);
    } else {
      newTable.reset(new Table2(tableInfo->tableName,
                                key,
                                tableSize,
                                expiration));
      _p2dl << conf_assign(newTable.get(), 
                           conf_function("Table2",
                                         tableInfo->tableName,
                                         conf_UIntVec(key), 
                                         tableSize, 
                                         boost::posix_time::
                                         to_simple_string(expiration)));
    }
    
    // And store it in the table index
    _tables.insert(std::make_pair(newTableName, newTable));      
  }


  _p2dl << conf_comment("CREATE ALL TABLE FACTS");
  // Now handle facts
  for (unsigned int k = 0;
       k < _ctxt->getFacts().size();
       k++) {
    TuplePtr tr = _ctxt->getFacts().at(k);
    ValuePtr vr = (*tr)[0];
    std::cout << "Insert tuple "
              << tr->toString()
              << " into table " 
	      << vr->toString()
              << " "
              << tr->size()
              << "\n";
    Table2Ptr tableToInsert = getTableByName(nodeID, vr->toString());     
    tableToInsert->insert(tr);
    std::cout << "Tuple inserted: "
              << tr->toString() 
	      << " into table "
              << vr->toString() 
	      << " "
              << tr->size()
              << "\n";
    _p2dl << conf_call(tableToInsert.get(),
                       conf_function("insert", tr->toConfString()), false)
          << ";"
          << std::endl;
  }
  _p2dl << conf_comment("END OF TABLE FACTS");
  _p2dl << conf_comment("END OF CREATING TABLE VARIABLES");
  _p2dl << std::endl;
}



/////////////////////////////////////////////

///////////// Helper functions

void Plmb_ConfGen::hookUp(ElementSpecPtr firstElement, int firstPort,
			 ElementSpecPtr secondElement, int secondPort)
{
  fprintf(_output, "Connect: \n");
  fprintf(_output, "  %s %s %d\n", firstElement->toString().c_str(), 
	  firstElement->element()->_name.c_str(), firstPort);
  fprintf(_output, "  %s %s %d\n", secondElement->toString().c_str(), 
	  secondElement->element()->_name.c_str(), secondPort);
  fflush(_output);
  
  _conf->hookUp(firstElement, firstPort, secondElement, secondPort);
  _p2dl << conf_hookup(conf_var(firstElement.get()), firstPort, 
                       conf_var(secondElement.get()), secondPort);

  if (_currentElementChain.size() == 0) {
    // first time
    _currentElementChain.push_back(firstElement);
  } 
  _currentElementChain.push_back(secondElement);

}

void Plmb_ConfGen::hookUp(ElementSpecPtr secondElement, int secondPort)
{
  if (_currentElementChain.size() == 0) {
    _currentElementChain.push_back(secondElement);
    assert(secondPort == 0);
    return;
  }
  hookUp(_currentElementChain.back(), 0, secondElement, secondPort);
}


int
Plmb_ConfGen::numFunctors(OL_Context::Rule* rule)
{
  int count = 0;

  for (std::list< Parse_Term* >::iterator j = rule->terms.begin();
       j != rule->terms.end();
       j++) {    
    Parse_Functor* pf = dynamic_cast<Parse_Functor*>(*j);
    if (pf != NULL) {
      count ++;
      continue;
    }
  }
  
  return count;
}






bool
Plmb_ConfGen::hasPeriodicTerm(OL_Context::Rule* curRule)
{
  for (std::list< Parse_Term* >::iterator j = curRule->terms.begin();
       j != curRule->terms.end();
       j++) {    
    Parse_Functor* pf = dynamic_cast<Parse_Functor*>(*j);
    if (pf == NULL) {
      continue;
    }
    checkFunctor(pf, curRule);
    string termName = pf->fn->name;
    if (termName == "periodic") {
      // Move it to the front if not already there
      if (j != curRule->terms.begin()) {
        curRule->terms.push_front(pf);
        curRule->terms.erase(j);
      }
      return true;
    }
  }
  return false;
}



///////////////////////////////////////////////////////////////////////////

Plmb_ConfGen::FieldNamesTracker::FieldNamesTracker() { }

Plmb_ConfGen::FieldNamesTracker::FieldNamesTracker(Parse_Term* pf)
{
  initialize(pf);
}


void 
Plmb_ConfGen::FieldNamesTracker::initialize(Parse_Term* term)
{
  Parse_Functor* pf = dynamic_cast<Parse_Functor* > (term);    
 
  if (pf != NULL) {
    for (int k = 0; k < pf->args(); k++) {
      Parse_Var* parse_var = dynamic_cast<Parse_Var*>(pf->arg(k));
      fieldNames.push_back(parse_var->toString());
    }  
  }
}


int 
Plmb_ConfGen::FieldNamesTracker::fieldPosition(string var)
{
  for (uint k = 0; k < fieldNames.size(); k++) {
    if (fieldNameEq(fieldNames.at(k), var)) {
      return k;
    }
  }
  return -1;
}

void 
Plmb_ConfGen::FieldNamesTracker::mergeWith(std::vector<string> names)
{
  for (uint k = 0; k < names.size(); k++) {
    string nextStr = names.at(k);
    if (fieldPosition(nextStr) == -1) {
      fieldNames.push_back(nextStr);
    }
  }
}

void 
Plmb_ConfGen::FieldNamesTracker::mergeWith(std::vector<string> names, 
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

string Plmb_ConfGen::FieldNamesTracker::toString()
{
  ostringstream toRet;

  toRet << "FieldNamesTracker<";
  
  for (uint k = 0; k < fieldNames.size(); k++) {
    toRet << fieldNames.at(k) << " ";
  }
  toRet << ">";
  return toRet.str();
}

void Plmb_ConfGen::error(string msg)
{
  std::cerr << "PLANNER ERROR: " << msg << "\n";
  exit(-1);
}


void
Plmb_ConfGen::error(string msg, 
                    OL_Context::Rule* rule)
{
  std::cerr << "PLANNER ERROR: " << msg 
	    << " for rule " << rule->ruleID << ". Planner exits.\n";
  exit(-1);
}


