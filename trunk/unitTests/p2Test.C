#include "boost/test/unit_test.hpp"

#include "testMarshal.C"

boost::unit_test_framework::test_suite*
init_unit_test_suite(int, char**)
{
  std::auto_ptr<boost::unit_test_framework::test_suite>
    top(BOOST_TEST_SUITE("P2 Unit Test Suite"));

  top->add(new testMarshal_testSuite());

  return top.release();
}
