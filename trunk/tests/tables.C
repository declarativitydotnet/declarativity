/*
 * @(#)$Id$
 *
 * Copyright (c) 2005 Intel Corporation. All rights reserved.
 *
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
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

int main(int argc, char **argv)
{
  std::cout << "TABLES\n";

  // Create our test set of tuples
  for( int i=0; i < N_TPLS; i++) {
    TupleRef t = Tuple::mk();
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
  TableRef tbl = New refcounted<Table>("test_table", 200);
  tbl->add_unique_index(0);
  for( int i=0; i < N_TPLS/2; i++) { 
    tbl->insert(tpls[i]);
  }
  for( int i=0; i< N_TPLS/2; i++) { 
    if (tbl->lookup(0,Val_Int32::mk(i)) == NULL) {
      FAIL << "tuple " << i << " doesn't seem to be in the table\n";
    }
    if (tbl->lookup(0,Val_Int32::mk(i + N_TPLS/2)) != NULL) {
      FAIL << "tuple " << i << " seems to be in the table after all\n";
    }
  }

  return 0;
}
  

/*
 * End of file 
 */
