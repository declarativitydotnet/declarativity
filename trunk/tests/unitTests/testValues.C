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
#include "val_int64.h"
#include "val_double.h"
#include "val_opaque.h"
#include "val_tuple.h"
#include "val_time.h"
#include "val_id.h"
#include "oper.h"
#include "val_str.h"
#include "val_list.h"
#include "val_set.h"
#include "val_vector.h"
#include "val_matrix.h"

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
    
    TEST_VAL(Int64, 0, INT64, "int64");
    TEST_VAL(Int64, 1, INT64, "int64");
    TEST_VAL(Int64, 2000, INT64, "int64");
    TEST_VAL(Int64, LONG_LONG_MAX, INT64, "int64");
    TEST_VAL(Int64, -1, INT64, "int64");
    TEST_VAL(Int64, -2000, INT64, "int64");
    TEST_VAL(Int64, LONG_LONG_MIN, INT64, "int64");
  
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

	// Vectors
	uint64_t one = 1;
	ValuePtr valp = Val_Vector::mk2(one);
	VectorPtr vp = Val_Vector::cast(valp);
	((*vp)[0]) = Val_Int64::mk(1);
	TEST_VAL(Vector, vp, VECTOR, "vector");

	// Matrix
	valp = Val_Matrix::mk2(one, one);
	MatrixPtr mp = Val_Matrix::cast(valp);
	((*mp)(0,0)) = Val_Int64::mk(1);
	TEST_VAL(Matrix, mp, MATRIX, "matrix");
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
    TEST_CAST(Int64, 0, int64_t, Int64, 0);
    TEST_CAST(Int64, -1, int64_t, Int64, -1);
    TEST_CAST(Int64, 1, int64_t, Int64, 1);
    
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

	// Vectors
	uint64_t one = 1;
	ValuePtr valp = Val_Vector::mk2(one);
	VectorPtr vp = Val_Vector::cast(valp);
	((*vp)[0]) = Val_Int64::mk(1);
	TEST_CAST(Vector, vp, VectorPtr, Vector, vp);

	// Matrices
	valp = Val_Matrix::mk2(one,one);
	MatrixPtr mp = Val_Matrix::cast(valp);
	((*mp)(0,0)) = Val_Int64::mk(1);
	TEST_CAST(Matrix, mp, MatrixPtr, Matrix, mp);

  }


  void
  testCorrectCasts()
  {
  // Test casting to int64.
#define TEST_CAST_T(_t,_v,_r) TEST_CAST(_t,_v,int64_t,Int64,_r)

  TEST_CAST_T(Null, , 0);

  TEST_CAST_T(Int64, 0, 0);
  TEST_CAST_T(Int64, 1, 1);
  TEST_CAST_T(Int64, 2000, 2000);
  TEST_CAST_T(Int64, LONG_LONG_MAX, LONG_LONG_MAX);
  TEST_CAST_T(Int64, -1, -1);
  TEST_CAST_T(Int64, -2000, -2000);
  TEST_CAST_T(Int64, LONG_LONG_MIN, LONG_LONG_MIN);
  
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
  

  // Test casting to double.
#define TEST_CAST_T(_t,_v,_r) TEST_CAST(_t,_v,double,Double,_r)

  TEST_CAST_T(Null, , 0);

  TEST_CAST_T(Int64, 0, 0);
  TEST_CAST_T(Int64, 1, 1);
  TEST_CAST_T(Int64, 2000, 2000);
  TEST_CAST_T(Int64, LONG_LONG_MAX, LONG_LONG_MAX);
  TEST_CAST_T(Int64, -1, -1);
  TEST_CAST_T(Int64, -2000, -2000);
  TEST_CAST_T(Int64, LONG_LONG_MIN, LONG_LONG_MIN);
  
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

  TEST_CAST_T(Int64, 0, "0");
  TEST_CAST_T(Int64, 1, "1");
  TEST_CAST_T(Int64, 2000, "2000");
  TEST_CAST_T(Int64, LONG_LONG_MAX, "9223372036854775807");
  TEST_CAST_T(Int64, -1, "-1");
  TEST_CAST_T(Int64, -2000, "-2000");
  TEST_CAST_T(Int64, LONG_LONG_MIN, "-9223372036854775808");
  
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
  TEST_CAST_T(Opaque, u2, "0x546869732069732055494f2032");


#undef TEST_CAST_T





  // Test casting to Time, time_t.tv_sec is of type int32
  
  // Modified this to test with ptimes instead of timespecs --ACR

#define TEST_CAST_T(_t,_v,_s,_ns) TEST_PTIME_CAST(_t,_v,Time,_s,_ns)

  TEST_CAST_T(Int64, 0, 0, 0);
  TEST_CAST_T(Int64, 1, 1, 0);
  TEST_CAST_T(Int64, 2000, 2000, 0);
  TEST_CAST_T(Int64, LONG_LONG_MAX, -1, 0);
  TEST_CAST_T(Int64, -1, -1, 0);
  TEST_CAST_T(Int64, -2000, -2000, 0);
  TEST_CAST_T(Int64, LONG_LONG_MIN, 0, 0);
  
  TEST_CAST_T(Double, 0, 0, 0);
  TEST_CAST_T(Double, 1.0, 1, 0);
  // TEST_CAST_T(Double, -1.0, -1, 0); // not robust cross-platform
  
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
 * Reinstated to fix type system 12/4/07 - PM
 */ 

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
  TEST_ID_CAST(Int64, 0, 0, 0, 0, 0, 0);
  TEST_ID_CAST(Int64, 1, 0, 0, 0, 0, 1);
  TEST_ID_CAST(Int64, 2000, 0, 0, 0, 0, 2000);
  TEST_ID_CAST(Int64, LONG_LONG_MAX, 0, 0, 0, ((uint32_t) (LONG_LONG_MAX >> 32)), ((uint32_t) LONG_LONG_MAX));
  TEST_ID_CAST(Int64, -1, 0, 0, 0, 0xffffffff, (uint32_t) -1);
  TEST_ID_CAST(Int64, -2000, 0, 0, 0, 0xffffffff, (uint32_t) -2000);
  TEST_ID_CAST(Int64, LONG_LONG_MIN, 0, 0, 0, ((uint32_t) (LONG_LONG_MIN >> 32)), ((uint32_t) LONG_LONG_MIN));
}
#undef TEST_ID_CAST

  
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

  TEST_BADCAST(Int64, -1, Null);
  TEST_BADCAST(Double, 1.0, Null);
  TEST_BADCAST(Str, "", Null);
  TEST_BADCAST(Str, "Hello", Null);
  TEST_BADCAST(Opaque, u2, Null);
  TEST_BADCAST(Opaque, u2, Int64);
  TEST_BADCAST(Opaque, u2, Double);

  TEST_BADCAST(Null,, Opaque);
  TEST_BADCAST(Int64, 0, Opaque);
  TEST_BADCAST(Double, 0, Opaque);

  TEST_BADCAST(Int64, 0, Tuple);
  TEST_BADCAST(Double, 0, Tuple);
  
  TEST_BADCAST(Int64, 0, List);
  TEST_BADCAST(Double, 0, List);

  TEST_BADCAST(Int64, 0, Set);
  TEST_BADCAST(Double, 0, Set);

  TEST_BADCAST(Int64, 0, Vector);
  TEST_BADCAST(Double, 0, Vector);

  TEST_BADCAST(Int64, 0, Matrix);
  TEST_BADCAST(Double, 0, Matrix);
}
#undef TEST_BADCAST


void
testImplicitConversions()
{
  {
    ValuePtr i1 = Val_Int64::mk(5);
    ValuePtr i2 = Val_Str::mk("10");
    ValuePtr r1 = i1 + i2;
    BOOST_CHECK_MESSAGE(r1->typeCode() == Value::INT64,
                        "int64 "
                        << i1->toString()
                        << " + str "
                        << i2->toString()
                        << " = not int64 "
                        << r1->toString());
  }

  {
    ValuePtr i1 = Val_Int64::mk(5);
    ValuePtr i2 = Val_Str::mk("10");
    ValuePtr r1 = i2 + i1;
    BOOST_CHECK_MESSAGE(r1->typeCode() == Value::STR,
                        "str "
                        << i2->toString()
                        << " + int64 "
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
