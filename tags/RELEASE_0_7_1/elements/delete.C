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

Delete::Delete(string name,
               Table2Ptr table)
  : Element(name, 1, 0),
    _table(table)
{
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

