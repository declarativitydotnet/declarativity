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
 * DESCRIPTION: Test suite for benchmarking
 *
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */
#include <async.h>
#include <arpc.h>
#include <iostream>
#include <cmath>

#include "tuple.h"

#include "val_null.h"
#include "val_str.h"
#include "val_int32.h"
#include "val_uint32.h"
#include "val_int64.h"
#include "val_uint64.h"
#include "val_double.h"


const int FIELD_TST_SZ=500;
const int TUPLE_TST_SZ=500;
const int MARSHAL_CHUNK_SZ=100;
const int MARSHAL_NUM_UIOS=5000;

static ValuePtr va[FIELD_TST_SZ];

static xdrsuio single_uios;
static xdrsuio xe[MARSHAL_NUM_UIOS];
static ValuePtr vf;

static double time_fn(cbv cb) 
{
  timespec before_ts;
  timespec after_ts;
  double elapsed;
  double past_average = 0;
  double average = 0;
  double sum;
  double deviation = 1;
  int iter;

  for (iter = 1; deviation > .05; iter = iter * 2) {
    if (clock_gettime(CLOCK_REALTIME,&before_ts)) {
      fatal << "clock_gettime:" << strerror(errno) << "\n";
    }

    for (int i = 0; i < iter; i++) {
      (cb) ();
    } 

    if (clock_gettime(CLOCK_REALTIME,&after_ts)) {
      fatal << "clock_gettime:" << strerror(errno) << "\n";
    }

    after_ts = after_ts - before_ts;
    elapsed = after_ts.tv_sec + (1.0 / 1000 / 1000 / 1000 * after_ts.tv_nsec);
    average = elapsed / iter;
    sum = (average - past_average);
    if (sum > 0) 
      deviation = sum/average;
    else 
      deviation = -sum/average;
    past_average = average;
  } 

  std::cout << "\n";

  std::cout << "iter: " << iter << " ";
  std::cout << "average: " << average * 1000 << " msecs, ";
  std::cout << "elasped: " << elapsed * 1000 << " secs (";
  std::cout << after_ts.tv_sec << " secs " << (after_ts.tv_nsec/1000) << " usecs)\n";
  return elapsed;
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
   a272863
   00000004 // Type code = 5 (string)
   00000010 // Length = 0x10 (16)
   54686973 // String...
   20697320 //
   61207374 // 
   72696e67 //
 */

static ValueRef create_val_null_1() {
  return Val_Null::mk();
}

static void create_lots_val_null() {
  for (int i = 0; i < FIELD_TST_SZ; i++) {
    va[i] = create_val_null_1();
  }
}

static ValueRef create_val_uint64_1() {
  return Val_UInt64::mk((uint64_t)64);
}

static void create_lots_val_uint64() {
  for (int i = 0; i < FIELD_TST_SZ; i++) {
    va[i] = create_val_uint64_1();
  }
}

static ValueRef create_val_int32_1() {
  return Val_Int32::mk((int32_t)-32);
}

static void create_lots_val_int32() {
  for (int i = 0; i < FIELD_TST_SZ; i++) {
    va[i] = create_val_int32_1();
  }
}

static ValueRef create_val_double_1() {
  return Val_Double::mk(0.012345);
}

static void create_lots_val_double() {
  for (int i = 0; i < FIELD_TST_SZ; i++) {
    va[i] = create_val_double_1();
  }
}

static ValueRef create_val_str_1() {
  return Val_Str::mk("This is a string");
}

static void create_lots_val_str() {
  for (int i = 0; i < FIELD_TST_SZ; i++) {
    va[i] = create_val_str_1();
  }
}

static void marshal_lots_of_values() {
  for (int c = 0; c < MARSHAL_NUM_UIOS; c++) { 
    // clear uio first, then write.
    // this prevent memory from growing indefinitely
    xe[c].uio()->clear();
    for (int i = 0; i < MARSHAL_CHUNK_SZ; i++) {
      va[i]->xdr_marshal(&xe[c]); 
    }
  }
}

static xdrsuio encode_uios[MARSHAL_NUM_UIOS];

static void unmarshal_lots_of(Value::TypeCode t) {
  for (int c = 0; c < MARSHAL_NUM_UIOS; c++) {
    suio *u = xe[5].uio();
    char *buf = suio_flatten(u);
    size_t sz = u->resid();
    xdrmem xd(buf, sz);
    for (int i = 0; i < MARSHAL_CHUNK_SZ; i++) {
      switch (t) {
        case Value::NULLV:   va[i] = Val_Null::xdr_unmarshal(&xd);   break;
        case Value::UINT64:  va[i] = Val_UInt64::xdr_unmarshal(&xd); break;
        case Value::INT32:   va[i] = Val_Int32::xdr_unmarshal(&xd);  break;
        case Value::DOUBLE:  va[i] = Val_Double::xdr_unmarshal(&xd); break;
        case Value::STR:     va[i] = Val_Str::xdr_unmarshal(&xd);    break;
      }
    }   
    free(buf);
  }
}

static void unmarshal_lots_of_null() {
  unmarshal_lots_of(Value::NULLV);
}

static void unmarshal_lots_of_uint64() {
  unmarshal_lots_of(Value::UINT64);
}

static void unmarshal_lots_of_int32() {
  unmarshal_lots_of(Value::INT32);
}

static void unmarshal_lots_of_double() {
  unmarshal_lots_of(Value::DOUBLE);
}

static void unmarshal_lots_of_str() {
  unmarshal_lots_of(Value::STR);
}

// for tuples

static TuplePtr ta[TUPLE_TST_SZ];
static TupleRef create_tuple_1() {
  TupleRef t = Tuple::mk();
  t->append(create_val_null_1());
  t->append(create_val_int32_1());
  t->append(create_val_uint64_1());
  t->append(create_val_double_1());
  t->append(create_val_str_1());
  t->freeze();
  return t;
}

static void create_lots_of_tuples() {
  for( int i=0; i<TUPLE_TST_SZ; i++) {
    ta[i] = create_tuple_1();
  }
}

static void marshal_lots_of_tuples() 
{
  assert( MARSHAL_CHUNK_SZ < TUPLE_TST_SZ);
  for( int c=0; c< MARSHAL_NUM_UIOS; c++) {
    encode_uios[c].uio()->clear();
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
    char *buf = suio_flatten(u);
    size_t sz = u->resid();
    xdrmem xd(buf,sz);
    for( int i=0; i< MARSHAL_CHUNK_SZ; i++) {
      ta[i]=Tuple::xdr_unmarshal(&xd);
    }
    free(buf);
  }
}


static void unit_test_for(Value::TypeCode t ) {
  switch (t) {
    case Value::NULLV:
      std::cout << "NULLV:\ncreating:";
      time_fn(wrap(create_lots_val_null));
      std::cout << "marshalling:";
      time_fn(wrap(marshal_lots_of_values));
      std::cout << "unmarshalling:";
      time_fn(wrap(unmarshal_lots_of_null));  
      std::cout << "\n";
      break;
    case Value::INT32: 
      std::cout << "INT32: creating:"; 
      time_fn(wrap(create_lots_val_int32));
      std::cout << " marshalling:";
      time_fn(wrap(marshal_lots_of_values));
      std::cout << " unmarshalling:";
      time_fn(wrap(unmarshal_lots_of_int32));  
      std::cout << "\n";
      break;
    case Value::UINT64:
      std::cout << "UINT64:\ncreating:";
      time_fn(wrap(create_lots_val_uint64));
      std::cout << "marshalling:";
      time_fn(wrap(marshal_lots_of_values));
      std::cout << "unmarshalling:";
      time_fn(wrap(unmarshal_lots_of_uint64));  
      std::cout << "\n";
      break;
    case Value::DOUBLE:
      std::cout << "DOUBLE:\ncreating:";
      time_fn(wrap(create_lots_val_double));
      std::cout << "marshalling:";
      time_fn(wrap(marshal_lots_of_values));
      std::cout << "unmarshalling:";
      time_fn(wrap(unmarshal_lots_of_double));  
      std::cout << "\n";
      break;
    case Value::STR:
      std::cout << "STR:\ncreating:";
      time_fn(wrap(create_lots_val_str));
      std::cout << "marshalling:";
      time_fn(wrap(marshal_lots_of_values));
      std::cout << "unmarshalling:";
      time_fn(wrap(unmarshal_lots_of_str));  
      std::cout << "\n";
      break;
    case Value::TUPLE:
      std::cout << "TUPLE:\ncreating:";
      time_fn(wrap(create_lots_of_tuples));
      std::cout << "marshalling:";
      time_fn(wrap(marshal_lots_of_tuples));
      std::cout << "unmarshalling:";
      time_fn(wrap(unmarshal_lots_of_tuples));

      break;
  }
}

int main(int argc, char **argv)
{
  TuplePtr t = create_tuple_1();
  xdrsuio singlet;
  t->xdr_marshal(&singlet);

  std::cout << " resid=" << singlet.uio()->resid();
  std::cout << " byteno=" << singlet.uio()->byteno();
  std::cout << " iovno=" << singlet.uio()->iovno() << "\n";
  const char *buf = suio_flatten(singlet.uio());
  size_t sz = singlet.uio()->resid();
  str s = strbuf() << hexdump(buf,sz);
  std::cout << " Hexdump: " << s << "\n";

  unit_test_for(Value::NULLV);
  unit_test_for(Value::INT32);
  unit_test_for(Value::UINT64);
  unit_test_for(Value::DOUBLE);
  unit_test_for(Value::STR);
  unit_test_for(Value::TUPLE);


  return 0;
}


/*
 * End of file 
 */
