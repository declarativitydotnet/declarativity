
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

string Parse_Event::toString()
{
  ostringstream b;
  if (_event == RECV) { b << "EVENT_RECV<" << _pf->toString() << ">"; }
  if (_event == PERIODIC) { b << "EVENT_PERIODIC<" << _pf->toString() << ">"; }
  if (_event == UPDATE) { b << "EVENT_UPDATE<" << _pf->toString() << ">"; }
  if (_event == AGGUPDATE) { b << "EVENT_AGGUPDATE<" << _pf->toString() << ">"; }
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
  
  warn << eca_rule->toString() << "\n";
  _ecaRules.push_back(eca_rule);  
}

void ECA_Context::activateLocalizedRule(OL_Context::Rule* rule, Catalog* catalog)
{
  // check if it is an aggTerm rule
  Parse_AggTerm *aggTerm = dynamic_cast<Parse_AggTerm*>(rule->terms.at(0)); 
  if (rule->terms.size() == 1 && aggTerm != NULL) {
    ECA_Rule* eca_rule = new ECA_Rule(rule->ruleID);    
    Parse_Functor *baseFunctor = dynamic_cast<Parse_Functor*>(aggTerm->_baseTerm); 
    eca_rule->_event = new Parse_Event(baseFunctor, Parse_Event::AGGUPDATE);
    eca_rule->_action = new Parse_Action(rule->head, Parse_Action::ADD);
    eca_rule->_aggTerm = aggTerm; 
    add_rule(eca_rule);
    return;
  }
  
  // activate each body functor.
  // a. recv if not materialized
  // b. update if materialized
  for (unsigned k = 0; k < rule->terms.size(); k++) {
    Parse_Term* nextTerm = rule->terms.at(k);
    Parse_Functor *nextFunctor = dynamic_cast<Parse_Functor*>(nextTerm); 
    if (nextFunctor == NULL) { continue; }

    // create an event
    ostringstream oss;
    oss << rule->ruleID << "-" << k;    
    ECA_Rule* eca_rule = new ECA_Rule(oss.str());    

    // if event is not materialized, do a recv, otherwise, update
    string functorName = nextFunctor->fn->name;
    Catalog::TableInfo* functorTableInfo = catalog->getTableInfo(functorName);
    if (functorTableInfo == NULL ||
	(functorTableInfo->_tableInfo->timeout == 0)) {
      eca_rule->_event = new Parse_Event(nextFunctor, Parse_Event::RECV);
    } else {
      eca_rule->_event = new Parse_Event(nextFunctor, Parse_Event::UPDATE);
    }

    // if head is remote, do a send, and a recv 
    if (rule->head->fn->loc == nextFunctor->fn->loc) {
      eca_rule->_action = new Parse_Action(rule->head, Parse_Action::ADD);
    } else {
      eca_rule->_action = new Parse_Action(rule->head, Parse_Action::SEND);
      // check to see if materialize, if so, create one more rule
      string headName = rule->head->fn->name;
      Catalog::TableInfo* headTableInfo = catalog->getTableInfo(headName);
      if (headTableInfo != NULL &&
	  (headTableInfo->_tableInfo->timeout != 0)) {
        ostringstream oss;
	oss << rule->ruleID << "-" << k << "-" << 1;    
	ECA_Rule* eca_rule1 = new ECA_Rule(oss.str());    
	eca_rule1->_event = new Parse_Event(rule->head, Parse_Event::RECV);
	eca_rule1->_action = new Parse_Action(rule->head, Parse_Action::ADD);      
	add_rule(eca_rule1);
      }
    }

    // create the other terms
    for (unsigned i = 0; i < rule->terms.size(); i++) {
      if (i == k) { continue; }
      Parse_Term* nextTerm = rule->terms.at(i);
      Parse_Functor *nextFunctor = dynamic_cast<Parse_Functor*>(nextTerm); 
      Parse_Select *nextSelect = dynamic_cast<Parse_Select*>(nextTerm); 
      Parse_Assign *nextAssign = dynamic_cast<Parse_Assign*>(nextTerm); 
      
      if (nextFunctor != NULL) {
	eca_rule->_probeTerms.push_back(nextFunctor);
      }
      if (nextSelect != NULL || nextAssign != NULL) {
	eca_rule->_selectAssignTerms.push_back(nextTerm);
      }
     }
    add_rule(eca_rule);
  }
}

OL_Context::Rule* generateSendRule(OL_Context::Rule* nextRule, 
				   Parse_Functor *functor, string loc, 
				   Catalog* catalog)
{
  ostringstream oss;
  std::vector<Parse_Term*> newTerms;
  newTerms.push_back(functor);
  Parse_Functor* newHead 
    = new Parse_Functor(
          new Parse_FunctorName(
              new Parse_Var(Val_Str::mk(functor->fn->name + nextRule->ruleID + loc)), 
      new Parse_Var(Val_Str::mk(loc))), functor->args_);
  OL_Context::Rule* newRule = new OL_Context::Rule(nextRule->ruleID + "-1", newHead, false);
  newRule->terms = newTerms;
  warn << "Generate send rule " << newRule->toString() << "\n";

  Catalog::TableInfo* ti = catalog->getTableInfo(functor->fn->name);  
  OL_Context::TableInfo* cti = new OL_Context::TableInfo();
  *cti = *(ti->_tableInfo);
  oss << functor->fn->name << nextRule->ruleID << loc;
  cti->tableName = oss.str();
  if (ti != NULL) {
    catalog->createTable(cti);
  }
  
  return newRule;
}

std::vector<OL_Context::Rule*> 
ECA_Context::localizeRule(OL_Context::Rule* nextRule, Catalog* catalog)
{
  std::vector<OL_Context::Rule*> toRet;
  Parse_AggTerm *aggTerm = dynamic_cast<Parse_AggTerm*>(nextRule->terms.at(0)); 
  if (nextRule->terms.size() == 1 && aggTerm != NULL) {
    // special aggregate view action rule
    toRet.push_back(nextRule);
    return toRet;
  }

  std::vector<Parse_Functor*> probeTerms;
  std::vector<Parse_Term*> otherTerms;
  // separate out the probeTerms and other terms
  for (unsigned k = 0; k < nextRule->terms.size(); k++) {
    Parse_Functor *functor = dynamic_cast<Parse_Functor*>(nextRule->terms.at(k));
    if (functor != NULL) {
      probeTerms.push_back(functor);
    } else {
      otherTerms.push_back(nextRule->terms.at(k));
    }
  }

  // go through all the probe terms,
  // do a left to right join ordering transformation
  PlanContext::FieldNamesTracker* namesTracker = new PlanContext::FieldNamesTracker();
  if (probeTerms.size() == 2 && 
      probeTerms.at(0)->fn->loc != probeTerms.at(1)->fn->loc) {
    // form a rule sending first term to location of second
    OL_Context::Rule* newRule = generateSendRule(nextRule, probeTerms.at(0), 
						 probeTerms.at(1)->fn->loc, 
						 catalog);
    toRet.push_back(newRule);
    
    std::vector<Parse_Term*> newTerms;
    newTerms.push_back(newRule->head);
    newTerms.push_back(probeTerms.at(1));
    for (unsigned k = 0; k < otherTerms.size(); k++) {
      newTerms.push_back(otherTerms.at(k));
    }
    OL_Context::Rule* newRuleTwo = new OL_Context::Rule(nextRule->ruleID + "-2", 
							nextRule->head, false);
    newRuleTwo->terms = newTerms;
    warn << "New rule " << newRuleTwo->toString() << "\n";
    toRet.push_back(newRuleTwo);

  } else {
    toRet.push_back(nextRule);
  }
  
  delete namesTracker;
  return toRet;
}

void ECA_Context::eca_rewrite(OL_Context* ctxt, Catalog* catalog)
{
  for (unsigned k = 0; k < ctxt->getRules()->size(); k++) {
    OL_Context::Rule* nextRule = ctxt->getRules()->at(k);
    // 1. Localize each rule, may return multiple rules
    
    std::vector<OL_Context::Rule*> localRules 
      = localizeRule(nextRule, catalog);

    // 2. For each localized rule, activate the rule
    for (unsigned i = 0; i < localRules.size(); i++) {
      activateLocalizedRule(localRules.at(i), catalog);
    }
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
