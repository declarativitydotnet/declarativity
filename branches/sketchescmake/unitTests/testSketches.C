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
#include <sstream>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

class testSketches 
{
private:
  
public:
  void testUniversalHashMarshal() 
  {
    std::ostringstream archive_stream;
    boost::archive::text_oarchive marshal_archive(archive_stream);
    
    Tools::UniversalHash hash;
    
    hash.marshal(&marshal_archive);
    
    std::istringstream unmarshal_stream(archive_stream.str());
    boost::archive::text_iarchive unmarshal_archive(unmarshal_stream);
    
    Tools::UniversalHash *newHash = Tools::UniversalHash::unmarshal(&unmarshal_archive);
    
    BOOST_REQUIRE(hash == *newHash);
  }

  void testFMSketch()
  {
    std::ostringstream archive_stream;
    boost::archive::text_oarchive marshal_archive(archive_stream);
    
    Sketches::FM fm;
    
    std::istringstream unmarshal_stream(archive_stream.str());
    boost::archive::text_iarchive unmarshal_archive(unmarshal_stream);
    
    Sketches::FM *newFm = Sketches::FM::unmarshal(&unmarshal_archive);
    
    BOOST_REQUIRE(fm.compareTo(newFm) == 0);
  }
  
  void testCountMinFMSketch()
  {
    //TODO: Finish me!
  }
};

 testSketches_testSuite::testSketches_testSuite()
   : boost::unit_test_framework::test_suite("testSketches")
 {
   boost::shared_ptr<testSketches> instance(new testSketches());
   add(BOOST_CLASS_TEST_CASE(&testSketches::testUniversalHashMarshal, 
     instance));
   
   add(BOOST_CLASS_TEST_CASE(&testSketches::testFMSketch, instance));

   // add(BOOST_CLASS_TEST_CASE(&testFdbufs::originalTests,
   //                           instance));
   // add(BOOST_CLASS_TEST_CASE(&testFdbufs::xdrTests,
   //                           instance));
 }
