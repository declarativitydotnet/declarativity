/*
 * @(#)$Id: aggSum.C 1243 2007-07-16 19:05:00Z maniatis $
 *
 * Copyright (c) 2005 Intel Corporation. All rights reserved.
 *
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 */

#include "aggSum.h"
#include "val_null.h"
#include "aggFactory.h"

AggSum::AggSum()
{
}


AggSum::~AggSum()
{
}

  
void
AggSum::reset()
{
  _currentSum = 0;
}
  

void
AggSum::first(ValuePtr v)
{
  _currentSum = Val_Double::cast(v);
}
  

void
AggSum::process(ValuePtr v)
{
  _currentSum = _currentSum + Val_Double::cast(v);
}


ValuePtr 
AggSum::result()
{
  return Val_Double::mk(_currentSum);
}


// This is necessary for the class to register itself with the
// factory.
DEFINE_INITS(AggSum,"SUM")


