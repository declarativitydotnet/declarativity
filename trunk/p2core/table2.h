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
 * DESCRIPTION: Second-generation in-memory table implementation.
 *
 * It has a single primary key (single or multiple-field index).
 *
 * It may have any number of secondary indices.
 *
 * It has a maximum lifetime of tuples and a maximum size.
 *
 * Insertions of existing tuples are no-ops.
 *
 * Deletions are always with regards to the primary key.
 *
 * Insertions and deletions do not require a copy of fields into/out of
 * tuples but can be performed leaving appropriate fields in place.
 *
 */

#include "tuple.h"

class Table2 {
public:
  ////////////////////////////////////////////////////////////
  // Secondary indices
  ////////////////////////////////////////////////////////////

  /** A comparator of keys */
  std::set::key_compare keyCompare;


  /** A key is a vector of unsigned field numbers */
  typedef std::vector< unsigned > Key;

  /** Create a secondary index on the given sequence of field
      numbers. The sequence must not be empty.  */
  void
  secondaryIndex(Key key);
  
  


  ////////////////////////////////////////////////////////////
  // Constructors
  ////////////////////////////////////////////////////////////

  /**  Create a new table.  name is an identifying string.  key is a
       vector containing a sequence of field numbers, which make up the
       primary key of the table; an empty key vector means the implicit
       record number (a counter) is the primary key.  max_size is how
       many tuples it will hold before discarding (FIFO) and must be
       non-negative.  lifetime is how long to keep tuples for before
       discarding and must be a positive time duration; a lifetime that
       is not a date and time means tuples never expire. */
  Table(string tableName,
        Key key,
        size_t max_size,
        boost::posix_time::time_duration& lifetime);

  /** A convenience constructor that allows the use of string
      representations for maximum tuple lifetime. */
  Table(string tableName,
        Key key,
        size_t max_size,
        string lifetime);
  
  /** A convenience constructor that does not expire tuples. */
  Table(string tableName,
        Key key,
        size_t max_size);
  

  /** A destructor. It empties out the table and then destructs it. */
  ~Table();




  ////////////////////////////////////////////////////////////
  // Metadata checks
  ////////////////////////////////////////////////////////////

  /** Table size. It returns the number of tuples within the table
      (excluding those that may have been logically deleted but not yet
      physically removed). */
  size_t
  size();

  
  

  ////////////////////////////////////////////////////////////
  // Updates
  ////////////////////////////////////////////////////////////

  /** Insert a tuple into the table. Return true if the insertion
      modified the table, by growing by a tuple or replacing an existing
      tuple. Return false if the insertion did not affect the table at
      all, e.g., it corresponded to a tuple equal (as per tuple
      comparison) to an existing tuple. Insertion time, for the purposes
      of tuple expiration, indicates the first time a particular tuple
      was inserted; any attempts to reinsert that tuple do not update
      the original insertion time.

      The semantics is as follows: find the tuple by the same primary
      key. If it's equal (as per compareTo) do nothing. If it's
      different, replace it. If it doesn't exist, insert the new one
      in. */
  bool
  insert(TuplePtr t);

  
  /** Remove a tuple from the table. Return true if the removal modified
      the table by shrinking it by one tuple. Return false if the
      removal did not affect the table, e.g., because the given tuple
      was not there to be removed.  Removal happens with regards to the
      primary key of the table.  Therefore, the tuple given the remove
      method and the tuple actually removed from the table may not be
      the same or equal in terms of Tuple::compareTo. */
  bool
  remove(TuplePtr t);




private:
  /** My name (human readable). */
  string _name;


  /** My primary key. If empty, use the tuple ID. */
  Key _key;


  /** My maximum size in tuples. If 0, size is unlimited. */
  size_t _maximumSize;

  
  /** My maximum lifetime. If not a date or time (as per
      is_not_a_date_time()), lifetime is unlimited. */
  boost::posix_time::time_duration _maximumLifetime;


  /** My secondary indices, indexed by index key.  Recall that index
      keys are represented as sequences of field numbers. */
  std::set< Key, valueRefVectorLess> _indices;
}
