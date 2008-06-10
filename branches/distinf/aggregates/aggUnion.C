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

#include "aggUnion.h"
#include "val_set.h"
#include "aggFactory.h"

AggUnion::AggUnion()
{
}


AggUnion::~AggUnion()
{
}

  
void
AggUnion::reset()
{
  currentUnion.clear();
}
  

void
AggUnion::first(ValuePtr v)
{
  SetPtr s = Val_Set::cast(v);
  currentUnion.clear();
  currentUnion.insert(s->begin(), s->end());
}
  

void
AggUnion::process(ValuePtr v)
{
  SetPtr s = Val_Set::cast(v);
  currentUnion.insert(s->begin(), s->end());
}


ValuePtr 
AggUnion::result()
{
  return Val_Set::mk(SetPtr(new Set(currentUnion)));
}


// This is necessary for the class to register itself with the
// factory.
DEFINE_INITS(AggUnion,"UNION")


