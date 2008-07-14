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
 */

#include "aggCountDistinct.h"
#include "val_uint64.h"
#include "aggFactory.h"

AggCountDistinct::AggCountDistinct()
{
}


AggCountDistinct::~AggCountDistinct()
{
}

  
void
AggCountDistinct::reset()
{
  _valueSet.clear();
}
  

void
AggCountDistinct::first(ValuePtr v)
{
  _valueSet.insert(v);
}
  

void
AggCountDistinct::process(ValuePtr v)
{
  _valueSet.insert(v);
}


ValuePtr 
AggCountDistinct::result()
{
  return Val_UInt64::mk(_valueSet.size());
}


// This is necessary for the class to register itself with the
// factory.
DEFINE_INITS(AggCountDistinct,"COUNTDISTINCT")


