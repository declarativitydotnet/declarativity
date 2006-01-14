#include "boost/test/unit_test.hpp"
#include "tuple.h"

#include "marshal.h"
#include "unmarshal.h"
#include "marshalField.h"
#include "unmarshalField.h"

#include "val_str.h"
#include "val_tuple.h"
#include "val_uint64.h"

class testMarshal
{
private:
  Marshal m;
  Unmarshal u;
  MarshalField mF;
  UnmarshalField uF;
  TuplePtr flat;
  
public:
  testMarshal()
    : m("Marshal"),
      u("Unmarshal"),
      mF("MarshalField", 0),
      uF("UnmarshalField", 0)
  {
    // Create a straight tuple
    // The input tuple
    flat = Tuple::mk();
    flat->append(Val_Str::mk("String"));
    flat->append(Val_UInt64::mk(13500975));
    flat->freeze();  
  }

  void
  testM()
  {
    // Marshal it
    TuplePtr result = m.simple_action(flat);
    
    BOOST_REQUIRE_MESSAGE(result != NULL, "Didn't get a marshaling result");
    BOOST_CHECK_MESSAGE(result->size() == 1, "Marshalled tuple has size != 1\n");
    BOOST_CHECK_MESSAGE((*result)[0]->typeCode() == Value::OPAQUE,
                        "Marshalled field is not OPAQUE 1\n");
  }

  void
  testU()
  {
    // Marshal it and unmarshal it
    TuplePtr result = m.simple_action(flat);
    TuplePtr reFlat = u.simple_action(result);

    BOOST_REQUIRE_MESSAGE(reFlat != NULL, "Didn't get a unmarshaling result");
    BOOST_CHECK_MESSAGE(reFlat->size() == flat->size(),
                        "Marshalled/unmarshalled tuple is different size");
    BOOST_CHECK_MESSAGE(reFlat->compareTo(flat) == 0,
                        "Marshalled/unmarshalled tuple does not match original");
  }
};

class testMarshal_testSuite
  : public boost::unit_test_framework::test_suite
{
public:
  testMarshal_testSuite()
    : boost::unit_test_framework::test_suite("testMarshal: Marshaling/Unmarshaling")
  {
    boost::shared_ptr<testMarshal> instance(new testMarshal());

    add(BOOST_CLASS_TEST_CASE(&testMarshal::testM, instance));
    add(BOOST_CLASS_TEST_CASE(&testMarshal::testU, instance));
  }
};
