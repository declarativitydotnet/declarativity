// -*- c-basic-offset: 2; related-file-name: "delete.h" -*-
/*
 * @(#)$Id$
 * 
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 */

#include "delete.h"

Delete::Delete(string name,
               TablePtr table,
               unsigned indexFieldNo,
               unsigned keyFieldNo)
  : Element(name, 1, 0),
    _table(table),
    _indexFieldNo(indexFieldNo),
    _keyFieldNo(keyFieldNo)
{
}

int
Delete::push(int port, TuplePtr t, b_cbv cb)
{
  // Is this the right port?
  assert(port == 0);

  // Fetch the search field
  ValuePtr key = (*t)[_keyFieldNo];
  if (key == NULL) {
    // No input field? WTF?
    log(LoggerI::WARN, 0, "push: tuple without lookup field received");
    
    // Didn't work out.  Ask for more.
    return 1;
  } else {
    // Erase the entry by that key
    std::vector<unsigned> vkey;
    vkey.push_back(_indexFieldNo);
    _table->remove(vkey, t);

    return 1;
  }
}

