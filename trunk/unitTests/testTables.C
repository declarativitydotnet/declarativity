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
#include "table.h"

#include "val_str.h"
#include "val_tuple.h"
#include "val_uint64.h"
#include "val_int32.h"

#include "limits.h" // for INT_MAX

class testTables
{
public:
  testTables()
  {
  }

  static const int N_TPLS = 100;

  TuplePtr tpls[N_TPLS];

  void
  testIndexing();

  void
  testBatchRemovals();

  void
  testBatchMultikeyRemovals();

  void
  testUniqueTupleRemovals();

  void
  testSuperimposedIndexRemoval();

  void
  testAggregates();

  void
  testMultiFieldKeys();
};

// Add tests for flushing due to length

// Add tests for flushing due to time

// Fix aggregates test

// Add tests for multi-field group by fields

void
testTables::testIndexing()
{
  // Create our test set of tuples
  for(int i = 0;
      i < N_TPLS;
      i++) {
    TuplePtr t = Tuple::mk();
    t->append(Val_Int32::mk(i));
    t->append(Val_Int32::mk(i/2));
    t->append(Val_Int32::mk(i/4));
    t->append(Val_Int32::mk(i % (N_TPLS/2)));
    t->append(Val_Int32::mk(i % (N_TPLS/4)));
    t->freeze();
    tpls[i] = t;
  }
  
  //  First: tuples inserted can be looked up.  Tuples not inserted
  // cannot be looked up.  Create a very simple table
  TablePtr tbl(new Table("test_table", 200));
  tbl->add_unique_index(0);
  for( int i=0; i < N_TPLS/2; i++) { 
    tbl->insert(tpls[i]);
  }
  
  
  for( int i=0; i< N_TPLS/2; i++) { 
    {
      std::ostringstream message;
      message << "Table test. Lookup "
              << i
              << ".  Tuple is not in the table but should.";
      BOOST_CHECK_MESSAGE(!tbl->lookup(0, Val_Int32::mk(i))->done(),
                          message.str().c_str());
    }
    
    {
      std::ostringstream message;
      message << "Table test. Lookup "
              << (i + N_TPLS/2)
              << ".  Tuple is in the table but shouldn't";
      BOOST_CHECK_MESSAGE(tbl->lookup(0,Val_Int32::mk(i + N_TPLS/2))->done(),
                          message.str().c_str());
    }
  }
  
  
  // Check multi indices
  tbl.reset(new Table("test_table", 200));
  tbl->add_multiple_index(4);
  for( int i=0; i < N_TPLS; i++) { 
    tbl->insert(tpls[i]);
  }
  
  // Now every key we find in the first quarter must have four
  // distinct instances in the index
  for( int i=0; i < N_TPLS / 4; i++) { 
    Table::MultIterator iter = tbl->lookupAll(4, Val_Int32::mk(i));
    for (int counter = 0;
         counter < 4;
         counter++) {
      TuplePtr result = iter->next();
      {
        std::ostringstream message;
        message << "Table test. Key "
                << i
                << " seems to have too few tuples in the mult index";
        BOOST_CHECK_MESSAGE(result != NULL,
                            message.str().c_str());
      }
    }
    {
      std::ostringstream message;
      message << "Table test. Key "
              << i
              << " seems to have too many tuples in the mult index";
      BOOST_CHECK_MESSAGE(iter->next() == NULL,
                          message.str().c_str());
    }
  }
  
  
}



void
testTables::testBatchRemovals()
{
  // Create our test set of tuples
  for(int i = 0;
      i < N_TPLS;
      i++) {
    TuplePtr t = Tuple::mk();
    t->append(Val_Int32::mk(i));
    t->append(Val_Int32::mk(i/2));
    t->append(Val_Int32::mk(i/4));
    t->append(Val_Int32::mk(i % (N_TPLS/2)));
    t->append(Val_Int32::mk(i % (N_TPLS/4)));
    t->freeze();
    tpls[i] = t;
  }
  
  // Create a unique index on first field
  TablePtr tbl(new Table("test_table", 200));
  tbl->add_unique_index(0);
  for(int i = 0;
      i < N_TPLS;
      i++) { 
    tbl->insert(tpls[i]);
  }

  for(int i = 0;
      i < N_TPLS;
      i++) { 
    tbl->remove(0, (*tpls[i])[0]);
  }

  // I should be unable to look up any elements at all
  for(int i = 0;
      i< N_TPLS;
      i++) { 
    BOOST_CHECK_MESSAGE(tbl->lookup(0, (*tpls[i])[0])->done(),
                        "Table test. Lookup of removed tuple "
                        << tpls[i]->toString()
                        << " should return no results.");
  }
}


void
testTables::testBatchMultikeyRemovals()
{
  // Create our test set of tuples
  for(int i = 0;
      i < N_TPLS;
      i++) {
    TuplePtr t = Tuple::mk();
    t->append(Val_Int32::mk(i));
    t->append(Val_Int32::mk(i/2));
    t->append(Val_Int32::mk(i/4));
    t->append(Val_Int32::mk(i % (N_TPLS/2)));
    t->append(Val_Int32::mk(i % (N_TPLS/4)));
    t->freeze();
    tpls[i] = t;
  }
  
  // Create a unique index on first field
  TablePtr tbl(new Table("test_table", 200));
  std::vector< unsigned > keys;
  keys.push_back(0);
  keys.push_back(1);

  tbl->add_unique_index(keys);
  for(int i = 0;
      i < N_TPLS;
      i++) { 
    tbl->insert(tpls[i]);
  }

  for(int i = 0;
      i < N_TPLS;
      i++) { 
    std::vector< ValuePtr > keyValues;
    keyValues.push_back((*tpls[i])[0]);
    keyValues.push_back((*tpls[i])[1]);
    tbl->remove(keys, keyValues);
  }

  // I should be unable to look up any elements at all
  for(int i = 0;
      i< N_TPLS;
      i++) { 
    std::vector< ValuePtr > keyValues;
    keyValues.push_back((*tpls[i])[0]);
    keyValues.push_back((*tpls[i])[1]);
    BOOST_CHECK_MESSAGE(tbl->lookup(keys, keyValues)->done(),
                        "Table test. Lookup of removed tuple "
                        << tpls[i]->toString()
                        << " should return no results.");
  }
}


void
testTables::testUniqueTupleRemovals()
{
  // Create our test set of tuples
  for(int i = 0;
      i < N_TPLS;
      i++) {
    TuplePtr t = Tuple::mk();
    t->append(Val_Int32::mk(i));
    t->append(Val_Int32::mk(i/2));
    t->append(Val_Int32::mk(i/4));
    t->append(Val_Int32::mk(i % (N_TPLS/2)));
    t->append(Val_Int32::mk(i % (N_TPLS/4)));
    t->freeze();
    tpls[i] = t;
  }
  
  // Create a unique index on first field
  TablePtr tbl(new Table("test_table", 200));
  std::vector< unsigned > keys;
  keys.push_back(0);
  keys.push_back(1);
  keys.push_back(2);
  keys.push_back(3);
  keys.push_back(4);

  tbl->add_multiple_index(1);
  tbl->add_unique_index(keys);
  for(int i = 0;
      i < N_TPLS;
      i++) { 
    tbl->insert(tpls[i]);
  }

  for(int i = 0;
      i < N_TPLS;
      i++) { 
    std::vector< ValuePtr > keyValues;
    keyValues.push_back((*tpls[i])[0]);
    keyValues.push_back((*tpls[i])[1]);
    keyValues.push_back((*tpls[i])[2]);
    keyValues.push_back((*tpls[i])[3]);
    keyValues.push_back((*tpls[i])[4]);
    tbl->remove(keys, keyValues);
  }

  // I should be unable to look up any elements at all
  for(int i = 0;
      i< N_TPLS;
      i++) { 
    std::vector< ValuePtr > keyValues;
    keyValues.push_back((*tpls[i])[0]);
    keyValues.push_back((*tpls[i])[1]);
    keyValues.push_back((*tpls[i])[2]);
    keyValues.push_back((*tpls[i])[3]);
    keyValues.push_back((*tpls[i])[4]);
    BOOST_CHECK_MESSAGE(tbl->lookup(keys, keyValues)->done(),
                        "Table test. Lookup of removed tuple "
                        << tpls[i]->toString()
                        << " should return no results.");
  }
}


/**
 * This test corresponds on a particular problem encountered with
 * aggregate testing.  It creates a table whose tuples are <int, int>.
 * It creates a unique index on all fields, and a multiple index on the
 * first field.  It inserts <0, 10>, <0, 15>, <0, 5>, and then removes
 * <0, 5> from the table.  Then it looks up <0, 5> in the multiple
 * index.  It should not find it.
 */
void
testTables::testSuperimposedIndexRemoval()
{
  TuplePtr a = Tuple::mk();
  a->append(Val_Int32::mk(0));
  a->append(Val_Int32::mk(10));
  a->freeze();
  
  TuplePtr b = Tuple::mk();
  b->append(Val_Int32::mk(0));
  b->append(Val_Int32::mk(15));
  b->freeze();
  
  TuplePtr c = Tuple::mk();
  c->append(Val_Int32::mk(0));
  c->append(Val_Int32::mk(5));
  c->freeze();
  

  // Create a table of unique tuples with a multiple key on its first
  // field
  TablePtr tbl(new Table("test_table", 200));
  std::vector< unsigned > keys;
  keys.push_back(0);
  keys.push_back(1);

  tbl->add_unique_index(keys);
  tbl->add_multiple_index(0);


  // Insert the three tuples in that order
  tbl->insert(a);
  tbl->insert(b);
  tbl->insert(c);

  // Remove the last tuple
  std::vector< ValuePtr > keyValues;
  keyValues.push_back((*c)[0]);
  keyValues.push_back((*c)[1]);
  tbl->remove(keys, keyValues);

  // Lookup the tuple itself in the unique index
  BOOST_CHECK_MESSAGE(tbl->lookup(keys, keyValues)->done(),
                      "Table test. Lookup of removed tuple "
                      << c->toString()
                      << " should return no results.");

  // Count the multiple index. It should contain two elements
  Table::MultIterator m = tbl->lookupAll(0, (*c)[0]);
  int count = 0;
  while (!m->done()) {
    m->next();
    count++;
  }
  BOOST_CHECK_MESSAGE(count == 2,
                      "Table test. Lookup of removed tuple "
                      << c->toString()
                      << " in multiple index should return "
                      << " 2 results exactly but returned "
                      << count
                      << " instead.");
}








////////////////////////////////////////////////////////////
// Tracker Superclass
////////////////////////////////////////////////////////////


class tableTest {
public:
  tableTest(std::string,
            int);
  std::string _script;
  int _line;
};

tableTest::tableTest(std::string script,
                     int line)
  : _script(script),
    _line(line)
{
}


/**
 * This object tracks a single interactive test for a single script. It
 * creates an empty table, prepares it accordingly, parses the script,
 * applies it to the table, and evaluates the result. If the script is
 * executed without mismatch, the test is a success.  Otherwise, an
 * error is generated and the rest of the test is aborted.
 */
class Tracker {
public:
  /** Create the new tracker */
  Tracker(tableTest &);

  /** Execute the tracked script */
  void
  test();

  /** Advance the script by a command. Returns false if the tracking is
      aborted, true otherwise. */
  bool
  fetchCommand();

  /** My test */
  tableTest & _test;
  
  /** My table */
  Table _table;

  /** The remainder of my script */
  std::string _remainder;

  /** The current command type */
  char _command;

  /** The current tuple */
  TuplePtr _tuple;

  /** The all-fields key vector */
  std::vector< unsigned > _allFields;
};


/**
 * Execute the script.  First parse the script string.  Then execute it.
 * Parse errors abort the current test.
 */
void
Tracker::test()
{
//   std::cout << "Testing script \""
//             << _test._script
//             << "\"\n";
  _remainder = std::string(_test._script);
  
  while (!_remainder.empty()) {
    bool result = fetchCommand();
    
    if (result) {
      switch(_command) {
      case 'i':
        // Insert into table
        _table.insert(_tuple);
        break;
      case 'd':
        // Delete from table
        {
          std::vector< ValuePtr > fields;
          for (uint i = 0;
               i < _tuple->size();
               i++) {
            fields.push_back((*_tuple)[i]);
          }
          _table.remove(_allFields, fields);
        }
        break;
      case 'f':
        // Find in table (lookup successfully)
        {
          // Lookup the tuple in the command and ensure it was found.
        }
        break;
      case 'm':
        // Miss in table (lookup unsuccessfully)
        break;
      default:
        // I should not receive anything else
        BOOST_ERROR("Error in test line "
                    << _test._line
                    << " with suffix "
                    << "\""
                    << _remainder
                    << "\". Unexpected command '"
                    << _command
                    << "'.");
        return;
      }
    } else {
//       std::cout << "Aborting script \""
//                 << _test._script
//                 << "\"\n";
      return;
    }
  }
}



/**
 * Fetch another command from the beginning of the _remainder script.
 * The command is placed in the _command field.  The tuple argument of
 * the command is placed in the _tuple field.  The _remainder string is
 * shrunk by the removed command.
 *
 * This assumes that _remainder is not empty.
 *
 * If the fetch was successful, returns true. False if a syntax error in
 * the test specification was identified.
 */
bool
Tracker::fetchCommand()
{
//   std::cout << "Fetching command from \""
//             << _remainder
//             << "\"\n";

  // This will require a new tuple
  _tuple = Tuple::mk();

  // Fetch another token
  std::string::size_type tokenEnd = _remainder.find(';');
  if (tokenEnd == std::string::npos) {
    // Syntax error. I should always end with a semicolon
    BOOST_ERROR("Syntax error in test line "
                << _test._line
                << " with suffix "
                << "\""
                << _remainder
                << "\". Missing a trailing semicolon.");
    return false;
  }
  
  // Take out token, leaving semicolon behind
  std::string token = _remainder.substr(0, tokenEnd);
  
  // Check command
  _command = token[0];
  switch(_command) {
  case 'i':
  case 'd':
  case 'u':
  case 'f':
  case 'm':
    break;
  default:
    BOOST_ERROR("Syntax error in test line "
                << _test._line
                << " with token "
                << "\""
                << token
                << "\". Unknown command '"
                << _command
                << "'.");
    return false;
  }
  
  // Check tuple containers
  if ((token[1] != '<') ||
      (token[token.length() - 1] != '>')) {
    BOOST_ERROR("Syntax error in test line "
                << _test._line
                << " with token "
                << "\""
                << token
                << "\". Tuple must be enclosed in '<>'.");
    return false;
  }
  
  // Parse and construct tuple
  std::string tuple = token.substr(2, token.length() - 3);
  
//   std::cout << "Parsing tuple \""
//             << tuple
//             << "\"\n";
  
  while (!tuple.empty()) {
    // Fetch another field delimiter
    std::string::size_type fieldEnd = tuple.find(',');
    
    // Isolate the field
    std::string field;
    if (fieldEnd == std::string::npos) {
      // The rest of the tuple makes up a field
      field = tuple;
      tuple = std::string();
    } else {
      // Take a field up to the delimiter
      field = tuple.substr(0, fieldEnd);
      tuple = tuple.substr(fieldEnd + 1);
    }
    
    // Interpret the field as an int32
    ValuePtr intField = Val_Int32::mk(Val_Int32::cast(Val_Str::mk(field)));
    _tuple->append(intField);
    
//     std::cout << "Found int32 field \""
//               << intField->toString()
//               << "\"\n";
  }
  
  // Shrink remainder
  _remainder = _remainder.substr(tokenEnd + 1);

  // Freeze the tuple
  _tuple->freeze();

  return true;
}





Tracker::Tracker(tableTest & test)
  : _test(test),
    // Create a very simple table with the following schema
    // Schema is <A int, B int>
    // No maximum lifetime or size
    _table("trackerTest", INT_MAX)
{
  // Create a unique index on all fields to enable removal of entire
  // tuples
  _allFields.push_back(0);
  _allFields.push_back(1);
  _table.add_unique_index(_allFields);
}

















////////////////////////////////////////////////////////////
// Aggregate Testing
////////////////////////////////////////////////////////////

class intAggTest : public tableTest {
public:
  intAggTest(std::string,
             int, int,
             Table::AggregateFunction&,
             int);
  int _keyFieldNo;
  int _aggFieldNo;
  Table::AggregateFunction& _function;
};

intAggTest::intAggTest(std::string script,
                       int keyFieldNo,
                       int aggFieldNo,
                       Table::AggregateFunction&  function,
                       int line)
  : tableTest(script, line),
    _keyFieldNo(keyFieldNo),
    _aggFieldNo(aggFieldNo),
    _function(function)
{
}



/**
 * This object tracks a single integer aggregate test for a single
 * script. It creates an empty table, parses the script, applies it to
 * the table, and evaluates the result. If the script is executed
 * without mismatch, the test is a success.  Otherwise, an error is
 * generated and the rest of the test is aborted.
 */
class AggTracker : public Tracker {
public:
  /** Create the new tracker, parse the script */
  AggTracker(intAggTest & test);

  /** Listener for aggregate updates */
  void
  listener(TuplePtr t);
};


AggTracker::AggTracker(intAggTest & test)
  : Tracker(test)
{
  // Create a multiple index on the first field
  _table.add_multiple_index(test._keyFieldNo);

  // My group-by field is the indexed field
  std::vector< unsigned > groupBy;
  groupBy.push_back(test._keyFieldNo);

  // Create a multiple-value aggregate with the given function
  Table::MultAggregate u =
    _table.add_mult_groupBy_agg(test._keyFieldNo,
                                groupBy,
                                test._aggFieldNo,
                                test._function);

  u->addListener(boost::bind(&AggTracker::listener, this, _1));
}


void
AggTracker::listener(TuplePtr t)
{
//   std::cout << "Aggregate received \""
//             << t->toString()
//             << "\"\n";

  // Fetch a command
  if (!_remainder.empty()) {
    bool result = fetchCommand();

    if (result) {
      // Is it an update?
      if (_command != 'u') {
        // I should have an update in there
        BOOST_ERROR("Semantic error in test line "
                    << _test._line
                    << " with suffix "
                    << "\""
                    << _remainder
                    << "\". Script should expect an update with '"
                    << t->toString()
                    << "' but contained a '"
                    << _command
                    << "' instead.");
        _remainder = std::string();
        return;
      }

      // Compare the tuples
      if (t->compareTo(_tuple) != 0) {
        BOOST_ERROR("Error in test line "
                    << _test._line
                    << " with suffix "
                    << "\""
                    << _remainder
                    << "\". Script expected tuple '"
                    << _tuple->toString()
                    << "' but received '"
                    << t->toString()
                    << "' instead.");
        _remainder = std::string();
      }
    } else {
      // Something went wrong. Just exist zeroing out the remainder
      _remainder = std::string();
    }
  } else {
    // I should have an update in there
    BOOST_ERROR("Error in test line "
                << _test._line
                << " with suffix "
                << "\""
                << _remainder
                << "\". Script should expect an update with '"
                << t->toString()
                << "' but didn't.");
    _remainder = std::string();
  }
}








/** 
 * The purpose of this test is to check that table aggregates work
 * correctly.  The way we implement the test is to submit to the table a
 * particular sequence of insertions and deletions and the corresponding
 * sequence of aggregates updates.  If the sequences match, then the
 * test has succeeded.
 *
 * We define each test as an EXPECT-like script, made up of insertions,
 * deletions, and updates.
 */
void
testTables::testAggregates()
{
  intAggTest intAggTests[] = {
    // The following tests script expects the sequence
    // INSERT <0, 10>
    // UPDATE <0, 10>
    // INSERT <0, 15>
    // INSERT <0, 5>
    // UPDATE <0, 5>
    // DELETE <0, 5>
    // UPDATE <0, 10>
    // Scripts must always end with a semicolon!!!!!!!!!
    // Scripts must always end with a semicolon!!!!!!!!!
    // Scripts must always end with a semicolon!!!!!!!!!
    // Scripts must always end with a semicolon!!!!!!!!!
    // Scripts must always end with a semicolon!!!!!!!!!
    intAggTest("i<0,10>;u<0,10>;i<0,15>;i<0,5>;u<0,5>;d<0,5>;u<0,10>;",
               0, // first field is indexed
               1, // second field is aggregated
               Table::agg_min(), // aggregate function is MIN
               __LINE__),

    // The following tests script for MIN expects the sequence
    // INSERT <0, 10>
    // UPDATE <0, 10>
    // INSERT <0, 5>
    // UPDATE <0, 5>
    // First field is indexed, second field is aggregated
    intAggTest("i<0,10>;u<0,10>;i<0,5>;u<0,5>;",
               0, 1, Table::agg_min(),
               __LINE__),

    intAggTest("i<0,10>;u<0,10>;i<0,5>;u<0,5>;i<0,20>;i<0,30>;",
               0, 1, Table::agg_min(),
               __LINE__),

    intAggTest("i<0,10>;u<0,10>;i<0,5>;u<0,5>;i<0,20>;i<0,3>;u<0,3>;i<0,4>;",
               0, 1, Table::agg_min(),
               __LINE__),


    ////////////////////////////////////////////////////////////
    // MAX

    intAggTest("i<0,10>;u<0,10>;i<0,5>;",
               0, 1, Table::agg_max(),
               __LINE__),

    intAggTest("i<0,10>;u<0,10>;i<0,15>;u<0,15>;i<0,3>;",
               0, 1, Table::agg_max(),
               __LINE__),

    intAggTest("i<0,10>;u<0,10>;i<0,5>;i<0,15>;u<0,15>;",
               0, 1, Table::agg_max(),
               __LINE__),

    intAggTest("i<0,10>;u<0,10>;i<0,10>;u<0,10>;",
               0, 1, Table::agg_max(),
               __LINE__)

  };
  int noIntAggTests = sizeof(intAggTests) / sizeof(intAggTest);

  for (int i = 0;
       i < noIntAggTests;
       i++) {
    // Create a script tracker
    AggTracker tracker(intAggTests[i]);
    // Run it
    tracker.test();
  }
}














////////////////////////////////////////////////////////////
// Multi-field Key Testing
////////////////////////////////////////////////////////////

class multiTest : public tableTest {
public:
  multiTest(std::string,
            std::vector< uint >,
            uint);
  std::vector< uint > _key;
};

multiTest::multiTest(std::string script,
                     std::vector< uint > key,
                     uint line)
  : tableTest(script, line),
    _key(key)
{
}

class MultiTracker : public Tracker {
public:
  MultiTracker(multiTest & test);
};


MultiTracker::MultiTracker(multiTest & test)
  : Tracker(test)
{
  // Create a multiple index on the key fields
  _table.add_unique_index(test._key);
}



/** 
 * The purpose of this test is to check that primary keys containing
 * multiple fields work correctly.  The way we implement the test is to
 * submit to the table a particular sequence of insertions, deletions,
 * successful lookups, and unsuccessful lookups. If the sequences match,
 * then the test has succeeded.
 *
 * We define each test as an EXPECT-like script, made up of insertions,
 * deletions, and successful/unsuccessful lookups.
 */
void
testTables::testMultiFieldKeys()
{
  multiTest multiTests[] = {
    multiTest("i<0,1>;i<0,2>;i<0,3>;f<0,2>;",
              std::vector<unsigned> (0, 1),
              __LINE__),
    

    multiTest("i<0,1>;i<0,2>;i<0,3>;f<0,2>;",
              std::vector<unsigned> (0, 1),
              __LINE__)
  };
  uint noIntMultiFieldTests = sizeof(multiTests) / sizeof(multiTest);

  for (uint i = 0;
       i < noIntMultiFieldTests;
       i++) {
    // Create a script tracker
    MultiTracker tracker(multiTests[i]);
    // Run it
    tracker.test();
  }
}






class testTables_testSuite
  : public boost::unit_test_framework::test_suite
{
public:
  testTables_testSuite()
    : boost::unit_test_framework::test_suite("testTables: Marshaling/Unmarshaling")
  {
    boost::shared_ptr<testTables> instance(new testTables());

    add(BOOST_CLASS_TEST_CASE(&testTables::testIndexing, instance));
    add(BOOST_CLASS_TEST_CASE(&testTables::testBatchRemovals, instance));
    add(BOOST_CLASS_TEST_CASE(&testTables::testBatchMultikeyRemovals, instance));
    add(BOOST_CLASS_TEST_CASE(&testTables::testUniqueTupleRemovals, instance));
    add(BOOST_CLASS_TEST_CASE(&testTables::testSuperimposedIndexRemoval, instance));
    add(BOOST_CLASS_TEST_CASE(&testTables::testAggregates, instance));
    add(BOOST_CLASS_TEST_CASE(&testTables::testMultiFieldKeys, instance));
  }
};
