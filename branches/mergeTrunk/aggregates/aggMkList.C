/*
 * Copyright (c) 2005 Intel Corporation. All rights reserved.
 *
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 */

#include "aggMkList.h"
#include "val_list.h"
#include "list.h"
#include "aggFactory.h"

AggMkList::AggMkList()
{
  _list = List::mk();
}


AggMkList::~AggMkList()
{
}

  
void
AggMkList::reset()
{
  _list = List::mk();
}
  

void
AggMkList::first(ValuePtr v)
{
  _list->append(v);
}
  

void
AggMkList::process(ValuePtr v)
{
  _list->append(v);
}


ValuePtr 
AggMkList::result()
{
  return Val_List::mk(_list);
}


// This is necessary for the class to register itself with the
// factory.
DEFINE_INITS(AggMkList,"MKLIST")


