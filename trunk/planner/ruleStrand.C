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

string RuleStrand::toString()
{
  ostringstream b;
  b << "Rule Strand " << _strandID << ": " << _eca_rule->toString() << "\n";
  for (unsigned k = 0; k < _elementChain.size(); k++) {
    b << " -> Element " << k << " " << _elementChain.at(k)->toString() << "\n";
  }
  return b.str();
}

void RuleStrand::addElement(Plumber::DataflowPtr conf, ElementSpecPtr elementSpecPtr)
{
  if (_elementChain.size() > 0) {
    conf->hookUp(_elementChain.at(_elementChain.size()-1), 0, elementSpecPtr, 0);
  }
  _elementChain.push_back(elementSpecPtr);
}
