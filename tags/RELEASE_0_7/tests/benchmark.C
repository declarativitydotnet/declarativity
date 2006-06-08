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
 * DESCRIPTION: Test suite for benchmarking
 *
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */
#include <iostream>
#include <cmath>

#include "fdbuf.h"
#include "xdrbuf.h"
#include "tuple.h"
#include "loop.h"
#include "oper.h"

#include "val_null.h"
#include "val_str.h"
#include "val_int32.h"
#include "val_uint32.h"
#include "val_int64.h"
#include "val_uint64.h"
#include "val_double.h"
#include "val_time.h"
#include "testerr.h"

using namespace opr;

const int FIELD_TST_SZ=500;
const int TUPLE_TST_SZ=500;
const int MARSHAL_CHUNK_SZ=100;
const int MARSHAL_NUM_UIOS=5000;

static ValuePtr va[FIELD_TST_SZ];

static Fdbuf xe[MARSHAL_NUM_UIOS];
static ValuePtr vf;

static double time_fn(b_cbv cb) 
{
  boost::posix_time::ptime before_ts;
  boost::posix_time::ptime after_ts;
  boost::posix_time::time_duration tdiff;
  double elapsed;
  double past_average = 0;
  double average = 0;
  double sum;
  double deviation = 1;
  int iter;

  for (iter = 1; deviation > .05; iter = iter * 2) {
    getTime(before_ts, LOOP_TIME_WALLCLOCK);

    for (int i = 0; i < iter; i++) {
      (cb) ();
    } 

    getTime(after_ts, LOOP_TIME_WALLCLOCK);

    tdiff = after_ts - before_ts; 
    // ensure we compute nanosecs (1/(10^9) sec) 
    // even if boost is compiled to lower precision 
    elapsed = tdiff.fractional_seconds() * PTIME_SECS_FACTOR;
    average = elapsed / iter;
    sum = (average - past_average);
    if (sum > 0) 
      deviation = sum / average;
    else 
      deviation = -sum / average;
    past_average = average;
  } 

  iter /= 2;
  std::cout << " Total numbers of values: " << iter << " * " << FIELD_TST_SZ << " = " << FIELD_TST_SZ * iter;
  std::cout << "\n";
  std::cout << "average: " << average * 1000 << " msecs, ";
  std::cout << "elapsed: " << elapsed * 1000 << " msecs (";
  std::cout << tdiff.seconds() << " secs " << elapsed - (tdiff.seconds()/1000) << " usecs)\n";
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

static ValuePtr create_val_null_1() {
  return Val_Null::mk();
}

static void create_lots_val_null() {
  for (int i = 0; i < FIELD_TST_SZ; i++) {
    va[i] = create_val_null_1();
  }
}

static ValuePtr create_val_uint64_1() {
  return Val_UInt64::mk((uint64_t)64);
}

static void create_lots_val_uint64() {
  for (int i = 0; i < FIELD_TST_SZ; i++) {
    va[i] = create_val_uint64_1();
  }
}

static ValuePtr create_val_int32_1() {
  return Val_Int32::mk((int32_t)-32);
}

static void create_lots_val_int32() {
  for (int i = 0; i < FIELD_TST_SZ; i++) {
    va[i] = create_val_int32_1();
  }
}

static ValuePtr create_val_double_1() {
  return Val_Double::mk(0.012345);
}

static void create_lots_val_double() {
  for (int i = 0; i < FIELD_TST_SZ; i++) {
    va[i] = create_val_double_1();
  }
}

static ValuePtr create_val_str_1() {
  return Val_Str::mk("This is a string");
}

static void create_lots_val_str() {
  for (int i = 0; i < FIELD_TST_SZ; i++) {
    va[i] = create_val_str_1();
  }
}

static void marshal_lots_of_values() {
  for (int c = 0; c < MARSHAL_NUM_UIOS; c++) { 
    XDR xdrs;
    // clear uio first, then write.
    // this prevent memory from growing indefinitely
    xe[c].clear();
    xdrfdbuf_create(&xdrs, &xe[c], false, XDR_ENCODE);
    for (int i = 0; i < MARSHAL_CHUNK_SZ; i++) {
      va[i]->xdr_marshal(&xdrs); 
    }
  }
}

static void unmarshal_lots_of(Value::TypeCode t) {
  for (int c = 0; c < MARSHAL_NUM_UIOS; c++) {
    XDR xdrs;
    xdrfdbuf_create(&xdrs, &xe[c], false, XDR_DECODE);
    for (int i = 0; i < MARSHAL_CHUNK_SZ; i++) {
      switch (t) {
        case Value::NULLV:   va[i] = Val_Null::xdr_unmarshal(&xdrs);   break;
        case Value::UINT64:  va[i] = Val_UInt64::xdr_unmarshal(&xdrs); break;
        case Value::INT32:   va[i] = Val_Int32::xdr_unmarshal(&xdrs);  break;
        case Value::DOUBLE:  va[i] = Val_Double::xdr_unmarshal(&xdrs); break;
        case Value::STR:     va[i] = Val_Str::xdr_unmarshal(&xdrs);    break;
        default: FAIL << "TypeCode: " << t << " not handle\n";       break;
      }
    }   
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

static TuplePtr create_tuple_1() {
  TuplePtr t = Tuple::mk();
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

static Fdbuf encode_uios[MARSHAL_NUM_UIOS];

static void marshal_lots_of_tuples() 
{
  assert( MARSHAL_CHUNK_SZ < TUPLE_TST_SZ);
  for( int c=0; c< MARSHAL_NUM_UIOS; c++) {
    XDR xdrs;
    encode_uios[c].clear();
    xdrfdbuf_create(&xdrs, &encode_uios[c], false, XDR_ENCODE);
    for( int i=0; i< MARSHAL_CHUNK_SZ; i++) {
      ta[i]->xdr_marshal(&xdrs);
    }
  }
}

static void unmarshal_lots_of_tuples() 
{
  assert( MARSHAL_CHUNK_SZ < TUPLE_TST_SZ);
  for( int c=0; c< MARSHAL_NUM_UIOS; c++) {
    XDR xdrs;
    xdrfdbuf_create(&xdrs, &encode_uios[c], false, XDR_DECODE);
    for( int i=0; i< MARSHAL_CHUNK_SZ; i++) {
      ta[i]=Tuple::xdr_unmarshal(&xdrs);
    }
  }
}


static void unit_test_for(Value::TypeCode t ) {
  switch (t) {
    case Value::NULLV:
      std::cout << "NULLV:\ncreating:";
      time_fn(boost::bind(create_lots_val_null));
      std::cout << "marshalling:";
      time_fn(boost::bind(marshal_lots_of_values));
      std::cout << "unmarshalling:";
      time_fn(boost::bind(unmarshal_lots_of_null));  
      std::cout << "\n";
      break;
    case Value::INT32: 
      std::cout << "INT32: creating:"; 
      time_fn(boost::bind(create_lots_val_int32));
      std::cout << "marshalling:";
      time_fn(boost::bind(marshal_lots_of_values));
      std::cout << "unmarshalling:";
      time_fn(boost::bind(unmarshal_lots_of_int32));  
      std::cout << "\n";
      break;
    case Value::UINT64:
      std::cout << "UINT64:\ncreating:";
      time_fn(boost::bind(create_lots_val_uint64));
      std::cout << "marshalling:";
      time_fn(boost::bind(marshal_lots_of_values));
      std::cout << "unmarshalling:";
      time_fn(boost::bind(unmarshal_lots_of_uint64));  
      std::cout << "\n";
      break;
    case Value::DOUBLE:
      std::cout << "DOUBLE:\ncreating:";
      time_fn(boost::bind(create_lots_val_double));
      std::cout << "marshalling:";
      time_fn(boost::bind(marshal_lots_of_values));
      std::cout << "unmarshalling:";
      time_fn(boost::bind(unmarshal_lots_of_double));  
      std::cout << "\n";
      break;
    case Value::STR:
      std::cout << "STR:\ncreating:";
      time_fn(boost::bind(create_lots_val_str));
      std::cout << "marshalling:";
      time_fn(boost::bind(marshal_lots_of_values));
      std::cout << "unmarshalling:";
      time_fn(boost::bind(unmarshal_lots_of_str));  
      std::cout << "\n";
      break;
    case Value::TUPLE:
      std::cout << "TUPLE:\ncreating:";
      time_fn(boost::bind(create_lots_of_tuples));
      std::cout << "marshalling:";
      time_fn(boost::bind(marshal_lots_of_tuples));
      std::cout << "unmarshalling:";
      time_fn(boost::bind(unmarshal_lots_of_tuples));
      break;
    default:
        FAIL << "TypeCode: " << t << " not handle\n";
      break;
  }
}

int main(int argc, char **argv)
{
  TuplePtr t = create_tuple_1();
  XDR xdrs;
  Fdbuf singlet;
  xdrfdbuf_create(&xdrs, &singlet, false, XDR_ENCODE);
  t->xdr_marshal(&xdrs);

  std::cout << " length=" << singlet.length();
  std::cout << " removed=" << singlet.removed();
  // const char *buf = singlet.cstr();
  // size_t sz = singlet.length();
  // string s = hexdump(buf,sz);
  // std::cout << " Hexdump: " << s << "\n";

  unit_test_for(Value::NULLV);
/*
  unit_test_for(Value::INT32);
  unit_test_for(Value::UINT64);
  unit_test_for(Value::DOUBLE);
  unit_test_for(Value::STR);
  unit_test_for(Value::TUPLE);
*/

  return 0;
}


/*
 * End of file 
 */
