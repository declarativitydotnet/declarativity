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

#define TEST_TYPING(TN,TF,TM,VAL) { \
  TupleField tf( (TN)VAL ); \
    if (tf.get_type() != TupleField::TF) { \
      std::cerr << "** Bad type field: " << tf.get_type() << "\n"; \
    } else if (tf.as_##TM() != VAL) { \
      std::cerr << "** Bad value returned: " << tf.as_##TM() << "\n"; \
    } else { \
      std::cout << "Value: " << tf.as_##TM() << "\n"; \
    } \
}; 


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

const int FIELD_TST_SZ=500000;
const int TUPLE_TST_SZ=500000;
const int MARSHAL_CHUNK_SZ=100;
const int MARSHAL_NUM_UIOS=5000;


static void create_lots_of_fields() {
  TupleField *t[FIELD_TST_SZ];
  for( int i=0; i<FIELD_TST_SZ; i++) {
    t[i] = New TupleField((int32_t)(3 * i));
  }
}

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
static Tuple *create_tuple_1() {
  Tuple *t = New Tuple();
  t->append(*New TupleField());
  t->append(*New TupleField((int32_t)-32));
  t->append(*New TupleField((uint64_t)64));
  t->append(*New TupleField(0.012345));
  t->append(*New TupleField("This is a string"));
  t->finalize();
  return t;
}

static Tuple *ta[TUPLE_TST_SZ];

static void create_lots_of_tuples() {
  for( int i=0; i<TUPLE_TST_SZ; i++) {
    ta[i] = create_tuple_1();
  }
}

static xdrsuio encode_uios[MARSHAL_NUM_UIOS];

static void marshal_lots_of_tuples() 
{
  assert( MARSHAL_CHUNK_SZ < TUPLE_TST_SZ);
  for( int c=0; c< MARSHAL_NUM_UIOS; c++) {
    for( int i=0; i< MARSHAL_CHUNK_SZ; i++) {
      ta[i]->xdr_marshal(&encode_uios[c]);
    }
  }
}
static void unmarshal_lots_of_tuples() 
{
  assert( MARSHAL_CHUNK_SZ < TUPLE_TST_SZ);
  for( int c=0; c< MARSHAL_NUM_UIOS; c++) {
    suio *u = encode_uios[c].uio();
    const char *buf = suio_flatten(u);
    size_t sz = u->resid();
    xdrmem xd(buf,sz);
    for( int i=0; i< MARSHAL_CHUNK_SZ; i++) {
      ta[i]=Tuple::xdr_unmarshal(&xd);
    }
  }
}

int main(int argc, char **argv)
{
  std::cout << "TUPLES\n";
  std::cout << "sizeof(TupleField)=" << sizeof(TupleField) << "\n";
  std::cout << "sizeof(size_t)=" << sizeof(size_t) << "\n";

  TEST_TYPING(int32_t, INT32, i32, -123456);
  TEST_TYPING(uint32_t,UINT32, ui32, 123456);
  TEST_TYPING(int64_t, INT64, i64, -123456);
  TEST_TYPING(uint64_t, UINT64, ui64, 123456);
  TEST_TYPING(double, DOUBLE, d, 1.23456);
  TEST_TYPING(str, STRING, s, "Hello world");

  std::cout << "Creating " << FIELD_TST_SZ << " fields: ";
  time_fn(wrap(create_lots_of_fields));

  TupleField tf((uint64_t)1234);
  try {
    str s = tf.as_s();
    std::cerr << "** Failed to raise type error exception\n";
  } catch (TupleField::TypeError) {
    std::cout << "Correctly caught type exception\n";
    // Pass
  }

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
  str s = strbuf() << hexdump(buf,sz);
  std::cout << " Hexdump: " << s << "\n";
  
  // Now try unmarshalling said tuple...
  std::cout << "Unmarshalling... ";
  xdrmem xd(buf,sz);
  Tuple *t = Tuple::xdr_unmarshal(&xd);
  std::cout << "read " << t->size() << " fields.\n";

  std::cout << "Marshalling " << MARSHAL_NUM_UIOS << " of " << MARSHAL_CHUNK_SZ << " tuples each: ";
  el = time_fn(wrap(marshal_lots_of_tuples));
  std::cout << " (rate=" << (el / MARSHAL_NUM_UIOS / MARSHAL_CHUNK_SZ * 1000 * 1000) << " usec/tuple)\n";

  // Now try freeing up all those unmarshalled tuples...
  std::cout << "Freeing up original tuples...\n";
  for(int i=0; i<TUPLE_TST_SZ; i++) {
    delete ta[i];
  }

  std::cout << "Unmarshalling " << MARSHAL_NUM_UIOS << " of " << MARSHAL_CHUNK_SZ << " tuples each: ";
  el = time_fn(wrap(unmarshal_lots_of_tuples));
  std::cout << " (rate=" << (el / MARSHAL_NUM_UIOS / MARSHAL_CHUNK_SZ * 1000 * 1000) << " usec/tuple)\n";



  return 0;
}
  

/*
 * End of file 
 */
