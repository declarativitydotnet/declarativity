// -*- c-basic-offset: 2; related-file-name: "timedstampSource.h" -*-
/*
 * @(#)$Id$
 * 
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 */

#include <timestampSource.h>
#include "val_str.h"
#include "val_uint64.h"

TimestampSource::TimestampSource(string name)
  : Element(name, 0, 1)
{
}
    
TuplePtr TimestampSource::pull(int port, b_cbv cb)
{
  // Always produce a result, never block

  struct timespec t;
  getTime(t);
  
  TuplePtr tuple = Tuple::mk();
  tuple->append(Val_Str::mk("Time"));
  tuple->append(Val_UInt64::mk(t.tv_sec));
  tuple->append(Val_UInt64::mk(t.tv_nsec));
  tuple->append(Val_Str::mk("End of time"));
  tuple->freeze();

  return tuple;
}
