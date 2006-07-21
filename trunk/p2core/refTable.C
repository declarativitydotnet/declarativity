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
 * IMPLEMENTATION: Each tuple is contained within a wrapper entry, which
 * includes the tuple itself and its insertion counter.  Tuple entries
 * are indexed by the primary key in a sorted container.  Secondary
 * indices also point to the tuple.  Since tuples are immutable, there
 * are no update consistency issues here.
 */

#include "refTable.h"

////////////////////////////////////////////////////////////
// Constructors
////////////////////////////////////////////////////////////


RefTable::RefTable(std::string tableName,
                   Key& key)
  : CommonTable(tableName, key)
{
}


/** Empty out all indices and kill their heap-allocated components */
RefTable::~RefTable()
{
  //  std::cout << "Destroying table " << _name << "\n";

  ////////////////////////////////////////////////////////////
  // Secondary indices
  SecondaryIndexIndex::iterator iter =
    _indices.begin();
  while (iter != _indices.end()) {
    // Kill the index
    SecondaryIndex* index = (*iter).second;
    delete index;

    // Kill the entry in the index index
    _indices.erase(iter++);
  }

  // Kill all secondary index keyed comparators
  while (_keyedComparators.size() > 0) {
    KeyedEntryComparator* last = _keyedComparators.back();
    delete last;
    _keyedComparators.pop_back();
  }

  // Kill all aggregate objects
  while (_aggregates.size() > 0) {
    AggregateObj* o = _aggregates.back();
    _aggregates.pop_back();

    delete o;
  }


  // And all listeners
  _updateListeners.clear();
  _removalListeners.clear();
  

  // Now empty out actual entries.
  while (_primaryIndex.size() > 0) {
    // Fetch the first entry
    Entry* toKill = *(_primaryIndex.begin());
    
    // erase it from the index
    _primaryIndex.erase(toKill);
    
    // And delete it from the heap
    delete toKill;
  }

  ////////////////////////////////////////////////////////////
  // Primary index
  _primaryIndex.clear();
  

  //  std::cout << "Destroyed table " << _name << "\n";
}




////////////////////////////////////////////////////////////
// Insert tuples
////////////////////////////////////////////////////////////

bool
RefTable::insert(TuplePtr t)
{
  // Find tuple with same primary key
  static Entry searchEntry(Tuple::EMPTY);
  searchEntry.tuple = t;
  PrimaryIndex::iterator found = _primaryIndex.find(&searchEntry);

  // If no tuple exists with same primary key
  if (found == _primaryIndex.end()) {
    // No need to do anything. We'll insert the new tuple below.
  }
  // Otherwise, tuple with same primary key exists
  else {
    // Is it identical to given tuple?
    if ((*found)->tuple->compareTo(t) == 0) {
      // Yes. We won't be replacing the tuple already there. Just
      // increment it's counter
      (*found)->refCount++;

      return false;
    }
    // Otherwise, tuple has same primary key but is different
    else {
      PrimaryIndex::iterator toErase = found;
      found++;
      
      // We will replace the existing tuple, so remove it first
      removeTuple(toErase);
    }
  }

  // We've established that by now no tuple with same primary key
  // exists in the table. Insert the new one and return true.
  insertTuple(t, found);
  
  // Return true
  return true;
}




////////////////////////////////////////////////////////////
// Remove tuples
////////////////////////////////////////////////////////////

bool
RefTable::remove(TuplePtr t)
{
  // Find tuple with same primary key
  static Entry searchEntry(Tuple::EMPTY);
  searchEntry.tuple = t;
  PrimaryIndex::iterator found = _primaryIndex.find(&searchEntry);

  // If no tuple exists with same primary key
  if (found == _primaryIndex.end()) {
    // No need to do anything. There's nothing to remove
    return false;
  }
  // Otherwise, tuple with same primary key exists
  else {
    // Decrement its counter
    (*found)->refCount--;

    // If the counter has reached 0, get rid of it
    if ((*found)->refCount == 0) {
      // Get rid of it
      removeTuple(found);
      return true;              // we actually removed something
    } else {
      // We won't remove it this time.
      return false;
    }
  }
}




////////////////////////////////////////////////////////////
// Convenience lookups
////////////////////////////////////////////////////////////

/** To insert the tuple, create a fresh new entry and insert into the
    primary set, all secondaries, and all aggregates. */
void
RefTable::insertTuple(TuplePtr t,
                      PrimaryIndex::iterator position)
{
  // Create a fresh new entry
  Entry* newEntry = new Entry(t);
  newEntry->refCount++;

  CommonTable::commonInsertTuple(newEntry, t, position);
}


void
RefTable::removeTuple(PrimaryIndex::iterator position)
{
  CommonTable::removeTuple(position);

  // I have to delete the entry now.
  Entry* toKill = *position;
  _primaryIndex.erase(position);
  delete toKill;
}





