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

#include <sstream>

#include "val_null.h"
#include "val_str.h"
#include "val_int32.h"
#include "val_uint32.h"
#include "val_int64.h"
#include "val_uint64.h"
#include "val_double.h"
#include "val_opaque.h"


class testIDs
{
public:
  testIDs()
  {
  }



  ////////////////////////////////////////////////////////////
  // Comparison Tests
  ////////////////////////////////////////////////////////////
  
private:
  struct CmpTest {
    int	line;		// Line number of test
    uint32_t lw0;
    uint32_t lw1;
    uint32_t lw2;
    uint32_t lw3;
    uint32_t lw4;
    uint32_t rw0;
    uint32_t rw1;
    uint32_t rw2;
    uint32_t rw3;
    uint32_t rw4;
    int result;
  };

  static const CmpTest idCmpTests[];

  static const size_t num_cmptests;

public:
  
  void
  testComparisons();





  ////////////////////////////////////////////////////////////
  // Between Tests
  ////////////////////////////////////////////////////////////
private:
  

};



void
testIDs::testComparisons()
{
  for (size_t i = 0;
       i < num_cmptests;
       i++) {
    uint32_t left[5];
    uint32_t right[5];
    const CmpTest * idTest = &idCmpTests[i];

    left[0] = idTest->lw0;
    left[1] = idTest->lw1;
    left[2] = idTest->lw2;
    left[3] = idTest->lw3;
    left[4] = idTest->lw4;

    right[0] = idTest->rw0;
    right[1] = idTest->rw1;
    right[2] = idTest->rw2;
    right[3] = idTest->rw3;
    right[4] = idTest->rw4;

    IDPtr leftID = ID::mk(left);
    IDPtr rightID = ID::mk(right);

    int result = leftID->compareTo(rightID);
    std::ostringstream message;
    message << "ID test at line "
            << idTest->line
            << ". Comparing ID '"
            << leftID->toString()
            << "' from ["
            << left[4]
            << ","
            << left[3]
            << ","
            << left[2]
            << ","
            << left[1]
            << ","
            << left[0]
            << "] to ID '"
            << rightID->toString()
            << "' from ["
            << right[4]
            << ","
            << right[3]
            << ","
            << right[2]
            << ","
            << right[1]
            << ","
            << right[0]
            << "]. Expected '"
            << idTest->result
            << "' but got '"
            << result
            << "'.";
    BOOST_CHECK_MESSAGE(idTest->result == result,
                        message.str().c_str());
  }
}

#define CMPTST(_lword0, _lword1, _lword2, _lword3, _lword4, _rword0, _rword1, _rword2, _rword3, _rword4, _result) {__LINE__, _lword0, _lword1, _lword2, _lword3, _lword4, _rword0, _rword1, _rword2, _rword3, _rword4, _result}

const testIDs::CmpTest
testIDs::idCmpTests[] = {
  // Least significant comparisons
  CMPTST(0, 0, 0, 0, 0,            0, 0, 0, 0, 2,          -1),
  CMPTST(0, 0, 0, 0, 0,            0, 0, 0, 0, 100,        -1),
  CMPTST(0, 0, 0, 0, 0,            0, 0, 0, 0, 100000,     -1),
  CMPTST(0, 0, 0, 0, 1,            0, 0, 0, 0, 2,          -1),
  CMPTST(0, 0, 0, 0, 10,           0, 0, 0, 0, 100,        -1),
  CMPTST(0, 0, 0, 0, 100,          0, 0, 0, 0, 100000,     -1),

  CMPTST(0, 0, 0, 0, 2,            0, 0, 0, 0, 1,          1),
  CMPTST(0, 0, 0, 0, 101,          0, 0, 0, 0, 100,        1),
  CMPTST(0, 0, 0, 0, 100002,       0, 0, 0, 0, 100000,     1),
  CMPTST(0, 0, 0, 0, 100,          0, 0, 0, 0, 1,          1),
  CMPTST(0, 0, 0, 0, 100000,       0, 0, 0, 0, 100,        1),
  CMPTST(0, 0, 0, 0, 10000000,     0, 0, 0, 0, 100000,     1),

  CMPTST(0, 0, 0, 0, 1,            0, 0, 0, 0, 1,          0),
  CMPTST(0, 0, 0, 0, 100,          0, 0, 0, 0, 100,        0),
  CMPTST(0, 0, 0, 0, 100000,       0, 0, 0, 0, 100000,     0),


  // Most significant comparisons 
  CMPTST(0, 0, 0, 0, 0,            2, 0, 0, 0, 0,          -1),
  CMPTST(0, 0, 0, 0, 0,            100, 0, 0, 0, 0,        -1),
  CMPTST(0, 0, 0, 0, 0,            100000, 0, 0, 0, 0,     -1),
  CMPTST(1, 0, 0, 0, 0,            2, 0, 0, 0, 0,          -1),
  CMPTST(10, 0, 0, 0, 0,           100, 0, 0, 0, 0,        -1),
  CMPTST(100, 0, 0, 0, 0,          100000, 0, 0, 0, 0,     -1),

  CMPTST(2, 0, 0, 0, 0,            1, 0, 0, 0, 0,          1),
  CMPTST(101, 0, 0, 0, 0,          100, 0, 0, 0, 0,        1),
  CMPTST(100002, 0, 0, 0, 0,       100000, 0, 0, 0, 0,     1),
  CMPTST(100, 0, 0, 0, 0,          1, 0, 0, 0, 0,          1),
  CMPTST(100000, 0, 0, 0, 0,       100, 0, 0, 0, 0,        1),
  CMPTST(10000000, 0, 0, 0, 0,     100000, 0, 0, 0, 0,     1),

  CMPTST(1, 0, 0, 0, 0,            1, 0, 0, 0, 0,          0),
  CMPTST(100, 0, 0, 0, 0,          100, 0, 0, 0, 0,        0),
  CMPTST(100000, 0, 0, 0, 0,       100000, 0, 0, 0, 0,     0),


  // Equal then different
  CMPTST(0, 0, 0, 0, 0,            0, 0, 2, 0, 0,          -1),
  CMPTST(0, 0, 0, 0, 0,            0, 0, 100, 0, 0,        -1),
  CMPTST(0, 0, 0, 0, 0,            0, 0, 100000, 0, 0,     -1),
  CMPTST(0, 0, 1, 0, 0,            0, 0, 2, 0, 0,          -1),
  CMPTST(0, 0, 10, 0, 0,           0, 0, 100, 0, 0,        -1),
  CMPTST(0, 0, 100, 0, 0,          0, 0, 100000, 0, 0,     -1),

  CMPTST(0, 0, 2, 0, 0,            0, 0, 1, 0, 0,          1),
  CMPTST(0, 0, 101, 0, 0,          0, 0, 100, 0, 0,        1),
  CMPTST(0, 0, 100002, 0, 0,       0, 0, 100000, 0, 0,     1),
  CMPTST(0, 0, 100, 0, 0,          0, 0, 1, 0, 0,          1),
  CMPTST(0, 0, 100000, 0, 0,       0, 0, 100, 0, 0,        1),
  CMPTST(0, 0, 10000000, 0, 0,     0, 0, 100000, 0, 0,     1),

  CMPTST(0, 0, 1, 0, 0,            0, 0, 1, 0, 0,          0),
  CMPTST(0, 0, 100, 0, 0,          0, 0, 100, 0, 0,        0),
  CMPTST(0, 0, 100000, 0, 0,       0, 0, 100000, 0, 0,     0),


  CMPTST(0, 0, 0, 0, 0,            0, 0, 0, 0, 0,          0)
};


const size_t
testIDs::num_cmptests = (sizeof(testIDs::idCmpTests) /
                         sizeof(testIDs::CmpTest));

#undef CMPTST




class testIDs_testSuite
  : public boost::unit_test_framework::test_suite
{
public:
  testIDs_testSuite()
    : boost::unit_test_framework::test_suite("testIDs")
  {
    boost::shared_ptr<testIDs> instance(new testIDs());
    
    add(BOOST_CLASS_TEST_CASE(&testIDs::testComparisons,
                              instance));
  }
};
