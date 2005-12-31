// -*- c-basic-offset: 2; related-file-name: "insert.h" -*-
/*
 * @(#)$Id$
 * 
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 */

#include "insert.h"

Insert::Insert(str name,
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
