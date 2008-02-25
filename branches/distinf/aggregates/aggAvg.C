/*
 * @(#)$Id: aggAvg.C 1243 2007-07-16 19:05:00Z maniatis $
 *
 * Copyright (c) 2005 Intel Corporation. All rights reserved.
 *
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 */

#include "aggAvg.h"
#include "val_null.h"
#include "aggFactory.h"

AggAvg::AggAvg()
{
}


AggAvg::~AggAvg()
{
}

  
void
AggAvg::reset()
{
  _currentSum = 0;
  _count = 0;
}
  

void
AggAvg::first(ValuePtr v)
{
  _currentSum = Val_Double::cast(v);
  _count = 1;
}
  

void
AggAvg::process(ValuePtr v)
{
  _currentSum = _currentSum + Val_Double::cast(v);
  _count++;
}


ValuePtr 
AggAvg::result()
{
  return Val_Double::mk(_currentSum / _count);
}


// This is necessary for the class to register itself with the
// factory.
DEFINE_INITS(AggAvg,"AVG")


