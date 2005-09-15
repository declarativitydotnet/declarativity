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

#include "ruleStrand.h"

str RuleStrand::toString()
{
  strbuf b;
  b << "Rule Strand " << _ruleStrandID << ": " << _eca_rule->toString() << "\n";
  for (unsigned k = 0; k < _elementChain.size(); k++) {
    b << " -> Element " << k << " " << _elementChain.at(k)->toString() << "\n";
  }
  return str(b);
}

void RuleStrand::addElement(Router::ConfigurationRef conf, ElementSpecRef elementSpecRef)
{
  if (_elementChain.size() > 0) {
    conf->hookUp(_elementChain.at(_elementChain.size()-1), 0, elementSpecRef, 0);
  }
  _elementChain.push_back(elementSpecRef);
}
