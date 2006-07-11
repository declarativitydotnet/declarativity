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
  if (_aggTerm != NULL) {
    b << _aggTerm->toString();
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
  
  std::cout << "  Add rule: " << eca_rule->toString() << "\n";
  _ecaRules.push_back(eca_rule);  
}

void ECA_Context::rewriteViewRule(OL_Context::Rule* rule, TableStore* tableStore)
{
  std::cout << "Perform ECA rewrite on " << rule->toString() << "\n";

  std::list<Parse_Term*>::iterator t = rule->terms.begin();
  int count = 0;
  for(; t != rule->terms.end(); t++) {
    Parse_Term* nextTerm = (*t);
    Parse_Functor *nextFunctor = dynamic_cast<Parse_Functor*>(nextTerm); 
    if (nextFunctor == NULL) { continue; }

    // create an event
    ostringstream oss;
    oss << rule->ruleID << "-eca" << count;    
    ECA_Rule* eca_insert_rule = new ECA_Rule(oss.str() + "-ins");    
    ECA_Rule* eca_delete_rule = new ECA_Rule(oss.str() + "-del");    

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
      // if this is local, we can simply add local table
      eca_insert_rule->_action = new Parse_Action(rule->head, Parse_Action::ADD);
      eca_delete_rule->_action = new Parse_Action(rule->head, Parse_Action::DELETE);
    } else {
      // if the head is remote, we have to do a send, 
      // followed by another recv/add rule strand
      eca_insert_rule->_action = new Parse_Action(rule->head, Parse_Action::SEND);
      eca_delete_rule->_action = new Parse_Action(deleteFunctor, Parse_Action::SEND);
      string headName = rule->head->fn->name;
      OL_Context::TableInfo* headTableInfo = tableStore->getTableInfo(headName);
      if (headTableInfo != NULL) {
        ostringstream oss;
	oss << rule->ruleID << "-eca" << count << "-remote";    
	ECA_Rule* eca_insert_rule1 = new ECA_Rule(oss.str() + "-ins");    
	eca_insert_rule1->_event = new Parse_Event(rule->head, Parse_Event::RECV);
	eca_insert_rule1->_action = new Parse_Action(rule->head, Parse_Action::ADD);      
	add_rule(eca_insert_rule1);

	ECA_Rule* eca_delete_rule1 = new ECA_Rule(oss.str() + "-del");    
	eca_delete_rule1->_event = new Parse_Event(deleteFunctor, Parse_Event::RECV);
	eca_delete_rule1->_action = new Parse_Action(rule->head, Parse_Action::DELETE);      
	add_rule(eca_delete_rule1);
      }
    }

    // create the other terms
    int count1 = 0;
    std::list<Parse_Term*>::iterator t = rule->terms.begin();
    for(; t != rule->terms.end(); t++) {
      if (count1 == count) { continue; }
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
      count1++;
    }
    add_rule(eca_insert_rule);
    add_rule(eca_delete_rule);
    count++;
  }
}

void ECA_Context::rewriteEventRule(OL_Context::Rule* rule, 
				   TableStore* tableStore)
{
}

void ECA_Context::rewrite(Localize_Context* lctxt, TableStore* tableStore)
{
  for (unsigned k = 0; k < lctxt->getRules().size(); k++) {
    OL_Context::Rule* nextRule = lctxt->getRules().at(k);
    rewriteViewRule(nextRule, tableStore);
  }

    // To check:
    // 1) At most one event in rule
    // 2) Rules have been localized

    // localize each overlog rule
    /*(std::vector<OL_Context::Rule*> localRules 
      = localizeRewrite(nextRule, tableStore);
    */

    // 2. For each set of localized rules, translate into ECA
    //for (unsigned i = 0; i < localRules.size(); i++) {

    //}

    // 3. For all localized ECA rules, translate into ECA context trivally
}

string ECA_Context::toString()
{
  ostringstream b;
  for (unsigned k = 0; k < _ecaRules.size(); k++) {
    b << _ecaRules.at(k)->toString() << "\n";
  }
  return b.str();
}
