#include "boost/test/unit_test.hpp"

#include "fdbuf.h"


// The test macro
#define TST(_e,_l,_s,_c) { _c;                                          \
    BOOST_CHECK_MESSAGE(fb.length() == _l, "Bad fdbuf length");         \
    BOOST_CHECK_MESSAGE(fb.last_errno() == _e, "Bad errno");            \
    BOOST_CHECK_MESSAGE(fb.str() == _s, "Bad fdbuf contents");          \
}


class testFdbufs
{
private:
  Fdbuf fb;



public:
  testFdbufs()
    : fb(0)
  {
  }

  
  void
  tests()
  {
    TST(0,0,"",);
    TST(0,0,"",fb.clear());
    TST(0,1,"1",fb.push_back(1));
    TST(0,2,"12",fb.push_back(2));
    TST(0,3,"122",fb.push_back("2"));
    TST(0,0,"",fb.clear());
    TST(0,12,"Hello, world",fb.push_back("Hello, world"));
    TST(0,30,"Hello, world, and other worlds", std::string s(", and other worlds"); fb.push_back(s));
    TST(0,0,"",fb.clear());
    TST(0,5,"1.234",fb.push_back(1.234));
    TST(0,6,"1.234a",fb.push_back('a'));
    TST(0,0,"",fb.clear());
    TST(0,5,"This ",fb.push_back("This is a string",5));
  }
};







class testFdbufs_testSuite
  : public boost::unit_test_framework::test_suite
{
public:
  testFdbufs_testSuite()
    : boost::unit_test_framework::test_suite("testFdbufs")
  {
    boost::shared_ptr<testFdbufs> instance(new testFdbufs());
    
    
    add(BOOST_CLASS_TEST_CASE(&testFdbufs::tests,
                              instance));
  }
};
