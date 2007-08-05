/* 
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * Or
 * UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
 * Berkeley, CA,  94707. Attention: P2 Group.
 */

#include "testSketches.h"
#include <Tools.h>
#include <Sketches.h>

class testSketches 
{
private:
  
public:
  void testUniversalHashMarshal() 
  {
    Tools::UniversalHash hash();
  }
};

 testSketches_testSuite::testSketches_testSuite()
   : boost::unit_test_framework::test_suite("testSketches")
 {
   boost::shared_ptr<testSketches> instance(new testSketches());

   

   // add(BOOST_CLASS_TEST_CASE(&testFdbufs::originalTests,
   //                           instance));
   // add(BOOST_CLASS_TEST_CASE(&testFdbufs::xdrTests,
   //                           instance));
 }
