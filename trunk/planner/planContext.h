

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

#ifndef __PL_PLANCONTEXT_H__
#define __PL_PLANCONTEXT_H__

#include "catalog.h"
#include "ol_context.h"
#include "ruleStrand.h"
#include "router.h"

class PlanContext {

public:
  PlanContext(Router::ConfigurationPtr conf, Catalog* catalog, 
	      RuleStrand* ruleStrand, string nodeID, FILE* outputDebugFile);
  ~PlanContext();
  Catalog* _catalog;
  RuleStrand* _ruleStrand;
  string _nodeID;
  FILE* _outputDebugFile;
  Router::ConfigurationPtr _conf;

  // convince placeholder to figure out the cur fields in a tuple in flight
  class FieldNamesTracker {
  public:
    std::vector<string> fieldNames;    
    FieldNamesTracker();   
    FieldNamesTracker(Parse_Term* pf);

    void initialize(Parse_Term* pf);
    std::vector<int> matchingJoinKeys(std::vector<string> names);    
    void mergeWith(std::vector<string> names);
    void mergeWith(std::vector<string> names, int numJoinKeys);
    int fieldPosition(string var);
    string toString();
  };

  FieldNamesTracker* _namesTracker;
  
};

#endif
