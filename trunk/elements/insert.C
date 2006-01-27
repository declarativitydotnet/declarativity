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

#include "insert.h"

Insert::Insert(string name,
               TablePtr table)
  : Element(name, 1, 1),
    _table(table)
{
}

TuplePtr Insert::simple_action(TuplePtr p)
{
  // Nothing to do. Just insert
  _table->insert(p);

  return p;
}
