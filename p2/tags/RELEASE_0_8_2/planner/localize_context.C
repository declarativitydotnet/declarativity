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

#include "planner.h"
#include "localize_context.h"
#include "planContext.h"
#include <set>

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

  PLANNER_INFO_NOPC("Add localized rule: " << rule->toString());
  _localizedRules.push_back(rule);  
}


OL_Context::Rule* 
Localize_Context::addSendRule(OL_Context::Rule* nextRule,
			      std::list<Parse_Term*> newTerms,
			      string newFunctorName,
			      Parse_Functor* functor,
			      string loc, 
			      boost::posix_time::time_duration minLifetime,
			      std::vector<string> fieldNames,
			      TableStore* tableStore)
{
  Parse_ExprList* pe = new Parse_ExprList();
  for (uint k = 0; k < fieldNames.size(); k++) {

    std::string varName = fieldNames.at(k);
    Parse_Var* pv = new Parse_Var(Val_Str::mk(varName));
    if (fieldNameEq(loc, varName)) {
      // The new variable should be a locspec
      pv->setLocspec();
    }
    pe->push_back(pv);
  }
  
  Parse_Functor* newHead 
    = new Parse_Functor(new Parse_FunctorName
                        (new Parse_Var(Val_Str::
                                       mk(newFunctorName))), pe);
  OL_Context::Rule* newRule = new OL_Context::Rule(nextRule->ruleID 
						   + "Local1",
                                                   newHead, false);

  newRule->terms = newTerms;
  PLANNER_INFO_NOPC("Localized send rule " << newRule->toString());

  // Materialize what we send if the source has been materialized
  OL_Context::TableInfo* tableInfo 
    = tableStore->getTableInfo(functor->fn->name);  

  boost::posix_time::time_duration zeroLifetime =
    boost::posix_time::seconds(0);

  if (tableInfo != NULL && minLifetime != zeroLifetime) {
    OL_Context::TableInfo* newTableInfo = new OL_Context::TableInfo();
    newTableInfo->tableName = newFunctorName;
    newTableInfo->size = tableInfo->size; // XXX depends on size of first table
    newTableInfo->timeout = minLifetime;
    newTableInfo->primaryKeys = tableInfo->primaryKeys; // XXX depends on first table
    tableStore->addTableInfo(newTableInfo);
    PLANNER_INFO_NOPC("Old table " << tableInfo->toString());
    PLANNER_INFO_NOPC(" Create table for " << newTableInfo->toString());
    tableStore->createTable(newTableInfo);
  }  
  return newRule;
}


void
Localize_Context::rewrite(OL_Context* ctxt,
                          TableStore* tableStore)
{
  for (unsigned k = 0;
       k < ctxt->getRules()->size();
       k++) {
    OL_Context::Rule* nextRule = ctxt->getRules()->at(k);
    rewriteRule(nextRule, tableStore);
  }
}


void
Localize_Context::rewriteRule(OL_Context::Rule* nextRule, 
                              TableStore* tableStore)
{
  PLANNER_WORDY_NOPC("Perform localization rewrite on "
                     << nextRule->toString());

  std::vector<OL_Context::Rule*> toRet;
  std::vector<Parse_Functor*> probeTerms;
  std::vector<Parse_Term*> otherTerms;
  bool hasEvent = false;        // Does this rule have an event?
  std::set< string, std::less<string> > probeLocales; // The locspecs of
                                                      // materialized
                                                      // tables in the
                                                      // rule body?

  // separate out the probeTerms and other terms
  for(std::list<Parse_Term*>::iterator t = nextRule->terms.begin();
      t != nextRule->terms.end();
      t++) {
    Parse_Term* nextTerm = (*t);    
    Parse_Functor *functor = dynamic_cast<Parse_Functor*>(nextTerm);
    if (functor != NULL) {
      // Is this materialized?
      OL_Context::TableInfo* tableInfo 
	= tableStore->getTableInfo(functor->fn->name);        
      if (tableInfo != NULL) {
        // This is materialized
	probeTerms.push_back(functor);

        // It's locspec is
        string locspec = functor->getlocspec();
        probeLocales.insert(locspec);
      } else {
	// put events first
	PLANNER_WORDY_NOPC("Put to front " << functor->fn->name);
	probeTerms.insert(probeTerms.begin(), functor);
        hasEvent = true;
      }
    } else {
      otherTerms.push_back(nextTerm);
    }
  }

  // go through all the probe terms,
  // do a left to right join ordering transformation
  bool local = true;
  ostringstream headName;
  headName << nextRule->ruleID;
  
  boost::posix_time::time_duration
    minLifetime(boost::date_time::pos_infin);
  boost::posix_time::time_duration zeroLifetime = 
    boost::posix_time::seconds(0);

  uint boundary = 0;

  std::list<Parse_Term*> beforeBoundaryTerms;
  PlanContext::FieldNamesTracker* namesTracker = NULL;
  for (uint k = 0; k < probeTerms.size()-1; k++) {
    if (k == 0) {
      namesTracker = new PlanContext::FieldNamesTracker(probeTerms.at(k));
    } else {
      PlanContext::FieldNamesTracker* otherNamesTracker = 
	new PlanContext::FieldNamesTracker(probeTerms.at(k));
      namesTracker->mergeWith(otherNamesTracker->fieldNames);
    }
    headName << probeTerms.at(k)->fn->name;
    OL_Context::TableInfo* tableInfo 
      = tableStore->getTableInfo(probeTerms.at(k)->fn->name);  
    PLANNER_WORDY_NOPC("Get table " << probeTerms.at(k)->fn->name);
    if (tableInfo != NULL) {
      if (minLifetime > tableInfo->timeout) {
	minLifetime = tableInfo->timeout;
      }
    } else {
      minLifetime = zeroLifetime;
    }      
    beforeBoundaryTerms.push_back(probeTerms.at(k));
    if (!fieldNameEq(probeTerms.at(k)->getlocspec(),
                     probeTerms.at(k+1)->getlocspec())) {
      // getlocspec is guaranteed to start with @ sign
      headName << probeTerms.at(k+1)->getlocspec().substr(1);
      local = false;
      boundary = k;      
      break;
    }
  }

  if (local == true) {
    PLANNER_INFO_NOPC(nextRule->toString() << " is already localized");
    add_rule(nextRule);
    delete namesTracker;
    return;
  } else if (hasEvent &&        // it's not a view
             probeLocales.size() > 1 && // the materialized tables are
                                        // on multiple locations
             nextRule->head->aggregate() != -1) { // and it's an aggregate
    // This rule is an non-local event aggregate. We don't handle these currently
    PLANNER_ERROR_NOPC("Rule '"
                       << nextRule->toString()
                       << "' is a non-local streaming aggregate, which "
                       << "is not currently supported.");
    exit(-1);
  }

  PLANNER_WORDY_NOPC(headName.str() << " minimumLifetime " 
                     << boost::posix_time::to_simple_string(minLifetime)
                     << " boundaryIndex " 
                     << boundary << " " << namesTracker->toString());


  // add a new rule that takes all terms up to boundary, and send them to dst
  OL_Context::Rule* newRule =
    addSendRule(nextRule, beforeBoundaryTerms,
                headName.str(), probeTerms.at(0),
                probeTerms.at(boundary + 1)->getlocspec(), 
                minLifetime, namesTracker->fieldNames, tableStore);
  add_rule(newRule);

  // recursively call localization on new rule that has
  std::list<Parse_Term*> newTerms; 
  newTerms.push_back(newRule->head);
  for (unsigned k = boundary+1; k < probeTerms.size(); k++) {
    newTerms.push_back(probeTerms.at(k));
  }
  for (unsigned k = 0; k < otherTerms.size(); k++) {
    newTerms.push_back(otherTerms.at(k));
  }

  OL_Context::Rule* newRuleTwo 
    = new OL_Context::Rule(nextRule->ruleID + "Local2", 
			   nextRule->head, false);
  
  newRuleTwo->terms = newTerms;
  rewriteRule(newRuleTwo, tableStore);
}

