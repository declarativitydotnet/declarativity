// -*- c-basic-offset: 2; related-file-name: "insert.h" -*-
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

#ifdef WIN32
#include "p2_win32.h"
#endif // WIN32

#include "insert.h"
#include "commonTable.h"
#include "val_str.h"
#include "plumber.h"

DEFINE_ELEMENT_INITS(Insert, "Insert");

Insert::Insert(string name,
               CommonTablePtr table)
  : Element(name, 1, 1),
    _table(table)
{
}

/**
 * Generic constructor.
 * Arguments:
 * 2. Val_Str:    Element Name.
 * 3. Val_Str:    Table Name.
 */
Insert::Insert(TuplePtr args)
  : Element(Val_Str::cast((*args)[2]), 1, 1),
    _table(Plumber::catalog()->table(Val_Str::cast((*args)[3])))
{
}

int
Insert::push(int port, TuplePtr p, b_cbv cb)
{
  assert(p != 0);
  
  if (_table->insert(p)) {
    // The table changed. Push the tuple to the output port
    return output(0)->push(p, cb);
  } else {
    // The table didn't change. Take this silently
    return 1;
  }
}

