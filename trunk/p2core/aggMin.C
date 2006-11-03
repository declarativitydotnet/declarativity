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

#include "aggMin.h"
#include "val_null.h"

AggMin::AggMin()
{
}


AggMin::~AggMin()
{
}

  
void
AggMin::reset()
{
  _currentMin.reset();
}
  

void
AggMin::first(ValuePtr v)
{
  _currentMin = v;
}
  

void
AggMin::process(ValuePtr v)
{
  assert(_currentMin != 0);
  if (v->compareTo(_currentMin) < 0) {
    _currentMin = v;
  }
}


ValuePtr 
AggMin::result()
{
  if (_currentMin != 0) {
    return _currentMin;
  } else {
    return Val_Null::mk();
  }
}


AggMin*
AggMin::mk()
{
  return new AggMin();
}


std::string
AggMin::name()
{
  return "MIN";
}
