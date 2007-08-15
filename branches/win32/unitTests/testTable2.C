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
#include "val_uint32.h"
#include "val_uint64.h"
#include "val_id.h"
#include "val_null.h"
#include "ID.h"

#include "aggMin.h"
#include "aggMax.h"
#include "aggCount.h"

#include "testTable2.h"
#include <boost/bind.hpp>
#include "vector"

#include "stdlib.h"

class testTable2
{
private:
  static const uint SIZE = 100 / 4 * 4;

  static const uint EXTRA_TUPLES = 1000;


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


  /** Test insert/remove/lookup scripts */
  void
  testInsertRemoveLookupScripts();


  /** A special-case insert, overwrite, lookup sequence. */
  void
  testPrimaryOverwrite();


  /** Ensure that when a look something up, I get only it and not its
      successor, non-matching elements. This was a bug in which when
      using index.find() in common table, we'd end the iterator with
      index.end() as opposed to with index.upper_bound(). */
  void
  testAvoidIndexTail();


  /** Check that a secondary index search is equivalent to scanning and
      selecting. */
  void
  testSecondaryEquivalence();


  /** Test aggregate scripts */
  void
  testAggregates();


  /** Old indexing test from v1 ported. */
  void
  testIndexing();

  
  /** Old batch removal test from v1 ported. */
  void
  testBatchRemovals();


  /** Old batch removal from multi-field keys */
  void
  testBatchMultikeyRemovals();

  
  /** Old unique tuple removal test from v1 ported */
  void
  testUniqueTupleRemovals();


  /** Projected lookups */
  void
  testProjectedLookups();

  
  /** Random inserts/deletes */
  void
  testPseudoRandomInsertDeleteSequences();
};



void
testTable2::testCreateDestroy()
{
  // Shortest constructor
  {
    Table2 table1("table1", Table2::theKey(CommonTable::KEYID));
    Table2 table2("table2", Table2::theKey(CommonTable::KEY0));
    Table2 table3("table3", Table2::theKey(CommonTable::KEY01));
    Table2 table4("table4", Table2::theKey(CommonTable::KEY012));
    Table2 table5("table5", Table2::theKey(CommonTable::KEY13));
  }

  // Lifetime-less constructor with 0 size limit
  {
    Table2 table1("table1", Table2::theKey(CommonTable::KEYID), 0);
    Table2 table2("table2", Table2::theKey(CommonTable::KEY0), 0);
    Table2 table3("table3", Table2::theKey(CommonTable::KEY01), 0);
    Table2 table4("table4", Table2::theKey(CommonTable::KEY012), 0);
    Table2 table5("table5", Table2::theKey(CommonTable::KEY13), 0);
  }

  // Lifetime-less constructor with non-zero size limit
  {
    Table2 table1("table1", Table2::theKey(CommonTable::KEYID), 100);
    Table2 table2("table2", Table2::theKey(CommonTable::KEY0), 100);
    Table2 table3("table3", Table2::theKey(CommonTable::KEY01), 100);
    Table2 table4("table4", Table2::theKey(CommonTable::KEY012), 100);
    Table2 table5("table5", Table2::theKey(CommonTable::KEY13), 100);
  }
}


void
testTable2::testSizeLimitID()
{
  // A table with a size limit indexed by ID
  Table2 table("LimitedSize", Table2::theKey(CommonTable::KEYID), SIZE);

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
  Table2 table("LimitedSize", Table2::theKey(CommonTable::KEY1), SIZE);

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
  Table2 table("LimitedSize", Table2::theKey(CommonTable::KEY13), SIZE);

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
  Table2 table("test_table", Table2::theKey(CommonTable::KEY01), 200);
  table.secondaryIndex(Table2::theKey(CommonTable::KEY0));

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
  BOOST_CHECK_MESSAGE(table.lookup(Table2::theKey(CommonTable::KEY01), c)->done(),
                      "Table test. Lookup of removed tuple "
                      << c->toString()
                      << " should return no results.");

  // Count the elements matching a on the multiple index (i.e., all
  // elements). It should contain two elements
  Table2::Iterator m = table.lookup(Table2::theKey(CommonTable::KEY0), a);
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


/**
 * This test tries a simple tuple overwrite using the primary index.  It
 * inserts <0,10> and then overwrites it with i<0,15>, expecting to find
 * f<0,15> when looking up by <0, 15>.  The table's primary key is 0.
 */
void
testTable2::testPrimaryOverwrite()
{
  TuplePtr a = Tuple::mk();
  a->append(Val_Int32::mk(0));
  a->append(Val_Int32::mk(10));
  a->freeze();
  
  TuplePtr b = Tuple::mk();
  b->append(Val_Int32::mk(0));
  b->append(Val_Int32::mk(15));
  b->freeze();
  
  // Create a table with unique first fields.
  Table2 table("test_table", Table2::theKey(CommonTable::KEY0));

  // Insert the first tuples
  BOOST_CHECK_MESSAGE(table.insert(a),
                      "Tuple a '"
                      << a->toString()
                      << "' should be newly inserted");
  BOOST_CHECK_MESSAGE(table.insert(b),
                      "Tuple b '"
                      << b->toString()
                      << "' should be newly inserted (overwriting a).");

  // Lookup b in the primary index
  Table2::Iterator i = table.lookup(Table2::theKey(CommonTable::KEY0), b);
  BOOST_CHECK_MESSAGE(!i->done(),
                      "Lookup of tuple b '"
                      << b->toString()
                      << "' should return results.");

  if (!i->done()) {
    TuplePtr t = i->next();
    BOOST_CHECK_MESSAGE(t->compareTo(b) == 0,
                        "Lookup of tuple b '"
                        << b->toString()
                        << "' should return b.");

    BOOST_CHECK_MESSAGE(i->done(),
                        "Lookup of tuple b '"
                        << b->toString()
                        << "' should return exactly and only b.");
  }
}


/**
 * This test exercises projected lookups.  Especially of tuples with
 * different sizes.
 */
void
testTable2::testProjectedLookups()
{
  TuplePtr a = Tuple::mk();
  a->append(Val_Int32::mk(0));
  a->append(Val_Int32::mk(10));
  a->append(Val_Int32::mk(0));
  a->freeze();
  
  TuplePtr b = Tuple::mk();
  b->append(Val_Int32::mk(1));
  b->append(Val_Int32::mk(0));
  b->freeze();
  
  // Create a table with unique first fields.
  Table2 table("table1", Table2::theKey(CommonTable::KEY0));

  // Insert the first tuple
  BOOST_CHECK_MESSAGE(table.insert(a),
                      "Tuple a '"
                      << a->toString()
                      << "' should be newly inserted");

  // Lookup the second tuple with its first field on the primary index
  Table2::Iterator i = table.lookup(Table2::theKey(CommonTable::KEY0), Table2::theKey(CommonTable::KEY0), b);
  BOOST_CHECK_MESSAGE(i->done(),
                      "Lookup of tuple b '"
                      << b->toString()
                      << "' on field 0 should return no results.");


  // Lookup the second tuple with its second field on the primary index
  i = table.lookup(Table2::theKey(CommonTable::KEY1), Table2::theKey(CommonTable::KEY0), b);
  BOOST_CHECK_MESSAGE(!i->done(),
                      "Lookup of tuple b '"
                      << b->toString()
                      << "' on field 1 should return results.");
  
  if (!i->done()) {
    TuplePtr t = i->next();
    BOOST_CHECK_MESSAGE(t->compareTo(a) == 0,
                        "Lookup of tuple b '"
                        << b->toString()
                        << "' on field 1 should return "
                        << a->toString());

    BOOST_CHECK_MESSAGE(i->done(),
                        "Lookup of tuple b '"
                        << b->toString()
                        << "' on field 1 should return exactly "
                        << "one result.");
  }
}



////////////////////////////////////////////////////////////
// Insert/Delete/Lookup scripts
////////////////////////////////////////////////////////////

/** Superclass of script-based tests */
class table2Test {
public:
  table2Test(std::string script,
             int line,
             Table2::Key& key,
             uint32_t size);

  // this is needed for VC++
  table2Test& operator=(const table2Test& t) {
	  _script = t._script;
	  _line = t._line;
	  _key = t._key;
	  _size = t._size;
	  return *this;
  }

  std::string _script;


  int _line;


  Table2::Key& _key;

  
  uint32_t _size;
};


table2Test::table2Test(std::string script,
                       int line,
                       Table2::Key& key,
                       uint32_t size = Table2::DEFAULT_SIZE)
  : _script(script),
    _line(line),
    _key(key),
    _size(size)
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
  /** Process an array of tests */
  static void
  process(std::vector< table2Test >);




private:
  /** The all-fields key vector */
  std::vector< unsigned > _allFields;




protected:
  /** Create the new tracker */
  Tracker2(table2Test &);


  /** My test */
  table2Test & _test;
  

  /** My table */
  Table2 _table;


  /** The remainder of my script */
  std::string _remainder;


  /** Advance the script by a command. Returns false if the tracking is
      aborted, true otherwise. */
  bool
  fetchCommand();


  /** The current command type */
  char _command;


  /** The current tuple */
  TuplePtr _tuple;


  /** Execute the tracked script */
  void
  test();
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
//         std::cout << "Tracker:Inserting '"
//                   << _tuple->toString()
//                   << "'\n";
        _table.insert(_tuple);
        break;
      case 'd':
        // Delete from table
//         std::cout << "Tracker:Removing '"
//                   << _tuple->toString()
//                   << "'\n";
        _table.remove(_tuple);
        break;
      case 'f':
        // Find in table (lookup successfully)
        {
          // Lookup the tuple in the command and ensure it was found.
//           std::cout << "Tracker:Looking up '"
//                     << _tuple->toString()
//                     << "'\n";
          Table2::Iterator i = _table.lookup(_test._key, _tuple);
          BOOST_CHECK_MESSAGE(!i->done(),
                              "Did not find results for expected tuple '"
                              << _tuple->toString()
                              << "'.");
          
          if (i->done()) {
            // OK, we had no results, so we might as well skip the next
            //test 
          } else {
            // Ensure the results contain the expected tuple exactly
            bool found = false;
            while (!i->done()) {
              TuplePtr t = i->next();
              if (t->compareTo(_tuple) == 0) {
                found = true;
              }
            }
            // Ensure we did find it
            BOOST_CHECK_MESSAGE(found,
                                "Error in test line "
                                << _test._line
                                << " with suffix "
                                << "\""
                                << _remainder
                                << "\". Results did not contain "
                                << "expected tuple '"
                                << _tuple->toString()
                                << "'.");
          }
        }
        break;
      case 'm':
        // Miss in table (lookup unsuccessfully)
        {
          // Lookup the tuple in the command and ensure it is not found
//           std::cout << "Tracker:Looking up '"
//                     << _tuple->toString()
//                     << "'\n";
          Table2::Iterator i = _table.lookup(_test._key, _tuple);
          
          // If there were any results
          if (!i->done()) {
            // Ensure none match the tuple
            bool found = false;
            while (!i->done()) {
              TuplePtr t = i->next();
              if (t->compareTo(_tuple) == 0) {
                found = true;
              }
            }
            // Ensure we did not find it
            BOOST_CHECK_MESSAGE(!found,
                                "Results contained expected tuple '"
                                << _tuple->toString()
                                << "'.");
          } else {
            // We're done. Nothing else to check.
          }
        }
        break;
      case 's':
        // Create a secondary index
        {
          // Turn tuple into key vector
          Table2::Key k;
          for (uint i = 0;
               i < _tuple->size();
               i++) {
            k.push_back(Val_UInt32::cast((*_tuple)[i]));
          }
          _table.secondaryIndex(k);
        }
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
  case 's':
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
    
    // is this the null tuple?
    if (field == "null") {
      _tuple->append(Val_Null::mk());
    } else {
      // Identify any type information.
      ValuePtr intField;
      switch (field[field.length() - 1]) {
      case 'U':
        // This should be unsigned 64bit
        intField = Val_UInt64::mk(Val_UInt64::cast(Val_Str::mk(field)));
        break;
      case 'u':
        // This should be unsigned 32bit
        intField = Val_UInt32::mk(Val_UInt32::cast(Val_Str::mk(field)));
        break;
      default:
        // Interpret as signed 32bit
        intField = Val_Int32::mk(Val_Int32::cast(Val_Str::mk(field)));
        break;
      }
      _tuple->append(intField);
    }
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
    _table("trackerTest", _test._key, _test._size)
{
}


void
Tracker2::process(std::vector< table2Test > tests)
{
  for (std::vector< table2Test >::iterator i = tests.begin();
       i != tests.end();
       i++) {
    // Create a script tracker
    Tracker2 tracker((*i));
    // Run it
    tracker.test();
  }
}




////////////////////////////////////////////////////////////
// Specific insert/lookup/delete scripts
////////////////////////////////////////////////////////////

/** 
 * i<tuple> means insert into the primary key
 *
 * d<tuple> means delete from the primary key
 *
 * f<tuple> means find the identical tuple looking up into the primary
 * key
 *
 * m<tuple> means do not find this identical tuple looking into the
 * primary key
 *
 * s<key> means create the secondary index described by the key
 */
void
testTable2::testInsertRemoveLookupScripts()
{
  table2Test t[] =
    {
      table2Test("i<0,10>;i<0,15>;i<0,20>;i<0,25>;m<0,10>;" // first one flushed
                 //      >----------------------->  
                 "i<0,15>;"
                 //      >20, 25, 15>
                 "i<0,30>;f<0,15>;m<0,20>;" // 15 refreshed, 20 not
                 //      >25, 15, 30>
                 "i<0,15>;i<0,15>;i<0,35>;m<0,25>;",
                 __LINE__,
                 Table2::theKey(CommonTable::KEY01), 3),
      
      table2Test("i<0,10>;i<0,15>;f<0,15>;m<0,10>;d<0,15>;m<0,15>;",
                 __LINE__,
                 Table2::theKey(CommonTable::KEY0)),
      
      table2Test("i<0,10>;i<0,15>;f<0,15>;f<0,10>;d<0,15>;m<0,15>;"
                 "f<0,10>;d<0,10>;m<0,15>;m<0,10>;",
                 __LINE__,
                 Table2::theKey(CommonTable::KEY01)),
      
      table2Test("i<0,10>;i<0,15>;m<0,10>;m<0,15>;",
                 __LINE__,
                 Table2::theKey(CommonTable::KEYID)),
      
      table2Test("i<0,10>;f<0,10>;d<0,10>;m<0,10>;d<0,10>;m<0,10>;"
                 "i<0,15>;f<0,15>;",
                 __LINE__,
                 Table2::theKey(CommonTable::KEY0)),
      
      table2Test("i<0,10>;i<0,15>;i<0,20>;i<0,25>;i<0,30>;"
                 "i<0,15>;f<0,15>;",
                 __LINE__,
                 Table2::theKey(CommonTable::KEY0), 3),
      
      table2Test("i<0,10>;i<0,15>;i<0,20>;i<0,25>;i<0,30>;"
                 "d<0,15>;m<0,15>;",
                 __LINE__,
                 Table2::theKey(CommonTable::KEY0), 3),
      
      table2Test("i<0,10>;i<0,15>;i<0,20>;i<0,25>;i<0,30>;"
                 "i<0,15>;d<0,15>;m<0,15>;",
                 __LINE__,
                 Table2::theKey(CommonTable::KEY0), 3),
      
      table2Test("i<0,10>;d<0,10>;i<0,10>;"
                 "i<0,15>;i<0,20>;i<0,25>;i<0,30>;"
                 "i<0,15>;d<0,15>;m<0,15>;",
                 __LINE__,
                 Table2::theKey(CommonTable::KEY0), 3),
      
    };
  std::vector< table2Test > vec(t,
                                t + sizeof(t)/sizeof(t[0]));
  
  Tracker2::process(vec);
}




////////////////////////////////////////////////////////////
// Secondary Index completeness
////////////////////////////////////////////////////////////

void
testTable2::testSecondaryEquivalence()
{
  // Make sure that an index lookup is the same as a full scan with the
  // appropriate selection (predicate check.)

  Table2 table("secondary equivalence", Table2::theKey(CommonTable::KEY0));
  table.secondaryIndex(Table2::theKey(CommonTable::KEY1));

  // Add a bunch of tuples with some replacements
  uint TUPLES = 1000;
  uint GROUPS = 5;
  for (uint i = 0;
       i < TUPLES;
       i++) {
    TuplePtr t = Tuple::mk();
    t->append(Val_Str::mk(Val_Str::cast(Val_UInt32::mk((i + ((i + 1) % (GROUPS - 1)))/(GROUPS)))));
    t->append(Val_UInt32::mk(i % GROUPS));
    t->freeze();
  }

  // Now for every distinct second field (0, 1, 2, ..., GROUPS-1), get
  // the secondary index lookup results and store them. Compute the same
  // thing using a primary scan with selection. Are the two answer sets
  // identical?
  ValuePtr emptyString = Val_Str::mk("");
  for (uint i = 0;
       i < GROUPS;
       i++) {
    ValuePtr iVal = Val_UInt32::mk(i);
    TuplePtr lookupT = Tuple::mk();
    lookupT->append(emptyString);
    lookupT->append(iVal);
    Table2::Iterator it = table.lookup(Table2::theKey(CommonTable::KEY1), lookupT);

    TupleSet sResults;
    while (!it->done()) {
      sResults.insert(it->next());
    }

    // Now scan the whole table and find all tuples whose second field
    // equals i. Remove every found tuple from the results set, ensuring
    // that every removal corresponds to a found element. If in the end
    // the results set is not empty, then we have a problem.
    Table2::Iterator s = table.scan();

    while (!s->done()) {
      TuplePtr found = s->next();
      if ((*found)[1] == iVal) {
        // we found a result
        BOOST_CHECK_MESSAGE(sResults.erase(found) == 1,
                            "A scan result is missing from secondary result");
      }
    }
    BOOST_CHECK_MESSAGE(sResults.size() == 0,
                        "There are secondary results not found during scan.");
  }
}






////////////////////////////////////////////////////////////
// Aggregate Testing
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

class intAggTest2 : public table2Test {
public:
  intAggTest2(std::string script,
              Table2::Key& key,
              uint32_t size,
              Table2::Key& groupBy,
              uint aggField,
              std::string func,
              int line);
  // this is needed for VC++
  intAggTest2& operator=(const intAggTest2& t) {
	  _groupBy = t._groupBy;
	  _aggFieldNo = t._aggFieldNo;
	  _function = t._function;
	  return *this;
  }

  Table2::Key& _groupBy;

  uint _aggFieldNo;

  std::string _function;
};

intAggTest2::intAggTest2(std::string script,
                         Table2::Key& key,
                         uint32_t size,
                         Table2::Key& groupBy,
                         uint aggFieldNo,
                         std::string func,
                         int line)
  : table2Test(script, line, key, size),
    _groupBy(groupBy),
    _aggFieldNo(aggFieldNo),
    _function(func)
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
  /** Process an array of tests */
  static void
  process(std::vector< intAggTest2 >);
  
  
  /** Create the new tracker, parse the script */
  AggTracker2(intAggTest2 & test);
  

  /** Listener for aggregate updates */
  void
  listener(TuplePtr t);
};


void
AggTracker2::process(std::vector< intAggTest2 > tests)
{
  for (std::vector< intAggTest2 >::iterator i = tests.begin();
       i != tests.end();
       i++) {
    // Create a script tracker
    AggTracker2 tracker((*i));
    // Run it
    tracker.test();
  }
}


AggTracker2::AggTracker2(intAggTest2 & test)
  : Tracker2(test)
{
  // Create a multiple index on the group by field
  _table.secondaryIndex(test._groupBy);

  // Create a multiple-value aggregate with the given function
  Table2::Aggregate u =
    _table.aggregate(test._groupBy,
                     test._aggFieldNo,
                     test._function);
  u->listener(boost::bind(&AggTracker2::listener, this, _1));
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
                    << "' but contained the '"
                    << _command
                    << "' command instead.");
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
                    << _tuple->toConfString()
                    << "' but received '"
                    << t->toConfString()
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
  intAggTest2 t[] = {
    // Modifications cause two aggregate updates, one when the old value
    // is removed and one when the new value is inserted.  here with a
    // non-null backup
    intAggTest2("i<0,2,5>;u<0,5>;"
                "i<0,1,2>;u<0,2>;"
                "i<0,1,3>;u<0,5>;u<0,3>;",
                Table2::theKey(CommonTable::KEY01), // first three fields are indexed
                Table2::DEFAULT_SIZE, // use default max size
                Table2::theKey(CommonTable::KEY0), // first field is group-by
                2, // third field is aggregated
                "MIN", // aggregate function is MIN
                __LINE__),

    // Test reinsertion without update but with time upate
    intAggTest2("i<0,10>;u<0,10>;i<0,5>;u<0,5>;i<0,20>;"
                // <10, 5, 20<
                "i<0,3>;u<0,3>;m<0,10>;"
                // <5, 20, 3<
                "i<0,8>;i<0,15>;m<0,20>;"
                // <3, 8, 15<
                "i<0,4>;u<0,4>;"
                // <8, 15, 4<
                "i<0,8>;i<0,20>;"
                // <4, 8, 20<
                "i<0,30>;u<0,8>;",
                // <8, 20, 30<
                Table2::theKey(CommonTable::KEY01), 3,
                Table2::theKey(CommonTable::KEY0), 1, "MIN",
                __LINE__),


    // Here there's an update for every insert that is not identical to
    // its preceeding one since the primary key is the first field, so
    // any new insertion removes the old tuple.
    intAggTest2("i<0,10>;u<0,10>;i<0,10>;"
                "i<0,15>;u<0,null>;u<0,15>;"
                "i<0,5>;u<0,null>;u<0,5>;"
                "d<0,5>;u<0,null>;",
                Table2::theKey(CommonTable::KEY0), // first field is indexed
                Table2::DEFAULT_SIZE, // use default max size
                Table2::theKey(CommonTable::KEY0), // first field is group-by
                1, // second field is aggregated
                "MIN", // aggregate function is MIN
                __LINE__),

    // The following tests script expects the sequence
    // INSERT <0, 10>
    // UPDATE <0, 10>
    // INSERT <0, 15>
    // INSERT <0, 5>
    // UPDATE <0, 5>
    // DELETE <0, 5>
    // UPDATE <0, 10>
    intAggTest2("i<0,10>;u<0,10>;i<0,15>;i<0,5>;u<0,5>;d<0,5>;u<0,10>;",
                Table2::theKey(CommonTable::KEY01), // first field is indexed
                Table2::DEFAULT_SIZE, // use default max size
                Table2::theKey(CommonTable::KEY0), // first field is group-by
                1, // second field is aggregated
                "MIN", // aggregate function is MIN
                __LINE__),

    // The following tests script for MIN expects the sequence
    // INSERT <0, 10>
    // UPDATE <0, 10>
    // INSERT <0, 5>
    // UPDATE <0, 5>
    intAggTest2("i<0,10>;u<0,10>;i<0,5>;u<0,5>;",
                Table2::theKey(CommonTable::KEY01), Table2::DEFAULT_SIZE,
                Table2::theKey(CommonTable::KEY0),
                1, "MIN",
                __LINE__),
    
    intAggTest2("i<0,10>;u<0,10>;i<0,5>;u<0,5>;i<0,20>;i<0,30>;",
                Table2::theKey(CommonTable::KEY01), Table2::DEFAULT_SIZE,
                Table2::theKey(CommonTable::KEY0), 1, "MIN",
                __LINE__),
    
    intAggTest2("i<0,10>;u<0,10>;i<0,5>;u<0,5>;i<0,20>;i<0,3>;u<0,3>;i<0,4>;",
                Table2::theKey(CommonTable::KEY01), Table2::DEFAULT_SIZE,
                Table2::theKey(CommonTable::KEY0), 1, "MIN",
                __LINE__),


    ////////////////////////////////////////////////////////////
    // MAX

    intAggTest2("i<0,10>;u<0,10>;i<0,5>;",
                Table2::theKey(CommonTable::KEY01), Table2::DEFAULT_SIZE,
                Table2::theKey(CommonTable::KEY0), 1, "MAX",
                __LINE__),
    
    intAggTest2("i<0,10>;u<0,10>;i<0,15>;u<0,15>;i<0,3>;",
                Table2::theKey(CommonTable::KEY01), Table2::DEFAULT_SIZE,
                Table2::theKey(CommonTable::KEY0), 1, "MAX",
                __LINE__),
    
    intAggTest2("i<0,10>;u<0,10>;i<0,5>;i<0,15>;u<0,15>;",
                Table2::theKey(CommonTable::KEY01), Table2::DEFAULT_SIZE,
                Table2::theKey(CommonTable::KEY0), 1, "MAX",
                __LINE__),
    
    intAggTest2("i<0,10>;u<0,10>;i<0,10>;",
                Table2::theKey(CommonTable::KEY01), Table2::DEFAULT_SIZE,
                Table2::theKey(CommonTable::KEY0), 1, "MAX",
                __LINE__),

    intAggTest2("i<0,10>;u<0,10>;d<0,10>;u<0,null>;",
                Table2::theKey(CommonTable::KEY01), Table2::DEFAULT_SIZE,
                Table2::theKey(CommonTable::KEY0), 1, "MAX",
                __LINE__),


    ////////////////////////////////////////////////////////////
    // COUNT

    intAggTest2(// Insertions of same group-by
                "i<0,10>;u<0,1U>;i<0,5>;u<0,2U>;"
                // Insertion of other group-by
                "i<1,5>;u<1,1U>;"
                // Interspersed group-bys
                "i<0,12>;u<0,3U>;i<1,8>;u<1,2U>;"
                // No-op insertions
                "i<1,5>;i<0,12>;"
                // Deletions
                "d<0,5>;u<0,2U>;d<1,5>;u<1,1U>;d<0,10>;u<0,1U>;"
                // Run down to 0
                "d<0,12>;u<0,0U>;d<1,8>;u<1,0U>;",
                Table2::theKey(CommonTable::KEY01), Table2::DEFAULT_SIZE,
                Table2::theKey(CommonTable::KEY0), 1, "COUNT",
                __LINE__),
    
  };

  std::vector< intAggTest2 > vec(t,
                                 t + sizeof(t)/sizeof(t[0]));
  
  AggTracker2::process(vec);
}




// Add tests for flushing due to length

// Add tests for flushing due to time

// Fix aggregates test

// Add tests for multi-field group by fields

void
testTable2::testIndexing()
{
  TuplePtr tpls[SIZE];

  // Create our test set of tuples
  for(uint i = 0;
      i < SIZE;
      i++) {
    TuplePtr t = Tuple::mk();
    t->append(Val_UInt32::mk(i));
    t->append(Val_UInt32::mk(i/2));
    t->append(Val_UInt32::mk(i/4));
    t->append(Val_UInt32::mk(i % (SIZE/2)));
    t->append(Val_UInt32::mk(i % (SIZE/4)));
    t->freeze();
    tpls[i] = t;
  }

  {
    //  First: tuples inserted can be looked up.  Tuples not inserted
    // cannot be looked up.  Create a very simple table
    Table2 tbl("test_table", Table2::theKey(CommonTable::KEY0));
    for(uint i=0;
        i < SIZE/2;
        i++) { 
      tbl.insert(tpls[i]);
    }
    
    
    for(uint i = 0;
        i< SIZE/2;
        i++) { 
      TuplePtr t = Tuple::mk();
      t->append(Val_UInt32::mk(i));
      t->append(Val_UInt32::mk(i/2));
      t->append(Val_UInt32::mk(i/4));
      t->append(Val_UInt32::mk(i % (SIZE/2)));
      t->append(Val_UInt32::mk(i));
      t->freeze();
      BOOST_CHECK_MESSAGE(!tbl.lookup(Table2::theKey(CommonTable::KEY0), t)->done(),
                          "Table test. Lookup "
                          << i
                          << ".  Tuple is not in the table but should.");
      
      t = Tuple::mk();
      t->append(Val_UInt32::mk(i + SIZE/2));
      t->append(Val_UInt32::mk(i/2));
      t->append(Val_UInt32::mk(i/4));
      t->append(Val_UInt32::mk(i % (SIZE/2)));
      t->append(Val_UInt32::mk(i));
      t->freeze();
      BOOST_CHECK_MESSAGE(tbl.lookup(Table2::theKey(CommonTable::KEY0), t)->done(),
                          "Table test. Lookup "
                          << (i + SIZE/2)
                          << ".  Tuple is in the table but shouldn't");
    }
  }
  {
    // Check secondary indices
    Table2 tbl("test_table", Table2::theKey(CommonTable::KEY0), SIZE);
    tbl.secondaryIndex(Table2::theKey(CommonTable::KEY4));
    for(uint i = 0;
        i < SIZE;
        i++) { 
      tbl.insert(tpls[i]);
    }
    
    // Now every tuple we find with the first quarter of counters in field
    // 4 must have four exactly distinct instances in the index
    for(uint i = 0;
        i < SIZE / 4;
        i++) { 
      TuplePtr t = Tuple::mk();
      t->append(Val_UInt32::mk(i));
      t->append(Val_UInt32::mk(i));
      t->append(Val_UInt32::mk(i));
      t->append(Val_UInt32::mk(i));
      t->append(Val_UInt32::mk(i));
      t->freeze();
      Table2::Iterator iter = tbl.lookup(Table2::theKey(CommonTable::KEY4), t);
      for (uint counter = 0;
           counter < 4;
           counter++) {
        TuplePtr result = iter->next();
        BOOST_CHECK_MESSAGE(result != NULL,
                            "Table test. Key "
                            << i
                            << " seems to have too few "
                            << "tuples in the secondary index");
      }
      
      BOOST_CHECK_MESSAGE(iter->done() == true,
                          "Table test. Key "
                          << i
                          << " seems to have too many tuples "
                          << "in the secondary index");
    }
  }
}


void
testTable2::testAvoidIndexTail()
{
  // We are using the pattern from the bug that triggered this test
  Table2 table("coordinator", Table2::theKey(CommonTable::KEY1),
               Table2::DEFAULT_SIZE, Table2::DEFAULT_EXPIRATION);

  TuplePtr c10000 = Tuple::mk();
  c10000->append(Val_Str::mk("coordinator"));
  c10000->append(Val_Str::mk("localhost:10000"));
  c10000->append(Val_Str::mk("localhost:10000"));
  c10000->freeze();
  table.insert(c10000);
  
  TuplePtr c10001 = Tuple::mk();
  c10001->append(Val_Str::mk("coordinator"));
  c10001->append(Val_Str::mk("localhost:10001"));
  c10001->append(Val_Str::mk("localhost:10000"));
  c10001->freeze();
  table.insert(c10001);
  
  TuplePtr c10002 = Tuple::mk();
  c10002->append(Val_Str::mk("coordinator"));
  c10002->append(Val_Str::mk("localhost:10002"));
  c10002->append(Val_Str::mk("localhost:10000"));
  c10002->freeze();
  table.insert(c10002);
  

  // Now lookup without projection the per tuple:
  TuplePtr per = Tuple::mk();
  per->append(Val_Str::mk("per"));
  per->append(Val_Str::mk("localhost:10001"));
  per->append(Val_Int32::mk(0));
  per->freeze();

  Table2::Iterator i = table.lookup(Table2::theKey(CommonTable::KEY1), per);
  BOOST_CHECK_MESSAGE(!i->done(),
                      "Lookup of per tuple should return at least "
                      << "one single result");

  // Count the elements returned. There should be exactly one
  int count = 0;
  while (!i->done()) {
    i->next();
    count++;
  }
  BOOST_CHECK_MESSAGE(count == 1,
                      "Lookup of per tuple should return exactly "
                      << "one result. It returned "
                      << count
                      << " instead.");  
}


void
testTable2::testBatchRemovals()
{
  TuplePtr tpls[SIZE];
  // Create our test set of tuples
  for(uint i = 0;
      i < SIZE;
      i++) {
    TuplePtr t = Tuple::mk();
    t->append(Val_Int32::mk(i));
    t->append(Val_Int32::mk(i/2));
    t->append(Val_Int32::mk(i/4));
    t->append(Val_Int32::mk(i % (SIZE/2)));
    t->append(Val_Int32::mk(i % (SIZE/4)));
    t->freeze();
    tpls[i] = t;
  }
  
  // Create a unique index on first field
  Table2 tbl("test_table", Table2::theKey(CommonTable::KEY0), SIZE);
  for(uint i = 0;
      i < SIZE;
      i++) { 
    tbl.insert(tpls[i]);
  }
  
  for(uint i = 0;
      i < SIZE;
      i++) { 
    tbl.remove(tpls[i]);
  }
  
  // I should be unable to look up any elements at all
  for(uint i = 0;
      i< SIZE;
      i++) { 
    BOOST_CHECK_MESSAGE(tbl.lookup(Table2::theKey(CommonTable::KEY0), tpls[i])->done(),
                        "Table test. Lookup of removed tuple "
                        << tpls[i]->toString()
                        << " should return no results.");
  }
}


void
testTable2::testBatchMultikeyRemovals()
{
  TuplePtr tpls[SIZE];
  // Create our test set of tuples
  for(uint i = 0;
      i < SIZE;
      i++) {
    TuplePtr t = Tuple::mk();
    t->append(Val_Int32::mk(i));
    t->append(Val_Int32::mk(i/2));
    t->append(Val_Int32::mk(i/4));
    t->append(Val_Int32::mk(i % (SIZE/2)));
    t->append(Val_Int32::mk(i % (SIZE/4)));
    t->freeze();
    tpls[i] = t;
  }
  
  // Create a unique index on first two fields
  Table2 tbl("test_table", Table2::theKey(CommonTable::KEY01), 200);
  for(uint i = 0;
      i < SIZE;
      i++) { 
    tbl.insert(tpls[i]);
  }
  
  for(uint i = 0;
      i < SIZE;
      i++) { 
    tbl.remove(tpls[i]);
  }

  // I should be unable to look up any elements at all
  for(uint i = 0;
      i< SIZE;
      i++) { 
    BOOST_CHECK_MESSAGE(tbl.lookup(Table2::theKey(CommonTable::KEY01), tpls[i])->done(),
                        "Table test. Lookup of removed tuple "
                        << tpls[i]->toString()
                        << " should return no results.");
  }
}


void
testTable2::testUniqueTupleRemovals()
{
  TuplePtr tpls[SIZE];

  // Create our test set of tuples
  for(uint i = 0;
      i < SIZE;
      i++) {
    TuplePtr t = Tuple::mk();
    t->append(Val_Int32::mk(i));
    t->append(Val_Int32::mk(i/2));
    t->append(Val_Int32::mk(i/4));
    t->append(Val_Int32::mk(i % (SIZE/2)));
    t->append(Val_Int32::mk(i % (SIZE/4)));
    t->freeze();
    tpls[i] = t;
  }
  
  // Create a unique index on all five fields
  Table2 tbl("test_table", Table2::theKey(CommonTable::KEY01234), SIZE);
  tbl.secondaryIndex(Table2::theKey(CommonTable::KEY1));
  for(uint i = 0;
      i < SIZE;
      i++) { 
    tbl.insert(tpls[i]);
  }
  
  for(uint i = 0;
      i < SIZE;
      i++) { 
    tbl.remove(tpls[i]);
  }

  // I should be unable to look up any elements at all
  for(uint i = 0;
      i< SIZE;
      i++) { 
    BOOST_CHECK_MESSAGE(tbl.lookup(Table2::theKey(CommonTable::KEY01234), tpls[i])->done(),
                        "Table test. Lookup of removed tuple "
                        << tpls[i]->toString()
                        << " should return no results.");
  }
}


void
testTable2::testPseudoRandomInsertDeleteSequences()
{
  srand(0);

  {
    // Create a table with fixed lifetime but no size
    boost::posix_time::
      time_duration expiration(boost::posix_time::milliseconds(200));
    Table2 t("succ", Table2::theKey(CommonTable::KEY2), Table2::NO_SIZE, expiration);
    for (uint i = 0;
         i < SIZE + EXTRA_TUPLES;
         i++) {
      // Make a random tuple
      TuplePtr tup = Tuple::mk();
      
      // My tuple name
      tup->append(Val_Str::mk("succ"));
      
      // My node address
      ostringstream nodeID;
      nodeID << "127.0.0.1:";
      int port = rand() % 5;
      nodeID << port;
      tup->append(Val_Str::mk(nodeID.str()));
    
      // My Node identifier
      unsigned int nestedSeed = rand() % 5;
      uint32_t words[ID::WORDS];
      for (uint w = 0;
           w < ID::WORDS;
           w++) {
#ifdef WIN32
        words[w] = rand_s(&nestedSeed);
#else
        words[w] = rand_r(&nestedSeed);
#endif // WIN32
      }
      tup->append(Val_ID::mk(ID::mk(words)));
    
      tup->freeze();
    
      // Choose between insert and delete
      int r = rand();
      //    std::cout << "Random number " << r << "\n";
      if ((r & 1) == 0) {
        //      std::cout << "Inserting " << tup->toString() << "\n";
        t.insert(tup);
      } else {
        //      std::cout << "Deleting " << tup->toString() << "\n";
        t.remove(tup);
      }
    }
  }
  {
    // Create a table with fixed size but not lifetime
    Table2 t("succ", Table2::theKey(CommonTable::KEY2), 100, Table2::NO_EXPIRATION);
    for (uint i = 0;
         i < SIZE + EXTRA_TUPLES;
         i++) {
      // Make a random tuple
      TuplePtr tup = Tuple::mk();
      
      // My tuple name
      tup->append(Val_Str::mk("succ"));
      
      // My node address
      ostringstream nodeID;
      nodeID << "127.0.0.1:";
      int port = rand() % 5;
      nodeID << port;
      tup->append(Val_Str::mk(nodeID.str()));
    
      // My Node identifier
      unsigned int nestedSeed = rand() % 5;
      uint32_t words[ID::WORDS];
      for (uint w = 0;
           w < ID::WORDS;
           w++) {
#ifdef WIN32
        words[w] = rand_s(&nestedSeed);
#else
        words[w] = rand_r(&nestedSeed);
#endif // WIN32
      }
      tup->append(Val_ID::mk(ID::mk(words)));
    
      tup->freeze();
    
      // Choose between insert and delete
      int r = rand();
      //    std::cout << "Random number " << r << "\n";
      if ((r & 1) == 0) {
        //      std::cout << "Inserting " << tup->toString() << "\n";
        t.insert(tup);
      } else {
        //      std::cout << "Deleting " << tup->toString() << "\n";
        t.remove(tup);
      }
    }
  }
  {
    // Create a table with fixed lifetime and size
    boost::posix_time::
      time_duration expiration(boost::posix_time::milliseconds(200));
    Table2 t("succ", Table2::theKey(CommonTable::KEY2), 100, expiration);
    for (uint i = 0;
         i < SIZE + EXTRA_TUPLES;
         i++) {
      // Make a random tuple
      TuplePtr tup = Tuple::mk();
      
      // My tuple name
      tup->append(Val_Str::mk("succ"));
      
      // My node address
      ostringstream nodeID;
      nodeID << "127.0.0.1:";
      int port = rand() % 5;
      nodeID << port;
      tup->append(Val_Str::mk(nodeID.str()));
    
      // My Node identifier
      unsigned int nestedSeed = rand() % 5;
      uint32_t words[ID::WORDS];
      for (uint w = 0;
           w < ID::WORDS;
           w++) {
#ifdef WIN32
        words[w] = rand_s(&nestedSeed);
#else
        words[w] = rand_r(&nestedSeed);
#endif
      }
      tup->append(Val_ID::mk(ID::mk(words)));
    
      tup->freeze();
    
      // Choose between insert and delete
      int r = rand();
      //    std::cout << "Random number " << r << "\n";
      if ((r & 1) == 0) {
        //      std::cout << "Inserting " << tup->toString() << "\n";
        t.insert(tup);
      } else {
        //      std::cout << "Deleting " << tup->toString() << "\n";
        t.remove(tup);
      }
    }
  }
}





testTable2_testSuite::testTable2_testSuite()
  : boost::unit_test_framework::test_suite("testTable2: Marshaling/Unmarshaling")
{
  // Ensure the aggregate functions are initialized
  AggMin::ensureInit();
  AggMax::ensureInit();
  AggCount::ensureInit();


  boost::shared_ptr<testTable2> instance(new testTable2());
  
  add(BOOST_CLASS_TEST_CASE(&testTable2::testAggregates, instance));
  add(BOOST_CLASS_TEST_CASE(&testTable2::testInsertRemoveLookupScripts, instance));
  add(BOOST_CLASS_TEST_CASE(&testTable2::testBatchMultikeyRemovals, instance));
  add(BOOST_CLASS_TEST_CASE(&testTable2::testPseudoRandomInsertDeleteSequences, instance));
  add(BOOST_CLASS_TEST_CASE(&testTable2::testProjectedLookups, instance));
  add(BOOST_CLASS_TEST_CASE(&testTable2::testSecondaryEquivalence, instance));
  add(BOOST_CLASS_TEST_CASE(&testTable2::testAvoidIndexTail, instance));
  add(BOOST_CLASS_TEST_CASE(&testTable2::testPrimaryOverwrite, instance));
  add(BOOST_CLASS_TEST_CASE(&testTable2::testCreateDestroy, instance));
  add(BOOST_CLASS_TEST_CASE(&testTable2::testSizeLimitID, instance));
  add(BOOST_CLASS_TEST_CASE(&testTable2::testSuperimposedIndexRemoval, instance));
  add(BOOST_CLASS_TEST_CASE(&testTable2::testSizeLimitSingle, instance));
  add(BOOST_CLASS_TEST_CASE(&testTable2::testSizeLimitMulti, instance));
  add(BOOST_CLASS_TEST_CASE(&testTable2::testIndexing, instance));
  add(BOOST_CLASS_TEST_CASE(&testTable2::testBatchRemovals, instance));
  add(BOOST_CLASS_TEST_CASE(&testTable2::testUniqueTupleRemovals, instance));
}

