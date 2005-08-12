#if 0
// -*- c-basic-offset: 2; related-file-name: "scan.h" -*-
/*
 * @(#)$Id$
 * 
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 */

#include "scan.h"

Scan::Scan(str name,
           TableRef table,
           unsigned fieldNo)
  : Element(name, 0, 1),
    _table(table),
    _iterator(table->scanAll(fieldNo))
{
}

TuplePtr Scan::pull(int port, cbv cb) 
{
  // Is this the right port?
  assert(port == 0);
  
  // Does the table have elements?
  if (_table->size() > 0) {
    // Is the iterator at the end?
    if (_iterator->done()) {
      // Reset it 
      _iterator->reset();
    }
    
    // Return the next element
    return _iterator->next();
  } else {
    // No elements.  Just return the empty tuple
    return Tuple::EMPTY;
  }
}
#endif
