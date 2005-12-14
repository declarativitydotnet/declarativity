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

#include "slot.h"
#include "print.h"

#include "val_null.h"
#include "val_str.h"
#include "val_int32.h"
#include "val_uint32.h"
#include "val_int64.h"
#include "val_uint64.h"
#include "val_double.h"
#include "val_opaque.h"

#if 0
// Unused
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
  t->append(Val_Null::mk());
  t->append(Val_Int32::mk(-32));
  t->append(Val_UInt64::mk(64));
  t->append(Val_Double::mk(0.012345));
  t->append(Val_Str::mk("This is a string"));
  t->freeze();
  return t;
}


static void slot_pull_cb() {
  std::cout << "Slot: Pull callback called.\n";
}

static void slot_push_cb() {
  std::cout << "Slot: Push callback called.\n";
}

int main(int argc, char **argv)
{
  TupleRef t = create_tuple_1();

  std::cout << "BASICELEMENTS\n";

  Slot *s = New Slot("slot");
  for(int i=0; i<5; i++) {
    TuplePtr tp = s->pull(0, &slot_pull_cb);
    if (tp == NULL) {
      std::cout << "Null tuple\n";
    } else {
      std::cout << "Got tuple " << t->toString() << "\n";
    }
    s->push(0,t, &slot_push_cb);
  }
  return 0;
}
  

/*
 * End of file 
 */
