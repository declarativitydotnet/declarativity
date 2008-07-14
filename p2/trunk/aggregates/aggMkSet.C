/*
 * @(#)$Id: aggMkSet.C 1243 2007-07-16 19:05:00Z maniatis $
 *
 * Copyright (c) 2005 Intel Corporation. All rights reserved.
 *
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 */

#include "aggMkSet.h"
#include "val_set.h"
#include "set.h"
#include "aggFactory.h"

AggMkSet::AggMkSet()
{
  _set = Set::mk();
}


AggMkSet::~AggMkSet()
{
}

  
void
AggMkSet::reset()
{
  _set->clear();
}
  

void
AggMkSet::first(ValuePtr v)
{
  _set->insert(v);
}
  

void
AggMkSet::process(ValuePtr v)
{
  _set->insert(v);
}


ValuePtr 
AggMkSet::result()
{
  return Val_Set::mk(_set);
}


// This is necessary for the class to register itself with the
// factory.
DEFINE_INITS(AggMkSet,"MKSET")


