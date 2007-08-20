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
#include "p2core/val_int64.h"
#include <Sketches.h>
#include <sstream>
#include <cmath>
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
    
    Sketches::FM fm(32, 64, Sketches::HT_UNIVERSAL);
    
    fm.insert("0", 42);
    fm.insert("1", 17);
    fm.insert("2", 99);
    
    fm.marshal(&marshal_archive);
    
    std::istringstream unmarshal_stream(archive_stream.str());
    boost::archive::text_iarchive unmarshal_archive(unmarshal_stream);
    
    Sketches::FM *newFm = Sketches::FM::unmarshal(&unmarshal_archive);
    
    BOOST_REQUIRE(fm.compareTo(newFm) == 0);
  }
  
  void testCountMinFMSketch()
  {
    double epsilon = 0.05;
    double delta = 0.01;
    
    size_t width = (size_t) ceil(M_E / epsilon);
    size_t depth = (size_t) ceil(log(1.0 / delta));
    
    std::map<std::string, uint64_t> map;

    Tools::Random r;
    std::map<std::string, uint64_t>::iterator itEx;

    for (size_t i = 1; i <= 1000; i++)
    {
        uint32_t l = r.nextUniformLong(0, 10000);
        std::ostringstream ss;
        ss << l << std::flush;

        itEx = map.find(ss.str());
        if (itEx != map.end())
            (*itEx).second += 100;
        else
            map[ss.str()] = 100;
    }

    Sketches::CountMinFM sketch("spoon", map, width, depth, 32, 64, 
        Sketches::HT_UNIVERSAL);
        
    std::ostringstream archive_stream;
    boost::archive::text_oarchive marshal_archive(archive_stream);
    
    sketch.marshal(&marshal_archive);
    
    std::istringstream unmarshal_stream(archive_stream.str());
    boost::archive::text_iarchive unmarshal_archive(unmarshal_stream);
    
    Sketches::CountMinFM *newSketch = Sketches::CountMinFM::unmarshal(
        &unmarshal_archive);
    
    BOOST_REQUIRE(sketch.compareTo(newSketch) == 0);
  }
  
  void testSampleRuntime()
  {
    Tools::Random r;
    
    Val_Sketch *sketch = new Val_Sketch(42, 0.01, 0.01);

    std::map<std::string, uint64_t> testMap;

    // Insert values from a highly skewed Zipfian distribution.
    Tools::PRGZipf zipfianDistribution(0, 1000, 0.9, &r);
    
    for (size_t i = 1; i <= 100000; i++)
    {
      uint32_t l = zipfianDistribution.nextLong();
      std::ostringstream ss;
      ss << l << std::flush;
      
      sketch->insert(ss.str(), 100);
      
      testMap[ss.str()] += 100;
    }
    
    // Done inserting values, now we freeze the sketch. Freezing the sketch 
    // actually instantiates the Count-Min FM sketch object sitting underneath; 
    // this is mainly due to the fact that all values are inserted into the 
    // sketch at construction time.
    sketch->freeze();
    
    // At this point we can query the sketch for frequencies of values.
    std::map<std::string, uint64_t>::iterator iter;
    for (iter = testMap.begin(); iter != testMap.end(); iter++)
    {
      Val_Int64 frequency = sketch->getFrequency(iter->first);
    }
  }
};

testSketches_testSuite::testSketches_testSuite()
   : boost::unit_test_framework::test_suite("testSketches")
{
   boost::shared_ptr<testSketches> instance(new testSketches());
   add(BOOST_CLASS_TEST_CASE(&testSketches::testUniversalHashMarshal, 
     instance));
   add(BOOST_CLASS_TEST_CASE(&testSketches::testUniversalHashMarshal, 
       instance));
   add(BOOST_CLASS_TEST_CASE(&testSketches::testFMSketch, instance));
   add(BOOST_CLASS_TEST_CASE(&testSketches::testCountMinFMSketch, instance));
   add(BOOST_CLASS_TEST_CASE(&testSketches::testSampleRuntime, instance));
}
