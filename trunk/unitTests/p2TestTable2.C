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
using boost::unit_test_framework::test_suite; 

#include "testTable2.h"

test_suite* init_unit_test_suite(int, char**)
{
  test_suite *top = BOOST_TEST_SUITE("P2 Unit Test Suite for Table2");
  
  top->add(new testTable2_testSuite());
  return top;
}

