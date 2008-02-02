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

#include "aggCount.h"
#include "aggFactory.h"
#include "val_int64.h"

AggCount::AggCount()
{
}


AggCount::~AggCount()
{
}

  
void
AggCount::reset()
{
  _current = 0;
}
  

void
AggCount::first(ValuePtr v)
{
  _current = 1;
}
  

void
AggCount::process(ValuePtr v)
{
  _current++;
}


ValuePtr 
AggCount::result()
{
  return Val_Int64::mk(_current);
}


// This is necessary for the class to register itself with the
// factory.
DEFINE_INITS(AggCount,"COUNT")


