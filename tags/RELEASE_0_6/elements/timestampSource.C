// -*- c-basic-offset: 2; related-file-name: "timedstampSource.h" -*-
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

#include <timestampSource.h>
#include "val_str.h"
#include "val_uint64.h"
#include "val_time.h"

TimestampSource::TimestampSource(string name)
  : Element(name, 0, 1)
{
}
    
TuplePtr TimestampSource::pull(int port, b_cbv cb)
{
  // Always produce a result, never block

  boost::posix_time::ptime t;
  getTime(t);
  
  TuplePtr tuple = Tuple::mk();
  tuple->append(Val_Str::mk("Time"));
  tuple->append(Val_Time::mk(t));
  tuple->append(Val_Str::mk("End of time"));
  tuple->freeze();

  return tuple;
}
