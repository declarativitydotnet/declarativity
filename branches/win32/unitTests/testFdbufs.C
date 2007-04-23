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

#include "fdbuf.h"
#include "value.h"
#include <rpc/xdr.h>
#include "xdrbuf.h"


#include "val_null.h"
#include "val_str.h"
#include "val_int32.h"
#include "val_uint32.h"
#include "val_int64.h"
#include "val_uint64.h"
#include "val_double.h"
#include "val_opaque.h"
#include "val_tuple.h"
#include "val_time.h"
#include "val_id.h"
#include "val_list.h"

#include "testFdbufs.h"

using namespace boost::gregorian;
using namespace boost::posix_time;

class testFdbufs
{
private:


public:
  testFdbufs()
  {
  }

  
  void
  originalTests()
  {
    Fdbuf fb(0);


    // The test macro
#define TST(_e,_l,_s,_c) { _c;                                          \
      BOOST_CHECK_MESSAGE(fb.length() == _l, "Bad fdbuf length");       \
      BOOST_CHECK_MESSAGE(fb.last_errno() == _e, "Bad errno");          \
      BOOST_CHECK_MESSAGE(fb.str() == _s, "Bad fdbuf contents");        \
    }
    
    TST(0,0,"",);
    TST(0,0,"",fb.clear());
    TST(0,1,"1",fb.pushBack(1));
    TST(0,2,"12",fb.pushBack(2));
    TST(0,3,"122",fb.pushBack("2"));
    TST(0,0,"",fb.clear());
    TST(0,12,"Hello, world",fb.pushBack("Hello, world"));
    TST(0,30,"Hello, world, and other worlds", std::string s(", and other worlds"); fb.pushBack(s));
    TST(0,0,"",fb.clear());
    TST(0,5,"1.234",fb.pushBack(1.234));
    TST(0,6,"1.234a",fb.pushBack('a'));
    TST(0,0,"",fb.clear());
    TST(0,5,"This ",fb.push_bytes("This is a string",5));

#undef TST
  }


  void
  xdrTest(ValuePtr in)
  {
    Fdbuf fin(0);
    P2_XDR xe;
    xdrfdbuf_create(&xe, &fin, false, XDR_ENCODE);
    in->xdr_marshal(&xe);
    
    P2_XDR xd;
    Fdbuf fout(0);
    fout.pushFdbuf(fin, fin.length());
    xdrfdbuf_create(&xd, &fout, false, XDR_DECODE);
    ValuePtr out = Value::xdr_unmarshal(&xd);
    
    BOOST_CHECK_MESSAGE(out->compareTo(in) == 0,
                        "Marshalled/unmarshalled mismatch!\n");
    
    if (out->compareTo(in) != 0) {
      char mybuf[1024];
      BOOST_CHECK_MESSAGE(1, mybuf);
    }
  }
  
  void
  xdrTests()
  {
    xdrTest(Val_Null::mk());
    xdrTest(Val_Str::mk("Test String"));
    xdrTest(Val_Int32::mk(-12));
    xdrTest(Val_UInt32::mk(12));
    xdrTest(Val_Int64::mk(-24));
    xdrTest(Val_UInt64::mk(24));
    xdrTest(Val_Double::mk(0.1856));

    TuplePtr t = Tuple::mk();
    t->append(Val_Str::mk("Flat"));
    t->append(Val_UInt64::mk(13500975));
    t->freeze();  
    xdrTest(Val_Tuple::mk(t));
    
    xdrTest(Val_List::mk(List::mk()));
    
    ListPtr l = List::mk();
    l->append(Val_Str::mk("Foo"));
    l->append(Val_UInt64::mk(13117));
    l->append(Val_Int32::mk(42));
    
    xdrTest(Val_List::mk(l));
    
    //	boost::posix_time::ptime time = boost::posix_time::second_clock::universal_time();
    //getTime(time);
    
    boost::posix_time::ptime time(date(1970, Jan, 1));
    time += hours(2) + seconds(25) + nanoseconds(50203);
	
    xdrTest(Val_Time::mk(time));
    
    boost::posix_time::time_duration td(0,0,5,0);
    td += boost::posix_time::nanoseconds(1020430);
    xdrTest(Val_Time_Duration::mk(td));
    
    IDPtr id(new ID((uint32_t) 102040));
    xdrTest(Val_ID::mk(id));
  }
};







testFdbufs_testSuite::testFdbufs_testSuite()
  : boost::unit_test_framework::test_suite("testFdbufs")
{
  boost::shared_ptr<testFdbufs> instance(new testFdbufs());
  
  
  add(BOOST_CLASS_TEST_CASE(&testFdbufs::originalTests,
                            instance));
  add(BOOST_CLASS_TEST_CASE(&testFdbufs::xdrTests,
                            instance));
}
