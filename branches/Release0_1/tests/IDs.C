/*
 * @(#)$Id$
 *
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * Or
 * UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
 * Berkeley, CA,  94707. Attention: P2 Group.
 * 
 * DESCRIPTION: Test suite for PEL lexer
 *
 */

#include <iostream>
#include "val_id.h"


#define CMPTST(_lword0, _lword1, _lword2, _lword3, _lword4, _rword0, _rword1, _rword2, _rword3, _rword4, _result) {__LINE__, _lword0, _lword1, _lword2, _lword3, _lword4, _rword0, _rword1, _rword2, _rword3, _rword4, _result}
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

#define BETWEENTEST(_mword0, _mword1, _mword2, _mword3, _mword4, _lword0, _lword1, _lword2, _lword3, _lword4, _rword0, _rword1, _rword2, _rword3, _rword4, _ooresult, _ocresult, _coresult, _ccresult) {__LINE__, _mword0, _mword1, _mword2, _mword3, _mword4, _lword0, _lword1, _lword2, _lword3, _lword4, _rword0, _rword1, _rword2, _rword3, _rword4, _ooresult, _ocresult, _coresult, _ccresult}
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

#define DISTTEST(_lword0, _lword1, _lword2, _lword3, _lword4, _rword0, _rword1, _rword2, _rword3, _rword4, _mword0, _mword1, _mword2, _mword3, _mword4) {__LINE__, _lword0, _lword1, _lword2, _lword3, _lword4, _rword0, _rword1, _rword2, _rword3, _rword4, _mword0, _mword1, _mword2, _mword3, _mword4}
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

#define SHIFTTST(_lword0, _lword1, _lword2, _lword3, _lword4, _rword0, _rword1, _rword2, _rword3, _rword4, _shift) {__LINE__, _lword0, _lword1, _lword2, _lword3, _lword4, _rword0, _rword1, _rword2, _rword3, _rword4, _shift}
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

#define ADDTEST(_lword0, _lword1, _lword2, _lword3, _lword4, _rword0, _rword1, _rword2, _rword3, _rword4, _mword0, _mword1, _mword2, _mword3, _mword4) {__LINE__, _lword0, _lword1, _lword2, _lword3, _lword4, _rword0, _rword1, _rword2, _rword3, _rword4, _mword0, _mword1, _mword2, _mword3, _mword4}
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

static const CmpTest idCmpTests[] = {
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
static const size_t num_cmptests = sizeof(idCmpTests) / sizeof(CmpTest);

static const BetweenTest idBetweenTests[] = {
  // Least significant comparisons ////////////////////////////////////////////////oo////oc////co////cc
  BETWEENTEST(0, 0, 0, 0, 1,            0, 0, 0, 0, 0,            0, 0, 0, 0, 2,        true, true, true, true),
  BETWEENTEST(0, 0, 0, 0, 0,            0, 0, 0, 0, 0,            0, 0, 0, 0, 2,        false, false, true, true),
  BETWEENTEST(0, 0, 0, 0, 2,            0, 0, 0, 0, 0,            0, 0, 0, 0, 2,        false, true, false, true),

  BETWEENTEST(0, 0, 0, 0, 10,           0, 0, 0, 0, 1,            0, 0, 0, 0, 523,      true, true, true, true),
  BETWEENTEST(0, 0, 0, 0, 1,            0, 0, 0, 0, 0,            0, 0, 0, 0, 0,        true, true, true, true),

  BETWEENTEST(0, 0, 0, 0, 2,            0, 0, 0, 0, 0,            0, 0, 0, 0, 2,        false, true, false, true)
};
static const size_t num_betweentests = sizeof(idBetweenTests) / sizeof(BetweenTest);

static const DistTest idDistTests[] = {
  // Least significant comparisons ////////////////////////////////////////////////oo////oc////co////cc
  DISTTEST(0, 0, 0, 0, 0,            0, 0, 0, 0, 1,            0, 0, 0, 0, 1),
  DISTTEST(0, 0, 0, 0, 0,            0, 0, 0, 0, 2,            0, 0, 0, 0, 2),
  DISTTEST(0, 0, 0, 0, 1,            0, 0, 0, 0, 2,            0, 0, 0, 0, 1),
  DISTTEST(0, 0, 0, 0, 1,            0, 0, 0, 0, 0,            0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff),

  DISTTEST(0, 0, 0, 0, 0xffffffff,   0, 0, 0, 1, 0,            0, 0, 0, 0, 1),

  DISTTEST(0, 0, 0, 0, 0xffffffff,   0, 0, 0, 1, 1,            0, 0, 0, 0, 2)
};
static const size_t num_disttests = sizeof(idDistTests) / sizeof(DistTest);

static const ShiftTest idShiftTests[] = {
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
static const size_t num_shiftTests = sizeof(idShiftTests) / sizeof(ShiftTest);

static const AddTest idAddTests[] = {
  // Least significant comparisons ////////////////////////////////////////////////oo////oc////co////cc
  ADDTEST(0, 0, 0, 0, 0,            0, 0, 0, 0, 1,            0, 0, 0, 0, 1),
  ADDTEST(0, 0, 0, 0, 0,            0, 0, 0, 0, 2,            0, 0, 0, 0, 2),
  ADDTEST(0, 0, 0, 0, 1,            0, 0, 0, 0, 2,            0, 0, 0, 0, 3),
  ADDTEST(0, 0, 0, 0, 1,            0, 0, 0, 0, 0,            0, 0, 0, 0, 1),

  ADDTEST(0, 0, 0, 0, 0xffffffff,   0, 0, 0, 1, 0,            0, 0, 0, 1, 0xffffffff),
  ADDTEST(0, 0, 0, 0, 0xffffffff,   0, 0, 0, 0, 1,            0, 0, 0, 1, 0),

  ADDTEST(0, 0, 0, 0, 0xffffffff,   0, 0, 0, 1, 1,            0, 0, 0, 2, 0)
};
static const size_t num_addtests = sizeof(idAddTests) / sizeof(AddTest);


int main(int argc, char **argv)
{
  std::cout << "IDs \n";

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
    if (idTest->result != result) {
      std::cerr << "***" << __FILE__ ":" << idTest->line << ": "
                << "Comparison of " << leftID->toString()
                << " to " << rightID->toString()
                << " returned " << result
                << " but should have returned " 
                << idTest->result << "\n";
    }
  }

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

    if (idTest->oores != resultoo) {
      std::cerr << "***" << __FILE__ ":" << idTest->line << ": "
                << "Containment of " << middleID->toString()
                << " in (" << leftID->toString()
                << ", " << rightID->toString()
                << ") returned " << resultoo
                << " but should have returned " 
                << idTest->oores << "\n";
    }
    if (idTest->ocres != resultoc) {
      std::cerr << "***" << __FILE__ ":" << idTest->line << ": "
                << "Containment of " << middleID->toString()
                << " in (" << leftID->toString()
                << ", " << rightID->toString()
                << "] returned " << resultoc
                << " but should have returned " 
                << idTest->ocres << "\n";
    }
    if (idTest->cores != resultco) {
      std::cerr << "***" << __FILE__ ":" << idTest->line << ": "
                << "Containment of " << middleID->toString()
                << " in [" << leftID->toString()
                << ", " << rightID->toString()
                << ") returned " << resultco
                << " but should have returned " 
                << idTest->cores << "\n";
    }
    if (idTest->ccres != resultcc) {
      std::cerr << "***" << __FILE__ ":" << idTest->line << ": "
                << "Containment of " << middleID->toString()
                << " in [" << leftID->toString()
                << ", " << rightID->toString()
                << "] returned " << resultcc
                << " but should have returned " 
                << idTest->ccres << "\n";
    }
  }

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

    if (middleID->compareTo(resultID) != 0) {
      std::cerr << "***" << __FILE__ ":" << idTest->line << ": "
                << "Distance of " << leftID->toString()
                << " to " << rightID->toString()
                << " should be " << middleID->toString()
                << " but I got " << resultID->toString()
                << "\n";
    }
  }


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

    if (middleID->compareTo(resultID) != 0) {
      std::cerr << "***" << __FILE__ ":" << idTest->line << ": "
                << "Sum of " << leftID->toString()
                << " and " << rightID->toString()
                << " should be " << middleID->toString()
                << " but I got " << resultID->toString()
                << "\n";
    }
  }


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

    if (resultID->compareTo(rightID) != 0) {
      std::cerr << "***" << __FILE__ ":" << idTest->line << ": "
                << "Shift of " << leftID->toString()
                << " by " << idTest->shift
                << " should be " << rightID->toString()
                << " but I got " << resultID->toString()
                << "\n";
    }
  }
}
