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
 * DESCRIPTION: Simple table implementation: in memory.
 *
 */

#ifndef __TABLE_H__
#define __TABLE_H__

#include <deque>
#include <map>
#include <vector>

#include "tuple.h"

class Table;
typedef ref<Table> TableRef;
typedef ptr<Table> TablePtr;

/*
 OK, what does a table _do_?

1) 0 or more unique field indices.
2) 0 or more multiple-key field indices.
3) max table size.
4) max tuple lifetime.

1) => map(field value -> tuple)
2) => map(field value -> [tuples])
3) => list of tuples in insertion order
4) => list of tuples in insertion order, plus timestamp.

Adding a tuple:
 - locate all tuples with matching unique fields, and delete them
 - add tuple to list head.
 - remove all tuples from list tail.
 - add tuple to each index.

Removing a tuple:
 - remove from the list (random access, by value)
 - remove from each map (random access, by field value)

Removal from maps is therefore _fast_. 

Removal from the list is therefore _slow_ (linear), since we have to
locate the tuple in the list.  And it can't be indexed, since then
we'd have to maintain the index across inserts and deletes. 

We could mark the tuple as erased, twiddle the tuple accounting (for
maximum table size), and lazily delete them as they reach the end.
But this leaves the problem of what to do when new tuples arrive very
quickly.  

*/

class Table {
public:

  // Create a new table.
  //  'name' is an identifying string.
  //  'max_size' is how many tuples it will hold before discarding (FIFO)
  //  'lifetime' is how long to keep tuples for before discarding
  Table(str tableName, size_t max_size, timespec *lifetime=NULL);

  // Creating and removing indices.  
  // Note that an index can only be created on a single field.  
  // Note also that you should not create one of these _after_ any
  // tuple has been inserted, otherwise unpredicatable things will
  // happen (though this limitation is easily fixed and may be in the
  // future). 
  //
  // And yet another caveat: tuples are indexed by calling toString()
  // on their index field.  This will cause trouble with "double"
  // (i.e. floating point) values, since toString() currently formats
  // them with printf("%g"), which rounds and approximates.  The Right
  // Thing To Do is to use Val_Str::cast() instead, and then change
  // its behaviour to use %a as a format specifier, which provides
  // accuracy down to the binary level by formatting the result as hex
  // floating point number.  But this would confuse anyone or anything
  // that currently assumes v.toString() == Val_Str::cast(v). 
  void add_unique_index(unsigned fn);
  void del_unique_index(unsigned fn); 
  void add_multiple_index(unsigned fn);
  void del_multiple_index(unsigned fn);

  // Setting and removing the expiry time
  void set_tuple_lifetime(timespec &lifetime);
  void unset_tuple_lifetime() { expiry_lifetime = false; };

  // Insert a tuple
  void insert(TupleRef t);

  // Lookup a tuple
  TuplePtr lookup(unsigned field, ValueRef key);

  // XXX Need a multiple lookup, return an iterator.

  // How big is the table
  size_t size() { return els.size(); };

  // Lookup based on a field
  
  //private:

  str name;
  size_t	max_tbl_size;
  timespec	max_lifetime;
  bool		expiry_lifetime;

  struct Entry {
    TupleRef t;
    timespec ts;
    Entry(TupleRef tp) : t(tp) { clock_gettime(CLOCK_REALTIME,&ts); };
  };
  std::deque<Entry *> els;

  typedef std::multimap<str, Entry *> MultIndex;
  typedef std::map<str, Entry *>  UniqueIndex;

  std::vector<MultIndex *> mul_indices;
  std::vector<UniqueIndex *> uni_indices;
  
  // Helper function to remove an Entry from all the indices in the system.
  void remove_from_indices(Entry *);
  void garbage_collect();

};

#endif /* __TABLE_H_ */
