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
 * DESCRIPTION: Next generation table implementation.
 *
 *
 * IMPLEMENTATION: Each tuple is contained within a wrapper entry, which
 * includes the tuple itself and its time of insertion.  Tuple entries
 * are indexed by the primary key in a sorted container.  Secondary
 * indices also point to the tuple. Since tuples are immutable, there
 * are no update consistency issues here.  If the table has a finite
 * maximum lifetime, tuples are also maintained in a queue, which helps
 * with flushes.
 */

#include "table2.h"
#include "p2Time.h"
#include "aggFactory.h"
#include "iostream"

////////////////////////////////////////////////////////////
// Constructors
////////////////////////////////////////////////////////////


Table2::Table2(std::string tableName,
               Key& key,
               uint32_t maxSize,
               boost::posix_time::time_duration& lifetime)
  : CommonTable(tableName, key),
    _maxSize(maxSize),
    _maxLifetime(lifetime),
    _flushing(((_maxSize > 0) ||
               (!_maxLifetime.is_pos_infinity())))
{
}


Table2::Table2(std::string tableName,
               Key& key)
  : CommonTable(tableName, key),
    _maxSize(0),
    _maxLifetime(DEFAULT_EXPIRATION),
    _flushing(((_maxSize > 0) ||
               (!_maxLifetime.is_pos_infinity())))
{
}


Table2::Table2(std::string tableName,
               Key& key,
               uint32_t maxSize)
  : CommonTable(tableName, key),
    _maxSize(maxSize),
    _maxLifetime(DEFAULT_EXPIRATION),
    _flushing(((_maxSize > 0) ||
               (!_maxLifetime.is_pos_infinity())))
{
}

Table2::Table2(std::string tableName,
               Key& key,
               uint32_t maxSize,
               string lifetime)
  : CommonTable(tableName, key),
    _maxSize(maxSize),
    _maxLifetime(DEFAULT_EXPIRATION),
    _flushing(((_maxSize > 0) ||
               (!_maxLifetime.is_pos_infinity())))
{
  if (lifetime == std::string("+infinity")) {
    _maxLifetime = boost::posix_time::time_duration(boost::date_time::pos_infin);
  }
  else if (lifetime == std::string("-infinity")) {
    _maxLifetime = boost::posix_time::time_duration(boost::date_time::neg_infin);
  }
  else {
    _maxLifetime = boost::posix_time::duration_from_string(lifetime);
  }
}




////////////////////////////////////////////////////////////
// Destructor
////////////////////////////////////////////////////////////

/** Empty out all indices and kill their heap-allocated components */
Table2::~Table2()
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
  

  // Now empty out actual entries.  If flushing, flush the
  // queue. Otherwise, go over the primary index and delete the entries.
  if (_flushing) {
    while (_queue.size() > 0) {
      Entry* toKill = _queue.back();
      _queue.pop_back();
      if (toKill->refCount > 0) {
        toKill->refCount--;
      } else {
        delete toKill;
      }
    }
  } else {
    while (_primaryIndex.size() > 0) {
      // Fetch the first entry
      Entry* toKill = *(_primaryIndex.begin());
      
      // erase it from the index
      _primaryIndex.erase(toKill);
      
      // And delete it from the heap
      delete toKill;
    }
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
Table2::insert(TuplePtr t)
{
/*
  boost::posix_time::ptime now_ts;
  getTime(now_ts);
  std::cerr << "INSERT TUPLE current time = " << boost::posix_time::to_simple_string(now_ts)
            << ", TUPLE = " << t->toString() << std::endl;
*/
  // Ensure we're operating on the correct view of the table as of right
  // now.
  flush();

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
      // Yes. We won't be replacing the tuple already there

      // Update insertion time if we're flushing tuples
      if (_flushing) {
/*
        std::cerr << "UPDATING TUPLE TIME" << std::endl; 
        std::cerr << "\tOLD: " << boost::posix_time::to_simple_string((*found)->time) << std::endl;
*/
        updateTime(*found);
        // std::cerr << "\tNEW: " << boost::posix_time::to_simple_string((*found)->time) << std::endl;
      }
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
  
  // Ensure we're still compliant
  flush();

  // Return true
  return true;
}




////////////////////////////////////////////////////////////
// Remove tuples
////////////////////////////////////////////////////////////

bool
Table2::remove(TuplePtr t)
{
  // Ensure we're operating on the correct view of the table as of right
  // now.
  flush();

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
    // Get rid of it, even if it's not identical
    removeTuple(found);
    
    // We haven't added new tuples so there's no reason to flush again.
    return true;
  }
}




////////////////////////////////////////////////////////////
// Convenience lookups
////////////////////////////////////////////////////////////

/** To insert the tuple, create a fresh new entry, assign the current
    timestamp, and insert into the primary set, all secondaries, and all
    aggregates. */
void
Table2::insertTuple(TuplePtr t,
                    PrimaryIndex::iterator position)
{
  // Create a fresh new entry
  Entry* newEntry = new Entry(t);

  CommonTable::commonInsertTuple(newEntry, t, position);

  // Insert into time-ordered queue if we're flushing.
  if (_flushing) {
    _queue.push_front(newEntry);
  }
}


void
Table2::flushEntry(Entry* e)
{
  // Is this entry the last instance in the queue?
  if (e->refCount > 0) {
    // Nope, it is not. Decrement the ref count, pop the entry, and be
    // gone 
    e->refCount--;
    _queue.pop_back();
  } else {
    // Ah! This is the last instance of this entry in the queue, so
    // we're doing a real removal from the table.
/*
    boost::posix_time::ptime now_ts;
    getTime(now_ts);
    std::cerr << "FLUSHING TUPLE: current time = " 
              << boost::posix_time::to_simple_string(now_ts)
              << ", tuple time = " << boost::posix_time::to_simple_string(e->time)
       << ", " << e->tuple->toString() << std::endl;
*/

    // Remove it from all derivatives (secondaries, aggs)
    removeDerivatives(e->tuple);

    // Primary index
    static Entry searchEntry(Tuple::EMPTY);
    searchEntry.tuple = e->tuple;
    _primaryIndex.erase(&searchEntry);
 
    // Pop the tuple from the queue
    _queue.pop_back();
    
    // And delete the entry from the heap
    delete e;
  }
}


void
Table2::removeTuple(PrimaryIndex::iterator position)
{
  CommonTable::removeTuple(position);

  // If I have a queue, then I don't need to delete the entry since it
  // will be flushed sooner or later. Otherwise, I have to delete the
  // entry now.
  if (_flushing) {
    _primaryIndex.erase(position);
  } else {
    Entry* toKill = *position;
    _primaryIndex.erase(position);
    delete toKill;
  }
}


/** Increment the copy count of the entry by one, and reinsert it at the
    beginning of the queue with the current time.  This method is only
    called if a queue is being maintained. */
void
Table2::updateTime(Entry* e)
{
  e->refCount++;
  getTime(e->time);
  _queue.push_front(e);
}


void
Table2::flush()
{
  // If we're not flushing, do nothing
  if (!_flushing) {
    return;
  }

  // Start from the bottom of the queue and keep removing until the
  // first non-expired entry is found, if tuples do expire.
  if (!_maxLifetime.is_pos_infinity()) {
    // We do have a queue
    boost::posix_time::ptime now;
    getTime(now);
    
    boost::posix_time::ptime expiryTime;
    expiryTime = now - _maxLifetime;

    while (_queue.size() > 0) {
      // Check this element's insertion time
      Entry * last = _queue.back();
      if (last->time < expiryTime) {
        // Flush the entry from the table
        flushEntry(last);
      } else {
        // We've found the first non-expiring entry. Stop the
        // iteration. 
        break;
      }
    }
  }

  // Start from the bottom of the queue and keep removing until the size
  // of the primary index is the maximum table size, if there is a
  // maximum table size
  if (_maxSize > 0) {
    while ((_queue.size() > 0) &&
           (_primaryIndex.size() > _maxSize)) {
      Entry * last = _queue.back();

      // Remove the entry from the table
      flushEntry(last);
    }
  }
}




////////////////////////////////////////////////////////////
// Lookups
////////////////////////////////////////////////////////////

Table2::Iterator
Table2::lookup(Table2::Key& lookupKey,
               Table2::Key& indexKey,
               TuplePtr t)  
{
  // Ensure we're operating on the correct view of the table as of right
  // now.
  flush();

  return CommonTable::lookup(lookupKey, indexKey, t);
} 


Table2::Iterator
Table2::lookup(Table2::Key& indexKey,
               TuplePtr t)  
{
  // Ensure we're operating on the correct view of the table as of right
  // now.
  flush();

  return CommonTable::lookup(indexKey, t);
} 




////////////////////////////////////////////////////////////
// Scans
////////////////////////////////////////////////////////////

Table2::Iterator
Table2::scan(Table2::Key& key)  
{
  // Ensure we're operating on the correct view of the table as of right
  // now.
  flush();

  return CommonTable::scan(key);
} 


Table2::Iterator
Table2::scan()  
{
  // Ensure we're operating on the correct view of the table as of right
  // now.
  flush();

  return CommonTable::scan();
} 




////////////////////////////////////////////////////////////
// Constants
////////////////////////////////////////////////////////////


/** No expiration is represented as positive infinity. */
boost::posix_time::time_duration
Table2::NO_EXPIRATION(boost::date_time::pos_infin);


/** Default table expiration time is no expiration */
boost::posix_time::time_duration
Table2::DEFAULT_EXPIRATION(Table2::NO_EXPIRATION);


/** No size is represented as 0 size. */
uint32_t
Table2::NO_SIZE = 0;


/** Default table max size is no max size */
uint32_t
Table2::DEFAULT_SIZE(Table2::NO_SIZE);
