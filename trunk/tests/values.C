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

#if HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */
#include <async.h>
#include <arpc.h>
#include <iostream>


#define TEST_VAL(_mkt, _mkv, _mktc, _mktn) \
{ \
  std::cout << "Making Val_" << #_mkt << "(" << #_mkv << ")\n"; \
  ValueRef v = Val_##_mkt::mk(_mkv);  \
  if ( v->typeCode() != Value::_mktc ) { \
    std::cerr << "** Bad typeCode from " #_mkt ", expected " #_mktc " but got " << v->typeCode() << "\n"; \
  } \
  str mktn(_mktn); \
  if (mktn != v->typeName()) { \
    std::cerr << "** Bad typeName from " #_mkt ", expected " #_mktn " but got " << v->typeName() << "\n"; \
  } \
}

#define TEST_CAST(_mkt, _mkv, _castct, _castt, _castv) \
{ \
  std::cout << "Casting Val_" #_mkt "(" #_mkv ") -> " #_castt "\n"; \
  ValueRef v = Val_##_mkt::mk(_mkv);  \
  try { \
    _castct cv = Val_##_castt::cast(v); \
    if ( cv != _castv ) { \
      std::cerr << "** Bad cast value from Val_" #_mkt "(" #_mkv ")->Val_" #_castt \
                << "; expected " #_castv " but got " << cv << "\n"; \
    } \
  } catch (Value::TypeError) { \
    std::cerr << "** Type exception casting Val_" #_mkt "(" #_mkv ")->Val_" #_castt "\n"; \
  } \
}

#define TEST_BADCAST(_mkt, _mkv, _castct, _castt) \
{ \
  std::cout << "Casting bad Val_" #_mkt "(" #_mkv ") -> " #_castt "\n"; \
  ValueRef v = Val_##_mkt::mk(_mkv);  \
  bool succ; \
  try { \
    Val_##_castt::cast(v); \
    succ = true; \
  } catch (Value::TypeError) { \
    succ = false; \
  } \
  if (succ) { \
    std::cerr << "** Didn't get an exception casting Val_" #_mkt "(" #_mkv ")->Val_" #_castt "\n"; \
  } \
}

int main(int argc, char **argv)
{
  std::cout << "VALUES\n";

  // 
  // First, make sure that typecodes and typenames are assigned
  // correctly.
  //
  TEST_VAL( Int32, 0, INT32, "int32" );
  TEST_VAL( Int32, -1, INT32, "int32" );
  TEST_VAL( Int32, 1000, INT32, "int32" );
  TEST_VAL( Int32, -0x6fffffff, INT32, "int32" );
  
  TEST_VAL( Int64, 0, INT64, "int64" );
  TEST_VAL( Int64, -1, INT64, "int64" );
  TEST_VAL( Int64, 1000, INT64, "int64" );
  TEST_VAL( Int64, -0x6fffffff, INT64, "int64" );
  
  TEST_VAL( UInt32, 0, UINT32, "uint32" );
  TEST_VAL( UInt32, 1000, UINT32, "uint32" );
  TEST_VAL( UInt32, 0x6fffffff, UINT32, "uint32" );
  
  TEST_VAL( UInt64, 0, UINT64, "uint64" );
  TEST_VAL( UInt64, 1000, UINT64, "uint64" );
  TEST_VAL( UInt64, 0x6fffffff, UINT64, "uint64" );

  TEST_VAL( Null, , NULLV, "null");
  
  TEST_VAL( Double, 0, DOUBLE, "double");
  TEST_VAL( Double, 0.0, DOUBLE, "double");
  TEST_VAL( Double, 1.2, DOUBLE, "double");

  TEST_VAL( Str, "", STR, "str");
  TEST_VAL( Str, "This is a string", STR, "str");

  //
  // Test trivial (T->T) cast operations
  //
  TEST_CAST( Int32, 0, int32_t, Int32, 0 );
  TEST_CAST( Int32, -1, int32_t, Int32, -1 );
  TEST_CAST( Int32, 1, int32_t, Int32, 1 );
  
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

  TEST_CAST( Str, "", str, Str, "");
  TEST_CAST( Str, "This is a string", str, Str, "This is a string");

  TEST_BADCAST( Int32, 1, int, Null );

//

  return 0;
}

/*
 * End of file 
 */
