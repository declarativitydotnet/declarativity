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

#include "value.h"
#include "val_null.h"
#include "val_str.h"
#include "val_int64.h"
#include "val_double.h"
#include "val_opaque.h"
#include "val_tuple.h"
#include "val_time.h"
#include "val_id.h"
#include "val_set.h"

#include "testSets.h"

class testSets
{
public:
   testSets()
   {
   }
   
   
public:

void testSetsSize(Set theSets, uint32_t expected) 
{
   std::string testID;
   {
      std::ostringstream ID;
      ID << "Set size test.";
      testID = ID.str();
   }
   
   uint32_t setsize = theSets.size();
   
   std::ostringstream message;
   message << testID
           << "The size of the set " << theSets.toString() 
           << " was reported incorrectly as " << setsize;
   
   BOOST_CHECK_MESSAGE(setsize == expected, 
                        message.str().c_str());
}

#define TEST_SETMEMBER(set, valTypeExt, value, result) \
  { \
    std::string testID; \
    { \
      std::ostringstream ID; \
      ID << "Value casting test. "; \
      testID = ID.str(); \
    } \
 \
    ValuePtr v = Val_##valTypeExt::mk(value); \
    bool success; \
    if((set).member(v) == 1) { \
      success = true;\
    } else {\
      success = false;\
    }\
    std::ostringstream message;                 \
    message << testID                                          \
            << "Checking whether " #value   \
            << " is a member of " << (set).toString() \
            << " returned an unexpected result (" \
            << success \
            << ")."; \
    BOOST_CHECK_MESSAGE(success == result,                            \
                        message.str().c_str());                       \
  }

void testSetsDifference(SetPtr set1, SetPtr set2, SetPtr expected)
{
   std::string testID;
   {
      std::ostringstream ID;
      ID << "Set difference test.";
      testID = ID.str();
   }
   
   SetPtr result = set1->difference(set2);
   
   std::ostringstream message;
   message << testID
           << "The difference of sets " << set1->toString() 
           << " and " << set2->toString() 
           << " should have been " << expected->toString() 
           << " but was " << result->toString();
           
   BOOST_CHECK_MESSAGE(result->compareTo(expected) == 0, 
                        message.str().c_str());
}


void testSetsIntersect(SetPtr set1, SetPtr set2, SetPtr expected)
{
   std::string testID;
   {
      std::ostringstream ID;
      ID << "Set intersection test.";
      testID = ID.str();
   }
   
   SetPtr result = set1->intersect(set2);
   
   std::ostringstream message;
   message << testID
           << "The intersection of sets " << set1->toString() 
           << " and " << set2->toString() 
           << " should have been " << expected->toString() 
           << " but was " << result->toString();
           
   BOOST_CHECK_MESSAGE(result->compareTo(expected) == 0, 
                        message.str().c_str());
}

void testProperSubset(SetPtr set1, SetPtr set2, bool expected)
{
   std::string testID;
   {
      std::ostringstream ID;
      ID << "proper subset test.";
      testID = ID.str();
   }
   
   bool result = set1->propersubset(set2);
   
   std::ostringstream message;
   message << testID
           << "The propersubset of sets " << set1->toString() 
           << " and " << set2->toString() 
           << " should have been " << expected 
           << " but was " << result;
           
   BOOST_CHECK_MESSAGE(result == expected, 
                        message.str().c_str());
}

void testSubset(SetPtr set1, SetPtr set2, bool expected)
{
   std::string testID;
   {
      std::ostringstream ID;
      ID << "subset test.";
      testID = ID.str();
   }
   
   bool result = set1->subset(set2);
   
   std::ostringstream message;
   message << testID
           << "The subset of sets " << set1->toString() 
           << " and " << set2->toString() 
           << " should have been " << expected 
           << " but was " << result;
           
   BOOST_CHECK_MESSAGE(result == expected, 
                        message.str().c_str());
}

void testSetsUnion(SetPtr set1, SetPtr set2)
{
   std::string testID;
   {
      std::ostringstream ID;
      ID << "Set union test.";
      testID = ID.str();
   }
   
   SetPtr test = Set::mk();
      
   ValPtrSet::const_iterator iter = set1->begin();

   ValPtrSet::const_iterator end = set1->end();
   
   while(iter != end) {
      test->insert(*iter);
      iter++;
   }
      

   iter = set2->begin();
   end = set2->end();
   
   while(iter != end) {
      test->insert(*iter);
      iter++;
   }
   
   
   SetPtr result = set1->setunion(set2);
   
   std::ostringstream message;
   message << testID
           << "The concatenation of sets " << set1->toString() 
           << " and " << set2->toString() 
           << " should have been " << test->toString() 
           << " but was " << result->toString();
   
      BOOST_CHECK_MESSAGE(result->compareTo(test) == 0, 
                        message.str().c_str());
}

void testSetsCompare(SetPtr L1, SetPtr L2, int expected) {
   std::string testID;
   {
      std::ostringstream ID;
      ID << "Set comparison test.";
      testID = ID.str();
   }
   
   string expectedString;
   string resultString;
      
   string EQ = "equal";
   string LT = "less than";
   string GT = "greater than";
   
   int result = L1->compareTo(L2);
   
   if(result == 0) {
      resultString = EQ;
   } 
   else if(result > 0) {
      resultString = GT;
      result = 1;
   } 
   else {
      resultString = LT;
      result = -1;
   }
   
   if(expected == 0) {
      expectedString = EQ; 
   }
   else if(expected > 0) {
      expectedString = GT;
      expected = 1;
   } 
   else {
      expectedString = LT;
      expected = -1;
   }
   
   std::ostringstream message;
   
   message << testID
           << "The comparison of sets " << L1->toString() 
           << " and " << L1->toString() 
           << " should have been " << expectedString
           << " but was " << resultString;
   
      BOOST_CHECK_MESSAGE(result - expected == 0, 
                        message.str().c_str());
}

// ==================
// Intersection tests
// ==================

void testIntersection()
{
   // Make some sample sets.
   SetPtr test1 = Set::mk();
   SetPtr test2 = Set::mk();
   SetPtr test3 = Set::mk();
   SetPtr empty = Set::mk();
   
   // Populate the sets with values (which should henceforth retain 
   // their ordering.
   test1->insert(Val_Int64::mk(42));
   test1->insert(Val_Int64::mk(17));
   test1->insert(Val_Int64::mk(570));
   test1->insert(Val_Int64::mk(-12));
   test1->insert(Val_Int64::mk(0));
   test1->insert(Val_Int64::mk(-450));
   
   test2->insert(Val_Int64::mk(42));
   test2->insert(Val_Int64::mk(97));
   test2->insert(Val_Int64::mk(-12));
   test2->insert(Val_Int64::mk(-5));   
   test2->insert(Val_Int64::mk(570));

   test3->insert(Val_Str::mk("garply"));
   test3->insert(Val_Str::mk("bar"));
   test3->insert(Val_Str::mk("baz"));
   test3->insert(Val_Str::mk("quux"));
   test3->insert(Val_Str::mk("quuuux"));

   // Test set sizes
   testSetsSize(*test1, 6);
   testSetsSize(*test2, 5);
   testSetsSize(*test3, 5);
   testSetsSize(*empty, 0);

   SetPtr intersectSet = Set::mk();
   intersectSet->insert(Val_Int64::mk(-12));
   intersectSet->insert(Val_Int64::mk(42));
   intersectSet->insert(Val_Int64::mk(570));

   testSetsIntersect(test1, test2, intersectSet);
   
   SetPtr test1sorted = Set::mk();
   
   test1sorted->insert(Val_Int64::mk(-450));
   test1sorted->insert(Val_Int64::mk(-12));
   test1sorted->insert(Val_Int64::mk(0));
   test1sorted->insert(Val_Int64::mk(17));
   test1sorted->insert(Val_Int64::mk(42));
   test1sorted->insert(Val_Int64::mk(570));
   
   testSetsIntersect(test1, empty, empty);
   testSetsIntersect(test1, test3, empty);
   testSetsIntersect(test1, test1, test1sorted);
}

// ==================
// Subset tests
// ==================

void testSubset()
{
   // Make some sample sets.
   SetPtr test1 = Set::mk();
   SetPtr test2 = Set::mk();
   SetPtr test3 = Set::mk();
   SetPtr empty = Set::mk();
   
   // Populate the sets with values (which should henceforth retain 
   // their ordering.
   test1->insert(Val_Int64::mk(42));
   test1->insert(Val_Int64::mk(17));
   test1->insert(Val_Int64::mk(570));
   test1->insert(Val_Int64::mk(-12));
   test1->insert(Val_Int64::mk(0));
   test1->insert(Val_Int64::mk(-450));
   
   test2->insert(Val_Int64::mk(42));
   test2->insert(Val_Int64::mk(-12));
   test2->insert(Val_Int64::mk(570));

   test3->insert(Val_Int64::mk(32));
   test3->insert(Val_Int64::mk(42));
   test3->insert(Val_Int64::mk(-12));
   test3->insert(Val_Int64::mk(570));

   // Test set sizes
   testSetsSize(*test1, 6);
   testSetsSize(*test2, 3);
   testSetsSize(*test3, 5);
   testSetsSize(*empty, 0);

   testSubset(test1, empty, true);
   testSubset(test1, test2, true);
   testSubset(test1, test3, false);
   testSubset(empty, empty, true);
   testProperSubset(empty, empty, false);
   testProperSubset(test1, test1, false);
   testProperSubset(test1, test2, true);

}

// ================
// Difference tests
// ================

void testDifference()
{
   // Make some sample sets.
   SetPtr test1 = Set::mk();
   SetPtr test2 = Set::mk();
   SetPtr test3 = Set::mk();
   SetPtr test4 = Set::mk();
   SetPtr empty = Set::mk();
   
   // Populate the sets with values (which should henceforth retain 
   // their ordering.
   test1->insert(Val_Int64::mk(42));
   test1->insert(Val_Int64::mk(17));
   test1->insert(Val_Int64::mk(570));
   test1->insert(Val_Int64::mk(-12));
   test1->insert(Val_Int64::mk(0));
   test1->insert(Val_Int64::mk(-450));
   
   test2->insert(Val_Int64::mk(42));
   test2->insert(Val_Int64::mk(97));
   test2->insert(Val_Int64::mk(-12));
   test2->insert(Val_Int64::mk(-5));   
   test2->insert(Val_Int64::mk(570));

   test3->insert(Val_Str::mk("garply"));
   test3->insert(Val_Str::mk("bar"));
   test3->insert(Val_Str::mk("baz"));
   test3->insert(Val_Str::mk("quux"));
   test3->insert(Val_Str::mk("quuuux"));

   test4->insert(Val_Str::mk("garpllksdgly"));
   test4->insert(Val_Str::mk("barslkgdj"));
   test4->insert(Val_Str::mk("baz"));
   test4->insert(Val_Str::mk("quux"));
   test4->insert(Val_Str::mk("quuuux"));

   // Test set sizes
   testSetsSize(*test1, 6);
   testSetsSize(*test2, 5);
   testSetsSize(*test3, 5);
   testSetsSize(*empty, 0);

   SetPtr differenceSets = Set::mk();
   differenceSets->insert(Val_Int64::mk(17));
   differenceSets->insert(Val_Int64::mk(0));
   differenceSets->insert(Val_Int64::mk(-450));

   SetPtr differenceSetsStr = Set::mk();
   differenceSetsStr->insert(Val_Str::mk("garply"));
   differenceSetsStr->insert(Val_Str::mk("bar"));
   testSetsDifference(test3, test4, differenceSetsStr);
   
   SetPtr test1sorted = Set::mk();
   
   test1sorted->insert(Val_Int64::mk(-450));
   test1sorted->insert(Val_Int64::mk(-12));
   test1sorted->insert(Val_Int64::mk(0));
   test1sorted->insert(Val_Int64::mk(17));
   test1sorted->insert(Val_Int64::mk(42));
   test1sorted->insert(Val_Int64::mk(570));
   
   testSetsDifference(test1, empty, test1);
   testSetsDifference(test1, test3, test1);
   testSetsDifference(test1, test1, empty);
}

// ================
// Membership tests
// ================

void testMembership()
{
   // Make some sample sets.
   SetPtr test1 = Set::mk();
   SetPtr test2 = Set::mk();
   SetPtr test3 = Set::mk();
   SetPtr empty = Set::mk();
   
   // Populate the sets with values (which should henceforth retain 
   // their ordering.
   test1->insert(Val_Int64::mk(42));
   test1->insert(Val_Int64::mk(17));
   test1->insert(Val_Int64::mk(570));
   test1->insert(Val_Int64::mk(-12));
   test1->insert(Val_Int64::mk(0));
   test1->insert(Val_Int64::mk(-450));
   
   test2->insert(Val_Int64::mk(42));
   test2->insert(Val_Int64::mk(97));
   test2->insert(Val_Int64::mk(-12));
   test2->insert(Val_Int64::mk(-5));   
   test2->insert(Val_Int64::mk(570));

   test3->insert(Val_Str::mk("garply"));
   test3->insert(Val_Str::mk("bar"));
   test3->insert(Val_Str::mk("baz"));
   test3->insert(Val_Str::mk("quux"));
   test3->insert(Val_Str::mk("quuuux"));

   // Test set sizes
   testSetsSize(*test1, 6);
   testSetsSize(*test2, 5);
   testSetsSize(*test3, 5);
   testSetsSize(*empty, 0);
   
      // Membership tests
   TEST_SETMEMBER(*test3, Str, "garply", true);
   TEST_SETMEMBER(*test3, Str, "nothere", false);
   TEST_SETMEMBER(*test3, Double, 4.2, false);
   TEST_SETMEMBER(*test3, Int64, 75, false);
   TEST_SETMEMBER(*test2, Int64, 75, false);
   TEST_SETMEMBER(*test1, Int64, -12, true);
   TEST_SETMEMBER(*test1, Str, "NotANumber", false);
}

// =============
// SetUnion tests
// =============

void testSetUnion()
{
   // Make some sample sets.
   SetPtr test1 = Set::mk();
   SetPtr test2 = Set::mk();
   SetPtr test3 = Set::mk();
   SetPtr empty = Set::mk();
   
   // Populate the sets with values (which should henceforth retain 
   // their ordering.
   test1->insert(Val_Int64::mk(42));
   test1->insert(Val_Int64::mk(17));
   test1->insert(Val_Int64::mk(570));
   test1->insert(Val_Int64::mk(-12));
   test1->insert(Val_Int64::mk(0));
   test1->insert(Val_Int64::mk(-450));
   
   test2->insert(Val_Int64::mk(42));
   test2->insert(Val_Int64::mk(97));
   test2->insert(Val_Int64::mk(-12));
   test2->insert(Val_Int64::mk(-5));   
   test2->insert(Val_Int64::mk(570));

   test3->insert(Val_Str::mk("garply"));
   test3->insert(Val_Str::mk("bar"));
   test3->insert(Val_Str::mk("baz"));
   test3->insert(Val_Str::mk("quux"));
   test3->insert(Val_Str::mk("quuuux"));
   
   testSetsUnion(test1, test2);
   testSetsUnion(test2, test3);
   testSetsUnion(test2, empty);
   testSetsUnion(test3, test2);
   testSetsUnion(test1, empty);
   testSetsUnion(empty, empty);
   testSetsUnion(test1, test1);
}

void testCompare() {
   // (1,2,3)
   SetPtr test1 = Set::mk();
   test1->insert(Val_Int64::mk(1));
   test1->insert(Val_Int64::mk(2));
   test1->insert(Val_Int64::mk(3));
   // (2,3,4)
   SetPtr test2 = Set::mk();
   test2->insert(Val_Int64::mk(2));
   test2->insert(Val_Int64::mk(3));
   test2->insert(Val_Int64::mk(4));
   // (5)
   SetPtr test3 = Set::mk();
   test3->insert(Val_Int64::mk(5));
   // (1,2,3,4)
   SetPtr test4 = Set::mk();
   test4->insert(Val_Int64::mk(1));
   test4->insert(Val_Int64::mk(2));
   test4->insert(Val_Int64::mk(3));
   test4->insert(Val_Int64::mk(4));
   
   testSetsCompare(test1, test1, 0);
   
   testSetsCompare(test1, test2, -1);
   testSetsCompare(test2, test1, 1);
   
   testSetsCompare(test1, test4, -1);
   testSetsCompare(test4, test1, 1);
   
   testSetsCompare(test3, test2, 1);
   testSetsCompare(test2, test3, -1);
   
   testSetsCompare(test4, test2, -1);
   testSetsCompare(test2, test4, 1);
}

};

#undef TEST_SETMEMBER

testSets_testSuite::testSets_testSuite() 
  : boost::unit_test_framework::test_suite("testSets")
{
  boost::shared_ptr<testSets> instance(new testSets());
  add(BOOST_CLASS_TEST_CASE(&testSets::testIntersection, instance));
  add(BOOST_CLASS_TEST_CASE(&testSets::testDifference, instance));
  add(BOOST_CLASS_TEST_CASE(&testSets::testMembership, instance));
  add(BOOST_CLASS_TEST_CASE(&testSets::testSetUnion, instance));
  add(BOOST_CLASS_TEST_CASE(&testSets::testCompare, instance));
}
