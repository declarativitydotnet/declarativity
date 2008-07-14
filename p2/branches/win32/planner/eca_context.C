// -*- c-basic-offset: 2; related-file-name: "ol_context.C" -*-
/*
 * @(#)$Id$
 *
 *
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * Or
 * UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
 * Berkeley, CA,  94707. Attention: P2 Group.
 * 
 * DESCRIPTION: Rewrite environment for ECA rules
 *
 */

#include "eca_context.h"
#include "planContext.h"
#include "parser_util.h"
#include "planner.h"

string
Parse_Event::toString()
{
  ostringstream b;
  if (_event == P2_RECV) { b << "EVENT_RECV<" << _pf->toString() << ">"; }
  if (_event == P2_INSERT) { b << "EVENT_INSERT<" << _pf->toString() << ">"; }
  if (_event == P2_DELETE) { b << "EVENT_DELETE<" << _pf->toString() << ">"; }
  if (_event == P2_REFRESH) { b << "EVENT_REFRESH<" << _pf->toString() << ">"; }
  return b.str();
}

Parse_Event::Parse_Event(Parse_Functor *pf,
                         Event e)
  : _pf(pf),
    _event(e)
{
}
  

string Parse_Action::toString()
{
  ostringstream b;
  if (_action == P2_SEND) { b << "ACTION_SEND<" << _pf->toString() << ">"; }
  if (_action == P2_ADD) { b << "ACTION_ADD<" << _pf->toString() << ">"; }
  if (_action == P2_DELETE) { b << "ACTION_DELETE<" << _pf->toString() << ">"; }
  if (_action == P2_DROP) { b << "ACTION_DROP<" << _pf->toString() << ">"; }
  return b.str();
}


string
ECA_Rule::toString()
{
  ostringstream b;
  b << "ECA Rule "
    << _ruleID
    << " "
    << toRuleString();
  return b.str();  
}


string
ECA_Rule::toRuleString()
{
  ostringstream b;
  if (_event != NULL) {
    b << _action->toString() << " :- " << _event->toString();

    if (_probeTerms.size() + _selectAssignTerms.size() == 0) {
      b << ".";
    } else {
      b << ", ";
    }
  } else {
    b << _action->toString() << " :- ";
  }

  unsigned k = 0;
  for (k = 0;
       k + 1 < _probeTerms.size();
       k++) {
    b << _probeTerms.at(k)->toString()
      << ", ";
  }
  if (_probeTerms.size() > 0) {
    b << _probeTerms.at(k)->toString();
    if (_selectAssignTerms.size() > 0) {
      b << ", ";
    } else {
      b << ".";
    }
  }
  for (k = 0;
       k + 1 < _selectAssignTerms.size();
       k++) {
    b << _selectAssignTerms.at(k)->toString()
      << ", ";
  }
  if (_selectAssignTerms.size() > 0) {
    b << _selectAssignTerms.at(k)->toString()
      << ".";
  }
  return b.str();  
}


ECA_Rule::ECA_Rule(string r) 
  : _ruleID(r) 
{
  _event = NULL;
  _action = NULL;
  _aggWrap = false;
}


string
ECA_Rule::getEventName()
{
  return _event->_pf->fn->name;
}


void
ECA_Context::rewrite(Localize_Context* lctxt,
                     TableStore* tableStore)
{
  for (unsigned k = 0;
       k < lctxt->getRules().size();
       k++) {
    OL_Context::Rule* nextRule = lctxt->getRules().at(k);
    int countEvents = 0;

    // First count the events in the next rule
    std::list< Parse_Term* >::iterator t =
      nextRule->terms.begin();
    for(;
        t != nextRule->terms.end();
        t++) {
      Parse_Term* nextTerm = (*t);    
      // Is the next rule term a tuple?
      Parse_Functor* nextFunctor = dynamic_cast< Parse_Functor* >(nextTerm); 
      if (nextFunctor != NULL) {
        // Next term is a tuple
        string termName = nextFunctor->fn->name;
        // Is it materialized?
        OL_Context::TableInfo* termTableInfo =
          tableStore->getTableInfo(termName);	  
        if (termTableInfo == NULL) {
          // Not materialized, so this is an event
          countEvents ++;
        }
      }
    }

    // Do I have more than 1 events in the rule?
    if (countEvents > 1) {
      PLANNER_ERROR_NOPC(nextRule->toString()
                         << " should have at most one event");
      exit(-1);
    }

    // Does this rule have 0 events?
    if (countEvents == 0) {
      // view rules with no events
      int aggField = nextRule->head->aggregate();
      if (aggField >= 0) { // there is an aggregate
        // This is an aggregate view rule. Handle it
	rewriteAggregateView(nextRule, tableStore);
      } else {
        // No aggregate. This is a normal view rule
        rewriteViewRule(nextRule, tableStore);
      }
    } else {
      // This rule has exactly 1 event. Handle like a traditional ECA
      // rules
      rewriteEventRule(nextRule, tableStore);
    }
  }

  // We're done dealing with localized rules and they have all turned
  // into ECA rules.

  // Finally, create dummy rules for all watched receive events
  OL_Context::WatchTableType watches =
    tableStore->getWatchTables();
  for (OL_Context::WatchTableType::iterator i =
         watches.begin();
       i != watches.end();
       i++) {
    string watchedName = i->first;
    string watchedNameModifiers = i->second;
    PLANNER_INFO_NOPC("Watching '"
                      << watchedName
                      << "' with mods '"
                      << watchedNameModifiers
                      << "'");
    if ((watchedNameModifiers == "") ||
        (watchedNameModifiers.find("c") != string::npos)) {
      PLANNER_INFO_NOPC("The reception of tuple '"
                        << watchedName
                        << "' should be watched.");

      // Now, do I already have a rule with a RECV_EVENT<watchedName>?
      bool found = false;
      std::vector< ECA_Rule* > rules = getRules();
      for (std::vector< ECA_Rule* >::iterator i = rules.begin();
           i != rules.end();
           i++) {
        ECA_Rule* thisRule = (*i);
        
        // What's the event?
        Parse_Event* theEvent = thisRule->_event;

        // Is the event a receive?
        if (theEvent->_event == Parse_Event::P2_RECV) {
          // And is it a receive of the watched name?
          if (theEvent->_pf->fn->name == watchedName) {
            PLANNER_INFO_NOPC("The reception of tuple '"
                              << watchedName
                              << "' is already watched, no stub needed.");
            found = true;
            break;
          }
        }
      }

      // Is my tuple watched?
      if (found) {
        // No need for extra stub rule
      } else {
        watchStubRule(watchedName);
      }
    }
  }
}


void
ECA_Context::watchStubRule(string watchedName)
{
  PLANNER_INFO_NOPC("Creating watch-stub rule for tuple '"
                    << watchedName
                    << "'");

  ECA_Rule* watchStubRule = new ECA_Rule(watchedName +
                                         "_watchStub");

  Parse_ExprList* periodicArgs = new Parse_ExprList(); 
  Parse_Var* dummyLocspec = new Parse_Var(Val_Str::mk("A"));
  dummyLocspec->setLocspec();

  periodicArgs->push_back(dummyLocspec); // the dummy locspec

  Parse_Functor* recvFunctor = 
    new Parse_Functor(new Parse_FunctorName
                      (new Parse_Val(Val_Str::mk(watchedName))), 
                      periodicArgs);
  watchStubRule->_event = new Parse_Event(recvFunctor,
                                          Parse_Event::P2_RECV);


  watchStubRule->_action = new Parse_Action(recvFunctor,
                                            Parse_Action::P2_DROP);

  add_rule(watchStubRule);
}



void
ECA_Context::rewriteEventRule(OL_Context::Rule* rule, 
                              TableStore* tableStore)
{
  // figure out which is the event. 
  PLANNER_INFO_NOPC("Perform ECA rewrite on "
                    << rule->toString());

  // The location specifier of the rule body. Since this is a localized
  // rule, this is also the location specifier of the event tuple
  string bodyloc("");
  string ruleIDPrefix = rule->ruleID + "_eca";

  /** The new (currently empty) ECA rule */
  ECA_Rule* eca_rule = new ECA_Rule(ruleIDPrefix);    

  /** For all old-rule terms */
  int counter = 1;
  for(std::list< Parse_Term* >::iterator t =
        rule->terms.begin();
      t != rule->terms.end();
      t++, counter++) {
    // The actual term
    Parse_Term* nextTerm = (*t);    

    // Attempt to cast it as a tuple, selection, or assignment
    Parse_Functor *nextFunctor = dynamic_cast<Parse_Functor*>(nextTerm); 
    Parse_Select *nextSelect = dynamic_cast<Parse_Select*>(nextTerm); 
    Parse_Assign *nextAssign = dynamic_cast<Parse_Assign*>(nextTerm); 

    // Is it a tuple?
    if (nextFunctor != NULL) {     
      // This is a tuple term
      bodyloc = nextFunctor->getlocspec();
      string termName = nextFunctor->fn->name;
      OL_Context::TableInfo* termTableInfo =
        tableStore->getTableInfo(termName);	  
      if (termTableInfo != NULL) {    
	// this is not the event, append it to the conditions of the ECA
	// rule
	eca_rule->_probeTerms.push_back(nextFunctor);
      } else {
        // This is indeed an event. Handle the "periodic" event in a
        // separate way.
	if (termName == "periodic") {
	  // Break this up into two rules. eca_rule1 will be
          // SEND_ACTION<ruleID+periodic(@NodeID, EventID)> :-
          // EVENT_RECV<periodic(NodeID, EventID, Period, Repeats)>.
          string periodicRuleID = ruleIDPrefix + "periodic";
          ECA_Rule* eca_rule1 = new ECA_Rule(periodicRuleID);

          // The event generated by the periodic
	  Parse_ExprList* periodicArgs = new Parse_ExprList(); 
	  periodicArgs->push_back(nextFunctor->arg(0)); // the locspec
	  periodicArgs->push_back(nextFunctor->arg(1)); // the event ID
	  ValuePtr name = Val_Str::mk(periodicRuleID);
	  ValuePtr loc = Val_Str::mk(nextFunctor->getlocspec());
	  Parse_Functor* sendFunctor = 
	    new Parse_Functor(new Parse_FunctorName(new Parse_Val(name)), 
                              periodicArgs, new Parse_Val(loc));

          // The actual rule has the sendFunctor as the action and the
	  // periodic tuple as the event
	  eca_rule1->_action = new Parse_Action(sendFunctor, Parse_Action::P2_SEND);    	  
	  eca_rule1->_event = new Parse_Event(nextFunctor, Parse_Event::P2_INSERT);    	  
	  add_rule(eca_rule1);


          // The rest of the original periodic rule, to be stored in
	  // eca_rule. Listening for the tuples generated by eca_rule1
	  Parse_Functor* recvFunctor = 
	    new Parse_Functor(new Parse_FunctorName(new Parse_Val(name)), 
                              periodicArgs, new Parse_Val(loc));
	  eca_rule->_event = new Parse_Event(recvFunctor,
                                             Parse_Event::P2_RECV);
          // The rest of the processing of further original terms will
          // be appended to eca_rule
	} else {
          // Just plonk down the event to the eca rule
	  eca_rule->_event = new Parse_Event(nextFunctor, Parse_Event::P2_RECV);    
	}
      }
    } else {
      // OK this is not a tuple. It might be a selection or an
      // assignment. Regardless of what it is, plonk it into the "other
      // terms" of the ECA rule.
      if (nextSelect != NULL || nextAssign != NULL) {
        eca_rule->_selectAssignTerms.push_back(nextTerm);
      } else {
        // This is a term type that we didn't think of. Complain but
        // move on
        PLANNER_WARN_NOPC("The "
                          << counter
                          << "-th term of rule '"
                          << rule->toString()
                          << "' has an unknown type. Ignoring.");
      }
    }
  }

  // now generate the head action
  generateActionHead(rule, bodyloc, tableStore, eca_rule);

  // And carry over the aggregate-ness of this rule into the newly
  // minted ECA rule
  int aggField = rule->head->aggregate();
  if (aggField >= 0) { // there is an aggregate
    eca_rule->_aggWrap = true;
  }

  add_rule(eca_rule);
}


void
ECA_Context::generateActionHead(OL_Context::Rule* rule,
                                string bodyLoc,
                                TableStore* tableStore,
                                ECA_Rule* eca_rule)
{

  // Is the rule head an unmaterialized tuple name?
  string headName = rule->head->fn->name;
  OL_Context::TableInfo* headTableInfo = tableStore->getTableInfo(headName);
  if (headTableInfo == NULL) {
    // It's unmaterialized, i.e., an event. Just ACTION_SEND it
    eca_rule->_action = new Parse_Action(rule->head, Parse_Action::P2_SEND);
  } else {
    // to be materialized
    if (!rule->head->getlocspec().empty()
        && fieldNameEq(rule->head->getlocspec(), bodyLoc)) {
      // local materialization
      if (rule->deleteFlag) {
	eca_rule->_action = new Parse_Action(rule->head, Parse_Action::P2_DELETE);
      } else {
	eca_rule->_action = new Parse_Action(rule->head, Parse_Action::P2_ADD);
      }
    } else {
      // remote materialization. Generate a separate send rule and a
      // separate local materialization rule. The current rule eca_rule
      // does the sending. The new rule eca_rule1 will do the storing.
      ValuePtr name = Val_Str::mk(rule->ruleID + rule->head->fn->name + "send");
      ValuePtr loc = Val_Str::mk(rule->head->getlocspec());
      Parse_Functor* sendFunctor = 
        new Parse_Functor(new Parse_FunctorName(new Parse_Val(name)), 
                          rule->head->args_, new Parse_Val(loc));
      eca_rule->_action = new Parse_Action(sendFunctor, Parse_Action::P2_SEND);


      string headName = rule->head->fn->name;
      OL_Context::TableInfo* headTableInfo = tableStore->getTableInfo(headName);
      if (headTableInfo != NULL) {
        string materializationRuleName = rule->ruleID + "ECAMat";
	ECA_Rule* eca_rule1 = new ECA_Rule(materializationRuleName);    
        // The event is the reception of the head of the rule above
	eca_rule1->_event = new Parse_Event(sendFunctor, Parse_Event::P2_RECV);
	if (rule->deleteFlag) {
	  eca_rule1->_action = new Parse_Action(rule->head, Parse_Action::P2_DELETE);
	} else {
	  eca_rule1->_action = new Parse_Action(rule->head, Parse_Action::P2_ADD);      
	}
	add_rule(eca_rule1);
      }
    }
  }
}


void
ECA_Context::add_rule(ECA_Rule* eca_rule)
{
  for (unsigned k = 0; k < _ecaRules.size(); k++) {
    if (_ecaRules.at(k)->toRuleString() == eca_rule->toRuleString()) {
      return;
    }
  }  
  PLANNER_INFO_NOPC("  Add rule: " << eca_rule->toString());
  _ecaRules.push_back(eca_rule);  
}


void
ECA_Context::rewriteViewRule(OL_Context::Rule* rule,
                             TableStore* tableStore)
{ 
  PLANNER_INFO_NOPC("Perform ECA view rewrite on " << rule->toString());

  string headName = rule->head->fn->name;
  OL_Context::TableInfo* headTableInfo = tableStore->getTableInfo(headName);	  

  bool softStateRule = false;
  std::list<Parse_Term*>::iterator t = rule->terms.begin();
  for(;
      t != rule->terms.end();
      t++) {
    Parse_Term* nextTerm = (*t);
    Parse_Functor *nextFunctor = dynamic_cast<Parse_Functor*>(nextTerm); 
    if (nextFunctor == NULL) {
      continue;
    }
    OL_Context::TableInfo* tableInfo = tableStore->getTableInfo(nextFunctor->fn->name);   
    if (tableInfo == NULL || tableInfo->timeout != Table2::NO_EXPIRATION) {
      softStateRule = true; // if any rule body is soft-state, rule is soft-state
      break;
    }
  }
  if (headTableInfo == NULL ||
      headTableInfo->timeout != Table2::NO_EXPIRATION) {
    softStateRule = true;     // if head is unmaterialized or softstate,
                              // then the rule is soft-state
  }
  PLANNER_INFO_NOPC("Processing soft state rule " << softStateRule
                    << " " << rule->toString());

  t = rule->terms.begin();
  int count = 0;  

  // For every materialized rule-body term, create a set of delta rules
  // (one for insertions, one for refreshes, and one for deletes)
  // containing all other terms. Softstate rules only get the insert
  // rule per materialized term, whereas hardstate rules get all three
  // rules per term. If the action is a local materialized table, the
  // actions are performed directly (insert for insertions and
  // refreshes, delete for deletes).  If the action is a remote
  // materialized table, then a proxy rule is generated that sends the
  // appropriate message across, which causes the remote materialized
  // table to be affected as would have been the case for a locally
  // materialized table. If the action is a non-materialized table
  // (local or remote), then only a send action is created (local or
  // remote as the case may be).
  for(;
      t != rule->terms.end();
      t++) {
    Parse_Term* nextTerm = (*t);
    Parse_Functor* nextFunctor = dynamic_cast<Parse_Functor*>(nextTerm); 
    if (nextFunctor == NULL) {
      count++;
      continue;
    }

    // create an event
    ostringstream oss;
    oss << rule->ruleID << "Eca" << count;    
    ECA_Rule* eca_insert_rule = new ECA_Rule(oss.str() + "Ins");    
    ECA_Rule* eca_delete_rule = new ECA_Rule(oss.str() + "Del");    
    ECA_Rule* eca_refresh_rule = new ECA_Rule(oss.str() + "Ref");    

    // delete functor generated from delete event
    ValuePtr name = Val_Str::mk(rule->head->fn->name + "delete");
    ValuePtr loc = Val_Str::mk(rule->head->getlocspec());
    Parse_Functor *deleteFunctor = 
      new Parse_Functor(new Parse_FunctorName(new Parse_Val(name)), 
						rule->head->args_, new Parse_Val(loc));

    ValuePtr nameSend = Val_Str::mk(rule->ruleID + rule->head->fn->name + "send");
    ValuePtr locSend = Val_Str::mk(rule->head->getlocspec());
    Parse_Functor* sendFunctor = 
      new Parse_Functor(new Parse_FunctorName(new Parse_Val(nameSend)), 
						rule->head->args_, new Parse_Val(locSend));
    
    // create the events
    eca_insert_rule->_event 
      = new Parse_Event(nextFunctor, Parse_Event::P2_INSERT);
    eca_refresh_rule->_event 
      = new Parse_Event(nextFunctor, Parse_Event::P2_REFRESH);
    eca_delete_rule->_event 
      = new Parse_Event(nextFunctor, Parse_Event::P2_DELETE);    

    bool softStatePredicate = false;
    OL_Context::TableInfo* tableInfo = tableStore->getTableInfo(nextFunctor->fn->name);   
    if (tableInfo == NULL || tableInfo->timeout != Table2::NO_EXPIRATION) {
      softStatePredicate = true;
    }

    if (!rule->head->getlocspec().empty() 
        && fieldNameEq(rule->head->getlocspec(), nextFunctor->getlocspec())) {
      // if this is local, we can simply add local table or send as an event
      if (headTableInfo != NULL) {
	eca_insert_rule->_action 
	  = new Parse_Action(rule->head, Parse_Action::P2_ADD);
	eca_refresh_rule->_action 
	  = new Parse_Action(rule->head, Parse_Action::P2_ADD);
	eca_delete_rule->_action 
	  = new Parse_Action(rule->head, Parse_Action::P2_DELETE);
      } else {
	// send head events
	eca_insert_rule->_action 
	  = new Parse_Action(rule->head, Parse_Action::P2_SEND);
	eca_refresh_rule->_action 
	  = new Parse_Action(rule->head, Parse_Action::P2_SEND);
	eca_delete_rule->_action 
	  = new Parse_Action(deleteFunctor, Parse_Action::P2_SEND);
      }
    } else {
      // if the head is remote, we have to do a send, 
      // followed by another recv/add rule strand
      eca_insert_rule->_action = new Parse_Action(sendFunctor, Parse_Action::P2_SEND);
      eca_delete_rule->_action = new Parse_Action(deleteFunctor, Parse_Action::P2_SEND);
      eca_refresh_rule->_action = new Parse_Action(sendFunctor, Parse_Action::P2_SEND);
      if (headTableInfo == NULL) {
        // The rule head is a remote event. Just send the appropriate info
	eca_insert_rule->_action 
	  = new Parse_Action(rule->head, Parse_Action::P2_SEND);
	eca_refresh_rule->_action 
	  = new Parse_Action(rule->head, Parse_Action::P2_SEND);
	eca_delete_rule->_action 
	  = new Parse_Action(deleteFunctor, Parse_Action::P2_SEND);
      } else {
        ostringstream oss;
	oss << rule->ruleID << "Eca" << count << "Remote";    

	// insert
	ECA_Rule* eca_insert_rule1 
	  = new ECA_Rule(oss.str() + "Ins");    
	eca_insert_rule1->_event 
	  = new Parse_Event(sendFunctor, Parse_Event::P2_RECV);
	eca_insert_rule1->_action 
	  = new Parse_Action(rule->head, Parse_Action::P2_ADD);      
	add_rule(eca_insert_rule1);

	// refresh
	ECA_Rule* eca_refresh_rule1 
	  = new ECA_Rule(oss.str() + "Ref");    
	eca_refresh_rule1->_event 
	  = new Parse_Event(sendFunctor, Parse_Event::P2_RECV);
	eca_refresh_rule1->_action 
	  = new Parse_Action(rule->head, Parse_Action::P2_ADD);      
	if (softStatePredicate == true) {
	  add_rule(eca_refresh_rule1);
	}

	// delete
	ECA_Rule* eca_delete_rule1 
	  = new ECA_Rule(oss.str() + "Del");    
	eca_delete_rule1->_event 
	  = new Parse_Event(deleteFunctor, Parse_Event::P2_RECV);
	eca_delete_rule1->_action 
	  = new Parse_Action(rule->head, Parse_Action::P2_DELETE);      
	if (!softStateRule) {
	  add_rule(eca_delete_rule1);
	}
      }
    }

    // create the other terms
    int count1 = 0;
    std::list<Parse_Term*>::iterator t = rule->terms.begin();
    for(; t != rule->terms.end(); t++) {
      if (count1 != count) { 
	Parse_Term* nextTerm1 = (*t);    
	Parse_Functor *nextFunctor1 = dynamic_cast<Parse_Functor*>(nextTerm1); 
	Parse_Select *nextSelect = dynamic_cast<Parse_Select*>(nextTerm1); 
	Parse_Assign *nextAssign = dynamic_cast<Parse_Assign*>(nextTerm1); 
	
	if (nextFunctor1 != NULL) {
	  eca_insert_rule->_probeTerms.push_back(nextFunctor1);
	  eca_delete_rule->_probeTerms.push_back(nextFunctor1);
	  eca_refresh_rule->_probeTerms.push_back(nextFunctor1);
	}
	if (nextSelect != NULL || nextAssign != NULL) {
	  eca_insert_rule->_selectAssignTerms.push_back(nextTerm1);
	  eca_refresh_rule->_selectAssignTerms.push_back(nextTerm1);
	  eca_delete_rule->_selectAssignTerms.push_back(nextTerm1);
	}
      }
      count1++;
    }
    add_rule(eca_insert_rule);
    if (softStatePredicate) {
      add_rule(eca_refresh_rule);
    }
    if (!softStateRule) {
      // only cascade deletes for hard-state rules
      add_rule(eca_delete_rule);
    }
    count++;
  }
}


void
ECA_Context::rewriteAggregateView(OL_Context::Rule* rule, 
                                  TableStore *tableStore)
{
  PLANNER_INFO_NOPC("Perform ECA aggregate view rewrite on "
               << rule->toString());
  if (rule->terms.size() != 1) {
    PLANNER_WARN_NOPC("Currently only support simple table view aggregates");
    exit(-1);
  }

  std::list<Parse_Term*>::iterator t = rule->terms.begin();
  Parse_Term* nextTerm = (*t);  

  Parse_Functor *nextFunctor = dynamic_cast<Parse_Functor*>(nextTerm); 
  if (nextFunctor == NULL) { 
    PLANNER_WARN_NOPC("Currently only support simple table view aggregates");
    exit(-1);
  }
  
  ostringstream oss;
  oss << rule->ruleID << "eca";    
  ECA_Rule* eca_rule = new ECA_Rule(oss.str());    
  eca_rule->_event = new Parse_Event(nextFunctor, Parse_Event::P2_INSERT);

  // generate the head action
  generateActionHead(rule, nextFunctor->getlocspec(), tableStore, eca_rule);
  add_rule(eca_rule);
}



string
ECA_Context::toString()
{
  ostringstream b;
  for (unsigned k = 0; k < _ecaRules.size(); k++) {
    b << _ecaRules.at(k)->toString() << "\n";
  }
  return b.str();
}
