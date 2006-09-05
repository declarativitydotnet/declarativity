
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
  RuleStrand(ECA_Rule* rule, string strandID) :
    _eca_rule(rule)
  { _ruleID = rule->_ruleID; _strandID = strandID; 
  _aggWrapFlag = false; }; 

  string toString();

  string eventFunctorName(); 

  string actionFunctorName(); 
  
  Parse_Event::Event eventType() 
  { return _eca_rule->_event->_event; }

  Parse_Action::Action actionType() 
  { return _eca_rule->_action->_action; }

  ElementSpecPtr getEventElement() 
  { return _elementChain.at(0); }

  ElementSpecPtr getFirstElement()
  { 
    if (_aggWrapFlag == false) {
      return getEventElement(); 
    }
    return _aggWrapperSpec;
  }

  ElementSpecPtr getActionElement() 
  { return _elementChain.at(_elementChain.size() - 1); }

  void aggWrapperElement(Plumber::DataflowPtr conf, ElementSpecPtr aggWrapperSpec);

  void addElement(Plumber::DataflowPtr conf, ElementSpecPtr elementSpecPtr);

  string getRuleID() { return _ruleID; }
  string getStrandID() { return _strandID; }
  ECA_Rule* getRule() { return _eca_rule; }

private:
  std::vector<ElementSpecPtr> _elementChain;
  bool _aggWrapFlag;
  ElementSpecPtr _aggWrapperSpec; // special agg wrap
  string _ruleID; // original source rule ID
  string _strandID; // unique ID given after rewrite
  ECA_Rule* _eca_rule;  

};

#endif

