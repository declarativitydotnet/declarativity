// -*- c-basic-offset: 2; related-file-name: "hexdump.h" -*-
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

#include "hexdump.h"

#include "val_str.h"
#include "val_opaque.h"

Hexdump::Hexdump(string name,
                 unsigned fieldNo)
  : Element(name, 1, 1),
    _fieldNo(fieldNo)
{
}

Hexdump::~Hexdump()
{
}

TuplePtr Hexdump::simple_action(TuplePtr p)
{
  // Get field
  ValuePtr first = (*p)[_fieldNo];
  if (first == NULL) {
    // No such field
    log(Reporting::WARN,
        -1,
        "Input tuple has no requested field");
    return TuplePtr();
  }

  // Is it an opaque?
  if (first->typeCode() != Value::P2_OPAQUE) {
    // Can't hexdump anything but opaques
    log(Reporting::WARN,
        -1,
        "Input tuple's field to hexdump is not an opaque");
    return TuplePtr();
  }
  
  // Hexdump and return new tuple
  FdbufPtr u = Val_Opaque::cast(first);
  char *buf = u->cstr();
  uint32_t sz = u->length();
  ostringstream sb;
  for (uint32_t i = 0;
       i<sz;
       i++) {
    char b[4]; 
    sprintf(b,"%02x", buf[i]);
    sb << b;
  }

  // And create the output tuple
  TuplePtr newTuple = Tuple::mk();
  for (unsigned field = 0;
       field < _fieldNo;
       field++) {
    newTuple->append((*p)[field]);
  }
  newTuple->append(Val_Str::mk(sb.str()));
  for (unsigned field = _fieldNo + 1;
       field < p->size();
       field++) {
    newTuple->append((*p)[field]);
  }
  newTuple->freeze();

  return newTuple;
}
