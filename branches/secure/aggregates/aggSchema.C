/*
 * Copyright (c) 2005 Intel Corporation. All rights reserved.
 *
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 */

#include "aggSchema.h"
#include "val_list.h"
#include "list.h"
#include "aggFactory.h"
#include "compileUtil.h"

AggSchema::AggSchema()
{
  _schema = List::mk();
}


AggSchema::~AggSchema()
{
}

  
void
AggSchema::reset()
{
  _schema = List::mk();
}
  

void
AggSchema::first(ValuePtr v)
{
  _schema = Val_List::cast(v);
}
  

void
AggSchema::process(ValuePtr v)
{
  _schema = compile::namestracker::merge(_schema, Val_List::cast(v));
}


ValuePtr 
AggSchema::result()
{
  return Val_List::mk(_schema);
}


// This is necessary for the class to register itself with the
// factory.
DEFINE_INITS(AggSchema,"SCHEMA")


