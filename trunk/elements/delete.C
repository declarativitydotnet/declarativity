// -*- c-basic-offset: 2; related-file-name: "delete.h" -*-
/*
 * @(#)$Id$
 * 
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * Or
 * UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
 * Berkeley, CA,  94707. Attention: P2 Group.
 * 
 */

#include "delete.h"
#include "val_str.h"
#include "val_uint32.h"
#include "plumber.h"

DEFINE_ELEMENT_INITS(Delete, "Delete")

Delete::Delete(string name,
               CommonTablePtr table)
  : Element(name, 1, 0),
    _table(table)
{
}

/**
 * Generic constructor.
 * Arguments:
 * 2. Val_Str:    Element Name.
 * 3. Val_Str:    Table Name.
 */
Delete::Delete(TuplePtr args)
  : Element(Val_Str::cast((*args)[2]), 1, 0)
{
  string tableName = Val_Str::cast((*args)[3]);
  _table = Plumber::catalog()->table(tableName);
}


int
Delete::push(int port, TuplePtr t, b_cbv cb)
{
  // Is this the right port?
  assert(port == 0);

  // Erase the entry by that key
  _table->remove(t);

  return 1;
}

