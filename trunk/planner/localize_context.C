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

#include "localize_context.h"
#include "planContext.h"

string Localize_Context::toString()
{
  ostringstream b;
  for (unsigned k = 0; k < _localizedRules.size(); k++) {
    b << _localizedRules.at(k)->toString() << "\n";
  }
  return b.str();
}

void Localize_Context::add_rule(OL_Context::Rule* rule)
{
  for (unsigned k = 0; k < _localizedRules.size(); k++) {
    if (_localizedRules.at(k)->toString() == rule->toString()) {
      return;
    }
  }

  warn << "  Add localized rule: " << rule->toString() << "\n";  
  _localizedRules.push_back(rule);  
}

OL_Context::Rule* Localize_Context::sendRewrite(OL_Context::Rule* nextRule, 
						Parse_Functor *functor, string loc, 
						TableStore* tableStore)
{

  ostringstream oss;
  std::list<Parse_Term*> newTerms;
  newTerms.push_back(functor);
  Parse_Functor* newHead 
    = new Parse_Functor(
          new Parse_FunctorName(
              new Parse_Var(Val_Str::mk(functor->fn->name + nextRule->ruleID 
					+ loc)), 
      new Parse_Var(Val_Str::mk(loc))), functor->args_);
  OL_Context::Rule* newRule = new OL_Context::Rule(nextRule->ruleID 
						   + "_local1", newHead, false);

  newRule->terms = newTerms;
  warn << "  Localizated rule " << newRule->toString() << "\n";

  // Materialize what we send if the source has been materialized
  OL_Context::TableInfo* tableInfo 
    = tableStore->getTableInfo(functor->fn->name);  
  oss << functor->fn->name << nextRule->ruleID << loc;
  if (tableInfo != NULL) {
    OL_Context::TableInfo* newTableInfo = new OL_Context::TableInfo();
    newTableInfo->tableName = oss.str();
    newTableInfo->size = tableInfo->size;
    newTableInfo->timeout = tableInfo->timeout;
    newTableInfo->primaryKeys = tableInfo->primaryKeys;
    tableStore->addTableInfo(newTableInfo);
    tableStore->createTable(newTableInfo);
  }  
  add_rule(newRule);
  return newRule;
}

void Localize_Context::rewrite(OL_Context* ctxt, TableStore* tableStore)
{
  for (unsigned k = 0; k < ctxt->getRules()->size(); k++) {
    OL_Context::Rule* nextRule = ctxt->getRules()->at(k);
    rewriteRule(nextRule, tableStore);
  }
}

void Localize_Context::rewriteRule(OL_Context::Rule* nextRule, 
				   TableStore* tableStore)
{
  warn << "Perform localization rewrite on " << nextRule->toString() 
	    << "\n";

  std::vector<OL_Context::Rule*> toRet;
  std::vector<Parse_Functor*> probeTerms;
  std::vector<Parse_Term*> otherTerms;

  // separate out the probeTerms and other terms
  std::list<Parse_Term*>::iterator t = nextRule->terms.begin();
  for(; t != nextRule->terms.end(); t++) {
    Parse_Term* nextTerm = (*t);    
    Parse_Functor *functor = dynamic_cast<Parse_Functor*>(nextTerm);
    if (functor != NULL) {
      probeTerms.push_back(functor);
    } else {
      otherTerms.push_back(nextTerm);
    }
  }

  // go through all the probe terms,
  // do a left to right join ordering transformation
  PlanContext::FieldNamesTracker* namesTracker 
    = new PlanContext::FieldNamesTracker();
  // XXX: Special case works for only 2 predicates. To fix and use recursive method
  if (probeTerms.size() == 2 && 
      probeTerms.at(0)->fn->loc != probeTerms.at(1)->fn->loc) {
    // form a rule sending first term to location of second
    OL_Context::Rule* newRule = sendRewrite(nextRule, probeTerms.at(0), 
					    probeTerms.at(1)->fn->loc, 
					    tableStore);
    
    std::list<Parse_Term*> newTerms; 
    newTerms.push_back(newRule->head);
    newTerms.push_back(probeTerms.at(1));
    for (unsigned k = 0; k < otherTerms.size(); k++) {
      newTerms.push_back(otherTerms.at(k));
    }
    OL_Context::Rule* newRuleTwo 
      = new OL_Context::Rule(nextRule->ruleID + "_local2", 
			     nextRule->head, false);

    std::list<Parse_Term*>::iterator t = nextRule->terms.begin();

    newRuleTwo->terms = newTerms;
    add_rule(newRuleTwo);
  } else {
    add_rule(nextRule);
  }
  
  delete namesTracker;
}
