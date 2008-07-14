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
  
  struct BetweenTest {
    int	line;		// Line number of test
    uint32_t mw0;
    uint32_t mw1;
    uint32_t mw2;
    uint32_t mw3;
    uint32_t mw4;
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
    bool oores;
    bool ocres;
    bool cores;
    bool ccres;
  };

  static const BetweenTest idBetweenTests[];

  static const size_t num_betweentests;

public:
  void
  testBetween();


  /////////////////////////////////////////////////////////////
  // Distance tests
  /////////////////////////////////////////////////////////////
private:
  struct DistTest {
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
    uint32_t mw0;
    uint32_t mw1;
    uint32_t mw2;
    uint32_t mw3;
    uint32_t mw4;
  };

  static const DistTest idDistTests[];

  static const size_t num_disttests;

public:

  void
  testDistance();


private:
  ////////////////////////////////////////////////////////////
  // Shift tests
  ////////////////////////////////////////////////////////////
  struct ShiftTest {
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
    uint32_t shift;
  };



  static const ShiftTest idShiftTests[];
  
  static const size_t num_shiftTests;

public:

  void
  testShifts();



  ////////////////////////////////////////////////////////////
  // Addition tests
  ////////////////////////////////////////////////////////////
private:
  struct AddTest {
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
    uint32_t mw0;
    uint32_t mw1;
    uint32_t mw2;
    uint32_t mw3;
    uint32_t mw4;
  };

  static const AddTest idAddTests[];

  static const size_t num_addtests;
public:

  void
  testAdditions();
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
















void
testIDs::testBetween()
{
  for (size_t i = 0;
       i < num_betweentests;
       i++) {
    uint32_t left[5];
    uint32_t right[5];
    uint32_t middle[5];
    const BetweenTest * idTest = &idBetweenTests[i];

    middle[0] = idTest->mw0;
    middle[1] = idTest->mw1;
    middle[2] = idTest->mw2;
    middle[3] = idTest->mw3;
    middle[4] = idTest->mw4;

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
    IDPtr middleID = ID::mk(middle);

    bool resultoo = middleID->betweenOO(leftID, rightID);
    bool resultoc = middleID->betweenOC(leftID, rightID);
    bool resultco = middleID->betweenCO(leftID, rightID);
    bool resultcc = middleID->betweenCC(leftID, rightID);



    {
      std::ostringstream message;
      message << "ID test at line "
              << idTest->line
              << ". Containment of '"
              << middleID->toString()
              << "' in ('"
              << leftID->toString()
              << "', '"
              << rightID->toString()
              << "') returned "
              << resultoo
              << " but should have returned "
              << idTest->oores
              << ".";
      BOOST_CHECK_MESSAGE(idTest->oores == resultoo,
                          message.str().c_str());
    }


    {
      std::ostringstream message;
      message << "ID test at line "
              << idTest->line
              << ". Containment of '"
              << middleID->toString()
              << "' in ('"
              << leftID->toString()
              << "', '"
              << rightID->toString()
              << "'] returned "
              << resultoc
              << " but should have returned "
              << idTest->ocres
              << ".";
      BOOST_CHECK_MESSAGE(idTest->ocres == resultoc,
                          message.str().c_str());
    }


    {
      std::ostringstream message;
      message << "ID test at line "
              << idTest->line
              << ". Containment of '"
              << middleID->toString()
              << "' in ['"
              << leftID->toString()
              << "', '"
              << rightID->toString()
              << "') returned "
              << resultco
              << " but should have returned "
              << idTest->cores
              << ".";
      BOOST_CHECK_MESSAGE(idTest->cores == resultco,
                          message.str().c_str());
    }


    {
      std::ostringstream message;
      message << "ID test at line "
              << idTest->line
              << ". Containment of '"
              << middleID->toString()
              << "' in ['"
              << leftID->toString()
              << "', '"
              << rightID->toString()
              << "'] returned "
              << resultcc
              << " but should have returned "
              << idTest->ccres
              << ".";
      BOOST_CHECK_MESSAGE(idTest->ccres == resultcc,
                          message.str().c_str());
    }
  }
}


#define BETWEENTEST(_mword0, _mword1, _mword2, _mword3, _mword4, _lword0, _lword1, _lword2, _lword3, _lword4, _rword0, _rword1, _rword2, _rword3, _rword4, _ooresult, _ocresult, _coresult, _ccresult) {__LINE__, _mword0, _mword1, _mword2, _mword3, _mword4, _lword0, _lword1, _lword2, _lword3, _lword4, _rword0, _rword1, _rword2, _rword3, _rword4, _ooresult, _ocresult, _coresult, _ccresult}

const testIDs::BetweenTest
testIDs::idBetweenTests[] = {
  // Least significant comparisons ////////////////////////////////////////////////oo////oc////co////cc
  BETWEENTEST(0, 0, 0, 0, 1,            0, 0, 0, 0, 0,            0, 0, 0, 0, 2,        true, true, true, true),
  BETWEENTEST(0, 0, 0, 0, 0,            0, 0, 0, 0, 0,            0, 0, 0, 0, 2,        false, false, true, true),
  BETWEENTEST(0, 0, 0, 0, 2,            0, 0, 0, 0, 0,            0, 0, 0, 0, 2,        false, true, false, true),

  BETWEENTEST(0, 0, 0, 0, 10,           0, 0, 0, 0, 1,            0, 0, 0, 0, 523,      true, true, true, true),
  BETWEENTEST(0, 0, 0, 0, 1,            0, 0, 0, 0, 0,            0, 0, 0, 0, 0,        true, true, true, true),

  BETWEENTEST(0, 0, 0, 0, 2,            0, 0, 0, 0, 0,            0, 0, 0, 0, 2,        false, true, false, true)
};

const size_t testIDs::num_betweentests =
               sizeof(testIDs::idBetweenTests) /
               sizeof(testIDs::BetweenTest);

#undef BETWEENTEST








void
testIDs::testDistance()
{
  for (size_t i = 0;
       i < num_disttests;
       i++) {
    uint32_t middle[5];
    uint32_t left[5];
    uint32_t right[5];
    const DistTest * idTest = &idDistTests[i];

    middle[0] = idTest->mw0;
    middle[1] = idTest->mw1;
    middle[2] = idTest->mw2;
    middle[3] = idTest->mw3;
    middle[4] = idTest->mw4;

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
    IDPtr middleID = ID::mk(middle);
    IDPtr resultID = leftID->distance(rightID);

    std::ostringstream message;
    message << "ID test at line "
            << idTest->line
            << ". Distance of '"
            << leftID->toString()
            << "' to '"
            << rightID->toString()
            << "' should be '"
            << middleID->toString()
            << "' but I got '"
            << resultID->toString()
            << "'.";
    BOOST_CHECK_MESSAGE(middleID->compareTo(resultID) == 0,
                        message.str().c_str());
  }
}

#define DISTTEST(_lword0, _lword1, _lword2, _lword3, _lword4, _rword0, _rword1, _rword2, _rword3, _rword4, _mword0, _mword1, _mword2, _mword3, _mword4) {__LINE__, _lword0, _lword1, _lword2, _lword3, _lword4, _rword0, _rword1, _rword2, _rword3, _rword4, _mword0, _mword1, _mword2, _mword3, _mword4}

const testIDs::DistTest
testIDs::idDistTests[] = {
  // Least significant comparisons ////////////////////////////////////////////////oo////oc////co////cc
  DISTTEST(0, 0, 0, 0, 0,            0, 0, 0, 0, 1,            0, 0, 0, 0, 1),
  DISTTEST(0, 0, 0, 0, 0,            0, 0, 0, 0, 2,            0, 0, 0, 0, 2),
  DISTTEST(0, 0, 0, 0, 1,            0, 0, 0, 0, 2,            0, 0, 0, 0, 1),
  DISTTEST(0, 0, 0, 0, 1,            0, 0, 0, 0, 0,            0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff),

  DISTTEST(0, 0, 0, 0, 0xffffffff,   0, 0, 0, 1, 0,            0, 0, 0, 0, 1),

  DISTTEST(0, 0, 0, 0, 0xffffffff,   0, 0, 0, 1, 1,            0, 0, 0, 0, 2)
};

const size_t
testIDs::num_disttests =
                 sizeof(testIDs::idDistTests) /
                 sizeof(testIDs::DistTest);



#undef DISTTEST
















void
testIDs::testShifts()
{

  for (size_t i = 0;
       i < num_shiftTests;
       i++) {
    uint32_t left[5];
    uint32_t right[5];
    const ShiftTest * idTest = &idShiftTests[i];

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
    IDPtr resultID = leftID->shift(idTest->shift);

    std::ostringstream message;
    message << "ID test at line "
            << idTest->line
            << ". Shift of '"
            << leftID->toString()
            << "' by "
            << idTest->shift
            << " should be '"
            << rightID->toString()
            << "' but I got '"
            << resultID->toString()
            << "'.";
    BOOST_CHECK_MESSAGE(resultID->compareTo(rightID) == 0,
                        message.str().c_str());
  }
}


#define SHIFTTST(_lword0, _lword1, _lword2, _lword3, _lword4, _rword0, _rword1, _rword2, _rword3, _rword4, _shift) {__LINE__, _lword0, _lword1, _lword2, _lword3, _lword4, _rword0, _rword1, _rword2, _rword3, _rword4, _shift}

const testIDs::ShiftTest
testIDs::idShiftTests[] = {
  // Least significant comparisons
  SHIFTTST(0, 0, 0, 0, 0,            0, 0, 0, 0, 0,          0),

  SHIFTTST(0, 0, 0, 0, 0,            0, 0, 0, 0, 0,          1),
  SHIFTTST(0, 0, 0, 0, 0,            0, 0, 0, 0, 0,        100),
  SHIFTTST(0, 0, 0, 0, 1,            0, 0, 0, 0, 1,          0),
  SHIFTTST(0, 0, 0, 0, 1,            0, 0, 0, 1, 0,         32),
  SHIFTTST(0, 0, 0, 0, 1,            0, 0, 1, 0, 0,         64),
  SHIFTTST(0, 0, 0, 0, 1,            0, 1, 0, 0, 0,         96),
  SHIFTTST(0, 0, 0, 0, 1,            1, 0, 0, 0, 0,        128),
  SHIFTTST(0, 0, 0, 0, 1,            0, 0, 0, 0, 0,        160),
  SHIFTTST(0, 0, 0, 0, 1,            0, 0, 0, 0, 0,        161),

  SHIFTTST(0, 0, 0, 0, 1,            0, 0, 0, 0, 2,          1),
  SHIFTTST(0, 0, 0, 0, 1,            0, 0, 0, 0, 4,          2),
  SHIFTTST(0, 0, 0, 0, 1,            0, 0, 0, 0, 8,          3),
  SHIFTTST(0, 0, 0, 0, 1,            0, 0, 0, 0, 0x8000,    15),
  SHIFTTST(0, 0, 0, 0, 1,            0, 0, 0, 0x8000, 0,    47),

  SHIFTTST(0, 0, 0, 0, 0,            0, 0, 0, 0, 0,        100)
};

const size_t
testIDs::num_shiftTests =
                 sizeof(testIDs::idShiftTests) /
                 sizeof(testIDs::ShiftTest);


#undef SHIFTTST













void
testIDs::testAdditions()
{
  for (size_t i = 0;
       i < num_addtests;
       i++) {
    uint32_t middle[5];
    uint32_t left[5];
    uint32_t right[5];
    const AddTest * idTest = &idAddTests[i];

    middle[0] = idTest->mw0;
    middle[1] = idTest->mw1;
    middle[2] = idTest->mw2;
    middle[3] = idTest->mw3;
    middle[4] = idTest->mw4;

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
    IDPtr middleID = ID::mk(middle);
    IDPtr resultID = leftID->add(rightID);

    std::ostringstream message;
    message << "ID test at line "
            << idTest->line
            << ". Sum of '"
            << leftID->toString()
            << "' and '"
            << rightID->toString()
            << "' should be '"
            << middleID->toString()
            << "' but I got '"
            << resultID->toString()
            << "'.";
    BOOST_CHECK_MESSAGE(middleID->compareTo(resultID) == 0,
                        message.str().c_str());
  }
}


#define ADDTEST(_lword0, _lword1, _lword2, _lword3, _lword4, _rword0, _rword1, _rword2, _rword3, _rword4, _mword0, _mword1, _mword2, _mword3, _mword4) {__LINE__, _lword0, _lword1, _lword2, _lword3, _lword4, _rword0, _rword1, _rword2, _rword3, _rword4, _mword0, _mword1, _mword2, _mword3, _mword4}

const testIDs::AddTest
testIDs::idAddTests[] = {
  // Least significant comparisons ////////////////////////////////////////////////oo////oc////co////cc
  ADDTEST(0, 0, 0, 0, 0,            0, 0, 0, 0, 1,            0, 0, 0, 0, 1),
  ADDTEST(0, 0, 0, 0, 0,            0, 0, 0, 0, 2,            0, 0, 0, 0, 2),
  ADDTEST(0, 0, 0, 0, 1,            0, 0, 0, 0, 2,            0, 0, 0, 0, 3),
  ADDTEST(0, 0, 0, 0, 1,            0, 0, 0, 0, 0,            0, 0, 0, 0, 1),

  ADDTEST(0, 0, 0, 0, 0xffffffff,   0, 0, 0, 1, 0,            0, 0, 0, 1, 0xffffffff),
  ADDTEST(0, 0, 0, 0, 0xffffffff,   0, 0, 0, 0, 1,            0, 0, 0, 1, 0),

  ADDTEST(0, 0, 0, 0, 0xffffffff,   0, 0, 0, 1, 1,            0, 0, 0, 2, 0)
};

const size_t
testIDs::num_addtests =
                 sizeof(testIDs::idAddTests) /
                 sizeof(testIDs::AddTest);

#undef ADDTEST










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
    add(BOOST_CLASS_TEST_CASE(&testIDs::testBetween,
                              instance));
    add(BOOST_CLASS_TEST_CASE(&testIDs::testDistance,
                              instance));
    add(BOOST_CLASS_TEST_CASE(&testIDs::testShifts,
                              instance));
    add(BOOST_CLASS_TEST_CASE(&testIDs::testAdditions,
                              instance));
  }
};
