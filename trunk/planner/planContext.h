/*
 * @(#)$Id$
 *
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * Or
 * UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
 * Berkeley, CA,  94707. Attention: P2 Group.
 * 
 * DESCRIPTION: Planning environment for planner
 *
 */

#ifndef __PL_PLANCONTEXT_H__
#define __PL_PLANCONTEXT_H__

#include "tableStore.h"
#include "ol_context.h"
#include "ruleStrand.h"
#include "plumber.h"

/* Stores all information on planning state */

class PlanContext {
public:
  PlanContext(Plumber::DataflowPtr conf, TableStore* tableStore, 
	      RuleStrand* ruleStrand, string nodeID, FILE* outputDebugFile);
  ~PlanContext();

  /** References to all tables */
  TableStore* _tableStore;

  /** Current strand being planned */
  RuleStrand* _ruleStrand;

  /** ID of node where planner is running */
  string _nodeID;

  /** File log output of planner */
  FILE* _outputDebugFile;
  
  /** Dataflow configuration */
  Plumber::DataflowPtr _conf;

  ECA_Rule* getRule() { return _ruleStrand->getRule(); }

  ElementSpecPtr createElementSpec(ElementPtr element) {
    return _conf->addElement(element);
  }
  
  void addElementSpec(ElementSpecPtr elementSpec) {
    _ruleStrand->addElement(_conf, elementSpec);
    }

  class FieldNamesTracker {
  public:
    std::vector<string> fieldNames;    
    FieldNamesTracker();   
    FieldNamesTracker(Parse_Term* pf);

    void initialize(Parse_Term* pf);
    std::vector<int> matchingJoinKeys(std::vector<string> names);    
    void mergeWith(std::vector<string> names);
    void mergeWith(std::vector<string> names, int numJoinKeys);
    void FieldNamesTracker::joinKeys(FieldNamesTracker* probeNames,
				     Table2::Key& lookupKey,
				     Table2::Key& indexKey,
				     Table2::Key& remainingBaseKey);

    int fieldPosition(string var);
    string toString();
  };

  FieldNamesTracker* _namesTracker;
  
};

#endif
