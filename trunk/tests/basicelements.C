/*
 * @(#)$Id$
 *
 * Copyright (c) 2004 Intel Corporation. All rights reserved.
 *
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 * DESCRIPTION: Test suite for tuples
 *
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */
#include <async.h>
#include <arpc.h>
#include <iostream>

#include "tuple.h"
#include "pushprint.h"

#if 0
static double time_fn(cbv cb) 
{
  timespec before_ts;
  timespec after_ts;
  double elapsed;
  
  if (clock_gettime(CLOCK_REALTIME,&before_ts)) {
    fatal << "clock_gettime:" << strerror(errno) << "\n";
  }
  (cb)();
  if (clock_gettime(CLOCK_REALTIME,&after_ts)) {
    fatal << "clock_gettime:" << strerror(errno) << "\n";
  }
  
  after_ts = after_ts - before_ts;
  elapsed = after_ts.tv_sec + (1.0 / 1000 / 1000 / 1000 * after_ts.tv_nsec);
  std::cout << elapsed << " secs (";
  std::cout << after_ts.tv_sec << " secs " << (after_ts.tv_nsec/1000) << " usecs)\n";
  return elapsed;
}
#endif

static TupleRef create_tuple_1() {
  TupleRef t = New refcounted<Tuple>;
  t->append(*New TupleField());
  t->append(*New TupleField((int32_t)-32));
  t->append(*New TupleField((uint64_t)64));
  t->append(*New TupleField(0.012345));
  t->append(*New TupleField("This is a string"));
  t->freeze();
  return t;
}

int main(int argc, char **argv)
{
  TupleRef t = create_tuple_1();

  std::cout << "BASICELEMENTS\n";

  std::cout << "PushPrint\n";
  PushPrint *p = New PushPrint();

  p->push(0,t,cbv_null);

  return 0;
}
  

/*
 * End of file 
 */
