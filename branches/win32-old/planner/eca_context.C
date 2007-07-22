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

string Parse_Event::toString()
{
  ostringstream b;
  if (_event == P2_RECV) { b << "EVENT_RECV<" << _pf->toString() << ">"; }
  if (_event == P2_INSERT) { b << "EVENT_INSERT<" << _pf->toString() << ">"; }
  if (_event == P2_DELETE) { b << "EVENT_DELETE<" << _pf->toString() << ">"; }
  if (_event == P2_REFRESH) { b << "EVENT_REFRESH<" << _pf->toString() << ">"; }
  return b.str();
}

string Parse_Action::toString()
{
  ostringstream b;
  if (_action == P2_SEND) { b << "ACTION_SEND<" << _pf->toString() << ">"; }
  if (_action == P2_ADD) { b << "ACTION_ADD<" << _pf->toString() << ">"; }
  if (_action == P2_DELETE) { b << "ACTION_DELETE<" << _pf->toString() << ">"; }
  return b.str();
}


string ECA_Rule::toString()
{
  ostringstream b;
  b << "ECA Rule " << _ruleID << " " << toRuleString();
  return b.str();  
}

string ECA_Rule::toRuleString()
{
  ostringstream b;
  if (_event != NULL) {
    b << _action->toString() << ":-" << _event->toString() << ",";
  } else {
    b << _action->toString() << ":-";
  }
  for (unsigned k = 0; k < _probeTerms.size(); k++) {
    b << _probeTerms.at(k)->toString() << ",";
  }
  for (unsigned k = 0; k < _selectAssignTerms.size(); k++) {
    b << _selectAssignTerms.at(k)->toString() << ",";
  }
  return b.str();  
}


void ECA_Context::add_rule(ECA_Rule* eca_rule)
{
  for (unsigned k = 0; k < _ecaRules.size(); k++) {
    if (_ecaRules.at(k)->toRuleString() == eca_rule->toRuleString()) {
      return;
    }
  }  
  PLANNER_INFO_NOPC("  Add rule: " << eca_rule->toString());
  _ecaRules.push_back(eca_rule);  
}

void ECA_Context::rewriteViewRule(OL_Context::Rule* rule, TableStore* tableStore)
{ 
  PLANNER_INFO_NOPC("Perform ECA view rewrite on " << rule->toString());

  string headName = rule->head->fn->name;
  OL_Context::TableInfo* headTableInfo = tableStore->getTableInfo(headName);	  

  bool softStateRule = false;
  std::list<Parse_Term*>::iterator t = rule->terms.begin();
  for(; t != rule->terms.end(); t++) {
    Parse_Term* nextTerm = (*t);
    Parse_Functor *nextFunctor = dynamic_cast<Parse_Functor*>(nextTerm); 
    if (nextFunctor == NULL) { continue; }
    OL_Context::TableInfo* tableInfo = tableStore->getTableInfo(nextFunctor->fn->name);   
    if (tableInfo == NULL || tableInfo->timeout != Table2::NO_EXPIRATION) {
      softStateRule = true; // if any rule body is soft-state, rule is soft-state
      break;
    }
  }
  OL_Context::TableInfo* tableInfo = tableStore->getTableInfo(rule->head->fn->name);   
  if (tableInfo == NULL || tableInfo->timeout != Table2::NO_EXPIRATION) {
    softStateRule = true; // if rule head is soft-state, rule is soft-state
  }
  PLANNER_INFO_NOPC("Processing soft state rule " << softStateRule
                    << " " << rule->toString());

  t = rule->terms.begin();
  int count = 0;  
  for(; t != rule->terms.end(); t++) {
    Parse_Term* nextTerm = (*t);
    Parse_Functor *nextFunctor = dynamic_cast<Parse_Functor*>(nextTerm); 
    if (nextFunctor == NULL) { count++; continue; }

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
    Parse_Functor *sendFunctor = 
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
      string headName = rule->head->fn->name;
      OL_Context::TableInfo* headTableInfo = tableStore->getTableInfo(headName);
      if (headTableInfo != NULL) {
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
	if (softStateRule == false) {
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
    if (softStatePredicate == true) {
      add_rule(eca_refresh_rule);
    }
    if (softStateRule == false) {
      // only cascade deletes for hard-state rules
      add_rule(eca_delete_rule);
    }
    count++;
  }
}

void ECA_Context::generateActionHead(OL_Context::Rule* rule, string bodyLoc,
				     TableStore* tableStore, ECA_Rule* eca_rule)
{

  // if event, just send
  string headName = rule->head->fn->name;
  OL_Context::TableInfo* headTableInfo = tableStore->getTableInfo(headName);
  if (headTableInfo == NULL) {
    // event, just send
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
      // remote materializatin. Send, followed by store
      ValuePtr name = Val_Str::mk(rule->ruleID + rule->head->fn->name + "send");
      ValuePtr loc = Val_Str::mk(rule->head->getlocspec());
      Parse_Functor *sendFunctor = 
		new Parse_Functor(new Parse_FunctorName(new Parse_Val(name)), 
						  rule->head->args_, new Parse_Val(loc));
      
      eca_rule->_action = new Parse_Action(sendFunctor, Parse_Action::P2_SEND);
      string headName = rule->head->fn->name;
      OL_Context::TableInfo* headTableInfo = tableStore->getTableInfo(headName);
      if (headTableInfo != NULL) {
        ostringstream oss;
	oss << rule->ruleID << "Eca" << "Mat";    
	ECA_Rule* eca_rule1 = new ECA_Rule(oss.str());    
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

void ECA_Context::rewriteEventRule(OL_Context::Rule* rule, 
				   TableStore* tableStore)
{
  // figure out which is the event. 
  PLANNER_INFO_NOPC("Perform ECA rewrite on " << rule->toString());

  // event-condition-action
  string loc("");
  std::list<Parse_Term*>::iterator t = rule->terms.begin();
  ostringstream oss;
  oss << rule->ruleID << "_eca";    
  ECA_Rule* eca_rule = new ECA_Rule(oss.str());    
  for(; t != rule->terms.end(); t++) {
    Parse_Term* nextTerm = (*t);    
    Parse_Functor *nextFunctor = dynamic_cast<Parse_Functor*>(nextTerm); 
    Parse_Select *nextSelect = dynamic_cast<Parse_Select*>(nextTerm); 
    Parse_Assign *nextAssign = dynamic_cast<Parse_Assign*>(nextTerm); 

    if (nextFunctor != NULL) {     
      loc = nextFunctor->getlocspec();
      string termName = nextFunctor->fn->name;
      OL_Context::TableInfo* termTableInfo = tableStore->getTableInfo(termName);	  
      if (termTableInfo != NULL) {    
	// this is not an event
	eca_rule->_probeTerms.push_back(nextFunctor);
      } else {
	if (termName == "periodic") {
	  // when there is a periodic, break this up into two rules
	  ECA_Rule* eca_rule1 = new ECA_Rule(oss.str() + "periodic");    

	  Parse_ExprList* periodicArgs = new Parse_ExprList();	  
	  periodicArgs->push_back(nextFunctor->arg(0));
	  periodicArgs->push_back(nextFunctor->arg(1));
	  ValuePtr name = Val_Str::mk(rule->ruleID + "periodic");
	  ValuePtr loc = Val_Str::mk(nextFunctor->getlocspec());
	  Parse_Functor *sendFunctor = 
	    new Parse_Functor(new Parse_FunctorName(new Parse_Val(name)), 
						  periodicArgs, new Parse_Val(loc));

	  eca_rule1->_action = new Parse_Action(sendFunctor, Parse_Action::P2_SEND);    	  
	  eca_rule1->_event = new Parse_Event(nextFunctor, Parse_Event::P2_INSERT);    	  
	  add_rule(eca_rule1);

	  Parse_Functor *recvFunctor = 
	    new Parse_Functor(new Parse_FunctorName(new Parse_Val(name)), 
						  periodicArgs, new Parse_Val(loc));

	  eca_rule->_event = new Parse_Event(recvFunctor, Parse_Event::P2_RECV);    	  
	} else {
	  eca_rule->_event = new Parse_Event(nextFunctor, Parse_Event::P2_RECV);    
	} 
      }
    }
    if (nextSelect != NULL || nextAssign != NULL) {
      eca_rule->_selectAssignTerms.push_back(nextTerm);
    }
  }

  // now generate the head action
  generateActionHead(rule, loc, tableStore, eca_rule);

  int aggField = rule->head->aggregate();
  if (aggField >= 0) { // there is an aggregate
    eca_rule->_aggWrap = true;
  }
  add_rule(eca_rule);
}

void ECA_Context::rewriteAggregateView(OL_Context::Rule* rule, 
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

void ECA_Context::rewrite(Localize_Context* lctxt, TableStore* tableStore)
{
   
  for (unsigned k = 0; k < lctxt->getRules().size(); k++) {
    OL_Context::Rule* nextRule = lctxt->getRules().at(k);
    int countEvents = 0;

    std::list<Parse_Term*>::iterator t = nextRule->terms.begin();
    for(; t != nextRule->terms.end(); t++) {
	Parse_Term* nextTerm = (*t);    
	Parse_Functor *nextFunctor = dynamic_cast<Parse_Functor*>(nextTerm); 
	if (nextFunctor != NULL) {
	  string termName = nextFunctor->fn->name;
	  OL_Context::TableInfo* termTableInfo = tableStore->getTableInfo(termName);	  
	  if (termTableInfo == NULL) {
	    countEvents ++;
	  }
	}
    }

    if (countEvents > 1) {
      PLANNER_ERROR_NOPC(nextRule->toString()
                         << " should have at most one event");
      exit(-1);
    }

    if (countEvents == 0) {
      // view rules with no events
      int aggField = nextRule->head->aggregate();
      if (aggField >= 0) { // there is an aggregate
	rewriteAggregateView(nextRule, tableStore);
	continue;
      }

      // otherwise, it is a normal view rule
      rewriteViewRule(nextRule, tableStore);
      continue;
    }

    // handle traditional ECA rules
    rewriteEventRule(nextRule, tableStore);    
  }
}

string ECA_Context::toString()
{
  ostringstream b;
  for (unsigned k = 0; k < _ecaRules.size(); k++) {
    b << _ecaRules.at(k)->toString() << "\n";
  }
  return b.str();
}
