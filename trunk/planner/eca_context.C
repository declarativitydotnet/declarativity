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

string Parse_Event::toString()
{
  ostringstream b;
  if (_event == RECV) { b << "EVENT_RECV<" << _pf->toString() << ">"; }
  if (_event == INSERT) { b << "EVENT_INSERT<" << _pf->toString() << ">"; }
  if (_event == DELETE) { b << "EVENT_DELETE<" << _pf->toString() << ">"; }
  return b.str();
}

string Parse_Action::toString()
{
  ostringstream b;
  if (_action == SEND) { b << "ACTION_SEND<" << _pf->toString() << ">"; }
  if (_action == ADD) { b << "ACTION_ADD<" << _pf->toString() << ">"; }
  if (_action == DELETE) { b << "ACTION_DELETE<" << _pf->toString() << ">"; }
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
  
 warn << "  Add rule: " << eca_rule->toString() << "\n";
  _ecaRules.push_back(eca_rule);  
}

void ECA_Context::rewriteViewRule(OL_Context::Rule* rule, TableStore* tableStore)
{
  warn << "Perform ECA view rewrite on " << rule->toString() << "\n";

  string headName = rule->head->fn->name;
  OL_Context::TableInfo* headTableInfo = tableStore->getTableInfo(headName);	  
  /*if (headTableInfo == NULL) {
    warn << "Head of " << rule->toString() << " must be materialized for view rules\n";
    exit(-1);
    }*/

  std::list<Parse_Term*>::iterator t = rule->terms.begin();
  int count = 0;
  for(; t != rule->terms.end(); t++) {
    Parse_Term* nextTerm = (*t);
    Parse_Functor *nextFunctor = dynamic_cast<Parse_Functor*>(nextTerm); 
    if (nextFunctor == NULL) { continue; }

    // create an event
    ostringstream oss;
    oss << rule->ruleID << "_eca" << count;    
    ECA_Rule* eca_insert_rule = new ECA_Rule(oss.str() + "_ins");    
    ECA_Rule* eca_delete_rule = new ECA_Rule(oss.str() + "_del");    

    // delete functor generated from delete event
    ValuePtr name = Val_Str::mk(rule->head->fn->name + "delete");
    ValuePtr loc = Val_Str::mk(rule->head->fn->loc);
    Parse_Functor *deleteFunctor = 
      new Parse_Functor(new Parse_FunctorName(new Parse_Val(name), 
					      new Parse_Val(loc)), 
			rule->head->args_);

    eca_insert_rule->_event = new Parse_Event(nextFunctor, Parse_Event::INSERT);
    eca_delete_rule->_event = new Parse_Event(nextFunctor, Parse_Event::DELETE);

    if (rule->head->fn->loc == nextFunctor->fn->loc) {
      // if this is local, we can simply add local table or send as an event
      if (headTableInfo != NULL) {
	eca_insert_rule->_action = new Parse_Action(rule->head, Parse_Action::ADD);
	eca_delete_rule->_action = new Parse_Action(rule->head, Parse_Action::DELETE);
      } else {
	// XXX: May not wish to support in future.
	eca_insert_rule->_action = new Parse_Action(rule->head, Parse_Action::SEND);
	eca_delete_rule->_action = new Parse_Action(deleteFunctor, Parse_Action::SEND);
      }
    } else {
      // if the head is remote, we have to do a send, 
      // followed by another recv/add rule strand
      eca_insert_rule->_action = new Parse_Action(rule->head, Parse_Action::SEND);
      eca_delete_rule->_action = new Parse_Action(deleteFunctor, Parse_Action::SEND);
      string headName = rule->head->fn->name;
      OL_Context::TableInfo* headTableInfo = tableStore->getTableInfo(headName);
      if (headTableInfo != NULL) {
        ostringstream oss;
	oss << rule->ruleID << "_eca" << count << "_remote";    
	ECA_Rule* eca_insert_rule1 = new ECA_Rule(oss.str() + "_ins");    
	eca_insert_rule1->_event = new Parse_Event(rule->head, Parse_Event::RECV);
	eca_insert_rule1->_action = new Parse_Action(rule->head, Parse_Action::ADD);      
	add_rule(eca_insert_rule1);

	ECA_Rule* eca_delete_rule1 = new ECA_Rule(oss.str() + "_del");    
	eca_delete_rule1->_event = new Parse_Event(deleteFunctor, Parse_Event::RECV);
	eca_delete_rule1->_action = new Parse_Action(rule->head, Parse_Action::DELETE);      
	add_rule(eca_delete_rule1);
      }
    }

    // create the other terms
    int count1 = 0;
    std::list<Parse_Term*>::iterator t = rule->terms.begin();
    for(; t != rule->terms.end(); t++) {
      if (count1 != count) { 
	Parse_Term* nextTerm = (*t);    
	Parse_Functor *nextFunctor = dynamic_cast<Parse_Functor*>(nextTerm); 
	Parse_Select *nextSelect = dynamic_cast<Parse_Select*>(nextTerm); 
	Parse_Assign *nextAssign = dynamic_cast<Parse_Assign*>(nextTerm); 
	
	if (nextFunctor != NULL) {
	  eca_insert_rule->_probeTerms.push_back(nextFunctor);
	  eca_delete_rule->_probeTerms.push_back(nextFunctor);
	}
	if (nextSelect != NULL || nextAssign != NULL) {
	  eca_insert_rule->_selectAssignTerms.push_back(nextTerm);
	  eca_delete_rule->_selectAssignTerms.push_back(nextTerm);
	}
      }
      count1++;
    }
    add_rule(eca_insert_rule);
    add_rule(eca_delete_rule);
    count++;
  }
}

void ECA_Context::generateActionHead(OL_Context::Rule* rule, string bodyLoc,
				     TableStore* tableStore, ECA_Rule* eca_rule)
{
  // if event, just send
  string headName = rule->head->fn->name;
  OL_Context::TableInfo* headTableInfo = tableStore->getTableInfo(headName);
  // if this is local, we can simply add or send      
  if (headTableInfo == NULL) {
    eca_rule->_action = new Parse_Action(rule->head, Parse_Action::SEND);
  } else {
    // if need to be materialized, 
    if (rule->head->fn->loc == bodyLoc) {
      eca_rule->_action = new Parse_Action(rule->head, Parse_Action::ADD);
    } else {
      // if the head is remote, we have to do a send, 
      // followed by another recv/add rule strand
      eca_rule->_action = new Parse_Action(rule->head, Parse_Action::SEND);
      string headName = rule->head->fn->name;
      OL_Context::TableInfo* headTableInfo = tableStore->getTableInfo(headName);
      if (headTableInfo != NULL) {
        ostringstream oss;
	oss << rule->ruleID << "_eca" << "_remote";    
	ECA_Rule* eca_rule1 = new ECA_Rule(oss.str());    
	eca_rule1->_event = new Parse_Event(rule->head, Parse_Event::RECV);
	eca_rule1->_action = new Parse_Action(rule->head, Parse_Action::ADD);      
	add_rule(eca_rule1);
      }
    }
  }

}
void ECA_Context::rewriteEventRule(OL_Context::Rule* rule, 
				   TableStore* tableStore)
{
  // figure out which is the event. 
  warn << "Perform ECA rewrite on " << rule->toString() << "\n";

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
      loc = nextFunctor->fn->loc;
      string termName = nextFunctor->fn->name;
      OL_Context::TableInfo* termTableInfo = tableStore->getTableInfo(termName);	  
      if (termTableInfo != NULL) {    
	// this is not an event
	eca_rule->_probeTerms.push_back(nextFunctor);
      } else {
	// an event
	if (termName != "periodic") {
	  eca_rule->_event = new Parse_Event(nextFunctor, Parse_Event::RECV);    
	} else {
	  eca_rule->_event = new Parse_Event(nextFunctor, Parse_Event::INSERT);    
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
  warn << "Perform ECA aggregate view rewrite on " << rule->toString() << "\n";
  if (rule->terms.size() != 1) {
    warn << "Currently only support simple table view aggregates\n";
    exit(-1);
  }

  std::list<Parse_Term*>::iterator t = rule->terms.begin();
  Parse_Term* nextTerm = (*t);  

  Parse_Functor *nextFunctor = dynamic_cast<Parse_Functor*>(nextTerm); 
  if (nextFunctor == NULL) { 
    warn << "Currently only support simple table view aggregates\n";
    exit(-1);
  }
  
  ostringstream oss;
  oss << rule->ruleID << "_eca";    
  ECA_Rule* eca_rule = new ECA_Rule(oss.str());    
  eca_rule->_event = new Parse_Event(nextFunctor, Parse_Event::INSERT);

  // generate the head action
  generateActionHead(rule, nextFunctor->fn->loc, tableStore, eca_rule);
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
      warn << nextRule->toString() << " should have at most one event\n";
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
