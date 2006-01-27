
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
 * DESCRIPTION: Rule strand
 *
 */

#ifndef __PL_RULESTRAND_H__
#define __PL_RULESTRAND_H__


#include "eca_context.h"
#include "elementSpec.h"
#include "element.h"
#include "plumber.h"

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

  void addElement(Plumber::ConfigurationPtr conf, ElementSpecPtr elementSpecPtr);

private:
  std::vector<ElementSpecPtr> _elementChain;
};

#endif

