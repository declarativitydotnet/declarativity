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

////////////////////////////////////////////////////////////
// Sorters
////////////////////////////////////////////////////////////

/** This comparator compares key specs. */
bool
Table2::KeyComparator::
operator()(const Table2::Key* first,
           const Table2::Key* second) const
{
  if (first->size() < second->size()) {
    return true; // first < second
  } else if (first->size() > second->size()) {
    return false; // second < first
  } else {
    Table2::Key::const_iterator firstIt = first->begin();
    Table2::Key::const_iterator secondIt = second->begin();
    while (firstIt != first->end()) {
      if ((*firstIt) >= (*secondIt)) {
        // We're done, second >= first
        return false;
      }
    }
    // We're done, first < second
    return true;
  }
}


/** A keyed comparator is only initialized with its key spec */
Table2::KeyedEntryComparator::KeyedEntryComparator(const Key& key)
  : _key(key)
{
}


/** The comparators checks the fields indicated by the key spec in the
    order of the key spec. It compares the fields lexicographically.
    Absence of a field is smaller than existence of a field.  If the key
    spec is empty, then the comparator compares tuple IDs. */
bool
Table2::KeyedEntryComparator::
operator()(const Table2::Entry* fEntry,
           const Table2::Entry* sEntry) const
{
  TuplePtr first = fEntry->tuple;
  TuplePtr second = sEntry->tuple;

  if (_key.empty()) {
    // Compare tuple IDs only
    return (first->ID() < second->ID());
  } else {
    Table2::Key::const_iterator fieldNos = _key.begin();
    while (fieldNos != _key.end()) {
      // The next field number is
      unsigned fieldNo = (*fieldNos);

      // Does the first have this field?
      if (first->size() > fieldNo) {
        // It does. Does the second have this field?
        if (second->size() > fieldNo) {
          // It does. Compare their values.  If the first is not less,
          // return false.
          if ((*first)[fieldNo]->compareTo((*second)[fieldNo]) >= 0) {
            return false;
          } else {
            // The first's field is less that the second's field. Keep
            // checking
          }
        } else {
          // The second vector is lacking the field but the first one has
          // it. The first vector is not less.
          return false;
        }
      } else {
        // The first vector is lacking the field.  So far, all fields
        // compared make the first vector less. Whether or not the second
        // vector has this field, this comparison result won't change. So
        // just return true.
        return true;
      }
    }
    // If we got to this point, all key fields of the first vector are
    // less than the key fields of the second vector, so return true.
    return true;
  }
}




////////////////////////////////////////////////////////////
// Constructors
////////////////////////////////////////////////////////////


Table2::Table2(std::string tableName,
               Key& key,
               size_t maxSize,
               boost::posix_time::time_duration& lifetime)
  : _name(tableName),
    _key(key),
    _maxSize(maxSize),
    _maxLifetime(lifetime),
    _primaryIndex(KeyedEntryComparator(key)),
    _flushing(((_maxSize > 0) ||
               (!_maxLifetime.is_pos_infinity())))
{
}


Table2::Table2(std::string tableName,
               Key& key)
  : _name(tableName),
    _key(key),
    _maxSize(0),
    _maxLifetime(boost::posix_time::time_duration(boost::date_time::pos_infin)),
    _primaryIndex(KeyedEntryComparator(key)),
    _flushing(((_maxSize > 0) ||
               (!_maxLifetime.is_pos_infinity())))
{
}


Table2::Table2(std::string tableName,
               Key& key,
               size_t maxSize)
  : _name(tableName),
    _key(key),
    _maxSize(maxSize),
    _maxLifetime(boost::posix_time::time_duration(boost::date_time::pos_infin)),
    _primaryIndex(KeyedEntryComparator(key)),
    _flushing(((_maxSize > 0) ||
               (!_maxLifetime.is_pos_infinity())))
{
}




////////////////////////////////////////////////////////////
// Destructor
////////////////////////////////////////////////////////////

/** Empty out all indices and kill their heap-allocated components */
Table2::~Table2()
{
  ////////////////////////////////////////////////////////////
  // Secondary indices
  SecondaryIndexIndex::iterator iter =
    _indices.begin();
  while (iter != _indices.end()) {
    // Kill the index
    delete (*iter).second;

    // Kill the entry in the index index
    _indices.erase(iter);
  }

  // Kill all secondary index keyed comparators
  while (_keyedComparators.size() > 0) {
    KeyedEntryComparator* last = _keyedComparators.back();
    delete last;
    _keyedComparators.pop_back();
  }
}




////////////////////////////////////////////////////////////
// Secondary Indices
////////////////////////////////////////////////////////////

bool
Table2::secondaryIndex(Table2::Key& key)
{
  // Is there such an index already?
  SecondaryIndexIndex::iterator iter =
    _indices.find(&key);

  if (iter == _indices.end()) {
    // If not, create it, update with all existing elements (from the
    // primary index), and return true
    KeyedEntryComparator* comp = new KeyedEntryComparator(key);
    _keyedComparators.push_front(comp);

    SecondaryIndex* index = new SecondaryIndex(*comp);
    _indices.insert(std::make_pair(&key, index));

    return true;
  } else {
    // If so, return false
    return false;
  }
}




////////////////////////////////////////////////////////////
// Insert tuples
////////////////////////////////////////////////////////////

bool
Table2::insert(TuplePtr t)
{
  // Ensure we're operating on the correct view of the table as of right
  // now.
  flush();

  // Find tuple with same primary key
  _searchEntry.tuple = t;
  PrimaryIndex::iterator found = _primaryIndex.find(&_searchEntry);

  // If no tuple exists with same primary key
  if (found == _primaryIndex.end()) {
    // No need to do anything. We'll insert the new tuple below.
  }
  // Otherwise, tuple with same primary key exists
  else {
    // Is it identical to given tuple?
    if ((*found)->tuple->compareTo(t)) {
      // Nope, it's different.  We won't be replacing the tuple already
      // there
      return false;
    }
    // Otherwise, tuple has same primary key but is different
    else {
      // Remove existing tuple
      removeTuple(t, found);
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
// Tuple wrapper
////////////////////////////////////////////////////////////

Table2::Entry::Entry(TuplePtr tp)
  : tuple(tp)
{
  getTime(time);
}




////////////////////////////////////////////////////////////
// Convenience lookups
////////////////////////////////////////////////////////////

/** Set up the static search entry */
Table2::Entry
Table2::_searchEntry(Tuple::EMPTY);


/** To insert the tuple, create a fresh new entry, assign the current
    timestamp, and insert into the primary set, all secondaries, and all
    aggregates. */
void
Table2::insertTuple(TuplePtr t,
                    PrimaryIndex::iterator position)
{
  // Create a fresh new entry
  Entry* newEntry = new Entry(t);

  // Primary index.
  _primaryIndex.insert(position, newEntry);
  
  // Secondary indices. For all indices, insert.
  SecondaryIndexIndex::iterator iter = _indices.begin();
  while (iter != _indices.end()) {
    (*iter).second->insert(newEntry);
  }

  // Insert into time-ordered queue if we're flushing.
  if (_flushing) {
    _queue.push_front(newEntry);
  }
  
  // Aggregates. For all aggregates, insert.
}


void
Table2::flushTuple(TuplePtr t)
{
  // Primary index
  _searchEntry.tuple = t;

  // Just remove it from all indices.  The entry will be killed in
  // flush()
  _primaryIndex.erase(&_searchEntry);

  // Secondary indices
  SecondaryIndexIndex::iterator iter = _indices.begin();
  while (iter != _indices.end()) {
    (*iter).second->erase(&_searchEntry);
    iter++;
  }
  
  // Aggregates
}


void
Table2::removeTuple(TuplePtr t,
                    PrimaryIndex::iterator position)
{
  // If I have a queue, then I don't need to delete the entry since it
  // will be flushed sooner or later. Otherwise, I have to delete the
  // entry now.
  if (_flushing) {
    _primaryIndex.erase(position);
  } else {
    delete (*position);
    _primaryIndex.erase(position);
  }

  // Secondary indices
  _searchEntry.tuple = t;
  SecondaryIndexIndex::iterator iter = _indices.begin();
  while (iter != _indices.end()) {
    (*iter).second->erase(&_searchEntry);
    iter++;
  }

  // Aggregates
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
        // Remove the contained tuple from the table
        flushTuple(last->tuple);

        // Remove the entry from the queue
        _queue.pop_back();

        // Eliminate the entry
        delete last;
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

      // Remove the contained tuple from the table
      flushTuple(last->tuple);
      
      // Remove the entry from the queue
      _queue.pop_back();
      
      // Eliminate the entry
      delete last;
    }
  }
}


Table2::Key Table2::KEYID = Table2::Key();
Table2::Key Table2::KEY0 = Table2::Key();
Table2::Key Table2::KEY1 = Table2::Key();
Table2::Key Table2::KEY2 = Table2::Key();
Table2::Key Table2::KEY3 = Table2::Key();
Table2::Key Table2::KEY01 = Table2::Key();
Table2::Key Table2::KEY12 = Table2::Key();
Table2::Key Table2::KEY23 = Table2::Key();
Table2::Key Table2::KEY13 = Table2::Key();
Table2::Key Table2::KEY012 = Table2::Key();
Table2::Key Table2::KEY123 = Table2::Key();


Table2::Initializer::Initializer()
{
  // No need to initialize KEYID. It's already empty.
  Table2::KEY0.push_back(0);

  Table2::KEY1.push_back(1);

  Table2::KEY2.push_back(2);

  Table2::KEY3.push_back(3);

  Table2::KEY01.push_back(0);
  Table2::KEY01.push_back(1);

  Table2::KEY12.push_back(1);
  Table2::KEY12.push_back(2);

  Table2::KEY23.push_back(2);
  Table2::KEY23.push_back(3);

  Table2::KEY13.push_back(1);
  Table2::KEY13.push_back(3);

  Table2::KEY012.push_back(0);
  Table2::KEY012.push_back(1);
  Table2::KEY012.push_back(2);

  Table2::KEY123.push_back(1);
  Table2::KEY123.push_back(2);
  Table2::KEY123.push_back(3);
}

/** Run the static initialization */
Table2::Initializer
Table2::_INITIALIZER;



////////////////////////////////////////////////////////////
// Lookup Iterators
////////////////////////////////////////////////////////////

/** An iterator contains only a queue of tuple pointers, which it
    dispenses when asked. XXX Replace by just returning the queue to the
    caller. */
Table2::Iterator::Iterator(std::deque< TuplePtr >* spool)
  : _spool(spool)
{
}


TuplePtr
Table2::Iterator::next()
{
  if (_spool->size() > 0) {
    TuplePtr back = _spool->back();
    _spool->pop_back();
    return back;
  } else {
    // We've run out of elements, period.
    return TuplePtr();
  }
}


bool
Table2::Iterator::done()
{
  return (_spool->size() > 0);
}


Table2::Iterator::~Iterator()
{
  // Delete the queue. Its contents are shared pointers to tuples, so no
  // need to worry about them
  delete _spool;
}


size_t
Table2::size() const
{
  size_t s = _primaryIndex.size();
  return s;
}




////////////////////////////////////////////////////////////
// Lookups
////////////////////////////////////////////////////////////

Table2::IteratorPtr
Table2::lookup(Table2::Key& key, TuplePtr t)  
{
  // Ensure we're operating on the correct view of the table as of right
  // now.
  flush();

  // Find the appropriate index. Is it the primary index?
  if (key == _key) {
    // Create the lookup iterator from all the matches and return it
    _searchEntry.tuple = t;
    PrimaryIndex::iterator tupleIter = _primaryIndex.find(&_searchEntry);

    // Create the lookup iterator by spooling all results found into a
    // queue and passing them into the iterator.
    std::deque< TuplePtr >* spool = new std::deque< TuplePtr >();
    while (tupleIter != _primaryIndex.end()) {
      spool->push_front((*(tupleIter++))->tuple);
    }
    IteratorPtr iterPtr = IteratorPtr(new Iterator(spool));
    return iterPtr;
  } 
  // If not, is it a secondary index?
  else {
    Table2::SecondaryIndexIndex::iterator indexIter =
      _indices.find(&key);

    // If not, return null
    if (indexIter == _indices.end()) {
      return IteratorPtr();
    }
    // Otherwise, create the iterator from all the matches and return it
    else {    
      SecondaryIndex& index = *(*indexIter).second;

      _searchEntry.tuple = t;
      SecondaryIndex::iterator tupleIter = index.find(&_searchEntry);

      // Create the lookup iterator by spooling all results found into a
      // queue and passing them into the iterator.
      std::deque< TuplePtr >* spool = new std::deque< TuplePtr >();
      while (tupleIter != _primaryIndex.end()) {
        spool->push_front((*(tupleIter++))->tuple);
      }
      IteratorPtr iterPtr = IteratorPtr(new Iterator(spool));
      return iterPtr;
    }
  }
} 

