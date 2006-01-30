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

#include <sstream>

#include "value.h"
#include "val_ip_addr.h"

class testValues
{
public:
  testValues()
  {
  }



private:
#define TEST_VAL(valTypeExt, value, typecode, typestr)  \
  { \
    std::string testID; \
    { \
      std::ostringstream ID; \
      ID << "Value construction test at line " \
         << __LINE__; \
      testID = ID.str(); \
    } \
     \
     \
    ValuePtr v = Val_##valTypeExt::mk(value); \
    { \
      std::ostringstream message; \
      message << testID \
              << ". Bad typeCode after construction of '" \
              << typestr \
              << "'. Expected '" \
              << Value::typecode \
              << "' but got '" \
              << v->typeCode() \
              << "'."; \
      BOOST_CHECK_MESSAGE(v->typeCode() == Value::typecode,     \
                          message); \
    } \
     \
    string mktn(typestr); \
    { \
      std::ostringstream message; \
      message << testID \
              << ". Bad typeName after construction of '" \
              << typestr \
              << "'. Expected '" \
              << typestr \
              << "' but got '" \
              << v->typeName() \
              << "'"; \
      BOOST_CHECK_MESSAGE(mktn == v->typeName(), \
                          message); \
    } \
  } \


public:
  void
  testConstructions()
  {
    TEST_VAL(Null, , NULLV, "null");
    
    TEST_VAL(Int32, 0, INT32, "int32" );
    TEST_VAL(Int32, 1, INT32, "int32" );
    TEST_VAL(Int32, 2000, INT32, "int32" );
    TEST_VAL(Int32, INT_MAX, INT32, "int32" );
    TEST_VAL(Int32, -1, INT32, "int32" );
    TEST_VAL(Int32, -2000, INT32, "int32" );
    TEST_VAL(Int32, INT_MIN, INT32, "int32" );

    TEST_VAL(Int64, 0, INT64, "int64" );
    TEST_VAL(Int64, 1, INT64, "int64" );
    TEST_VAL(Int64, 2000, INT64, "int64" );
    TEST_VAL(Int64, LONG_LONG_MAX, INT64, "int64" );
    TEST_VAL(Int64, -1, INT64, "int64" );
    TEST_VAL(Int64, -2000, INT64, "int64" );
    TEST_VAL(Int64, LONG_LONG_MIN, INT64, "int64" );
  
    TEST_VAL(UInt32, 0, UINT32, "uint32" );
    TEST_VAL(UInt32, 1, UINT32, "uint32" );
    TEST_VAL(UInt32, 1000, UINT32, "uint32" );
    TEST_VAL(UInt32, UINT_MAX, UINT32, "uint32" );

    TEST_VAL(UInt64, 0, UINT64, "uint64" );
    TEST_VAL(UInt64, 1, UINT64, "uint64" );
    TEST_VAL(UInt64, 1000, UINT64, "uint64" );
    TEST_VAL(UInt64, UINT_MAX, UINT64, "uint64" );

    TEST_VAL(Double, 0, DOUBLE, "double");
    TEST_VAL(Double, 1.0, DOUBLE, "double");
    TEST_VAL(Double, -1.0, DOUBLE, "double");
    TEST_VAL(Double, -1.79769E+308, DOUBLE, "double");
    TEST_VAL(Double, 1.79769E+308, DOUBLE, "double");
    TEST_VAL(Double, 2.225E-307, DOUBLE, "double");
    TEST_VAL(Double, -2.225E-307, DOUBLE, "double");

    TEST_VAL(Str, "", STR, "str");
    TEST_VAL(Str, "This is a string", STR, "str");
    
    FdbufPtr u1(new Fdbuf());
    u1->pushBack("This is UIO 1");
    TEST_VAL(Opaque, u1, OPAQUE, "opaque");
    
    std::string addr = "127.0.0.1:1000";
    TEST_VAL(IP_ADDR, addr, IP_ADDR, "ip_addr"); 

#undef TEST_VAL
  } 
};


class testValues_testSuite
  : public boost::unit_test_framework::test_suite
{
public:
  testValues_testSuite()
    : boost::unit_test_framework::test_suite("testValues")
  {
    boost::shared_ptr<testValues> instance(new testValues());
    
    
    add(BOOST_CLASS_TEST_CASE(&testValues::testConstructions,
                              instance));
  }
};
