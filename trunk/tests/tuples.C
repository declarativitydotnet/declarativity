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
static TupleRef create_tuple_1() {
  TupleRef t = New refcounted<Tuple>();
  t->append(New refcounted<TupleField>());
  t->append(New refcounted<TupleField>((int32_t)-32));
  t->append(New refcounted<TupleField>((uint64_t)64));
  t->append(New refcounted<TupleField>(0.012345));
  t->append(New refcounted<TupleField>("This is a string"));
  t->freeze();
  return t;
}

static TuplePtr ta[TUPLE_TST_SZ];

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

void test_conv( TupleFieldRef tf, 
		uint64_t u_val, bool u_ok, 
		int64_t s_val, bool s_ok )
{
  int64_t s_ans;
  uint64_t u_ans;
  
  if (tf->convert_unsigned(u_ans) != u_ok) {
    if (u_ok) {
      std::cerr << "** Expected " << tf->toTypeString() 
		<< " to convert to unsigned, but it didn't\n";
    } else {
      std::cerr << "** Didn't expect " << tf->toTypeString() 
		<< " to convert to unsigned, but it did\n";
    } 
  }
  if (u_val != u_ans) {
    std::cerr.setf(std::ios_base::hex,std::ios_base::basefield);
    std::cerr << "** Expected " << tf->toTypeString() 
	      << " to convert to unsigned as 0x" << u_val
	      << " but got 0x" << u_ans
	      << " instead.\n";
    std::cerr.setf(std::ios_base::dec,std::ios_base::basefield);
  }
  if (tf->convert_signed(s_ans) != s_ok) {
    if (s_ok) {
      std::cerr << "** Expected " << tf->toTypeString() 
		<< " to convert to signed, but it didn't\n";
    } else {
      std::cerr << "** Didn't expect " << tf->toTypeString() 
		<< " to convert to signed, but it did\n";
    } 
  }
  if (s_val != s_ans) {
    std::cerr << "** Expected " << tf->toTypeString() 
	      << " to convert to unsigned as " << s_val
	      << " but got " << s_ans
	      << " instead.\n";
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

  std::cout << "Testing type conversions..\n";

  test_conv(New refcounted<TupleField>(0), 0, true, 0, true);
  test_conv(New refcounted<TupleField>(1), 1, true, 1, true);
  test_conv(New refcounted<TupleField>(-1), 0xffffffffffffffffULL, true, -1, true);
  test_conv(New refcounted<TupleField>(-1LL), 0xffffffffffffffffULL, true, -1, true);
  test_conv(New refcounted<TupleField>(0xffffffffffffffffULL), 0xffffffffffffffffULL, true, -1, true);
  test_conv(New refcounted<TupleField>(1.0), 0, false, 0, false);
  test_conv(New refcounted<TupleField>(), 0, false, 0, false);
  test_conv(New refcounted<TupleField>("Hello"), 0, false, 0, false);
  test_conv(New refcounted<TupleField>(0xffffffff), 0xffffffffULL, true, 0xffffffffLL, true);


  //void test_conv( TupleField &tf, uint64_t u_val, bool u_ok, 
  //int64_t s_val, bool s_ok )



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
  TupleRef t = Tuple::xdr_unmarshal(&xd);
  std::cout << "read " << t->size() << " fields.\n";

  std::cout << "Marshalling " << MARSHAL_NUM_UIOS << " of " << MARSHAL_CHUNK_SZ << " tuples each: ";
  el = time_fn(wrap(marshal_lots_of_tuples));
  std::cout << " (rate=" << (el / MARSHAL_NUM_UIOS / MARSHAL_CHUNK_SZ * 1000 * 1000) << " usec/tuple)\n";

  std::cout << "Unmarshalling " << MARSHAL_NUM_UIOS << " of " << MARSHAL_CHUNK_SZ << " tuples each: ";
  el = time_fn(wrap(unmarshal_lots_of_tuples));
  std::cout << " (rate=" << (el / MARSHAL_NUM_UIOS / MARSHAL_CHUNK_SZ * 1000 * 1000) << " usec/tuple)\n";

  t = create_tuple_1();
  std::cout << t->toString() << "\n";

  return 0;
}
  

/*
 * End of file 
 */
