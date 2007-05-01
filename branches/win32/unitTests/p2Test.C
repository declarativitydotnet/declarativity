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

#include "testPel.h"
#include "testTable2.h"
#include "testRefTable.h"
#include "testMarshal.h"
#include "testBasicElementPlumbing.h"
#include "testFdbufs.h"
#include "testValues.h"
#include "testCsv.h"
#include "testIDs.h"
#include "testLists.h"
#include "testAggwrap.h"
#include "reporting.h"
#include "iostream"
#include "getopt.h"

test_suite* init_unit_test_suite(int argc, char** argv)
{
  Reporting::setLevel(Reporting::OUTPUT);

  // Set up reporting
  int c;
  while ((c = getopt(argc, argv, "r:")) != -1) {
    switch (c) {
    case 'r':
      {
        // My minimum reporting level is optarg
        std::string levelName(optarg);
        Reporting::Level level =
          Reporting::levelFromName()[levelName];
        Reporting::setLevel(level);
      }
      break;
    default:
      std::cerr << "Unrecognized option.\n";
      break;
    }
  }

  test_suite *top = BOOST_TEST_SUITE("P2 Unit Test Suite");
  
  top->add(new testIDs_testSuite());
  top->add(new testPel_testSuite());
  top->add(new testTable2_testSuite());
  top->add(new testRefTable_testSuite());
  top->add(new testMarshal_testSuite());
  top->add(new testBasicElementPlumbing_testSuite());
  top->add(new testFdbufs_testSuite());
  top->add(new testValues_testSuite());
  top->add(new testCsv_testSuite());
  top->add(new testLists_testSuite());
  top->add(new testAggwrap_testSuite());

  return top;
}
