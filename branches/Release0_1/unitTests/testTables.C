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

class testTables
{
public:
  testTables()
  {
  }

  static const int N_TPLS = 1000;

  TuplePtr tpls[N_TPLS];

  /** Listener for aggregates */
  void
  aggListener(TuplePtr t);

  void
  test();
};

void
testTables::test()
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
  
  
  // Now check aggregates
  tbl.reset(new Table("test_table", 4));
  tbl->add_multiple_index(4);
  
  // My group-by fields are 4
  std::vector< unsigned > groupBy;
  groupBy.push_back(4);
  Table::MultAggregate u =
    tbl->add_mult_groupBy_agg(4, groupBy, 3, &Table::AGG_MAX);
  u->addListener(boost::bind(&testTables::aggListener, this, _1));
  
  for(int i = 0;
      i < N_TPLS;
      i++) { 
    tbl->insert(tpls[i]);
  }
  
  // Check what the listener has collected
  // XXX
}


void
testTables::aggListener(TuplePtr t)
{
  std::cout << "Agg update: " << t->toString() << "\n";
}

class testTables_testSuite
  : public boost::unit_test_framework::test_suite
{
public:
  testTables_testSuite()
    : boost::unit_test_framework::test_suite("testTables: Marshaling/Unmarshaling")
  {
    boost::shared_ptr<testTables> instance(new testTables());

    add(BOOST_CLASS_TEST_CASE(&testTables::test, instance));
  }
};
