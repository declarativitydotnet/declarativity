// -*- c-basic-offset: 2; related-file-name: "filter.h" -*-
/*
 * @(#)$Id$
 * 
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 */

#include "filter.h"
#include "val_int32.h"

Filter::Filter(string name, unsigned filterNo)
  : Element(name, 1, 1),
    _filterNo(filterNo)
{
}


TuplePtr Filter::simple_action(TuplePtr p)
{
  // Extract the requested field
  ValuePtr field = (*p)[_filterNo];

  // Does the field exist?
  if (field == NULL) {
    // Nope, no such field. Log a warning and return nothing.
    log(LoggerI::WARN, -1, "Filtered field unavailable");
    return TuplePtr();
  } else {
    // Make sure it is an INT32
    if (field->typeCode() != Value::INT32) {
      // Drop this and issue a warning
      log(LoggerI::WARN, -1, "Filtered field not an INT32. Dropping");
      return TuplePtr();
    } else {
      // OK, this is the right type.  Now do the filtering
      if (field->equals(Val_Int32::ZERO)) {
        return TuplePtr();
      } else {
        return p;
      }
    }
  }
}
