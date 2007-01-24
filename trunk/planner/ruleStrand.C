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
 * DESCRIPTION: Rule strand
 *
 */

#include "ruleStrand.h"
#include "slot.h"
#include "timedPullPush.h"
#include "planner.h"

string
RuleStrand::eventFunctorName() 
{ 
  return _eca_rule->_event->_pf->fn->name;  
}


string
RuleStrand::actionFunctorName() 
{ 
  return _eca_rule->_action->_pf->fn->name; 
}


string
RuleStrand::toString()
{
  ostringstream b;
  b << "Rule Strand " << _strandID << ": " << _eca_rule->toString() << "\n";
  for (unsigned k = 0; k < _elementChain.size(); k++) {
    b << " -> Element " << k << " " << _elementChain.at(k)->toString() << "\n";
  }
  if (_aggWrapFlag == true) {
    b << " -> AggWrapElement " << " " << _aggWrapperSpec->toString() << "\n";
  }
  return b.str();
}


void
RuleStrand::addElement(Plumber::DataflowPtr conf, ElementSpecPtr elementSpecPtr)
{
  if (_elementChain.size() > 0) {
    conf->hookUp(_elementChain.at(_elementChain.size()-1), 0, elementSpecPtr, 0);
  }
  _elementChain.push_back(elementSpecPtr);
}


void
RuleStrand::aggWrapperElement(Plumber::DataflowPtr conf,
                              ElementSpecPtr aggWrapperSpec)
{
  _aggWrapperSpec = aggWrapperSpec;

  if (_elementChain.size() == 0) {
    PLANNER_WARN_NOPC("Rule strand "
                      << _strandID 
                      << " cannot have an agg wrap over an empty strand");
    exit(-1);
  }

  ElementSpecPtr pullPush =
    conf->addElement(ElementPtr(new TimedPullPush("AggWrapPullPush!" 
						  + _eca_rule->_ruleID, 0)));
  addElement(conf, pullPush);
  
  // push from aggwrap into start of strand
  conf->hookUp(aggWrapperSpec, 1, _elementChain.at(0), 0);

  // connect from last element of strand to aggwrap input
  conf->hookUp(_elementChain.at(_elementChain.size()-1), 0, aggWrapperSpec, 1);

  // result from aggwrap goes to external output
  ElementSpecPtr aggWrapSlot 
    = conf->addElement(ElementPtr(new Slot("aggWrapSlot!" + _strandID )));
  conf->hookUp(aggWrapperSpec, 0, aggWrapSlot, 0);

  _elementChain.push_back(aggWrapSlot); // connect this to end of strand

  _aggWrapFlag = true;
}
