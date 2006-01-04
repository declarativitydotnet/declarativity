
/*
 * @(#)$Id$
 *
 * Copyright (c) 2005 Intel Corporation. All rights reserved.
 *
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 * DESCRIPTION: Planner code
 *
 */

#include "planContext.h"

PlanContext::PlanContext(Router::ConfigurationPtr conf, Catalog* catalog, 
			 RuleStrand* ruleStrand, string nodeID, FILE* outputDebugFile) :
  _outputDebugFile(outputDebugFile), _conf(conf)
{
  _catalog = catalog;
  _ruleStrand = ruleStrand;
  _nodeID = nodeID;
  _namesTracker = 
    new FieldNamesTracker(ruleStrand->_eca_rule->_event->_pf);
}

PlanContext::~PlanContext()
{
  delete _namesTracker;
}

PlanContext::FieldNamesTracker::FieldNamesTracker() { }

PlanContext::FieldNamesTracker::FieldNamesTracker(Parse_Term* pf)
{
  initialize(pf);
}


void 
PlanContext::FieldNamesTracker::initialize(Parse_Term* term)
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
PlanContext::FieldNamesTracker::matchingJoinKeys(std::vector<string> 
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
PlanContext::FieldNamesTracker::fieldPosition(string var)
{
  for (uint k = 0; k < fieldNames.size(); k++) {
    if (fieldNames.at(k) == var) {
      return k;
    }
  }
  return -1;
}

void 
PlanContext::FieldNamesTracker::mergeWith(std::vector<string> names)
{
  for (uint k = 0; k < names.size(); k++) {
    string nextStr = names.at(k);
    if (fieldPosition(nextStr) == -1) {
      fieldNames.push_back(nextStr);
    }
  }
}

void 
PlanContext::FieldNamesTracker::mergeWith(std::vector<string> names, 
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

string 
PlanContext::FieldNamesTracker::toString()
{
  ostringstream toRet("FieldNamesTracker<");
  
  for (uint k = 0; k < fieldNames.size(); k++) {
    toRet << fieldNames.at(k) << " ";
  }
  return toRet.str() + ">";
}

