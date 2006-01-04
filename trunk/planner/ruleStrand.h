
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
 * DESCRIPTION: Rule strand
 *
 */

#ifndef __PL_RULESTRAND_H__
#define __PL_RULESTRAND_H__


#include "eca_context.h"
#include "elementSpec.h"
#include "element.h"
#include "router.h"

class RuleStrand
{
public:  
  RuleStrand(ECA_Rule* rule, string ruleStrandID) :
    _eca_rule(rule) 
  { _ruleID = rule->_ruleID; _ruleStrandID = ruleStrandID; }; 

  string _ruleID; // original rule ID
  string _ruleStrandID; // number, issued by planner
  ECA_Rule* _eca_rule; 
 
  string toString();
  
  Parse_Event::Event eventType() 
  { return _eca_rule->_event->_event; }

  Parse_Action::Action actionType() 
  { return _eca_rule->_action->_action; }

  string eventFunctorName() 
  { return _eca_rule->_event->_pf->fn->name; }

  string actionFunctorName() 
  { return _eca_rule->_action->_pf->fn->name; }

  ElementSpecPtr getEventElement() 
  { return _elementChain.at(0); }

  ElementSpecPtr getActionElement() 
  { return _elementChain.at(_elementChain.size() - 1); }

  void addElement(Router::ConfigurationPtr conf, ElementSpecPtr elementSpecPtr);

private:
  std::vector<ElementSpecPtr> _elementChain;
};

#endif

