/* 
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * Or
 * UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
 * Berkeley, CA,  94707. Attention: P2 Group.
 */

#define BOOST_TEST_MAIN
#ifndef P2_LINK_WITH_BOOST_TEST
///  COMPILE BOOST TEST IN THIS COMPILATION UNIT------------------------

// We do this by #including from the "included" directory

// Since we're all one big compilation unit, we're certainly not
// dynamically linked.  You'd think that the "included/" stuff would
// kill this for us, but it doesn't on my machine...

// (are there other pieces of boost configuration flags to kill off?)
#undef BOOST_ALL_DYN_LINK
#undef BOOST_TEST_DYN_LINK

// Using boost test's main() breaks for some reason.
// This trumps the BOOST_TEST_MAIN mentioned above...
#define BOOST_TEST_NO_MAIN
// ...in theory, but not practice.
#undef BOOST_TEST_MAIN

///  DETERMINE BOOST VERSION -------------------------------------------
// They've been moving headers out from under us...
#include <boost/version.hpp>

// This has been tested with one subrelease of 1.33 and 1.34, and worked
// (Assumes they renamed the header at 103400.  Did they?)

#if (BOOST_VERSION < 103400)
#define P2_BOOST_TEST_FRAMEWORK
#endif

#ifndef P2_BOOST_TEST_FRAMEWORK
#include <boost/test/included/unit_test.hpp>
using namespace boost::unit_test;
#else
#include <boost/test/included/unit_test_framework.hpp>
using namespace boost::unit_test_framework;
#endif

#else
// Attempt to link to the precompiled version of the test stuff.
#include "boost/test/unit_test.hpp"
using boost::unit_test_framework::test_suite;
#endif

#include "testPel.h"
#include "testTable2.h"
#include "testRefTable.h"
#include "testMarshal.h"
#include "testFdbufs.h"
#include "testValues.h"
#include "testIDs.h"
#include "testLists.h"
#include "testSets.h"
// #include "testSecure.h"
#include "testAggwrap.h"
#include "reporting.h"
#include "iostream"

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
  top->add(new testFdbufs_testSuite());
  top->add(new testValues_testSuite());
  top->add(new testLists_testSuite());
  top->add(new testSets_testSuite());
  // top->add(new testSecure_testSuite());
  top->add(new testAggwrap_testSuite());

  return top;
}

#ifndef P2_LINK_WITH_BOOST_TEST
#ifndef P2_BOOST_TEST_FRAMEWORK
int main(int argc, char * argv[]) {
  return boost::unit_test::unit_test_main(&init_unit_test_suite,argc,argv);
}
#endif
#endif
