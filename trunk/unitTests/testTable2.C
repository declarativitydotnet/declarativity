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
#include "table2.h"

#include "val_str.h"
#include "val_tuple.h"
#include "val_int32.h"

#include "testTable2.h"



class testTable2
{
private:
  static const uint SIZE = 5;

  static const uint EXTRA_TUPLES = 10;




public:
  testTable2()
  {
  }


  /** Create table, destroy table, with different primary keys, tuple
      lifetimes, size limits. */
  void
  testCreateDestroy();


  /** Length limits. Keep inserting different tuples and ensure the size
      limit is not violated. Use ID as primary key.*/
  void
  testSizeLimitID();


  /** Length limits. Keep inserting different tuples and ensure the size
      limit is not violated. Use a single-field primary key.*/
  void
  testSizeLimitSingle();


  /** Length limits. Keep inserting different tuples and ensure the size
      limit is not violated. Use a multi-field primary key.*/
  void
  testSizeLimitMulti();

  
  /** Test case demonstrating problems with delete in tables v1, when a
      multi-field primary key is combined with a single-field secondary
      key. */
  void
  testSuperimposedIndexRemoval();
};



void
testTable2::testCreateDestroy()
{
  // Shortest constructor
  {
    Table2 table1("table1", Table2::KEYID);
    Table2 table2("table2", Table2::KEY0);
    Table2 table3("table3", Table2::KEY01);
    Table2 table4("table4", Table2::KEY012);
    Table2 table5("table5", Table2::KEY13);
  }

  // Lifetime-less constructor with 0 size limit
  {
    Table2 table1("table1", Table2::KEYID, 0);
    Table2 table2("table2", Table2::KEY0, 0);
    Table2 table3("table3", Table2::KEY01, 0);
    Table2 table4("table4", Table2::KEY012, 0);
    Table2 table5("table5", Table2::KEY13, 0);
  }

  // Lifetime-less constructor with non-zero size limit
  {
    Table2 table1("table1", Table2::KEYID, 100);
    Table2 table2("table2", Table2::KEY0, 100);
    Table2 table3("table3", Table2::KEY01, 100);
    Table2 table4("table4", Table2::KEY012, 100);
    Table2 table5("table5", Table2::KEY13, 100);
  }
}


void
testTable2::testSizeLimitID()
{
  // A table with a size limit indexed by ID
  Table2 table("LimitedSize", Table2::KEYID, SIZE);

  // Until I reach the maximum size, the table size should keep
  // increasing with every insertion
  for (uint i = 1;
       i <= SIZE;
       i++) {
    // No need for fields. It's just a new tuple
    TuplePtr t(new Tuple());
    t->freeze();

    // The insertion should return true
    BOOST_CHECK_MESSAGE(table.insert(t),
                        "Insertion in key-less table did not grow it.");

    // The size of the table should be i
    BOOST_CHECK_MESSAGE(i == table.size(),
                        "Table size is not growing to its maximum.");
  }

  // Extra tuple insertions should keep the size to the maximum while
  // insertions still succeed.
  for (uint i = 0;
       i < EXTRA_TUPLES;
       i++) {
    TuplePtr t(new Tuple());
    t->freeze();

    // The insertion should return true
    BOOST_CHECK_MESSAGE(table.insert(t),
                        "Insertion in key-less table did not grow it.");

    // The size of the table should be i
    BOOST_CHECK_MESSAGE(table.size() == SIZE,
                        "Table size is not stuck at the maximum.");
  }
}


void
testTable2::testSizeLimitSingle()
{
  // A table with a size limit indexed by ID
  Table2 table("LimitedSize", Table2::KEY1, SIZE);

  // Until I reach the maximum size, the table size should keep
  // increasing with every insertion
  for (uint i = 1;
       i <= SIZE;
       i++) {
    // Field 1 is the counter
    TuplePtr t(new Tuple());
    t->append(Val_Str::mk("testTuple"));
    t->append(Val_Int32::mk(i));
    t->freeze();

    // The insertion should return true
    BOOST_CHECK_MESSAGE(table.insert(t),
                        "Insertion in single-field key table did not grow it.");

    // The size of the table should be i
    BOOST_CHECK_MESSAGE(i == table.size(),
                        "Table size is not growing to its maximum.");
  }

  // Reinserting the initial tuples should faile and the size should no
  // longer grow.
  for (uint i = 1;
       i <= SIZE;
       i++) {
    // Field 1 is the counter
    TuplePtr t(new Tuple());
    t->append(Val_Str::mk("testTuple"));
    t->append(Val_Int32::mk(i));
    t->freeze();

    // The insertion should return false
    BOOST_CHECK_MESSAGE(!table.insert(t),
                        "Reinsertion in single-field key table"
                        << " grew it.");

    // The size of the table should be i
    BOOST_CHECK_MESSAGE(SIZE == table.size(),
                        "Table size is other then maximum.");
  }

  // Extra tuple insertions should keep the size to the maximum while
  // insertions still succeed.
  for (uint i = 1;
       i <= EXTRA_TUPLES;
       i++) {
    TuplePtr t(new Tuple());
    t->append(Val_Str::mk("testTuple"));
    t->append(Val_Int32::mk(i + SIZE));
    t->freeze();

    // The insertion should return true
    BOOST_CHECK_MESSAGE(table.insert(t),
                        "Insertion in single-field key table did not grow it.");

    // The size of the table should be i
    BOOST_CHECK_MESSAGE(table.size() == SIZE,
                        "Table size is not stuck at the maximum.");
  }
}


void
testTable2::testSizeLimitMulti()
{
  // A table with a size limit indexed by ID
  Table2 table("LimitedSize", Table2::KEY13, SIZE);

  // Until I reach the maximum size, the table size should keep
  // increasing with every insertion
  for (uint i = 1;
       i <= SIZE;
       i++) {
    // Fields 1 and 3 are the counter
    TuplePtr t(new Tuple());
    t->append(Val_Str::mk("testTuple"));
    t->append(Val_Int32::mk(i));
    t->append(Val_Str::mk("middle"));
    t->append(Val_Int32::mk(i));
    t->freeze();

    // The insertion should return true
    BOOST_CHECK_MESSAGE(table.insert(t),
                        "Insertion in multi-field key table did "
                        << "not grow it.");

    // The size of the table should be i
    BOOST_CHECK_MESSAGE(i == table.size(),
                        "Table size is not growing to its maximum.");
  }

  // Reinsert and enjoy inactivity
  for (uint i = 1;
       i <= SIZE;
       i++) {
    // Fields 1 and 3 are the counter
    TuplePtr t(new Tuple());
    t->append(Val_Str::mk("testTuple"));
    t->append(Val_Int32::mk(i));
    t->append(Val_Str::mk("middle"));
    t->append(Val_Int32::mk(i));
    t->freeze();

    // The insertion should return false
    BOOST_CHECK_MESSAGE(!table.insert(t),
                        "Reinsertion in multi-field key table "
                        << "grew it.");

    // The size of the table should be i
    BOOST_CHECK_MESSAGE(SIZE == table.size(),
                        "Table size is not stuck to maximum during"
                        << " reinsertion.");
  }

  // Extra tuple insertions should keep the size to the maximum while
  // insertions still succeed.
  for (uint i = 1;
       i <= EXTRA_TUPLES;
       i++) {
    TuplePtr t(new Tuple());
    t->append(Val_Str::mk("testTuple"));
    t->append(Val_Int32::mk(i + SIZE));
    t->append(Val_Str::mk("middle"));
    t->append(Val_Int32::mk(i + SIZE));
    t->freeze();

    // The insertion should return true
    BOOST_CHECK_MESSAGE(table.insert(t),
                        "Insertion in multi-field key table did not grow it.");

    // The size of the table should be i
    BOOST_CHECK_MESSAGE(table.size() == SIZE,
                        "Table size is not stuck at the maximum.");
  }
}


/**
 * This test corresponds on a particular problem encountered with
 * aggregate testing on Tables v1.  It creates a table whose tuples are
 * <int, int>.  It creates a unique index on all fields, and a multiple
 * index on the first field.  It inserts <0, 10>, <0, 15>, <0, 5>, and
 * then removes <0, 5> from the table.  Then it looks up <0, 5> in the
 * multiple index.  It should not find it.
 */
void
testTable2::testSuperimposedIndexRemoval()
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
  Table2 table("test_table", Table2::KEY01, 200);
  table.secondaryIndex(Table2::KEY0);

  // Insert the three tuples in that order
  BOOST_CHECK_MESSAGE(table.insert(a),
                      "Tuple a '"
                      << a->toString()
                      << "' should be newly inserted");
  BOOST_CHECK_MESSAGE(table.insert(b),
                      "Tuple b '"
                      << b->toString()
                      << "' should be newly inserted");
  BOOST_CHECK_MESSAGE(table.insert(c),
                      "Tuple c '"
                      << c->toString()
                      << "' should be newly inserted");

  // Remove the last tuple
  BOOST_CHECK_MESSAGE(table.remove(c),
                      "Tuple c '"
                      << c->toString()
                      << "' should be fully deleted");

  // Lookup the tuple itself in the unique index
  BOOST_CHECK_MESSAGE(table.lookup(Table2::KEY01, c)->done(),
                      "Table test. Lookup of removed tuple "
                      << c->toString()
                      << " should return no results.");

  // Count the elements matching a on the multiple index (i.e., all
  // elements). It should contain two elements
  Table2::IteratorPtr m = table.lookup(Table2::KEY0, a);
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


























#if 0

// Add tests for flushing due to length

// Add tests for flushing due to time

// Fix aggregates test

// Add tests for multi-field group by fields

void
testTable2::testIndexing()
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
  Table2Ptr tbl(new Table2("test_table", Table2::KEY0,
                           200));
  for( int i=0; i < N_TPLS/2; i++) { 
    tbl->insert(tpls[i]);
  }
  
  
  for( int i=0; i< N_TPLS/2; i++) { 
    BOOST_CHECK_MESSAGE(!tbl->lookup(Table2::KEY0, Val_Int32::mk(i))->done(),
                        "Table test. Lookup "
                        << i
                        << ".  Tuple is not in the table but should.");
    
    BOOST_CHECK_MESSAGE(tbl->lookup(Table2::KEY0,Val_Int32::mk(i + N_TPLS/2))->done(),
                        "Table test. Lookup "
                        << (i + N_TPLS/2)
                        << ".  Tuple is in the table but shouldn't");
  }
  
  
  // Check secondary indices
  tbl.reset(new Table2("test_table", Table2::KEY0, 200));
  tbl->add_multiple_index(4);
  for( int i=0; i < N_TPLS; i++) { 
    tbl->insert(tpls[i]);
  }
  
  // Now every key we find in the first quarter must have four
  // distinct instances in the index
  for( int i=0; i < N_TPLS / 4; i++) { 
    Table2::MultIterator iter = tbl->lookupAll(4, Val_Int32::mk(i));
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
testTable2::testBatchRemovals()
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
  Table2Ptr tbl(new Table2("test_table", 200));
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
testTable2::testBatchMultikeyRemovals()
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
  Table2Ptr tbl(new Table2("test_table", 200));
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
testTable2::testUniqueTupleRemovals()
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
  Table2Ptr tbl(new Table2("test_table", 200));
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










////////////////////////////////////////////////////////////
// Tracker Superclass
////////////////////////////////////////////////////////////


class table2Test {
public:
  table2Test(std::string,
            int);
  std::string _script;
  int _line;
};

table2Test::table2Test(std::string script,
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
class Tracker2 {
public:
  /** Create the new tracker */
  Tracker2(table2Test &);

  /** Execute the tracked script */
  void
  test();

  /** Advance the script by a command. Returns false if the tracking is
      aborted, true otherwise. */
  bool
  fetchCommand();

  /** My test */
  table2Test & _test;
  
  /** My table */
  Table2 _table;

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
Tracker2::test()
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
Tracker2::fetchCommand()
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





Tracker2::Tracker2(table2Test & test)
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

class intAggTest2 : public table2Test {
public:
  intAggTest2(std::string,
             int, int,
             Table2::AggregateFunction&,
             int);
  int _keyFieldNo;
  int _aggFieldNo;
  Table2::AggregateFunction& _function;
};

intAggTest2::intAggTest2(std::string script,
                       int keyFieldNo,
                       int aggFieldNo,
                       Table2::AggregateFunction&  function,
                       int line)
  : table2Test(script, line),
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
class AggTracker2 : public Tracker2 {
public:
  /** Create the new tracker, parse the script */
  AggTracker2(intAggTest2 & test);

  /** Listener for aggregate updates */
  void
  listener(TuplePtr t);
};


AggTracker2::AggTracker2(intAggTest2 & test)
  : Tracker2(test)
{
  // Create a multiple index on the first field
  _table.add_multiple_index(test._keyFieldNo);

  // My group-by field is the indexed field
  std::vector< unsigned > groupBy;
  groupBy.push_back(test._keyFieldNo);

  // Create a multiple-value aggregate with the given function
  Table2::MultAggregate u =
    _table.add_mult_groupBy_agg(test._keyFieldNo,
                                groupBy,
                                test._aggFieldNo,
                                test._function);

  u->addListener(boost::bind(&AggTracker2::listener, this, _1));
}


void
AggTracker2::listener(TuplePtr t)
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
testTable2::testAggregates()
{
  intAggTest2 intAggTest2s[] = {
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
    intAggTest2("i<0,10>;u<0,10>;i<0,15>;i<0,5>;u<0,5>;d<0,5>;u<0,10>;",
               0, // first field is indexed
               1, // second field is aggregated
               Table2::agg_min(), // aggregate function is MIN
               __LINE__),

    // The following tests script for MIN expects the sequence
    // INSERT <0, 10>
    // UPDATE <0, 10>
    // INSERT <0, 5>
    // UPDATE <0, 5>
    // First field is indexed, second field is aggregated
    intAggTest2("i<0,10>;u<0,10>;i<0,5>;u<0,5>;",
               0, 1, Table2::agg_min(),
               __LINE__),

    intAggTest2("i<0,10>;u<0,10>;i<0,5>;u<0,5>;i<0,20>;i<0,30>;",
               0, 1, Table2::agg_min(),
               __LINE__),

    intAggTest2("i<0,10>;u<0,10>;i<0,5>;u<0,5>;i<0,20>;i<0,3>;u<0,3>;i<0,4>;",
               0, 1, Table2::agg_min(),
               __LINE__),


    ////////////////////////////////////////////////////////////
    // MAX

    intAggTest2("i<0,10>;u<0,10>;i<0,5>;",
               0, 1, Table2::agg_max(),
               __LINE__),

    intAggTest2("i<0,10>;u<0,10>;i<0,15>;u<0,15>;i<0,3>;",
               0, 1, Table2::agg_max(),
               __LINE__),

    intAggTest2("i<0,10>;u<0,10>;i<0,5>;i<0,15>;u<0,15>;",
               0, 1, Table2::agg_max(),
               __LINE__),

    intAggTest2("i<0,10>;u<0,10>;i<0,10>;u<0,10>;",
               0, 1, Table2::agg_max(),
               __LINE__)

  };
  int noIntAggTests = sizeof(intAggTest2s) / sizeof(intAggTest2);

  for (int i = 0;
       i < noIntAggTests;
       i++) {
    // Create a script tracker
    AggTracker2 tracker(intAggTest2s[i]);
    // Run it
    tracker.test();
  }
}














////////////////////////////////////////////////////////////
// Multi-field Key Testing
////////////////////////////////////////////////////////////

class multiTest2 : public table2Test {
public:
  multiTest2(std::string,
            std::vector< uint >,
            uint);
  std::vector< uint > _key;
};

multiTest2::multiTest2(std::string script,
                     std::vector< uint > key,
                     uint line)
  : table2Test(script, line),
    _key(key)
{
}

class MultiTracker2 : public Tracker2 {
public:
  MultiTracker2(multiTest2 & test);
};


MultiTracker2::MultiTracker2(multiTest2 & test)
  : Tracker2(test)
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
testTable2::testMultiFieldKeys()
{
  multiTest2 multiTest2s[] = {
    multiTest2("i<0,1>;i<0,2>;i<0,3>;f<0,2>;",
              std::vector<unsigned> (0, 1),
              __LINE__),
    

    multiTest2("i<0,1>;i<0,2>;i<0,3>;f<0,2>;",
              std::vector<unsigned> (0, 1),
              __LINE__)
  };
  uint noIntMultiFieldTests = sizeof(multiTest2s) / sizeof(multiTest2);

  for (uint i = 0;
       i < noIntMultiFieldTests;
       i++) {
    // Create a script tracker
    MultiTracker2 tracker(multiTest2s[i]);
    // Run it
    tracker.test();
  }
}



#endif


testTable2_testSuite::testTable2_testSuite()
  : boost::unit_test_framework::test_suite("testTable2: Marshaling/Unmarshaling")
{
  boost::shared_ptr<testTable2> instance(new testTable2());
  
  add(BOOST_CLASS_TEST_CASE(&testTable2::testSuperimposedIndexRemoval, instance));
  add(BOOST_CLASS_TEST_CASE(&testTable2::testCreateDestroy, instance));
  add(BOOST_CLASS_TEST_CASE(&testTable2::testSizeLimitID, instance));
  add(BOOST_CLASS_TEST_CASE(&testTable2::testSizeLimitSingle, instance));
  add(BOOST_CLASS_TEST_CASE(&testTable2::testSizeLimitMulti, instance));
#if 0
  add(BOOST_CLASS_TEST_CASE(&testTable2::testIndexing, instance));
  add(BOOST_CLASS_TEST_CASE(&testTable2::testBatchRemovals, instance));
  add(BOOST_CLASS_TEST_CASE(&testTable2::testBatchMultikeyRemovals, instance));
  add(BOOST_CLASS_TEST_CASE(&testTable2::testUniqueTupleRemovals, instance));
  add(BOOST_CLASS_TEST_CASE(&testTable2::testAggregates, instance));
  add(BOOST_CLASS_TEST_CASE(&testTable2::testMultiFieldKeys, instance));
#endif
}

