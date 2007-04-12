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
 * DESCRIPTION: In addition to the common table spec, this allows
 * limitations of the lifetime of tuples and of the maximum size of the
 * table.
 *
 * Insertions of existing tuples change no content, but do update the
 * insertion time of the existing tuple.
 */

#ifndef __TABLE2_H__
#define __TABLE2_H__

#include "commonTable.h"
#include "value.h"
#include "tuple.h"
#include <set>
#include "boost/date_time/posix_time/posix_time.hpp"
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include <deque>


class Table2 : public CommonTable {
public:
  ////////////////////////////////////////////////////////////
  // Special constants
  ////////////////////////////////////////////////////////////

  /** The non-expiring expiration time */
  static boost::posix_time::time_duration NO_EXPIRATION;


  /** The default table expiration time */
  static boost::posix_time::time_duration DEFAULT_EXPIRATION;


  /** The non-size-limited size */
  static uint32_t NO_SIZE;


  /** The default table size */
  static uint32_t DEFAULT_SIZE;




public:
  ////////////////////////////////////////////////////////////
  // Constructors
  ////////////////////////////////////////////////////////////

  /**  Create a new table.  name is an identifying string.  key is a
       vector containing a sequence of field numbers, which make up the
       primary key of the table; an empty key vector means the implicit
       tuple ID is the primary key.  maxSize is how many tuples it will
       hold before discarding (FIFO) and must be non-negative.  Max size
       of 0 means unlimited table size.  lifetime is how long to keep
       tuples for before discarding and must be a positive time
       duration; a lifetime may be positive infinite, indicating no
       expiration. */
  Table2(string tableName,
         Key& key,
         uint32_t maxSize,
         boost::posix_time::time_duration& lifetime);

  /** A convenience constructor that allows the use of string
      representations for maximum tuple lifetime. */
  Table2(string tableName,
         Key& key,
         uint32_t maxSize,
         string lifetime);
  

  /** A convenience constructor that does not expire tuples. */
  Table2(string tableName,
         Key& key,
         uint32_t maxSize);
  

  /** A convenience constructor with no size or time limits. */
  Table2(string tableName,
         Key& key);
  
  
  /** A destructor. It empties out the table and then destroys it. */
  ~Table2();




public:
  ////////////////////////////////////////////////////////////
  // Lookups
  ////////////////////////////////////////////////////////////

  /** Looks up tuple t in the index defined by indexKey.  If no such
      index exists, a null result is returned.  The lookup finds all
      elements in the table (searched in the order of the given index)
      whose index field values match the values of t on the lookup field
      values.  For example, lookup(<1, 3>, <2, 4>, t) searches the index
      on key <2, 4>, for all tuples s such that s[2] == t[1] and s[4] ==
      t[3].  The lookup and index keys are assumed to have the same
      size.  If the lookup and index keys are known to be identical,
      lookup(Key, TuplePtr) should be used instead for peformance
      reasons.  */
  Iterator
  lookup(Key& lookupKey, Key& indexKey, TuplePtr t);


  /** Looks up tuple t in the index defined by indexKey.  If no such
      index exists, a null result is returned.  The lookup finds all
      elements in the table (searched in the order of the given index)
      whose index field values match the values of t on the same fields.
      For example, lookup(<1, 3>, t) searches the index on key <1, 3>,
      for all tuples s such that s[1] == t[1] and s[3] == t[3].  This is
      equivalent to lookup(indexKey, indexKey, t), skipping the
      projection implied in the more complex method, however it is
      slightly faster than that method since it does not perform the
      projection. */
  Iterator
  lookup(Key& indexKey, TuplePtr t);


  /** Returns a pointer to a lookup iterator on all elements ordered by
      the given index.  If no such index exists, a null pointer is
      returned.  */
  Iterator
  scan(Key& key);


  /** Returns a pointer to a lookup iterator on all elements ordered by
      the primary index.*/
  Iterator
  scan();
  




  ////////////////////////////////////////////////////////////
  // Updates
  ////////////////////////////////////////////////////////////

  /** Insert a tuple into the table. Return true if the insertion
      modified the table, by growing by a tuple or replacing an existing
      tuple. Return false if the insertion did not affect the table at
      all, e.g., it corresponded to a tuple equal (as per tuple
      comparison) to an existing tuple. However, reinsertion of an
      existing tuple does update the insertion time of that tuple with
      the time of reinsertion.  No update is generated due to insertion
      time updates though.

      The semantics is as follows: find the tuple by the same primary
      key. If it's equal (as per compareTo), update the insertion
      time but otherwise do nothing (i.e., update no aggregates or
      listeners, and return false). If it's different, replace it. If it
      doesn't exist, insert the new one in. */
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
  /** My maximum size in tuples. If 0, size is unlimited. */
  uint32_t _maxSize;

  
  /** My maximum lifetime. It must be positive, and may be positive
      infinity, meaning table entries do not expire. */
  boost::posix_time::time_duration _maxLifetime;


  /** The time-order queue of entries, for fast garbage collection. This
      is only initialized if the table has a finite expiration time.  A
      given entry may appear multiple times in the queue, once for every
      reinsertion of the tuple.  Specifically, if an existing tuple is
      inserted again, the entry containing that tuple is reinserted in
      the beginning of this queue, with its time updated and its
      refcount incremented by one.  Any other existing instances of the
      entry in the queue remained untouched until they appear in the
      back during flushing.  During flushing, if an entry has a 0
      refcount, it is removed from the queue, from the indices, and from
      the memory heap.  If it has a non-zero refcount, its refcount is
      removed, its instance found is flushed from the queue, but the
      entry and its contained tuple are left alone.*/
  std::deque< Entry * > _queue;


  /** Are we garbage collecting?  We are only garbage collecting if
      entries can be auto-flushed, either due to expiration or due to
      table size limitations. */
  bool _flushing;




  ////////////////////////////////////////////////////////////
  // Queue management
  ////////////////////////////////////////////////////////////

  /** Flush expired or supernumerary entries */
  void
  flush();




  ////////////////////////////////////////////////////////////
  // Convenience Functions
  ////////////////////////////////////////////////////////////

  /** Flush an entry with an existing tuple from the table and all
      indices. */
  void
  flushEntry(Entry* e);

  
  /** Updates the insertion time of an existing tuple in the table. */
  void
  updateTime(Entry* e);


  /** Remove an existing tuple (in the primary index position given)
      from the database including all indices. This tuple always causes
      a tuple to be removed from the table and therefore always calls
      any deletion listeners. */
  void
  removeTuple(PrimaryIndex::iterator primaryPosition);


  /** Insert a brand new tuple into the database including all
      indices. This method *always* causes a new tuple to appear within
      the table and, therefore, always calls any insertion listeners. */
  void
  insertTuple(TuplePtr t,
              PrimaryIndex::iterator position);
};


/** A pointer to tables */
typedef boost::shared_ptr< Table2 > Table2Ptr;

#endif
