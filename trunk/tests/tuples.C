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
#include <iostream>
#include <errno.h>

#include "loop.h"
#include "tuple.h"

#include "val_null.h"
#include "val_str.h"
#include "val_int32.h"
#include "val_uint32.h"
#include "val_int64.h"
#include "val_uint64.h"
#include "val_double.h"
#include "oper.h"

using namespace opr;

static double time_fn(b_cbv cb) 
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

const int FIELD_TST_SZ=500;
const int TUPLE_TST_SZ=500;
const int MARSHAL_CHUNK_SZ=100;
const int MARSHAL_NUM_UIOS=5000;

/* What hex dump of the next tuple in XDR should look like... 
 00000004 // Tuple has 5 fields
 00000000 // Null (type code = 0)
 00000001 // Type code = 1 (int32)
 ffffffe0 // -32
 00000004 // Type code = 4 (uint64)
 00000000 // 0x0000000000000040 (64)
 00000040 // 
 00000005 // Type code = 6 (double)
 3f894855 
 da272863
 00000004 // Type code = 5 (string)
 00000010 // Length = 0x10 (16)
 54686973 // String...
 20697320 //
 61207374 // 
 72696e67 //
*/
static TuplePtr create_tuple_1() {
  TuplePtr t = Tuple::mk();
  t->append(Val_Null::mk());
  t->append(Val_Int32::mk((int32_t)-32));
  t->append(Val_UInt64::mk((uint64_t)64));
  t->append(Val_Double::mk(0.012345));
  t->append(Val_Str::mk("This is a string"));
  t->freeze();
  return t;
}

static TuplePtr ta[TUPLE_TST_SZ];

static void create_lots_of_tuples() {
  for( int i=0; i<TUPLE_TST_SZ; i++) {
    ta[i] = create_tuple_1();
  }
}

// static xdrsuio encode_uios[MARSHAL_NUM_UIOS];

static void marshal_lots_of_tuples() 
{
/** FIX ME SUIO
  assert( MARSHAL_CHUNK_SZ < TUPLE_TST_SZ);
  for( int c=0; c< MARSHAL_NUM_UIOS; c++) {
    for( int i=0; i< MARSHAL_CHUNK_SZ; i++) {
      ta[i]->xdr_marshal(&encode_uios[c]);
    }
  }
*/
}
static void unmarshal_lots_of_tuples() 
{
/** FIX ME SUIO
  assert( MARSHAL_CHUNK_SZ < TUPLE_TST_SZ);
  for( int c=0; c< MARSHAL_NUM_UIOS; c++) {
    suio *u = encode_uios[c].uio();
    const char *buf = suio_flatten(u);
    size_t sz = u->resid();
    xdrmem xd(buf,sz);
    for( int i=0; i< MARSHAL_CHUNK_SZ; i++) {
      ta[i]=Tuple::xdr_unmarshal(&xd);
    }
    delete [] buf;
  }
*/
}

int main(int argc, char **argv)
{
/** FIX ME XDR
  std::cout << "TUPLES\n";

  // Create a bunch of tuples...
  std::cout << "Creating tuples...\n";
  std::cout << "sizeof(Tuple)=" << sizeof(Tuple) << "\n";

  std::cout << "Creating " << TUPLE_TST_SZ << " tuples: ";
  double el = time_fn(wrap(create_lots_of_tuples));
  std::cout << " (rate=" << (el / TUPLE_TST_SZ * 1000 * 1000) << " usec/tuple)\n";

  // Now try marshalling them...
  std::cout << "Marshalling one of them tuples:";
  xdrsuio xe;
  ta[0]->xdr_marshal(&xe);
  std::cout << " resid=" << xe.uio()->resid();
  std::cout << " byteno=" << xe.uio()->byteno();
  std::cout << " iovno=" << xe.uio()->iovno() << "\n";
  const char *buf = suio_flatten(xe.uio());
  size_t sz = xe.uio()->resid();
  string s = hexdump(buf,sz);
  std::cout << " Hexdump: " << s << "\n";
  
  // Now try unmarshalling said tuple...
  std::cout << "Unmarshalling... ";
  xdrmem xd(buf,sz);
  TuplePtr t = Tuple::xdr_unmarshal(&xd);
  std::cout << "read " << t->size() << " fields.\n";

  std::cout << "Marshalling " << MARSHAL_NUM_UIOS << " of " << MARSHAL_CHUNK_SZ << " tuples each: ";
  el = time_fn(wrap(marshal_lots_of_tuples));
  std::cout << " (rate=" << (el / MARSHAL_NUM_UIOS / MARSHAL_CHUNK_SZ * 1000 * 1000) << " usec/tuple)\n";

  std::cout << "Unmarshalling " << MARSHAL_NUM_UIOS << " of " << MARSHAL_CHUNK_SZ << " tuples each: ";
  el = time_fn(wrap(unmarshal_lots_of_tuples));
  std::cout << " (rate=" << (el / MARSHAL_NUM_UIOS / MARSHAL_CHUNK_SZ * 1000 * 1000) << " usec/tuple)\n";

  t = create_tuple_1();
  std::cout << t->toString() << "\n";
*/

  return 0;
}
  

/*
 * End of file 
 */
