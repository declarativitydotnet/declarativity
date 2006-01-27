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
 * DESCRIPTION: Test suite for tuples
 *
 */

/*
  How do you test the table?  Well... Tables can be tested in
  isolation, so you only need one table.  The acid test is whether the
  tuples you thought were in the table are in fact in the table, and
  the tuples that should not be in the table are truly no longer in
  the table. 

  So we can start with a test set of tuples, which can be indexed,
  identified as such, and have the right mix of field values and/or
  keys. 

  What kind of properties do we want to capture about the table? 

  First: tuples inserted can be looked up.  Tuples not inserted cannot
  be looked up. 

*/

#include "table.h"
#include "tuple.h"
#include "val_int32.h"

#include <iostream>

#define N_TPLS 100
static TuplePtr tpls[ N_TPLS ];

#define FAIL std::cerr << __FILE__":" << __LINE__ << ": failed test: "

void aggListener(TuplePtr t)
{
  std::cout << "Agg update: " << t->toString() << "\n";
}


int main(int argc, char **argv)
{
  std::cout << "TABLES\n";

  // Create our test set of tuples
  for( int i=0; i < N_TPLS; i++) {
    TuplePtr t = Tuple::mk();
    t->append(Val_Int32::mk(i));
    t->append(Val_Int32::mk(i/2));
    t->append(Val_Int32::mk(i/4));
    t->append(Val_Int32::mk(i % (N_TPLS/2)));
    t->append(Val_Int32::mk(i % (N_TPLS/4)));
    t->freeze();
    tpls[i] = t;
  }

  //  First: tuples inserted can be looked up.  Tuples not inserted cannot
  // be looked up.  Create a very simple table
  std::cout << "Testing simple presence/absence...\n";
  TablePtr tbl(new Table("test_table", 200));
  tbl->add_unique_index(0);
  for( int i=0; i < N_TPLS/2; i++) { 
    tbl->insert(tpls[i]);
  }
  for( int i=0; i< N_TPLS/2; i++) { 
    if (tbl->lookup(0,Val_Int32::mk(i))->done()) {
      FAIL << "tuple " << i << " doesn't seem to be in the table\n";
    }
    if (!tbl->lookup(0,Val_Int32::mk(i + N_TPLS/2))->done()) {
      FAIL << "tuple " << i << " seems to be in the table after all\n";
    }
  }

  // Check multi indices
  std::cout << "Testing multiple indices \n";
  tbl.reset(new Table("test_table", 200));
  tbl->add_multiple_index(4);
  for( int i=0; i < N_TPLS; i++) { 
    tbl->insert(tpls[i]);
  }
  // Now every key we find in the first quarter must have four distinct
  // instances in the index
  for( int i=0; i < N_TPLS / 4; i++) { 
    Table::MultIterator iter = tbl->lookupAll(4, Val_Int32::mk(i));
    for (int counter = 0;
         counter < 4;
         counter++) {
      TuplePtr result = iter->next();
      if (result == NULL) {
        FAIL << "key " << i << " seems to have too few tuples in the mult index\n";
      }
    }
    if (iter->next() != NULL) {
      FAIL << "key " << i << " seems to have too many tuples in the mult index\n";
    }
  }

  // Now check aggregates
  std::cout << "Testing group-by-aggregates\n";
  tbl.reset(new Table("test_table", 4));
  tbl->add_multiple_index(4);
  // My group-by fields are 4
  std::vector< unsigned > groupBy;
  groupBy.push_back(4);
  Table::MultAggregate u =
    tbl->add_mult_groupBy_agg(4, groupBy, 3, &Table::AGG_MAX);
  u->addListener(&aggListener);
  for( int i=0; i < N_TPLS; i++) { 
    std::cout << "Inserting " << tpls[i]->toString() << "\n";
    tbl->insert(tpls[i]);
  }
  

  return 0;
}
  

/*
 * End of file 
 */
