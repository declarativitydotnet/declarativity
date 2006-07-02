/* 
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * Or
 * UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
 * Berkeley, CA,  94707. Attention: P2 Group.
 */

#include "boost/test/unit_test.hpp"
#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost/date_time/gregorian/gregorian.hpp"

#include <sstream>

#include "value.h"
#include "val_null.h"
#include "val_str.h"
#include "val_int32.h"
#include "val_uint32.h"
#include "val_int64.h"
#include "val_uint64.h"
#include "val_double.h"
#include "val_opaque.h"
#include "val_tuple.h"
#include "val_time.h"
#include "val_id.h"
#include "val_ip_addr.h"
#include "oper.h"
#include "val_int32.h"
#include "val_str.h"
#include "val_list.h"
#include "val_vector.h"

#include "testValues.h"

using namespace opr;


class testValues
{
public:
  testValues()
  {
  }


  ////////////////////////////////////////////////////////////
  // Value construction tests
  ////////////////////////////////////////////////////////////

public:
#define TEST_VAL(valTypeExt, value, typecode, typestr)  \
  { \
    std::string testID; \
    { \
      std::ostringstream ID; \
      ID << "Value construction test. "; \
      testID = ID.str(); \
    } \
     \
     \
    ValuePtr v = Val_##valTypeExt::mk(value); \
    { \
      std::ostringstream message; \
      message << testID \
              << "Bad typeCode after construction of '" \
              << typestr \
              << "'. Expected '" \
              << Value::typecode \
              << "' but got '" \
              << v->typeCode() \
              << "'."; \
      BOOST_CHECK_MESSAGE(v->typeCode() == Value::typecode,     \
                          message.str().c_str());               \
    } \
     \
    string mktn(typestr); \
    { \
      std::ostringstream message; \
      message << testID \
              << "Bad typeName after construction of '" \
              << typestr \
              << "'. Expected '" \
              << typestr \
              << "' but got '" \
              << v->typeName() \
              << "'"; \
      BOOST_CHECK_MESSAGE(mktn == v->typeName(), \
                          message.str().c_str());       \
    } \
  }


  void
  testConstructions()
  {
    TEST_VAL(Null, , NULLV, "null");
    
    TEST_VAL(Int32, 0, INT32, "int32");
    TEST_VAL(Int32, 1, INT32, "int32");
    TEST_VAL(Int32, 2000, INT32, "int32");
    TEST_VAL(Int32, INT_MAX, INT32, "int32");
    TEST_VAL(Int32, -1, INT32, "int32");
    TEST_VAL(Int32, -2000, INT32, "int32");
    TEST_VAL(Int32, INT_MIN, INT32, "int32");

    TEST_VAL(Int64, 0, INT64, "int64");
    TEST_VAL(Int64, 1, INT64, "int64");
    TEST_VAL(Int64, 2000, INT64, "int64");
    TEST_VAL(Int64, LONG_LONG_MAX, INT64, "int64");
    TEST_VAL(Int64, -1, INT64, "int64");
    TEST_VAL(Int64, -2000, INT64, "int64");
    TEST_VAL(Int64, LONG_LONG_MIN, INT64, "int64");
  
    TEST_VAL(UInt32, 0, UINT32, "uint32");
    TEST_VAL(UInt32, 1, UINT32, "uint32");
    TEST_VAL(UInt32, 1000, UINT32, "uint32");
    TEST_VAL(UInt32, UINT_MAX, UINT32, "uint32");

    TEST_VAL(UInt64, 0, UINT64, "uint64");
    TEST_VAL(UInt64, 1, UINT64, "uint64");
    TEST_VAL(UInt64, 1000, UINT64, "uint64");
    TEST_VAL(UInt64, UINT_MAX, UINT64, "uint64");

    TEST_VAL(Double, 0, DOUBLE, "double");
    TEST_VAL(Double, 1.0, DOUBLE, "double");
    TEST_VAL(Double, -1.0, DOUBLE, "double");
    TEST_VAL(Double, -1.79769E+308, DOUBLE, "double");
    TEST_VAL(Double, 1.79769E+308, DOUBLE, "double");
    TEST_VAL(Double, 2.225E-307, DOUBLE, "double");
    TEST_VAL(Double, -2.225E-307, DOUBLE, "double");

    TEST_VAL(Str, "", STR, "str");
    TEST_VAL(Str, "This is a string", STR, "str");
    
    // Opaques
    FdbufPtr u1(new Fdbuf());
    u1->pushBack("This is UIO 1");
    TEST_VAL(Opaque, u1, OPAQUE, "opaque");
    

    // Strings
    std::string addr = "127.0.0.1:1000";
    TEST_VAL(IP_ADDR, addr, IP_ADDR, "ip_addr"); 


    // Time
#define TEST_TIME(valTypeExt, secs, nsecs, typecode, typestr) \
    { \
      struct timespec ts; \
      ts.tv_sec = secs; \
      ts.tv_nsec = nsecs; \
      TEST_VAL(valTypeExt, ts, typecode, typestr); \
    }

    TEST_TIME(Time,  0,  0, TIME, "time");
    TEST_TIME(Time, 10, 10, TIME, "time");
    TEST_TIME(Time, -1,  0, TIME, "time");
    TEST_TIME(Time,  0, -1, TIME, "time");
    TEST_TIME(Time, -2, -2, TIME, "time");
#undef TEST_TIMEVAL
  } 
#undef TEST_VAL







  ////////////////////////////////////////////////////////////
  // Self casting tests
  ////////////////////////////////////////////////////////////

#define TEST_CAST(valTypeExt, value, cType, dstValTypeExt, dstValue) \
  { \
    std::string testID; \
    { \
      std::ostringstream ID; \
      ID << "Value casting test. "; \
      testID = ID.str(); \
    } \
 \
    ValuePtr v = Val_##valTypeExt::mk(value); \
    try { \
      cType cValue = Val_##dstValTypeExt::cast(v); \
 \
      std::ostringstream message; \
      message << testID \
              << "Bad cast value from 'Val_" #valTypeExt "' (" \
              << #value \
              << ")->'" #dstValTypeExt \
              << "'. Expected " \
              << #dstValue \
              << " but got " \
              << cValue \
              << "."; \
      BOOST_CHECK_MESSAGE(cValue == dstValue, \
                          message.str().c_str()); \
    } catch (Value::TypeError) { \
      std::ostringstream message; \
      message << testID \
              << "Type exception casting from 'Val_" #valTypeExt "' (" \
              << #value \
              << ")->'" #dstValTypeExt \
              << "'."; \
      BOOST_CHECK_MESSAGE(false, message.str().c_str()); \
    } \
  }


#define TEST_TIMESPEC_CAST(valTypeExt, value, dstValTypeExt, dstSec, dstNsec) \
  { \
    std::string testID; \
    { \
      std::ostringstream ID; \
      ID << "Value casting test. "; \
      testID = ID.str(); \
    } \
 \
    struct timespec dest; \
    dest.tv_sec = dstSec; \
    dest.tv_nsec = dstNsec; \
    ValuePtr v = Val_##valTypeExt::mk(value); \
    try { \
      struct timespec cValue = Val_##dstValTypeExt::cast(v); \
 \
      std::ostringstream message; \
      message << testID \
              << "Bad cast value from 'Val_" #valTypeExt "' (" \
              << #value \
              << ")->'" #dstValTypeExt \
              << "'. Expected <" \
              << #dstSec \
              << "," \
              << #dstNsec \
              << "> but got <" \
              << cValue.tv_sec \
              << "," \
              << cValue.tv_nsec \
              << ">."; \
      BOOST_CHECK_MESSAGE(!compare_timespec(cValue, dest), \
                          message.str().c_str()); \
    } catch (Value::TypeError) {                  \
      std::ostringstream message; \
      message << testID \
              << "Type exception casting from 'Val_" #valTypeExt "' (" \
              << #value \
              << ")->'" #dstValTypeExt \
              << "'."; \
      BOOST_CHECK_MESSAGE(false, message.str().c_str()); \
    }                                                    \
  }

/*
 * Defining a new test for casting to ptimes. --ACR
 */
 
#define TEST_PTIME_CAST(valTypeExt, value, dstValTypeExt, dstSec, dstNsec) \
  { \
    std::string testID; \
    { \
      std::ostringstream ID; \
      ID << "Value casting test. "; \
      testID = ID.str(); \
    } \
 \
    struct timespec dest; \
    dest.tv_sec = dstSec; \
    dest.tv_nsec = dstNsec; \
    \
    ValuePtr time = Val_##dstValTypeExt::mk(dest);\
	boost::posix_time::ptime testAgainst = Val_##dstValTypeExt::cast(time);\
    \
    ValuePtr v = Val_##valTypeExt::mk(value); \
    try { \
      boost::posix_time::ptime cValue = Val_##dstValTypeExt::cast(v); \
 \
      std::ostringstream message; \
      message << testID \
              << "Bad cast value from 'Val_" #valTypeExt "' (" \
              << #value \
              << ")->'" #dstValTypeExt \
              << "'. Expected <" \
              << to_simple_string(testAgainst) \
              << ">, but got <" \
              << to_simple_string(cValue) \
              << ">."; \
      BOOST_CHECK_MESSAGE(testAgainst == cValue, \
                          message.str().c_str()); \
    } catch (Value::TypeError) {                  \
      std::ostringstream message; \
      message << testID \
              << "Type exception casting from 'Val_" #valTypeExt "' (" \
              << #value \
              << ")->'" #dstValTypeExt \
              << "'."; \
      BOOST_CHECK_MESSAGE(false, message.str().c_str()); \
    }                                                    \
  }


  void
  testSelfCasts()
  {
    TEST_CAST(UInt32, 0, uint32_t, UInt32, 0);
    TEST_CAST(UInt32, 1, uint32_t, UInt32, 1);
    
    TEST_CAST(Int64, 0, int64_t, Int64, 0);
    TEST_CAST(Int64, -1, int64_t, Int64, -1);
    TEST_CAST(Int64, 1, int64_t, Int64, 1);
    
    TEST_CAST(UInt64, 0, uint64_t, UInt64, 0);
    TEST_CAST(UInt64, 1, uint64_t, UInt64, 1);
    
    TEST_CAST(Double, 0, double, Double, 0);
    TEST_CAST(Double, 0.0, double, Double, 0.0);
    TEST_CAST(Double, 1.2, double, Double, 1.2);
    
    TEST_CAST(Str, "", string, Str, "");
    TEST_CAST(Str, "This is a string", string, Str, "This is a string");
    
    // Opaques
    FdbufPtr u1(new Fdbuf());
    u1->pushBack("This is UIO 1");
    TEST_CAST(Opaque, u1, FdbufPtr, Opaque, u1);
    
    // Strings
    std::string addr = "127.0.0.1:1000";
    TEST_CAST(IP_ADDR, addr, string, IP_ADDR, addr);
  }


  void
  testCorrectCasts()
  {
    // Test casting to int32.
#define TEST_CAST_T(_t,_v,_r) TEST_CAST(_t,_v,int32_t,Int32,_r)
    TEST_CAST_T(Null, , 0);
    
    TEST_CAST_T(Int32, 0, 0);
    TEST_CAST_T(Int32, 1, 1);
    TEST_CAST_T(Int32, 2000, 2000);
    TEST_CAST_T(Int32, INT_MAX, INT_MAX);
    TEST_CAST_T(Int32, -1, -1);
    TEST_CAST_T(Int32, -2000, -2000);
    TEST_CAST_T(Int32, INT_MIN, INT_MIN);

    TEST_CAST_T(Int64, 0, 0);
    TEST_CAST_T(Int64, 1, 1);
    TEST_CAST_T(Int64, 2000, 2000);
    TEST_CAST_T(Int64, LONG_LONG_MAX, -1);
    TEST_CAST_T(Int64, -1, -1);
    TEST_CAST_T(Int64, -2000, -2000);
    TEST_CAST_T(Int64, LONG_LONG_MIN, 0);
  
    TEST_CAST_T(UInt32, 0, 0);
    TEST_CAST_T(UInt32, 1, 1);
    TEST_CAST_T(UInt32, 1000, 1000);
    TEST_CAST_T(UInt32, UINT_MAX, -1);

    TEST_CAST_T(UInt64, 0, 0);
    TEST_CAST_T(UInt64, 1, 1);
    TEST_CAST_T(UInt64, 1000, 1000);
    TEST_CAST_T(UInt64, ULONG_LONG_MAX, -1);

    TEST_CAST_T(Double, 0, 0);
    TEST_CAST_T(Double, 1.0, 1);
    TEST_CAST_T(Double, -1.0, -1);
    TEST_CAST_T(Double, -1.79769E+308, INT_MIN);
	//    TEST_CAST_T(Double, 1.79769E+308, INT_MIN); // not robust cross-platform
    TEST_CAST_T(Double, 2.225E-307, 0);
    TEST_CAST_T(Double, -2.225E-307, 0);
  
    TEST_CAST_T(Str, "", 0);
    TEST_CAST_T(Str, "0", 0);
    TEST_CAST_T(Str, "1", 1);
    TEST_CAST_T(Str, "0x1a", 26);
    TEST_CAST_T(Str, "011", 9);
    TEST_CAST_T(Str, "-200", -200);
    TEST_CAST_T(Str, "1.5", 1);
    TEST_CAST_T(Str, "-1.5", -1);
    TEST_CAST_T(Str, "Rubbish", 0);
#undef TEST_CAST_T


  // Test casting to uint32.
#define TEST_CAST_T(_t,_v,_r) TEST_CAST(_t,_v,uint32_t,UInt32,_r)

  TEST_CAST_T(Null, , 0);

  TEST_CAST_T(Int32, 0, 0);
  TEST_CAST_T(Int32, 1, 1);
  TEST_CAST_T(Int32, 2000, 2000);
  TEST_CAST_T(Int32, INT_MAX, INT_MAX);
  TEST_CAST_T(Int32, -1, UINT_MAX);
  TEST_CAST_T(Int32, -2000, UINT_MAX -2000 + 1);
  TEST_CAST_T(Int32, INT_MIN, INT_MAX+1U);

  TEST_CAST_T(Int64, 0, 0);
  TEST_CAST_T(Int64, 1, 1);
  TEST_CAST_T(Int64, 2000, 2000);
  TEST_CAST_T(Int64, LONG_LONG_MAX, UINT_MAX);
  TEST_CAST_T(Int64, -1, UINT_MAX);
  TEST_CAST_T(Int64, -2000, UINT_MAX-2000 +1);
  TEST_CAST_T(Int64, LONG_LONG_MIN, 0);
  
  TEST_CAST_T(UInt32, 0, 0);
  TEST_CAST_T(UInt32, 1, 1);
  TEST_CAST_T(UInt32, 1000, 1000);
  TEST_CAST_T(UInt32, UINT_MAX, UINT_MAX);

  TEST_CAST_T(UInt64, 0, 0);
  TEST_CAST_T(UInt64, 1, 1);
  TEST_CAST_T(UInt64, 1000, 1000);
  TEST_CAST_T(UInt64, ULONG_LONG_MAX, UINT_MAX);

  TEST_CAST_T(Double, 0, 0);
  TEST_CAST_T(Double, 1.0, 1);
  TEST_CAST_T(Double, -1.0, UINT_MAX);
  // TEST_CAST_T(Double, -1.79769E+308, 0); // not robust cross-platform
  // TEST_CAST_T(Double, 1.79769E+308, 0); // not robust cross-platform
  TEST_CAST_T(Double, 2.225E-307, 0);
  TEST_CAST_T(Double, -2.225E-307, 0);
  
  TEST_CAST_T(Str, "", 0);
  TEST_CAST_T(Str, "0", 0);
  TEST_CAST_T(Str, "1", 1);
  TEST_CAST_T(Str, "0x1a", 26);
  TEST_CAST_T(Str, "011", 9);
  TEST_CAST_T(Str, "-200", UINT_MAX-200 +1);
  TEST_CAST_T(Str, "1.5", 1);
  TEST_CAST_T(Str, "-1.5", UINT_MAX);
  TEST_CAST_T(Str, "Rubbish", 0);
  #undef TEST_CAST_T


  // Test casting to int64.
#define TEST_CAST_T(_t,_v,_r) TEST_CAST(_t,_v,int64_t,Int64,_r)

  TEST_CAST_T(Null, , 0);

  TEST_CAST_T(Int32, 0, 0);
  TEST_CAST_T(Int32, 1, 1);
  TEST_CAST_T(Int32, 2000, 2000);
  TEST_CAST_T(Int32, INT_MAX, INT_MAX);
  TEST_CAST_T(Int32, -1, -1);
  TEST_CAST_T(Int32, -2000, -2000);
  TEST_CAST_T(Int32, INT_MIN, INT_MIN);

  TEST_CAST_T(Int64, 0, 0);
  TEST_CAST_T(Int64, 1, 1);
  TEST_CAST_T(Int64, 2000, 2000);
  TEST_CAST_T(Int64, LONG_LONG_MAX, LONG_LONG_MAX);
  TEST_CAST_T(Int64, -1, -1);
  TEST_CAST_T(Int64, -2000, -2000);
  TEST_CAST_T(Int64, LONG_LONG_MIN, LONG_LONG_MIN);
  
  TEST_CAST_T(UInt32, 0, 0);
  TEST_CAST_T(UInt32, 1, 1);
  TEST_CAST_T(UInt32, 1000, 1000);
  TEST_CAST_T(UInt32, UINT_MAX, UINT_MAX);

  TEST_CAST_T(UInt64, 0, 0);
  TEST_CAST_T(UInt64, 1, 1);
  TEST_CAST_T(UInt64, 1000, 1000);
  TEST_CAST_T(UInt64, ULONG_LONG_MAX, -1);

  TEST_CAST_T(Double, 0, 0);
  TEST_CAST_T(Double, 1.0, 1);
  TEST_CAST_T(Double, -1.0, -1);
  // TEST_CAST_T(Double, -1.79769E+308, LONG_LONG_MIN); // not robust cross-platform
  // TEST_CAST_T(Double, 1.79769E+308, LONG_LONG_MIN); // not robust cross-platform
  TEST_CAST_T(Double, 2.225E-307, 0);
  TEST_CAST_T(Double, -2.225E-307, 0);
  
  TEST_CAST_T(Str, "", 0);
  TEST_CAST_T(Str, "0", 0);
  TEST_CAST_T(Str, "1", 1);
  TEST_CAST_T(Str, "0x1a", 26);
  TEST_CAST_T(Str, "011", 9);
  TEST_CAST_T(Str, "-200", -200);
  TEST_CAST_T(Str, "1.5", 1);
  TEST_CAST_T(Str, "-1.5", -1);
  TEST_CAST_T(Str, "Rubbish", 0);
  #undef TEST_CAST_T
  

  // Test casting to uint64.
#define TEST_CAST_T(_t,_v,_r) TEST_CAST(_t,_v,uint64_t,UInt64,_r)
  TEST_CAST_T(Null, , 0);

  TEST_CAST_T(Int32, 0, 0);
  TEST_CAST_T(Int32, 1, 1);
  TEST_CAST_T(Int32, 2000, 2000);
  TEST_CAST_T(Int32, INT_MAX, INT_MAX);
  TEST_CAST_T(Int32, -1, ULONG_LONG_MAX);
  TEST_CAST_T(Int32, -2000, ULONG_LONG_MAX -2000 + 1);
  TEST_CAST_T(Int32, INT_MIN, ULONG_LONG_MAX - INT_MAX);

  TEST_CAST_T(Int64, 0, 0);
  TEST_CAST_T(Int64, 1, 1);
  TEST_CAST_T(Int64, 2000, 2000);
  TEST_CAST_T(Int64, LONG_LONG_MAX, LONG_LONG_MAX);
  TEST_CAST_T(Int64, -1, ULONG_LONG_MAX);
  TEST_CAST_T(Int64, -2000, ULONG_LONG_MAX-2000 +1);
  TEST_CAST_T(Int64, LONG_LONG_MIN, 1UL + (uint64_t)(LONG_LONG_MAX));
  
  TEST_CAST_T(UInt32, 0, 0);
  TEST_CAST_T(UInt32, 1, 1);
  TEST_CAST_T(UInt32, 1000, 1000);
  TEST_CAST_T(UInt32, UINT_MAX, UINT_MAX);

  TEST_CAST_T(UInt64, 0, 0);
  TEST_CAST_T(UInt64, 1, 1);
  TEST_CAST_T(UInt64, 1000, 1000);
  TEST_CAST_T(UInt64, ULONG_LONG_MAX, ULONG_LONG_MAX);

  TEST_CAST_T(Double, 0, 0);
  TEST_CAST_T(Double, 1.0, 1);
  /**
     Removed due to GCC casting bug
     TEST_CAST_T(Double, -1.0, ULONG_LONG_MAX);
  */
  // TEST_CAST_T(Double, -1.79769E+308, 0); // not robust cross-platform
  // TEST_CAST_T(Double, 1.79769E+308, 0); // not robust cross-platform
  TEST_CAST_T(Double, 2.225E-307, 0);
  TEST_CAST_T(Double, -2.225E-307, 0);
  
  TEST_CAST_T(Str, "", 0);
  TEST_CAST_T(Str, "0", 0);
  TEST_CAST_T(Str, "1", 1);
  TEST_CAST_T(Str, "0x1a", 26);
  TEST_CAST_T(Str, "011", 9);
  TEST_CAST_T(Str, "-200", ULONG_LONG_MAX-200 +1);
  TEST_CAST_T(Str, "1.5", 1);
  TEST_CAST_T(Str, "-1.5", ULONG_LONG_MAX);
  TEST_CAST_T(Str, "Rubbish", 0);
#undef TEST_CAST_T
  


  // Test casting to double.
#define TEST_CAST_T(_t,_v,_r) TEST_CAST(_t,_v,double,Double,_r)

  TEST_CAST_T(Null, , 0);

  TEST_CAST_T(Int32, 0, 0);
  TEST_CAST_T(Int32, 1, 1);
  TEST_CAST_T(Int32, 2000, 2000);
  TEST_CAST_T(Int32, INT_MAX, INT_MAX);
  TEST_CAST_T(Int32, -1, -1);
  TEST_CAST_T(Int32, -2000, -2000);
  TEST_CAST_T(Int32, INT_MIN, INT_MIN);

  TEST_CAST_T(Int64, 0, 0);
  TEST_CAST_T(Int64, 1, 1);
  TEST_CAST_T(Int64, 2000, 2000);
  TEST_CAST_T(Int64, LONG_LONG_MAX, LONG_LONG_MAX);
  TEST_CAST_T(Int64, -1, -1);
  TEST_CAST_T(Int64, -2000, -2000);
  TEST_CAST_T(Int64, LONG_LONG_MIN, LONG_LONG_MIN);
  
  TEST_CAST_T(UInt32, 0, 0);
  TEST_CAST_T(UInt32, 1, 1);
  TEST_CAST_T(UInt32, 1000, 1000);
  TEST_CAST_T(UInt32, UINT_MAX, UINT_MAX);

  TEST_CAST_T(UInt64, 0, 0);
  TEST_CAST_T(UInt64, 1, 1);
  TEST_CAST_T(UInt64, 1000, 1000);
  TEST_CAST_T(UInt64, ULONG_LONG_MAX, ULONG_LONG_MAX);

  TEST_CAST_T(Double, 0, 0);
  TEST_CAST_T(Double, 1.0, 1);
  TEST_CAST_T(Double, -1.0, -1.0);
  TEST_CAST_T(Double, -1.79769E+308,  -1.79769E+308);
  TEST_CAST_T(Double, 1.79769E+308, 1.79769E+308);
  TEST_CAST_T(Double, 2.225E-307, 2.225E-307);
  TEST_CAST_T(Double, -2.225E-307, -2.225E-307);
  
  TEST_CAST_T(Str, "", 0);
  TEST_CAST_T(Str, "0", 0);
  TEST_CAST_T(Str, "1", 1);
  TEST_CAST_T(Str, "0x1a", 26);
  TEST_CAST_T(Str, "011", 11);
  TEST_CAST_T(Str, "-200", -200);
  TEST_CAST_T(Str, "1.5", 1.5);
  TEST_CAST_T(Str, "-1.5", -1.5);
  TEST_CAST_T(Str, "Rubbish", 0);
#undef TEST_CAST_T


  // Test casting to Str.
#define TEST_CAST_T(_t,_v,_r) TEST_CAST(_t,_v,string,Str,_r)

  TEST_CAST_T(Null, , "NULL");

  TEST_CAST_T(Int32, 0, "0");
  TEST_CAST_T(Int32, 1, "1");
  TEST_CAST_T(Int32, 2000, "2000");
  TEST_CAST_T(Int32, INT_MAX, "2147483647");
  TEST_CAST_T(Int32, -1, "-1");
  TEST_CAST_T(Int32, -2000, "-2000");
  TEST_CAST_T(Int32, INT_MIN, "-2147483648");

  TEST_CAST_T(Int64, 0, "0");
  TEST_CAST_T(Int64, 1, "1");
  TEST_CAST_T(Int64, 2000, "2000");
  TEST_CAST_T(Int64, LONG_LONG_MAX, "9223372036854775807");
  TEST_CAST_T(Int64, -1, "-1");
  TEST_CAST_T(Int64, -2000, "-2000");
  TEST_CAST_T(Int64, LONG_LONG_MIN, "-9223372036854775808");
  
  TEST_CAST_T(UInt32, 0, "0");
  TEST_CAST_T(UInt32, 1, "1");
  TEST_CAST_T(UInt32, 1000, "1000");
  TEST_CAST_T(UInt32, UINT_MAX, "4294967295");

  TEST_CAST_T(UInt64, 0, "0");
  TEST_CAST_T(UInt64, 1, "1");
  TEST_CAST_T(UInt64, 1000, "1000");
  TEST_CAST_T(UInt64, ULONG_LONG_MAX, "18446744073709551615");

  TEST_CAST_T(Double, 0, "0");
  TEST_CAST_T(Double, 1.0, "1");
  TEST_CAST_T(Double, -1.0, "-1");
  TEST_CAST_T(Double, -1.79769E+308,  "-1.79769e+308");
  TEST_CAST_T(Double, 1.79769E+308, "1.79769e+308");
  TEST_CAST_T(Double, 2.225E-307, "2.225e-307");
  TEST_CAST_T(Double, -2.225E-307, "-2.225e-307");
  
  TEST_CAST_T(Str, "", "");
  TEST_CAST_T(Str, "0", "0");
  TEST_CAST_T(Str, "1", "1");
  TEST_CAST_T(Str, "0x1a", "0x1a");
  TEST_CAST_T(Str, "011", "011");
  TEST_CAST_T(Str, "-200", "-200");
  TEST_CAST_T(Str, "1.5", "1.5");
  TEST_CAST_T(Str, "-1.5", "-1.5");
  TEST_CAST_T(Str, "Rubbish", "Rubbish");
  
  FdbufPtr u2(new Fdbuf());
  u2->pushBack("This is UIO 2");
  TEST_CAST_T(Opaque, u2, "This is UIO 2");


  string addr = "127.0.0.1:1000";
  TEST_CAST_T(IP_ADDR, addr, "127.0.0.1:1000");
#undef TEST_CAST_T



  // Casting to IP address
  // XXX What exactly are we testing here?
#define TEST_CAST_T(_t,_v,_r) TEST_CAST(_t, _v, string, IP_ADDR, _r)
  TEST_CAST_T(Str, "", "");
  TEST_CAST_T(Str, "0", "0");
  TEST_CAST_T(Str, "1", "1");
  TEST_CAST_T(Str, "0x1a", "0x1a");
  TEST_CAST_T(Str, "011", "011");
  TEST_CAST_T(Str, "-200", "-200");
  TEST_CAST_T(Str, "1.5", "1.5");
  TEST_CAST_T(Str, "-1.5", "-1.5");
  TEST_CAST_T(Str, "Rubbish", "Rubbish");
#undef TEST_CAST_T



  // Test casting to Time, time_t.tv_sec is of type int32
  
  // Modified this to test with ptimes instead of timespecs --ACR

#define TEST_CAST_T(_t,_v,_s,_ns) TEST_PTIME_CAST(_t,_v,Time,_s,_ns)

  TEST_CAST_T(Int32, 0, 0, 0);
  TEST_CAST_T(Int32, 1, 1, 0);
  TEST_CAST_T(Int32, 2000, 2000, 0);
  TEST_CAST_T(Int32, INT_MAX, INT_MAX, 0);
  TEST_CAST_T(Int32, -1, -1, 0);
  TEST_CAST_T(Int32, -2000, -2000, 0);
  TEST_CAST_T(Int32, INT_MIN + 1, INT_MIN + 1, 0);

  TEST_CAST_T(Int64, 0, 0, 0);
  TEST_CAST_T(Int64, 1, 1, 0);
  TEST_CAST_T(Int64, 2000, 2000, 0);
  TEST_CAST_T(Int64, LONG_LONG_MAX, -1, 0);
  TEST_CAST_T(Int64, -1, -1, 0);
  TEST_CAST_T(Int64, -2000, -2000, 0);
  TEST_CAST_T(Int64, LONG_LONG_MIN, 0, 0);
  
  TEST_CAST_T(UInt32, 0, 0, 0);
  TEST_CAST_T(UInt32, 1, 1, 0);
  TEST_CAST_T(UInt32, 1000, 1000, 0);
  TEST_CAST_T(UInt32, UINT_MAX, -1, 0);

  TEST_CAST_T(UInt64, 0, 0, 0);
  TEST_CAST_T(UInt64, 1, 1, 0);
  TEST_CAST_T(UInt64, 1000, 1000, 0);
  TEST_CAST_T(UInt64, ULONG_LONG_MAX, -1, 0);

  TEST_CAST_T(Double, 0, 0, 0);
  TEST_CAST_T(Double, 1.0, 1, 0);
  TEST_CAST_T(Double, -1.0, -1, 0);
  
  // These next casts assume nanosecond precision --ACR
  TEST_CAST_T(Double, (double) 1.000000002, 1, 2);
  TEST_CAST_T(Double, (double) 1.000000200, 1, 200);
  TEST_CAST_T(Double, (double) 4245674.123456789, 4245674, 123456789);
#undef TEST_CAST_T
  
  
  }
#undef TEST_CAST
#undef TEST_TIMESPEC_CAST
#undef TEST_PTIME_CAST

/* For whatever reason, this is causing gcc to be cranky. I'm going to 
 * disregard it for now in the interest of productivity, since it doesn't 
 * involve time. --ACR
 */ 

/*
#define TEST_ID_CAST(valTypeExt, value, d1, d2, d3, d4, d5)        \
  { \
    std::string testID; \
    { \
      std::ostringstream ID; \
      ID << "Value casting test. "; \
      testID = ID.str(); \
    } \
 \
    ValuePtr v = Val_##valTypeExt::mk(value); \
    uint32_t bytes[5] = {d1, d2, d3, d4, d5}; \
    IDPtr destination = ID::mk(bytes);   \
    try { \
      IDPtr cValue = Val_ID::cast(v); \
 \
      std::ostringstream message; \
      message << testID \
              << "Bad cast value from 'Val_" #valTypeExt "' (" \
              << #value \
              << ")->'ID'. Expected " \
              << *destination \
              << " but got " \
              << *cValue \
              << "."; \
      BOOST_CHECK_MESSAGE(cValue->compareTo(destination) == 0,  \
                          message.str().c_str()); \
    } catch (Value::TypeError) { \
      std::ostringstream message; \
      message << testID \
              << "Type exception casting from 'Val_" #valTypeExt "' ("  \
              << #value \
              << ")->'ID'."; \
      BOOST_CHECK_MESSAGE(false, message.str().c_str()); \
    } \
  }


void
testIDCasts()
{
  TEST_ID_CAST(Int32, INT_MAX, 0, 0, 0, 0, INT_MAX);
  TEST_ID_CAST(Int32, -1, 0, 0, 0, 0, ((uint32_t) -1));
  TEST_ID_CAST(Int32, -2000, 0, 0, 0, 0, ((uint32_t) -2000));
  TEST_ID_CAST(Int32, INT_MIN, 0, 0, 0, 0, ((uint32_t) INT_MIN));

  TEST_ID_CAST(Int64, 0, 0, 0, 0, 0, 0);
  TEST_ID_CAST(Int64, 1, 0, 0, 0, 0, 1);
  TEST_ID_CAST(Int64, 2000, 0, 0, 0, 0, 2000);
  TEST_ID_CAST(Int64, LONG_LONG_MAX, 0, 0, 0, ((uint32_t) (LONG_LONG_MAX >> 32)), ((uint32_t) LONG_LONG_MAX));
  TEST_ID_CAST(Int64, -1, 0, 0, 0, 0xffffffff, (uint32_t) -1);
  TEST_ID_CAST(Int64, -2000, 0, 0, 0, 0xffffffff, (uint32_t) -2000);
  TEST_ID_CAST(Int64, LONG_LONG_MIN, 0, 0, 0, ((uint32_t) (LONG_LONG_MIN >> 32)), ((uint32_t) LONG_LONG_MIN));

  TEST_ID_CAST(UInt32, 0, 0, 0, 0, 0, 0);
  TEST_ID_CAST(UInt32, 1, 0, 0, 0, 0, 1);
  TEST_ID_CAST(UInt32, 1000, 0, 0, 0, 0, 1000);
  TEST_ID_CAST(UInt32, UINT_MAX, 0, 0, 0, 0, UINT_MAX);

  TEST_ID_CAST(UInt64, 0, 0, 0, 0, 0, 0);
  TEST_ID_CAST(UInt64, 1, 0, 0, 0, 0, 1);
  TEST_ID_CAST(UInt64, 1000, 0, 0, 0, 0, 1000);
  TEST_ID_CAST(UInt64, ULONG_LONG_MAX, 0, 0, 0, 0xffffffff, 0xffffffff);
}
#undef TEST_ID_CAST

*/
  
#define TEST_BADCAST(valTypeExt, value, dstValTypeExt) \
  { \
    std::string testID; \
    { \
      std::ostringstream ID; \
      ID << "Value casting test. "; \
      testID = ID.str(); \
    } \
 \
    ValuePtr v = Val_##valTypeExt::mk(value); \
    bool success; \
    try { \
      Val_##dstValTypeExt::cast(v); \
      success = true; \
    } catch (Value::TypeError) { \
      success = false; \
    } \
    std::ostringstream message;                 \
    message << testID                                          \
            << "Missed type exception casting 'Val_" #valTypeExt "' ("  \
            << #value                                                   \
            << ")->'" #dstValTypeExt                                    \
            << "'.";                                                    \
    BOOST_CHECK_MESSAGE(success == false,                               \
                        message.str().c_str());                         \
  }
  
void
testBadCasts()
{
  FdbufPtr u2(new Fdbuf());
  u2->pushBack("This is UIO 2");
  string addr = "127.0.0.1:1000";

  TEST_BADCAST(Int32, -1, Null);
  TEST_BADCAST(UInt32, 1, Null);
  TEST_BADCAST(Int64, -1, Null);
  TEST_BADCAST(UInt64, 1, Null);
  TEST_BADCAST(Double, 1.0, Null);
  TEST_BADCAST(Str, "", Null);
  TEST_BADCAST(Str, "Hello", Null);
  TEST_BADCAST(Opaque, u2, Null);
  TEST_BADCAST(IP_ADDR, addr, Null);
  TEST_BADCAST(Opaque, u2, Int32);
  TEST_BADCAST(IP_ADDR, addr, Int32);
  TEST_BADCAST(Opaque, u2, UInt32);
  TEST_BADCAST(IP_ADDR, addr, UInt32);
  TEST_BADCAST(Opaque, u2, Int64);
  TEST_BADCAST(IP_ADDR, addr, Int64);
  TEST_BADCAST(Opaque, u2, UInt64);
  TEST_BADCAST(IP_ADDR, addr, Int64);
  TEST_BADCAST(Opaque, u2, Double);
  TEST_BADCAST(IP_ADDR, addr, Int64);

  TEST_BADCAST(Null,, Opaque);
  TEST_BADCAST(Int32, 0, Opaque);
  TEST_BADCAST(UInt64, 0, Opaque);
  TEST_BADCAST(Int32, 0, Opaque);
  TEST_BADCAST(UInt64, 0, Opaque);
  TEST_BADCAST(Double, 0, Opaque);
  TEST_BADCAST(IP_ADDR, addr, Opaque); 

  TEST_BADCAST(Null,, IP_ADDR);
  TEST_BADCAST(Int32, 0, IP_ADDR);
  TEST_BADCAST(UInt64, 0, IP_ADDR);
  TEST_BADCAST(Int32, 0, IP_ADDR);
  TEST_BADCAST(UInt64, 0, IP_ADDR);
  TEST_BADCAST(Double, 0, IP_ADDR);
  TEST_BADCAST(Opaque, u2, IP_ADDR);

  TEST_BADCAST(Null,, Tuple);
  TEST_BADCAST(Int32, 0, Tuple);
  TEST_BADCAST(UInt64, 0, Tuple);
  TEST_BADCAST(Int32, 0, Tuple);
  TEST_BADCAST(UInt64, 0, Tuple);
  TEST_BADCAST(Double, 0, Tuple);
  
  TEST_BADCAST(Int32, 0, List);
  TEST_BADCAST(UInt64, 0, List);
  TEST_BADCAST(Int32, 0, List);
  TEST_BADCAST(UInt64, 0, List);
  TEST_BADCAST(Double, 0, List);

  TEST_BADCAST(Int32, 0, Vector);
  TEST_BADCAST(UInt64, 0, Vector);
  TEST_BADCAST(Int32, 0, Vector);
  TEST_BADCAST(UInt64, 0, Vector);
  TEST_BADCAST(Double, 0, Vector);
}
#undef TEST_BADCAST


void
testImplicitConversions()
{
  {
    ValuePtr i1 = Val_Int32::mk(5);
    ValuePtr i2 = Val_Str::mk("10");
    ValuePtr r1 = i1 + i2;
    BOOST_CHECK_MESSAGE(r1->typeCode() == Value::INT32,
                        "int32 "
                        << i1->toString()
                        << " + str "
                        << i2->toString()
                        << " = not int32 "
                        << r1->toString());
  }

  {
    ValuePtr i1 = Val_Int32::mk(5);
    ValuePtr i2 = Val_Str::mk("10");
    ValuePtr r1 = i2 + i1;
    BOOST_CHECK_MESSAGE(r1->typeCode() == Value::STR,
                        "str "
                        << i2->toString()
                        << " + int32 "
                        << i1->toString()
                        << " = not str "
                        << r1->toString());
  }
}

};

testValues_testSuite::testValues_testSuite()
  : boost::unit_test_framework::test_suite("testValues")
{
  boost::shared_ptr<testValues> instance(new testValues());
  
  
  add(BOOST_CLASS_TEST_CASE(&testValues::testConstructions,
                            instance));
  add(BOOST_CLASS_TEST_CASE(&testValues::testSelfCasts,
                            instance));
  add(BOOST_CLASS_TEST_CASE(&testValues::testCorrectCasts,
                            instance));
  /*add(BOOST_CLASS_TEST_CASE(&testValues::testIDCasts,
    instance));
  */
  add(BOOST_CLASS_TEST_CASE(&testValues::testBadCasts,
                            instance));
  
  add(BOOST_CLASS_TEST_CASE(&testValues::testImplicitConversions,
                            instance));
}
