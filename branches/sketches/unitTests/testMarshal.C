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
#include "tuple.h"

#include "marshal.h"
#include "unmarshal.h"
#include "marshalField.h"
#include "unmarshalField.h"

#include "val_str.h"
#include "val_tuple.h"
#include "val_uint64.h"
#include "val_vector.h"
#include "val_matrix.h"

#include "testMarshal.h"

class testMarshal
{
private:
  Marshal m;
  Unmarshal u;
  MarshalField mF;
  UnmarshalField uF;
  TuplePtr flat;
  TuplePtr nested;
  TuplePtr vecmat;
  
public:
  testMarshal()
    : m("Marshal"),
      u("Unmarshal"),
      mF("MarshalField", 1),
      uF("UnmarshalField", 1)
  {
    // Create a straight tuple
    // The input tuple
    flat = Tuple::mk();
    flat->append(Val_Str::mk("Flat"));
    flat->append(Val_UInt64::mk(13500975));
    flat->freeze();  

    // Create a nested tuple
    nested = Tuple::mk();
    nested->append(Val_Str::mk("Nested"));
    nested->append(Val_Tuple::mk(flat));
    nested->freeze();

	// Create a tuple with vecmat fields
	vecmat = Tuple::mk();
	static unsigned int zero = 0;
	static unsigned int one = 1;
	static unsigned int two = 2;
	static unsigned int three = 3;
	static unsigned int four = 4;
	static unsigned int five = 5;
	static unsigned int six = 6;
	Val_Vector *v = new Val_Vector(four);
	Val_Matrix *m = new Val_Matrix(two, three);
	
	v->insert(zero, Val_UInt64::mk(one));
	v->insert(one, Val_UInt64::mk(two));
	v->insert(two, Val_UInt64::mk(three));
	v->insert(three, Val_UInt64::mk(four));
	m->insert(zero, zero, Val_UInt64::mk(one));
	m->insert(zero, one, Val_UInt64::mk(two));
	m->insert(zero, two, Val_UInt64::mk(three));
	m->insert(one, zero, Val_UInt64::mk(four));
	m->insert(one, one, Val_UInt64::mk(five));
	m->insert(one, two, Val_UInt64::mk(six));

    vecmat->append(Val_Str::mk("vecmat"));
	ValuePtr vp(v);
	ValuePtr vp2(m);
    vecmat->append(vp);
    vecmat->append(vp2);
    vecmat->freeze();  
  }

  void
  testM()
  {
    // Marshal it
    TuplePtr result = m.simple_action(flat);
    
    BOOST_REQUIRE_MESSAGE(result != NULL, "Didn't get a marshaling result");
    BOOST_CHECK_MESSAGE(result->size() == 1, "Marshalled tuple has size != 1\n");
    BOOST_CHECK_MESSAGE((*result)[0]->typeCode() == Value::P2_OPAQUE,
                        "Marshalled field is not OPAQUE 1\n");
  }

  void
  testU()
  {
    // Marshal it and unmarshal it
    TuplePtr result = m.simple_action(flat);
    TuplePtr reFlat = u.simple_action(result);

    BOOST_REQUIRE_MESSAGE(reFlat != NULL, "Didn't get an unmarshaling result");
    BOOST_CHECK_MESSAGE(reFlat->size() == flat->size(),
                        "Marshalled/unmarshalled tuple is different size");
    BOOST_CHECK_MESSAGE(reFlat->compareTo(flat) == 0,
                        "Marshalled/unmarshalled tuple does not match original");
  }


  void
  testMF()
  {
    // Marshal it
    TuplePtr result = mF.simple_action(nested);
    
    BOOST_REQUIRE_MESSAGE(result != NULL, "Didn't get a field marshaling result");
    BOOST_CHECK_MESSAGE(result->size() == 2, "Outer tuple has wrong size\n");
    BOOST_CHECK_MESSAGE((*result)[1]->typeCode() == Value::P2_OPAQUE,
                        "Marshalled field is not OPAQUE 1\n");
  }


  void
  testUF()
  {
    // Marshal it and unmarshal it
    TuplePtr result = mF.simple_action(nested);
    TuplePtr reNested = uF.simple_action(result);

    BOOST_REQUIRE_MESSAGE(reNested != NULL, "Didn't get a field unmarshaling result");
    BOOST_CHECK_MESSAGE(reNested->size() == nested->size(),
                        "Field marshalled/unmarshalled tuple is different size");
    BOOST_CHECK_MESSAGE(reNested->compareTo(nested) == 0,
                        "Field marshalled/unmarshalled tuple does not match original");
  }

  //  XXXXX
  void
  testMVM()
  {
    // Marshal it
    TuplePtr result = m.simple_action(vecmat);
    
    BOOST_REQUIRE_MESSAGE(result != NULL, "Didn't get a marshaling result for vecmat");
    BOOST_CHECK_MESSAGE(result->size() == 1, "Outer tuple has wrong size\n");
    BOOST_CHECK_MESSAGE((*result)[0]->typeCode() == Value::P2_OPAQUE,
                        "Marshalled field is not OPAQUE 1\n");
  }


  void
  testUVM()
  {
    // Marshal it and unmarshal it
    TuplePtr result = m.simple_action(vecmat);
    TuplePtr reVecmat = u.simple_action(result);

    BOOST_REQUIRE_MESSAGE(reVecmat != NULL, "Didn't get a field unmarshaling vecmat result");
    BOOST_CHECK_MESSAGE(reVecmat->size() == vecmat->size(),
                        "Field marshalled/unmarshalled vecmat tuple is different size");
    BOOST_CHECK_MESSAGE(reVecmat->compareTo(vecmat) == 0,
                        "Field marshalled/unmarshalled vecmat tuple does not match original");
  }
};

testMarshal_testSuite::testMarshal_testSuite()
  : boost::unit_test_framework::test_suite("testMarshal: Marshaling/Unmarshaling")
{
  boost::shared_ptr<testMarshal> instance(new testMarshal());
  
  add(BOOST_CLASS_TEST_CASE(&testMarshal::testM, instance));
  add(BOOST_CLASS_TEST_CASE(&testMarshal::testU, instance));
  add(BOOST_CLASS_TEST_CASE(&testMarshal::testMF, instance));
  add(BOOST_CLASS_TEST_CASE(&testMarshal::testUF, instance));
  add(BOOST_CLASS_TEST_CASE(&testMarshal::testMVM, instance));
  add(BOOST_CLASS_TEST_CASE(&testMarshal::testUVM, instance));
}
