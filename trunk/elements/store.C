// -*- c-basic-offset: 2; related-file-name: "store.h" -*-
/*
 * @(#)$Id$
 * 
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 */

#include "store.h"

Store::Store(unsigned fieldNo)
  : _fieldNo(fieldNo),
    _comparator(fieldNo),
    _table(_comparator)
{
}


Store::Insert::Insert(std::multiset< TupleRef > * table)
  : Element(1, 0),
    _table(table)
{
}

Store::Lookup::Lookup(std::multiset< TupleRef > * table)
  : Element(1, 1),
    _table(table)
{
}

int Store::Insert::push(int port, TupleRef p, cbv cb)
{
  _table->insert(p);
  warn << "Set has " << _table->size() << " elements\n";
  return 1;
}

int Store::Lookup::push(int port, TupleRef t, cbv cb)
{
  // Is this the right port?
  assert(port == 0);

  return 1;
}

TuplePtr Store::Lookup::pull(int port, cbv cb) 
{
  // Is this the right port?
  assert(port == 0);

  return 0;
}
