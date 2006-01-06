/*
 * @(#)$Id$
 *
 * Copyright (c) 2005 Intel Corporation. All rights reserved.
 *
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 * DESCRIPTION: Test suite for values
 *
 */

#include "value.h"

#include "val_int32.h"
#include "val_uint32.h"
#include "val_int64.h"
#include "val_uint64.h"
#include "val_double.h"
#include "val_str.h"
#include "val_null.h"
#include "val_opaque.h"
#include "val_time.h"
#include "val_ip_addr.h"
#include "val_id.h"
#include "val_tuple.h"
#include "testerr.h"
#include "p2Time.h"

#if HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */
#include <iostream>
#include <limits.h>

#define TEST_VAL(_mkt, _mkv, _mktc, _mktn) \
{ \
  std::cout << "Making Val_" << #_mkt << "(" << #_mkv << ")\n"; \
  ValuePtr v = Val_##_mkt::mk(_mkv);  \
  if ( v->typeCode() != Value::_mktc ) { \
    FAIL << "Bad typeCode from " #_mkt ", expected " #_mktc " but got " << v->typeCode() << "\n"; \
  } \
  string mktn(_mktn); \
  if (mktn != v->typeName()) { \
    FAIL << "Bad typeName from " #_mkt ", expected " #_mktn " but got " << v->typeName() << "\n"; \
  } \
}

#define TEST_TIMEVAL(_mkt, _mks, _mkns, _mktc, _mktn) \
{ \
  std::cout << "Making Val_" << #_mkt << "(" << #_mks << "," << #_mkns << ")\n"; \
  struct timespec t; \
  t.tv_sec = _mks; \
  t.tv_nsec = _mkns; \
  ValuePtr v = Val_##_mkt::mk(t); \
  if ( v->typeCode() != Value::TIME ) { \
    FAIL << "Bad typeCode from " #_mkt ", expected " #_mktc " but got " << v->typeCode() << "\n"; \
  } \
  string mktn(_mktn); \
  if (mktn != v->typeName()) { \
    FAIL << "Bad typeName from " #_mkt ", expected " #_mktn " but got " << v->typeName() << "\n"; \
  } \
}

#define TEST_CAST(_mkt, _mkv, _castct, _castt, _castv) \
{ \
  std::cout << "Casting Val_" #_mkt "(" #_mkv ") -> " #_castt "\n"; \
  ValuePtr v = Val_##_mkt::mk(_mkv);  \
  try { \
    _castct cv = Val_##_castt::cast(v); \
    if ( cv != _castv ) { \
      FAIL << "Bad cast value from Val_" #_mkt "(" #_mkv ")->Val_" #_castt \
                << "; expected " #_castv " but got " << cv << "\n"; \
    } \
  } catch (Value::TypeError) { \
    FAIL << "Type exception casting Val_" #_mkt "(" #_mkv ")->Val_" #_castt "\n"; \
  } \
}

#define TEST_TIMEVAL_CAST(_mkt, _mkv, _castt, _casts, _castns) \
{ \
  struct timespec t; \
  t.tv_sec = _casts; \
  t.tv_nsec = _castns; \
  std::cout << "Casting Val_" #_mkt "(" #_mkv ") -> " #_castt "\n"; \
  ValuePtr v = Val_##_mkt::mk(_mkv);  \
  try { \
    struct timespec cv = Val_##_castt::cast(v); \
    if ( compare_timespec(cv, t) ) { \
      FAIL << "Bad cast value from Val_" #_mkt "(" #_mkv ")->Val_" #_castt \
                << "; expected " << t.tv_sec << "s " << t.tv_nsec << "ns " \
                << " but got " << cv.tv_sec << "s " << cv.tv_nsec << "ns" \
                << "\n"; \
    } \
  } catch (Value::TypeError) { \
    FAIL << "Type exception casting Val_" #_mkt "(" #_mkv ")->Val_" #_castt "\n"; \
  } \
}

#define TEST_ID_CAST(_mkt, _mkv, _castct, _castt, _castv) \
{ \
  IDPtr i = ID::mk(Val_##_castt::cast(Val_##_mkt::mk(_castv))); \
  std::cout << "Casting Val_" #_mkt "(" #_mkv ") -> " #_castct "\n"; \
  ValuePtr v = Val_##_mkt::mk(_mkv);  \
  try { \
    IDPtr cv = Val_ID::cast(v); \
    if ( cv->compareTo(i) ) { \
      FAIL << "Bad cast value from Val_" #_mkt "(" #_mkv ")->Val_" #_castt \
                << "; expected " << i->toString() << " but got " \
                << cv->toString() << "\n"; \
    } \
  } catch (Value::TypeError) { \
    FAIL << "Type exception casting Val_" #_mkt "(" #_mkv ")->Val_" #_castt "\n"; \
  } \
}


#define TEST_BADCAST(_mkt, _mkv, _castct, _castt) \
{ \
  std::cout << "Testing bad cast Val_" #_mkt "(" #_mkv ") -> " #_castt "\n"; \
  ValuePtr v = Val_##_mkt::mk(_mkv);  \
  bool succ; \
  try { \
    Val_##_castt::cast(v); \
    succ = true; \
  } catch (Value::TypeError) { \
    succ = false; \
  } \
  if (succ) { \
    FAIL << "Didn't get an exception casting Val_" #_mkt "(" #_mkv ")->Val_" #_castt "\n"; \
  } \
}

int main(int argc, char **argv)
{
  std::cout << "VALUES\n";

  FdbufPtr u1(new Fdbuf());
  u1->push_back("This is UIO 1");
  FdbufPtr u2(new Fdbuf());
  u2->push_back("This is UIO 2");
  FdbufPtr u3(new Fdbuf());
  u3->push_back("This is UIO 3"); 

  string addr = "127.0.0.1:1000";

  // 
  // First, make sure that typecodes and typenames are assigned
  // correctly.
  //
  TEST_TIMEVAL( Time,  0,  0, TIME, "time");
  TEST_TIMEVAL( Time, 10, 10, TIME, "time");
  TEST_TIMEVAL( Time, -1,  0, TIME, "time");
  TEST_TIMEVAL( Time,  0, -1, TIME, "time");
  TEST_TIMEVAL( Time, -2, -2, TIME, "time");

  TEST_VAL( Null, , NULLV, "null");
  
  TEST_VAL( Int32, 0, INT32, "int32" );
  TEST_VAL( Int32, 1, INT32, "int32" );
  TEST_VAL( Int32, 2000, INT32, "int32" );
  TEST_VAL( Int32, INT_MAX, INT32, "int32" );
  TEST_VAL( Int32, -1, INT32, "int32" );
  TEST_VAL( Int32, -2000, INT32, "int32" );
  TEST_VAL( Int32, INT_MIN, INT32, "int32" );

  TEST_VAL( Int64, 0, INT64, "int64" );
  TEST_VAL( Int64, 1, INT64, "int64" );
  TEST_VAL( Int64, 2000, INT64, "int64" );
  TEST_VAL( Int64, LONG_LONG_MAX, INT64, "int64" );
  TEST_VAL( Int64, -1, INT64, "int64" );
  TEST_VAL( Int64, -2000, INT64, "int64" );
  TEST_VAL( Int64, LONG_LONG_MIN, INT64, "int64" );
  
  TEST_VAL( UInt32, 0, UINT32, "uint32" );
  TEST_VAL( UInt32, 1, UINT32, "uint32" );
  TEST_VAL( UInt32, 1000, UINT32, "uint32" );
  TEST_VAL( UInt32, UINT_MAX, UINT32, "uint32" );

  TEST_VAL( UInt64, 0, UINT64, "uint64" );
  TEST_VAL( UInt64, 1, UINT64, "uint64" );
  TEST_VAL( UInt64, 1000, UINT64, "uint64" );
  TEST_VAL( UInt64, UINT_MAX, UINT64, "uint64" );

  TEST_VAL( Double, 0, DOUBLE, "double");
  TEST_VAL( Double, 1.0, DOUBLE, "double");
  TEST_VAL( Double, -1.0, DOUBLE, "double");
  TEST_VAL( Double, -1.79769E+308, DOUBLE, "double");
  TEST_VAL( Double, 1.79769E+308, DOUBLE, "double");
  TEST_VAL( Double, 2.225E-307, DOUBLE, "double");
  TEST_VAL( Double, -2.225E-307, DOUBLE, "double");

  TEST_VAL( Str, "", STR, "str");
  TEST_VAL( Str, "This is a string", STR, "str");

  TEST_VAL( Opaque, u1, OPAQUE, "opaque");

  TEST_VAL( IP_ADDR, addr, IP_ADDR, "ip_addr"); 
 

  //
  // Test trivial (T->T) cast operations
  //
  
  TEST_CAST( UInt32, 0, uint32_t, UInt32, 0 );
  TEST_CAST( UInt32, 1, uint32_t, UInt32, 1 );
  
  TEST_CAST( Int64, 0, int64_t, Int64, 0 );
  TEST_CAST( Int64, -1, int64_t, Int64, -1 );
  TEST_CAST( Int64, 1, int64_t, Int64, 1 );
  
  TEST_CAST( UInt64, 0, uint64_t, UInt64, 0 );
  TEST_CAST( UInt64, 1, uint64_t, UInt64, 1 );
  
  TEST_CAST( Double, 0, double, Double, 0);
  TEST_CAST( Double, 0.0, double, Double, 0.0);
  TEST_CAST( Double, 1.2, double, Double, 1.2);

  TEST_CAST( Str, "", string, Str, "");
  TEST_CAST( Str, "This is a string", string, Str, "This is a string");

  TEST_CAST( Opaque, u1, FdbufPtr, Opaque, u1);

  TEST_CAST( IP_ADDR, addr, string, IP_ADDR, addr);

  // 
  //Test casting to NULL.
  //
  //TEST_CAST( Null, , int, Null, 0);
  { 
    std::cout << "Casting Val_Null() -> Null\n";
    ValuePtr v = Val_Null::mk();
    try {
      Val_Null::cast(v);
    } catch (Value::TypeError) { 
      std::cerr << "** Type exception casting Val_Null()->Val_Null\n"; 
    } 
  }
  TEST_BADCAST( Int32, -1, int, Null );
  TEST_BADCAST( UInt32, 1, int, Null );
  TEST_BADCAST( Int64, -1, int, Null );
  TEST_BADCAST( UInt64, 1, int, Null );
  TEST_BADCAST( Double, 1.0, int, Null );
  TEST_BADCAST( Str, "", int, Null );
  TEST_BADCAST( Str, "Hello", int, Null );
  TEST_BADCAST( Opaque, u2, int, Null );
  TEST_BADCAST( IP_ADDR, addr, int, Null);

  // Test casting to int32.
  #undef TEST_CAST_T
  #define TEST_CAST_T(_t,_v,_r) TEST_CAST(_t,_v,int32_t,Int32,_r)

  TEST_CAST_T( Null, , 0);

  TEST_CAST_T( Int32, 0, 0);
  TEST_CAST_T( Int32, 1, 1);
  TEST_CAST_T( Int32, 2000, 2000);
  TEST_CAST_T( Int32, INT_MAX, INT_MAX );
  TEST_CAST_T( Int32, -1, -1 );
  TEST_CAST_T( Int32, -2000, -2000);
  TEST_CAST_T( Int32, INT_MIN, INT_MIN);

  TEST_CAST_T( Int64, 0, 0 );
  TEST_CAST_T( Int64, 1, 1 );
  TEST_CAST_T( Int64, 2000, 2000 );
  TEST_CAST_T( Int64, LONG_LONG_MAX, -1 );
  TEST_CAST_T( Int64, -1, -1 );
  TEST_CAST_T( Int64, -2000, -2000 );
  TEST_CAST_T( Int64, LONG_LONG_MIN, 0 );
  
  TEST_CAST_T( UInt32, 0, 0 );
  TEST_CAST_T( UInt32, 1, 1 );
  TEST_CAST_T( UInt32, 1000, 1000 );
  TEST_CAST_T( UInt32, UINT_MAX, -1 );

  TEST_CAST_T( UInt64, 0, 0 );
  TEST_CAST_T( UInt64, 1, 1 );
  TEST_CAST_T( UInt64, 1000, 1000 );
  TEST_CAST_T( UInt64, ULONG_LONG_MAX, -1 );

  TEST_CAST_T( Double, 0, 0 );
  TEST_CAST_T( Double, 1.0, 1 );
  TEST_CAST_T( Double, -1.0, -1 );
  TEST_CAST_T( Double, -1.79769E+308, INT_MIN );
  TEST_CAST_T( Double, 1.79769E+308, INT_MIN );
  TEST_CAST_T( Double, 2.225E-307, 0 );
  TEST_CAST_T( Double, -2.225E-307, 0 );
  
  TEST_CAST_T( Str, "", 0 );
  TEST_CAST_T( Str, "0", 0 );
  TEST_CAST_T( Str, "1", 1 );
  TEST_CAST_T( Str, "0x1a", 26 );
  TEST_CAST_T( Str, "011", 9 );
  TEST_CAST_T( Str, "-200", -200 );
  TEST_CAST_T( Str, "1.5", 1 );
  TEST_CAST_T( Str, "-1.5", -1 );
  TEST_CAST_T( Str, "Rubbish", 0 );
  
  TEST_BADCAST( Opaque, u2, int32_t, Int32 );
  TEST_BADCAST( IP_ADDR, addr, int32_t, Int32);


  // Test casting to uint32.
  #undef TEST_CAST_T
  #define TEST_CAST_T(_t,_v,_r) TEST_CAST(_t,_v,uint32_t,UInt32,_r)

  TEST_CAST_T( Null, , 0);

  TEST_CAST_T( Int32, 0, 0);
  TEST_CAST_T( Int32, 1, 1);
  TEST_CAST_T( Int32, 2000, 2000);
  TEST_CAST_T( Int32, INT_MAX, INT_MAX );
  TEST_CAST_T( Int32, -1, UINT_MAX );
  TEST_CAST_T( Int32, -2000, UINT_MAX -2000 + 1);
  TEST_CAST_T( Int32, INT_MIN, INT_MAX+1U);

  TEST_CAST_T( Int64, 0, 0 );
  TEST_CAST_T( Int64, 1, 1 );
  TEST_CAST_T( Int64, 2000, 2000 );
  TEST_CAST_T( Int64, LONG_LONG_MAX, UINT_MAX );
  TEST_CAST_T( Int64, -1, UINT_MAX );
  TEST_CAST_T( Int64, -2000, UINT_MAX-2000 +1);
  TEST_CAST_T( Int64, LONG_LONG_MIN, 0 );
  
  TEST_CAST_T( UInt32, 0, 0 );
  TEST_CAST_T( UInt32, 1, 1 );
  TEST_CAST_T( UInt32, 1000, 1000 );
  TEST_CAST_T( UInt32, UINT_MAX, UINT_MAX );

  TEST_CAST_T( UInt64, 0, 0 );
  TEST_CAST_T( UInt64, 1, 1 );
  TEST_CAST_T( UInt64, 1000, 1000 );
  TEST_CAST_T( UInt64, ULONG_LONG_MAX, UINT_MAX );

  TEST_CAST_T( Double, 0, 0 );
  TEST_CAST_T( Double, 1.0, 1 );
  TEST_CAST_T( Double, -1.0, UINT_MAX );
  TEST_CAST_T( Double, -1.79769E+308, 0 );
  TEST_CAST_T( Double, 1.79769E+308, 0 );
  TEST_CAST_T( Double, 2.225E-307, 0 );
  TEST_CAST_T( Double, -2.225E-307, 0 );
  
  TEST_CAST_T( Str, "", 0 );
  TEST_CAST_T( Str, "0", 0 );
  TEST_CAST_T( Str, "1", 1 );
  TEST_CAST_T( Str, "0x1a", 26 );
  TEST_CAST_T( Str, "011", 9 );
  TEST_CAST_T( Str, "-200", UINT_MAX-200 +1 );
  TEST_CAST_T( Str, "1.5", 1 );
  TEST_CAST_T( Str, "-1.5", UINT_MAX );
  TEST_CAST_T( Str, "Rubbish", 0 );
  
  TEST_BADCAST( Opaque, u2, uint32_t, UInt32 );
  TEST_BADCAST( IP_ADDR, addr, IP_ADDR, UInt32);



  // Test casting to int64.
  #undef TEST_CAST_T
  #define TEST_CAST_T(_t,_v,_r) TEST_CAST(_t,_v,int64_t,Int64,_r)

  TEST_CAST_T( Null, , 0);

  TEST_CAST_T( Int32, 0, 0);
  TEST_CAST_T( Int32, 1, 1);
  TEST_CAST_T( Int32, 2000, 2000);
  TEST_CAST_T( Int32, INT_MAX, INT_MAX );
  TEST_CAST_T( Int32, -1, -1 );
  TEST_CAST_T( Int32, -2000, -2000);
  TEST_CAST_T( Int32, INT_MIN, INT_MIN);

  TEST_CAST_T( Int64, 0, 0 );
  TEST_CAST_T( Int64, 1, 1 );
  TEST_CAST_T( Int64, 2000, 2000 );
  TEST_CAST_T( Int64, LONG_LONG_MAX, LONG_LONG_MAX );
  TEST_CAST_T( Int64, -1, -1 );
  TEST_CAST_T( Int64, -2000, -2000 );
  TEST_CAST_T( Int64, LONG_LONG_MIN, LONG_LONG_MIN );
  
  TEST_CAST_T( UInt32, 0, 0 );
  TEST_CAST_T( UInt32, 1, 1 );
  TEST_CAST_T( UInt32, 1000, 1000 );
  TEST_CAST_T( UInt32, UINT_MAX, UINT_MAX );

  TEST_CAST_T( UInt64, 0, 0 );
  TEST_CAST_T( UInt64, 1, 1 );
  TEST_CAST_T( UInt64, 1000, 1000 );
  TEST_CAST_T( UInt64, ULONG_LONG_MAX, -1 );

  TEST_CAST_T( Double, 0, 0 );
  TEST_CAST_T( Double, 1.0, 1 );
  TEST_CAST_T( Double, -1.0, -1 );
  TEST_CAST_T( Double, -1.79769E+308, LONG_LONG_MIN );
  TEST_CAST_T( Double, 1.79769E+308, LONG_LONG_MIN );
  TEST_CAST_T( Double, 2.225E-307, 0 );
  TEST_CAST_T( Double, -2.225E-307, 0 );
  
  TEST_CAST_T( Str, "", 0 );
  TEST_CAST_T( Str, "0", 0 );
  TEST_CAST_T( Str, "1", 1 );
  TEST_CAST_T( Str, "0x1a", 26 );
  TEST_CAST_T( Str, "011", 9 );
  TEST_CAST_T( Str, "-200", -200 );
  TEST_CAST_T( Str, "1.5", 1 );
  TEST_CAST_T( Str, "-1.5", -1 );
  TEST_CAST_T( Str, "Rubbish", 0 );
  
  TEST_BADCAST( Opaque, u2, int64_t, Int64 );
  TEST_BADCAST( IP_ADDR, addr, int64_t, Int64);


  // Test casting to uint64.
  #undef TEST_CAST_T
  #define TEST_CAST_T(_t,_v,_r) TEST_CAST(_t,_v,uint64_t,UInt64,_r)

  TEST_CAST_T( Null, , 0);

  TEST_CAST_T( Int32, 0, 0);
  TEST_CAST_T( Int32, 1, 1);
  TEST_CAST_T( Int32, 2000, 2000);
  TEST_CAST_T( Int32, INT_MAX, INT_MAX );
  TEST_CAST_T( Int32, -1, ULONG_LONG_MAX );
  TEST_CAST_T( Int32, -2000, ULONG_LONG_MAX -2000 + 1);
  TEST_CAST_T( Int32, INT_MIN, ULONG_LONG_MAX - INT_MAX);

  TEST_CAST_T( Int64, 0, 0 );
  TEST_CAST_T( Int64, 1, 1 );
  TEST_CAST_T( Int64, 2000, 2000 );
  TEST_CAST_T( Int64, LONG_LONG_MAX, LONG_LONG_MAX );
  TEST_CAST_T( Int64, -1, ULONG_LONG_MAX );
  TEST_CAST_T( Int64, -2000, ULONG_LONG_MAX-2000 +1);
  TEST_CAST_T( Int64, LONG_LONG_MIN, 1UL + (uint64_t)(LONG_LONG_MAX) );
  
  TEST_CAST_T( UInt32, 0, 0 );
  TEST_CAST_T( UInt32, 1, 1 );
  TEST_CAST_T( UInt32, 1000, 1000 );
  TEST_CAST_T( UInt32, UINT_MAX, UINT_MAX );

  TEST_CAST_T( UInt64, 0, 0 );
  TEST_CAST_T( UInt64, 1, 1 );
  TEST_CAST_T( UInt64, 1000, 1000 );
  TEST_CAST_T( UInt64, ULONG_LONG_MAX, ULONG_LONG_MAX );

  TEST_CAST_T( Double, 0, 0 );
  TEST_CAST_T( Double, 1.0, 1 );
  TEST_CAST_T( Double, -1.0, UINT_MAX );
  TEST_CAST_T( Double, -1.79769E+308, 0 );
  TEST_CAST_T( Double, 1.79769E+308, 0 );
  TEST_CAST_T( Double, 2.225E-307, 0 );
  TEST_CAST_T( Double, -2.225E-307, 0 );
  
  TEST_CAST_T( Str, "", 0 );
  TEST_CAST_T( Str, "0", 0 );
  TEST_CAST_T( Str, "1", 1 );
  TEST_CAST_T( Str, "0x1a", 26 );
  TEST_CAST_T( Str, "011", 9 );
  TEST_CAST_T( Str, "-200", ULONG_LONG_MAX-200 +1 );
  TEST_CAST_T( Str, "1.5", 1 );
  TEST_CAST_T( Str, "-1.5", ULONG_LONG_MAX );
  TEST_CAST_T( Str, "Rubbish", 0 );
  
  TEST_BADCAST( Opaque, u2, uint64_t, UInt64 );
  TEST_BADCAST( IP_ADDR, addr, int64_t, Int64);

  // Test casting to double.
  #undef TEST_CAST_T
  #define TEST_CAST_T(_t,_v,_r) TEST_CAST(_t,_v,double,Double,_r)

  TEST_CAST_T( Null, , 0);

  TEST_CAST_T( Int32, 0, 0);
  TEST_CAST_T( Int32, 1, 1);
  TEST_CAST_T( Int32, 2000, 2000);
  TEST_CAST_T( Int32, INT_MAX, INT_MAX );
  TEST_CAST_T( Int32, -1, -1 );
  TEST_CAST_T( Int32, -2000, -2000);
  TEST_CAST_T( Int32, INT_MIN, INT_MIN);

  TEST_CAST_T( Int64, 0, 0 );
  TEST_CAST_T( Int64, 1, 1 );
  TEST_CAST_T( Int64, 2000, 2000 );
  TEST_CAST_T( Int64, LONG_LONG_MAX, LONG_LONG_MAX );
  TEST_CAST_T( Int64, -1, -1 );
  TEST_CAST_T( Int64, -2000, -2000);
  TEST_CAST_T( Int64, LONG_LONG_MIN, LONG_LONG_MIN );
  
  TEST_CAST_T( UInt32, 0, 0 );
  TEST_CAST_T( UInt32, 1, 1 );
  TEST_CAST_T( UInt32, 1000, 1000 );
  TEST_CAST_T( UInt32, UINT_MAX, UINT_MAX );

  TEST_CAST_T( UInt64, 0, 0 );
  TEST_CAST_T( UInt64, 1, 1 );
  TEST_CAST_T( UInt64, 1000, 1000 );
  TEST_CAST_T( UInt64, ULONG_LONG_MAX, ULONG_LONG_MAX );

  TEST_CAST_T( Double, 0, 0 );
  TEST_CAST_T( Double, 1.0, 1 );
  TEST_CAST_T( Double, -1.0, -1.0 );
  TEST_CAST_T( Double, -1.79769E+308,  -1.79769E+308 );
  TEST_CAST_T( Double, 1.79769E+308, 1.79769E+308 );
  TEST_CAST_T( Double, 2.225E-307, 2.225E-307 );
  TEST_CAST_T( Double, -2.225E-307, -2.225E-307 );
  
  TEST_CAST_T( Str, "", 0 );
  TEST_CAST_T( Str, "0", 0 );
  TEST_CAST_T( Str, "1", 1 );
  TEST_CAST_T( Str, "0x1a", 26 );
  TEST_CAST_T( Str, "011", 11 );
  TEST_CAST_T( Str, "-200", -200 );
  TEST_CAST_T( Str, "1.5", 1.5 );
  TEST_CAST_T( Str, "-1.5", -1.5 );
  TEST_CAST_T( Str, "Rubbish", 0 );
  
  TEST_BADCAST( Opaque, u2, double, Double );
  TEST_BADCAST( IP_ADDR, addr, int64_t, Int64);



  // Test casting to Str.
  #undef TEST_CAST_T
  #define TEST_CAST_T(_t,_v,_r) TEST_CAST(_t,_v,string,Str,_r)

  TEST_CAST_T( Null, , "NULL");

  TEST_CAST_T( Int32, 0, "0");
  TEST_CAST_T( Int32, 1, "1");
  TEST_CAST_T( Int32, 2000, "2000");
  TEST_CAST_T( Int32, INT_MAX, "2147483647" );
  TEST_CAST_T( Int32, -1, "-1" );
  TEST_CAST_T( Int32, -2000, "-2000");
  TEST_CAST_T( Int32, INT_MIN, "-2147483648");

  TEST_CAST_T( Int64, 0, "0" );
  TEST_CAST_T( Int64, 1, "1" );
  TEST_CAST_T( Int64, 2000, "2000" );
  TEST_CAST_T( Int64, LONG_LONG_MAX, "9223372036854775807" );
  TEST_CAST_T( Int64, -1, "-1" );
  TEST_CAST_T( Int64, -2000, "-2000");
  TEST_CAST_T( Int64, LONG_LONG_MIN, "-9223372036854775808" );
  
  TEST_CAST_T( UInt32, 0, "0" );
  TEST_CAST_T( UInt32, 1, "1" );
  TEST_CAST_T( UInt32, 1000, "1000" );
  TEST_CAST_T( UInt32, UINT_MAX, "4294967295" );

  TEST_CAST_T( UInt64, 0, "0" );
  TEST_CAST_T( UInt64, 1, "1" );
  TEST_CAST_T( UInt64, 1000, "1000" );
  TEST_CAST_T( UInt64, ULONG_LONG_MAX, "18446744073709551615" );

  TEST_CAST_T( Double, 0, "0" );
  TEST_CAST_T( Double, 1.0, "1" );
  TEST_CAST_T( Double, -1.0, "-1" );
  TEST_CAST_T( Double, -1.79769E+308,  "-1.79769e+308" );
  TEST_CAST_T( Double, 1.79769E+308, "1.79769e+308" );
  TEST_CAST_T( Double, 2.225E-307, "2.225e-307" );
  TEST_CAST_T( Double, -2.225E-307, "-2.225e-307" );
  
  TEST_CAST_T( Str, "", "" );
  TEST_CAST_T( Str, "0", "0" );
  TEST_CAST_T( Str, "1", "1" );
  TEST_CAST_T( Str, "0x1a", "0x1a" );
  TEST_CAST_T( Str, "011", "011" );
  TEST_CAST_T( Str, "-200", "-200" );
  TEST_CAST_T( Str, "1.5", "1.5" );
  TEST_CAST_T( Str, "-1.5", "-1.5" );
  TEST_CAST_T( Str, "Rubbish", "Rubbish" );
  
  TEST_CAST_T( Opaque, u2, "This is UIO 2" );
  TEST_CAST_T( IP_ADDR, addr, "127.0.0.1:1000");


  // Test casting to Opaque.
  #undef TEST_CAST_T
  #define TEST_CAST_T(_t,_v,_r) TEST_CAST(_t,_v,FdbufPtr,Opaque,_r)
  #define TEST_BADCAST_T(_t,_v) TEST_BADCAST(_t,_v,FdbufPtr,Opaque)

  TEST_BADCAST_T( Null, );

  TEST_BADCAST_T( Int32, 0 );
  TEST_BADCAST_T( UInt64, 0 );
  TEST_BADCAST_T( Int32, 0 );
  TEST_BADCAST_T( UInt64, 0 );
  TEST_BADCAST_T( Double, 0 );
  TEST_BADCAST_T( IP_ADDR, addr); 
#if 0
  TEST_CAST_T( Str, "", "" );
  TEST_CAST_T( Str, "0", "0" );
  TEST_CAST_T( Str, "1", "1" );
  TEST_CAST_T( Str, "0x1a", "0x1a" );
  TEST_CAST_T( Str, "011", "011" );
  TEST_CAST_T( Str, "-200", "-200" );
  TEST_CAST_T( Str, "1.5", "1.5" );
  TEST_CAST_T( Str, "-1.5", "-1.5" );
  TEST_CAST_T( Str, "Rubbish", "Rubbish" );
#endif  
  TEST_CAST_T( Opaque, u2, u2 );

// Test casting to IP_ADDR type.

  #undef TEST_CAST_T
  #define TEST_CAST_T(_t,_v,_r) TEST_CAST(_t, _v, string, IP_ADDR, _r)
  #undef TEST_BADCAST_T
  #define TEST_BADCAST_T(_t,_v) TEST_BADCAST(_t, _v, string, IP_ADDR)

  TEST_CAST_T( Str, "", "" );
  TEST_CAST_T( Str, "0", "0" );
  TEST_CAST_T( Str, "1", "1" );
  TEST_CAST_T( Str, "0x1a", "0x1a" );
  TEST_CAST_T( Str, "011", "011" );
  TEST_CAST_T( Str, "-200", "-200" );
  TEST_CAST_T( Str, "1.5", "1.5" );
  TEST_CAST_T( Str, "-1.5", "-1.5" );
  TEST_CAST_T( Str, "Rubbish", "Rubbish" );


  TEST_BADCAST_T( Null, );
  TEST_BADCAST_T( Int32, 0 );
  TEST_BADCAST_T( UInt64, 0 );
  TEST_BADCAST_T( Int32, 0 );
  TEST_BADCAST_T( UInt64, 0 );
  TEST_BADCAST_T( Double, 0 );
  TEST_BADCAST_T( Opaque, u2);


  // Test casting to Time, time_t.tv_sec is of type int32
  #undef TEST_CAST_T
  #define TEST_CAST_T(_t,_v,_s,_ns) TEST_TIMEVAL_CAST(_t,_v,Time,_s,_ns)

  TEST_CAST_T( Null, , 0, 0 );

  TEST_CAST_T( Int32, 0, 0, 0 );
  TEST_CAST_T( Int32, 1, 1, 0 );
  TEST_CAST_T( Int32, 2000, 2000, 0 );
  TEST_CAST_T( Int32, INT_MAX, INT_MAX, 0 );
  TEST_CAST_T( Int32, -1, -1, 0 );
  TEST_CAST_T( Int32, -2000, -2000, 0 );
  TEST_CAST_T( Int32, INT_MIN, INT_MIN, 0 );

  TEST_CAST_T( Int64, 0, 0, 0 );
  TEST_CAST_T( Int64, 1, 1, 0 );
  TEST_CAST_T( Int64, 2000, 2000, 0 );
  TEST_CAST_T( Int64, LONG_LONG_MAX, -1, 0 );
  TEST_CAST_T( Int64, -1, -1, 0 );
  TEST_CAST_T( Int64, -2000, -2000, 0 );
  TEST_CAST_T( Int64, LONG_LONG_MIN, 0, 0 );
  
  TEST_CAST_T( UInt32, 0, 0, 0 );
  TEST_CAST_T( UInt32, 1, 1, 0 );
  TEST_CAST_T( UInt32, 1000, 1000, 0 );
  TEST_CAST_T( UInt32, UINT_MAX, -1, 0 );

  TEST_CAST_T( UInt64, 0, 0, 0 );
  TEST_CAST_T( UInt64, 1, 1, 0 );
  TEST_CAST_T( UInt64, 1000, 1000, 0 );
  TEST_CAST_T( UInt64, ULONG_LONG_MAX, -1, 0 );

  TEST_CAST_T( Double, 0, 0, 0 );
  TEST_CAST_T( Double, 1.0, 1, 0 );
  TEST_CAST_T( Double, -1.0, -1, 0 );
  TEST_CAST_T( Double, -1.79769E+308, INT_MIN, 0 );
  TEST_CAST_T( Double, 1.79769E+308, INT_MIN, 0 );
  TEST_CAST_T( Double, 2.225E-307, 0, 0 );
  TEST_CAST_T( Double, -2.225E-307, 0, 0 );
  
  // Test casting to ID.
  #undef TEST_CAST_T
  #define TEST_CAST_T(_t,_v,_c,_r) TEST_ID_CAST(_t,_v,ID,_c,_r)

  TEST_CAST_T( Int32, INT_MAX, UInt32, INT_MAX );
  TEST_CAST_T( Int32, -1, UInt32, ULONG_LONG_MAX );
  TEST_CAST_T( Int32, -2000, UInt32, ULONG_LONG_MAX -2000 + 1 );
  TEST_CAST_T( Int32, INT_MIN, UInt32, ULONG_LONG_MAX - INT_MAX );

  TEST_CAST_T( Int64, 0, UInt64, 0 );
  TEST_CAST_T( Int64, 1, UInt64, 1 );
  TEST_CAST_T( Int64, 2000, UInt64, 2000 );
  TEST_CAST_T( Int64, LONG_LONG_MAX, UInt64, LONG_LONG_MAX );
  TEST_CAST_T( Int64, -1, UInt64, ULONG_LONG_MAX );
  TEST_CAST_T( Int64, -2000, UInt64, ULONG_LONG_MAX-2000 +1 );
  TEST_CAST_T( Int64, LONG_LONG_MIN, UInt64, 1UL + (uint64_t)(LONG_LONG_MAX) );
  
  TEST_CAST_T( UInt32, 0, UInt32, 0 );
  TEST_CAST_T( UInt32, 1, UInt32, 1 );
  TEST_CAST_T( UInt32, 1000, UInt32, 1000 );
  TEST_CAST_T( UInt32, UINT_MAX, UInt32, UINT_MAX );

  TEST_CAST_T( UInt64, 0, UInt64, 0 );
  TEST_CAST_T( UInt64, 1, UInt64, 1 );
  TEST_CAST_T( UInt64, 1000, UInt64, 1000 );
  TEST_CAST_T( UInt64, ULONG_LONG_MAX, UInt64, ULONG_LONG_MAX );

  // Test casting to Tuple
  #undef TEST_BADCAST_T
  #define TEST_BADCAST_T(_t,_v) TEST_BADCAST(_t,_v,tuple,Tuple)

  TEST_BADCAST_T( Null, );

  TEST_BADCAST_T( Int32, 0 );
  TEST_BADCAST_T( UInt64, 0 );
  TEST_BADCAST_T( Int32, 0 );
  TEST_BADCAST_T( UInt64, 0 );
  TEST_BADCAST_T( Double, 0 );
  
  std::cout.flush();
  std::cerr.flush();
  
  return 0;
}

/*
 * Representative values:
 * int32_t:  0, 1, 2000, INT_MAX, -1, -2000, INT_MIN
 * uint32_t: 0, 1, 2000, UINT_MAX
 * int64_t: 0, 1, 3000, LONG_LONG_MAX, -1, -3000, LONG_LONG_MIN
 * uint64_t: 0, 1, 3000, ULONG_LONG_MAX
 * double: 0, 1.0, -1.0, -1.79769E+308, 1.79769E+308, 2.225E-307, -2.225E-307
 * 
 * For IEEE784 Double Precision
 * Smallest DOUBLE value: -1.79769E+308
 * Largest DOUBLE value: 1.79769E+308
 * Smallest positive DOUBLE value: 2.225E-307
 * Largest negative DOUBLE value: -2.225E-307
 */
 

/*
 * End of file 
 */
