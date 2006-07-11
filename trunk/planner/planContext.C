
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
 * DESCRIPTION: Planner code
 *
 */

#include "planContext.h"

PlanContext::PlanContext(Plumber::DataflowPtr conf, TableStore* tableStore, 
			 RuleStrand* ruleStrand, string nodeID, FILE* outputDebugFile) :
  _outputDebugFile(outputDebugFile), _conf(conf)
{
  _tableStore = tableStore;
  _ruleStrand = ruleStrand;
  _nodeID = nodeID;
  _namesTracker = 
    new FieldNamesTracker(ruleStrand->getRule()->_event->_pf);
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

string PlanContext::FieldNamesTracker::toString()
{
  ostringstream toRet;

  toRet << "FieldNamesTracker<";
  
  for (uint k = 0; k < fieldNames.size(); k++) {
    toRet << fieldNames.at(k) << " ";
  }
  toRet << ">";
  return toRet.str();
}

void PlanContext::FieldNamesTracker::joinKeys(FieldNamesTracker* probeNames,
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
