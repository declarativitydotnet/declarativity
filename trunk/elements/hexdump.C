// -*- c-basic-offset: 2; related-file-name: "hexdump.h" -*-
/*
 * @(#)$Id$
 * 
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 */

#include "hexdump.h"

#include "val_str.h"
#include "val_opaque.h"

Hexdump::Hexdump(str name,
                 unsigned fieldNo)
  : Element(name, 1, 1),
    _fieldNo(fieldNo)
{
}

Hexdump::~Hexdump()
{
}

TuplePtr Hexdump::simple_action(TupleRef p)
{
  // Get field
  ValuePtr first = (*p)[_fieldNo];
  if (first == NULL) {
    // No such field
    log(LoggerI::WARN,
        -1,
        "Input tuple has no requested field");
    return 0;
  }

  // Is it an opaque?
  if (first->typeCode() != Value::OPAQUE) {
    // Can't hexdump anything but opaques
    log(LoggerI::WARN,
        -1,
        "Input tuple's field to hexdump is not an opaque");
    return 0;
  }
  
  // Hexdump and return new tuple
  ref<suio> u = Val_Opaque::cast(first);
  char *buf = suio_flatten(u);
  size_t sz = u->resid();
  str s = strbuf() << hexdump(buf, sz);

  // And create the output tuple
  TupleRef newTuple = Tuple::mk();
  for (unsigned field = 0;
       field < _fieldNo;
       field++) {
    newTuple->append((*p)[field]);
  }
  newTuple->append(Val_Str::mk(s));
  for (unsigned field = _fieldNo + 1;
       field < p->size();
       field++) {
    newTuple->append((*p)[field]);
  }
  newTuple->freeze();

  return newTuple;
}
