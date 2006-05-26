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

#include "aggMax.h"

AggMax::AggMax()
{
}


AggMax::~AggMax()
{
}

  
void
AggMax::reset()
{
  _currentMax.reset();
}
  

void
AggMax::first(ValuePtr v)
{
  _currentMax = v;
}
  

void
AggMax::process(ValuePtr v)
{
  assert(_currentMax != 0);
  if (v->compareTo(_currentMax) > 0) {
    _currentMax = v;
  }
}


ValuePtr 
AggMax::result()
{
  return _currentMax;
}


AggMax*
AggMax::mk()
{
  return new AggMax();
}
