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
 * DESCRIPTION: Stage strand
 *
 */

#ifndef __PL_STAGESTRAND_H__
#define __PL_STAGESTRAND_H__

#include "ol_context.h"
#include "elementSpec.h"
#include "element.h"
#include "plumber.h"

class StageStrand
{
public:  
  StageStrand(const OL_Context::ExtStageSpec* stageSpec,
              string strandID);
  

  string
  toString();


  ElementSpecPtr
  getFirstElement();


  ElementSpecPtr
  getLastElement();


  void
  addElement(Plumber::DataflowPtr conf,
             ElementPtr elementSpecPtr);

  
  string
  inputName();


  string
  getStrandID();


private:
  std::vector<ElementSpecPtr> _elementChain;


  /** Unique ID given after a rewrite */
  string _strandID;


  const OL_Context::ExtStageSpec* _stageSpec;  
};

#endif

