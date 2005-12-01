

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
  PlanContext(Router::ConfigurationRef conf, Catalog* catalog, 
	      RuleStrand* ruleStrand, str nodeID, 
	      FILE* outputDebugFile);
  ~PlanContext();
  Catalog* _catalog;
  RuleStrand* _ruleStrand;
  str _nodeID;
  FILE* _outputDebugFile;
  Router::ConfigurationRef _conf;

  // convince placeholder to figure out the cur fields in a tuple in flight
  class FieldNamesTracker {
  public:
    std::vector<str> fieldNames;    
    FieldNamesTracker();   
    FieldNamesTracker(Parse_Term* pf);

    void initialize(Parse_Term* pf);
    std::vector<int> matchingJoinKeys(std::vector<str> names);    
    void mergeWith(std::vector<str> names);
    void mergeWith(std::vector<str> names, int numJoinKeys);
    int fieldPosition(str var);
    str toString();
  };

  FieldNamesTracker* _namesTracker;
  
};

#endif
