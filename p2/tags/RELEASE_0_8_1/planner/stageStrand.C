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

#include <stageStrand.h>
#include "planner.h"


StageStrand::StageStrand(const OL_Context::ExtStageSpec* stageSpec,
                         string strandID)
{
  _strandID = strandID; 
  _stageSpec = stageSpec;
}


ElementSpecPtr
StageStrand::getFirstElement()
{ 
  if (_elementChain.size() > 0) {
    return _elementChain.at(0);
  } else {
    return ElementSpecPtr();
  }
}


ElementSpecPtr
StageStrand::getLastElement()
{ 
  if (_elementChain.size() > 0) {
    return _elementChain.at(_elementChain.size() - 1);
  } else {
    return ElementSpecPtr();
  }
}


string
StageStrand::getStrandID()
{
  return _strandID;
}


string
StageStrand::inputName()
{
  return _stageSpec->inputTupleName;
}


string
StageStrand::toString()
{
  ostringstream b;
  b << "Stage Strand "
    << _strandID
    << ": "
    << _stageSpec->inputTupleName
    << "("
    << _stageSpec->stageName
    << ")"
    << _stageSpec->outputTupleName
    << "\n";

  for (unsigned k = 0;
       k < _elementChain.size();
       k++) {
    b << " -> Element "
      << k
      << " "
      << _elementChain.at(k)->toString()
      << "\n";
  }

  return b.str();
}


void
StageStrand::addElement(Plumber::DataflowPtr conf,
                        ElementPtr elementPtr)
{
  ElementSpecPtr elementSpecPtr =
    conf->addElement(elementPtr);
  if (_elementChain.size() > 0) {
    conf->hookUp(_elementChain.at(_elementChain.size() - 1), 0,
                 elementSpecPtr, 0);
  }
  
  _elementChain.push_back(elementSpecPtr);
}

