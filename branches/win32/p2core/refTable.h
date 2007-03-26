/*
 * @(#)$Id$
 *
 * Copyright (c) 2005 Intel Corporation. All rights reserved.
 *
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * Or
 * UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
 * Berkeley, CA,  94707. Attention: P2 Group.
 * 
 * DESCRIPTION: Counting version of the P2 table class.  Its differences
 * from the Table2 class are the following
 *
 * * Tuples do not expire, either due to maximum table size or due to
 *   maximum tuple lifetime.
 *
 * * Reinsertions of a tuple into the table cause a counter associated
 *   with that tuple to be incremented.  Removals of a tuple from the
 *   table cause that counter to be decremented. A tuple is only removed
 *   when its counter reaches 0.
 *
 * * Update and removed events are only issued when a tuple is first
 *   inserted into the table or removed from the table, not when only
 *   the counter changes.
 *
 * * No refresh events are issued ever.
 *
 * * There is no flushing queue, since there are no expirations.
 *
 */

#ifndef __REFTABLE_H__
#define __REFTABLE_H__

#include "value.h"
#include "tuple.h"
#include <set>
#include <deque>
#include "boost/date_time/posix_time/posix_time.hpp"
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include "commonTable.h"


class RefTable : public CommonTable {
public:
  ////////////////////////////////////////////////////////////
  // Constructors
  ////////////////////////////////////////////////////////////
  
  /** Create a new reference counted table.  name is an identifying
      string.  key is a vector containing a sequence of field numbers,
      which make up the primary key of the table; an empty key vector
      means the implicit tuple ID is the primary key.  */
  RefTable(string tableName,
           Key& key);

  
  /** A destructor. It empties out the table and then destroys it. */
  ~RefTable();



  
public:
  ////////////////////////////////////////////////////////////
  // Updates
  ////////////////////////////////////////////////////////////

  /** Insert a tuple into the table. Return true if the insertion
      modified the table, by growing by a tuple or replacing an existing
      tuple. Return false if the insertion did not affect the table at
      all, e.g., it corresponded to a tuple equal (as per tuple
      comparison) to an existing tuple.  Reinsertion of an existing
      tuple increments the counter of that tuple.  No update is
      generated due to reinsertion.

      The semantics is as follows: find the tuple by the same primary
      key. If it's equal (as per compareTo), increment the counter but
      otherwise do nothing (i.e., update no aggregates or listeners, and
      return false). If it's different, replace it regardless of the
      existing tuple counter and generate removed events as
      appropriate. If it doesn't exist, insert the new one in. */
  bool
  insert(TuplePtr t);

  
  /** Remove a tuple from the table. Return true if the removal modified
      the table by shrinking it by one tuple. Return false if the
      removal did not affect the table, e.g., because the given tuple
      was not there to be removed or because its count was not reduced
      to 0.  Removal happens with regards to the primary key of the
      table.  Therefore, the tuple given the remove method and the tuple
      actually removed from the table may not be the same or equal in
      terms of Tuple::compareTo. */
  bool
  remove(TuplePtr t);




  ////////////////////////////////////////////////////////////
  // Convenience
  ////////////////////////////////////////////////////////////


  /** Insert a brand new tuple into the database including all
      indices. This method *always* causes a new tuple to appear within
      the table and, therefore, always calls any insertion listeners. */
  void
  insertTuple(TuplePtr t,
              PrimaryIndex::iterator position);


  /** Remove an existing tuple (in the primary index position given)
      from the database including all indices. This tuple always causes
      a tuple to be removed from the table and therefore always calls
      any deletion listeners. */
  void
  removeTuple(PrimaryIndex::iterator primaryPosition);
};


/** A pointer to tables */
typedef boost::shared_ptr< RefTable > RefTablePtr;

#endif
