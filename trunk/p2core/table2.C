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
// Sorters
////////////////////////////////////////////////////////////

/** This comparator performs first less second for two key specs. */
bool
Table2::KeyComparator::
operator()(const Table2::Key& first,
           const Table2::Key& second) const
{
  if (first.size() < second.size()) {
    return true; // first < second
  } else if (first.size() > second.size()) {
    return false; // second < first
  } else {
    // As long as they're equal, keep checking.
    Table2::Key::const_iterator firstIt = first.begin();
    Table2::Key::const_iterator secondIt = second.begin();
    while (firstIt != first.end()) {
      unsigned f = *firstIt++;
      unsigned s = *secondIt++;
      if (f < s) {
        return true;
      } else if (f > s) {
        return false;
      } else {
        // They're equal so far so I have to keep checking
      }
    }
    // They're equal throughout, so the first is not less
    return false;
  }
}


/** A keyed comparator is only initialized with its key spec */
Table2::KeyedEntryComparator::KeyedEntryComparator(const Key key)
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
  TuplePtr first(fEntry->tuple);
  TuplePtr second(sEntry->tuple);

  if (_key.empty()) {
    // Compare tuple IDs only
    return (first->ID() < second->ID());
  } else {
    Table2::Key::const_iterator fieldNos = _key.begin();
    while (fieldNos != _key.end()) {
      // The next field number is
      unsigned fieldNo = *(fieldNos++);

      // Does the first have this field?
      if (first->size() > fieldNo) {
        // It does. Does the second have this field?
        if (second->size() > fieldNo) {
          // It does. Compare their values. If they're equal, I must
          // keep with the next field. If the first is less, stop here
          // and return true. If the first is greater, stop here and
          // return false.
          int comp = (*first)[fieldNo]->compareTo((*second)[fieldNo]);
          if (comp < 0) {
            // First is less on this field, so it is also less overall
            return true;
          } else if (comp > 0) {
            // First is greater on this field, so it cannot be less
            // overall
            return false;
          } else {
            // The fields are equal, so I must check the next field if
            // it exists
          }
        } else {
          // The second vector is lacking the field but the first one has
          // it. The first vector is not less.
          return false;
        }
      } else {
        // The vectors are equal this far, but the first vector is
        // lacking the field. If the second vector has this field, then
        // the first is less. If the second vector does not have this
        // field, then since the two vectors are equal, the first vector
        // is not less.
        if (second->size() > fieldNo) {
          // The second vector has it, so the first is less
          return true;
        } else {
          // The two vectors are equal, so the first is not less
          return false;
        }
      }
    }
    // If we got to this point, the two vectors are equal, so the first
    // is not less
    return false;
  }
}


/** A keyed comparator is only initialized with its key spec */
Table2::KeyedTupleComparator::KeyedTupleComparator(const Key key)
  : _key(key)
{
}


/** The comparators checks the fields indicated by the key spec in the
    order of the key spec. It compares the fields lexicographically.
    Absence of a field is smaller than existence of a field.  If the key
    spec is empty, then the comparator compares tuple IDs. */
bool
Table2::KeyedTupleComparator::
operator()(const TuplePtr first,
           const TuplePtr second) const
{
  if (_key.empty()) {
    // Compare tuple IDs only
    return (first->ID() < second->ID());
  } else {
    Table2::Key::const_iterator fieldNos = _key.begin();
    while (fieldNos != _key.end()) {
      // The next field number is
      unsigned fieldNo = *(fieldNos++);

      // Does the first have this field?
      if (first->size() > fieldNo) {
        // It does. Does the second have this field?
        if (second->size() > fieldNo) {
          // It does. Compare their values. If they're equal, I must
          // keep with the next field. If the first is less, stop here
          // and return true. If the first is greater, stop here and
          // return false.
          int comp = (*first)[fieldNo]->compareTo((*second)[fieldNo]);
          if (comp < 0) {
            // First is less on this field, so it is also less overall
            return true;
          } else if (comp > 0) {
            // First is greater on this field, so it cannot be less
            // overall
            return false;
          } else {
            // The fields are equal, so I must check the next field if
            // it exists
          }
        } else {
          // The second vector is lacking the field but the first one has
          // it. The first vector is not less.
          return false;
        }
      } else {
        // The vectors are equal this far, but the first vector is
        // lacking the field. If the second vector has this field, then
        // the first is less. If the second vector does not have this
        // field, then since the two vectors are equal, the first vector
        // is not less.
        if (second->size() > fieldNo) {
          // The second vector has it, so the first is less
          return true;
        } else {
          // The two vectors are equal, so the first is not less
          return false;
        }
      }
    }
    // If we got to this point, the two vectors are equal, so the first
    // is not less
    return false;
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
               (!_maxLifetime.is_pos_infinity()))),
    _lookupSearchEntry(Tuple::mk())
{
  lookupSearchEntry(key);
}


Table2::Table2(std::string tableName,
               Key& key)
  : _name(tableName),
    _key(key),
    _maxSize(0),
    _maxLifetime(DEFAULT_EXPIRATION),
    _primaryIndex(KeyedEntryComparator(key)),
    _flushing(((_maxSize > 0) ||
               (!_maxLifetime.is_pos_infinity()))),
    _lookupSearchEntry(Tuple::mk())
{
  lookupSearchEntry(key);
}


Table2::Table2(std::string tableName,
               Key& key,
               size_t maxSize)
  : _name(tableName),
    _key(key),
    _maxSize(maxSize),
    _maxLifetime(DEFAULT_EXPIRATION),
    _primaryIndex(KeyedEntryComparator(key)),
    _flushing(((_maxSize > 0) ||
               (!_maxLifetime.is_pos_infinity()))),
    _lookupSearchEntry(Tuple::mk())
{
  lookupSearchEntry(key);
}

Table2::Table2(std::string tableName,
               Key& key,
               size_t maxSize,
               string lifetime)
  : _name(tableName),
    _key(key),
    _maxSize(maxSize),
    _maxLifetime(DEFAULT_EXPIRATION),
    _primaryIndex(KeyedEntryComparator(key)),
    _flushing(((_maxSize > 0) ||
               (!_maxLifetime.is_pos_infinity()))),
    _lookupSearchEntry(Tuple::mk())
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
  lookupSearchEntry(key);
}


void
Table2::lookupSearchEntry(Table2::Key& key)
{
  for (Table2::Key::iterator i = key.begin();
       i != key.end();
       i++) {
    unsigned fieldNo = *i;
    while (fieldNo + 1 > _lookupSearchEntry.tuple->size()) {
      _lookupSearchEntry.tuple->append(ValuePtr());
    }
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

  ////////////////////////////////////////////////////////////
  // Queue, eliminating entries
  while (_queue.size() > 0) {
    Entry * toKill = _queue.back();
    _queue.pop_back();
    delete toKill;
  }

  ////////////////////////////////////////////////////////////
  // Primary index
  _primaryIndex.clear();
  

  //  std::cout << "Destroyed table " << _name << "\n";
}




////////////////////////////////////////////////////////////
// Secondary Indices
////////////////////////////////////////////////////////////

bool
Table2::secondaryIndex(Table2::Key& key)
{
  if (findSecondaryIndex(key) == NULL) {
    createSecondaryIndex(key);
    return true;
  } else {
    return false;
  }
}


void
Table2::createSecondaryIndex(Table2::Key& key)
{
  // Create it, update with all existing elements (from the
  // primary index)
  KeyedEntryComparator* comp = new KeyedEntryComparator(key);
  _keyedComparators.push_front(comp);
  
  SecondaryIndex* index = new SecondaryIndex(*comp);
  _indices.insert(std::make_pair(key, index));
  
  // Insert all current elements
  PrimaryIndex::iterator i = _primaryIndex.begin();
  while (i != _primaryIndex.end()) {
    Entry* current = *i;
    index->insert(current);
    i++;
  }

  // And enlarget the lookup entry
  lookupSearchEntry(key);
}


Table2::SecondaryIndex*
Table2::findSecondaryIndex(Table2::Key& key)
{
  SecondaryIndexIndex::iterator iter =
    _indices.find(key);

  if (iter == _indices.end()) {
    return NULL;
  } else {
    return (*iter).second;
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
      return false;
    }
    // Otherwise, tuple has same primary key but is different
    else {
      // We will replace the existing tuple, so remove it first
      removeTuple(found);
      // Must move the found iterator forward before reaching the
      // insertTuple statement below, since after the removal this
      // iterator position will be invalid
      found++; 
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
// Tuple wrapper
////////////////////////////////////////////////////////////

Table2::Entry::Entry(TuplePtr tp)
  : tuple(tp)
{
  //  std::cout << "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@Creating entry at address " << this << "\n";
  getTime(time);
}


Table2::Entry::~Entry()
{
  //  std::cout << "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@Destroying entry at address " << this << "\n";
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

  // Primary index.
  _primaryIndex.insert(position, newEntry);
  
  // Secondary indices. For all indices, insert.
  for (SecondaryIndexIndex::iterator iter = _indices.begin();
       iter != _indices.end();
       iter++) {
    (*iter).second->insert(newEntry);
  }

  // Aggregates. For all aggregates, insert.
  for (AggregateVector::iterator i = _aggregates.begin();
       i != _aggregates.end();
       i++) {
    (*i)->update(t);
  }


  // Update listeners.
  for (ListenerVector::iterator i = _updateListeners.begin();
       i != _updateListeners.end();
       i++) {
    (*i)(t);
  }

  // Insert into time-ordered queue if we're flushing.
  if (_flushing) {
    _queue.push_front(newEntry);
  }
}


void
Table2::flushTuple(TuplePtr t)
{
  // Primary index
  static Entry searchEntry(Tuple::EMPTY);
  searchEntry.tuple = t;

  // Just remove it from all indices.  The entry will be killed in
  // flush()
  _primaryIndex.erase(&searchEntry);

  // Secondary indices
  SecondaryIndexIndex::iterator iter = _indices.begin();
  while (iter != _indices.end()) {
    (iter++)->second->erase(&searchEntry);
  }
  
  // Aggregates
  for (AggregateVector::iterator i = _aggregates.begin();
       i != _aggregates.end();
       i++) {
    (*i)->update(searchEntry.tuple);
  }

  // Delete Listeners
}


void
Table2::removeTuple(PrimaryIndex::iterator position)
{
  // Set the search entry to the tuple from the primary index. This is
  // the tuple we want to remove from all secondary indices below.
  TuplePtr toRemove = (*position)->tuple;

  // Secondary indices
  SecondaryIndexIndex::iterator iter = _indices.begin();
  while (iter != _indices.end()) {
    SecondaryIndex& index = *((iter++)->second);

    // Now find the tuple removed from the primary index in this
    // secondary index and erase it
    static Entry searchEntry(Tuple::EMPTY);
    searchEntry.tuple = toRemove;
    SecondaryIndex::iterator secIter = index.lower_bound(&searchEntry);
    SecondaryIndex::iterator secIterEnd = index.upper_bound(&searchEntry);
    while (secIter != secIterEnd) {
      // Is this tuple identical to the one removed from the primary
      // index?
      if ((*secIter)->tuple->ID() == toRemove->ID()) {
        // It's the exact tuple. erase it
        index.erase(secIter);
        break;
      }
      secIter++;
    }
  }

  // I must delete the Entry when no more indices point at it!!!!

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

  // Aggregates
  for (AggregateVector::iterator i = _aggregates.begin();
       i != _aggregates.end();
       i++) {
    (*i)->update(toRemove);
  }
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
Table2::Key Table2::KEY4 = Table2::Key();
Table2::Key Table2::KEY01 = Table2::Key();
Table2::Key Table2::KEY12 = Table2::Key();
Table2::Key Table2::KEY23 = Table2::Key();
Table2::Key Table2::KEY13 = Table2::Key();
Table2::Key Table2::KEY012 = Table2::Key();
Table2::Key Table2::KEY123 = Table2::Key();
Table2::Key Table2::KEY01234 = Table2::Key();


Table2::Initializer::Initializer()
{
  // No need to initialize KEYID. It's already empty.
  Table2::KEY0.push_back(0);

  Table2::KEY1.push_back(1);

  Table2::KEY2.push_back(2);

  Table2::KEY3.push_back(3);

  Table2::KEY4.push_back(4);

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

  Table2::KEY01234.push_back(0);
  Table2::KEY01234.push_back(1);
  Table2::KEY01234.push_back(2);
  Table2::KEY01234.push_back(3);
  Table2::KEY01234.push_back(4);
}

/** Run the static initialization */
Table2::Initializer
Table2::_INITIALIZER;



////////////////////////////////////////////////////////////
// Lookup IteratorObjs
////////////////////////////////////////////////////////////

/** An iterator contains only a queue of tuple pointers, which it
    dispenses when asked. XXX Replace by just returning the queue to the
    caller. */
Table2::IteratorObj::IteratorObj(std::deque< TuplePtr >* spool)
  : _spool(spool)
{
}


TuplePtr
Table2::IteratorObj::next()
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
Table2::IteratorObj::done()
{
  return (_spool->size() == 0);
}


Table2::IteratorObj::~IteratorObj()
{
  // Delete the queue. Its contents are shared pointers to tuples, so no
  // need to worry about them
  _spool->clear();
  delete _spool;
}


size_t
Table2::size() const
{
  size_t s = _primaryIndex.size();
  return s;
}


std::string
Table2::name() const
{
  return _name;
}




////////////////////////////////////////////////////////////
// Lookups
////////////////////////////////////////////////////////////

Table2::Iterator
Table2::lookup(Table2::Key& lookupKey,
               Table2::Key& indexKey,
               TuplePtr t)  
{
  // It is essential that the two keys have the exact number of field
  // numbers
  assert(lookupKey.size() == indexKey.size());

  // Ensure we're operating on the correct view of the table as of right
  // now.
  flush();

  // Find the appropriate index. Is it the primary index?
  if (indexKey == _key) {
    // Prepare the search entry.  Project the lookup tuple onto the
    // search tuple along the lookup key to index key projection
    project(t, lookupKey, _lookupSearchEntry.tuple, _key);
    
    // Just perform the lookup on the primary index.
    return lookupPrimary(&_lookupSearchEntry);
  } 
  // If not, is it a secondary index?
  else {
    Table2::SecondaryIndexIndex::iterator indexIter =
      _indices.find(indexKey);
    
    // If not, return null
    if (indexIter == _indices.end()) {
      return Iterator();
    }
    // Otherwise, create the iterator from all the matches and return it
    else {    
      SecondaryIndex& index = *(*indexIter).second;

      // Project the lookup tuple along the lookup keys onto the search
      // tuple.
      project(t, lookupKey, _lookupSearchEntry.tuple, indexKey);

      return lookupSecondary(&_lookupSearchEntry, index);
    }
  }
} 


Table2::Iterator
Table2::lookup(Table2::Key& indexKey,
               TuplePtr t)  
{
  // Ensure we're operating on the correct view of the table as of right
  // now.
  flush();

  // The search entry.  We don't need to project for this one so the
  // search entry can be initialized with a known tuple.
  static Entry searchEntry(Tuple::EMPTY);

  // Prepare the search entry by copying the tuple into it (no
  // projection needed).
  searchEntry.tuple = t;
  
  // Find the appropriate index. Is it the primary index?
  if (indexKey == _key) {
    // Just perform the lookup on the primary index.
    return lookupPrimary(&searchEntry);
  } 
  // If not, is it a secondary index?
  else {
    Table2::SecondaryIndexIndex::iterator indexIter =
      _indices.find(indexKey);
    
    // If not, return null
    if (indexIter == _indices.end()) {
      return Iterator();
    }
    // Otherwise, create the iterator from all the matches and return it
    else {    
      SecondaryIndex& index = *(*indexIter).second;
      
      return lookupSecondary(&searchEntry, index);
    }
  }
} 


Table2::Iterator
Table2::lookupSecondary(Table2::Entry* searchEntry,
                        Table2::SecondaryIndex& index)  
{
  SecondaryIndex::iterator tupleIter = index.lower_bound(searchEntry);
  SecondaryIndex::iterator tupleIterEnd = index.upper_bound(searchEntry);
  
  // Create the lookup iterator by spooling all results found into a
  // queue and passing them into the iterator.
  std::deque< TuplePtr >* spool = new std::deque< TuplePtr >();
  for (;
       tupleIter != tupleIterEnd;
       tupleIter++) {
    TuplePtr result = (*tupleIter)->tuple;
    spool->push_front(result);
  }
  Iterator iterPtr = Iterator(new IteratorObj(spool));
  return iterPtr;
} 


Table2::Iterator
Table2::lookupPrimary(Table2::Entry* searchEntry)  
{
  PrimaryIndex::iterator tupleIter = _primaryIndex.find(searchEntry);
  
  // Create the lookup iterator by spooling all results found into a
  // queue and passing them into the iterator.
  std::deque< TuplePtr >* spool = new std::deque< TuplePtr >();
  while (tupleIter != _primaryIndex.end()) {
    spool->push_front((*(tupleIter++))->tuple);
  }
  Iterator iterPtr = Iterator(new IteratorObj(spool));
  return iterPtr;
} 


void
Table2::project(TuplePtr source,
                Key& sourceKey,
                TuplePtr destination,
                Key& destinationKey)
{
  Key::iterator s;
  Key::iterator d;
  for (s = sourceKey.begin(),
         d = destinationKey.begin();
       s != sourceKey.end();
       s++, d++) {
    uint sourceFieldNo = *s;
    uint destinationFieldNo = *d;
    destination->set(destinationFieldNo, (*source)[sourceFieldNo]);
  }
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

  // Find the appropriate index. Is it the primary index?
  if (key == _key) {
    return scanPrimary();
  } 
  // If not, is it a secondary index?
  else {
    Table2::SecondaryIndexIndex::iterator indexIter =
      _indices.find(key);

    // If not, return null
    if (indexIter == _indices.end()) {
      return Iterator();
    }
    // Otherwise, create the iterator from all entries and return it
    else {    
      SecondaryIndex& index = *(*indexIter).second;

      SecondaryIndex::iterator tupleIter = index.begin();

      // Create the lookup iterator by spooling all results found into a
      // queue and passing them into the iterator.
      std::deque< TuplePtr >* spool = new std::deque< TuplePtr >();
      while (tupleIter != index.end()) {
        spool->push_front((*(tupleIter++))->tuple);
      }
      Iterator iterPtr = Iterator(new IteratorObj(spool));
      return iterPtr;
    }
  }
} 


Table2::Iterator
Table2::scan()  
{
  // Ensure we're operating on the correct view of the table as of right
  // now.
  flush();

  return scanPrimary();
} 


Table2::Iterator
Table2::scanPrimary()  
{
  // Create the lookup iterator from all the matches and return it
  PrimaryIndex::iterator tupleIter = _primaryIndex.begin();

  // Create the lookup iterator by spooling all results found into a
  // queue and passing them into the iterator.
  std::deque< TuplePtr >* spool = new std::deque< TuplePtr >();
  while (tupleIter != _primaryIndex.end()) {
    spool->push_front((*(tupleIter++))->tuple);
  }
  Iterator iterPtr = Iterator(new IteratorObj(spool));
  return iterPtr;
} 




////////////////////////////////////////////////////////////
// Update listeners
////////////////////////////////////////////////////////////

void
Table2::updateListener(Listener listener)
{
  _updateListeners.push_back(listener);
}












////////////////////////////////////////////////////////////
// Aggregation
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

Table2::AggFunc::AggFunc()
{
}


Table2::AggFunc::~AggFunc()
{
}


Table2::AggregateObj::AggregateObj(Table2::Key& key,
                                   Table2::SecondaryIndex* index,
                                   unsigned aggField,
                                   Table2::AggFunc* function)
  : _key(key),
    _index(index),
    _aggField(aggField),
    _aggregateFn(function),
    _listeners(),
    _comparator(key),
    _currentAggregates(_comparator)
{
}


Table2::AggregateObj::~AggregateObj()
{
  delete _aggregateFn;
}


/** This method does not check for duplication. If a caller places
    itself into the listener queue twice, it will receive two calls for
    every update. */
void
Table2::AggregateObj::listener(Table2::Listener listener)
{
  _listeners.push_back(listener);
}


void
Table2::AggregateObj::update(TuplePtr changedTuple)
{
  // Start a new computation of the aggregate function
  _aggregateFn->reset();

  // An exemplar, if necessary
  TuplePtr aMatchingTuple = TuplePtr();

  // Scan the index on the group-by fields
  static Entry searchEntry(Tuple::EMPTY);
  searchEntry.tuple = changedTuple;
  for (SecondaryIndex::iterator i = _index->lower_bound(&searchEntry);
       i != _index->upper_bound(&searchEntry);
       i++) {
    // Fetch the next tuple. Recall the secondary index lies over Entry
    // pointers 
    TuplePtr tuple = (*i)->tuple;

    // It is bound to match the group-by fields since we're still
    // between the lower bound and the upper bound
    if (aMatchingTuple.get() == NULL) {
      // This is the first matching tuple
      _aggregateFn->first((*tuple)[_aggField]);
      aMatchingTuple = tuple;
    } else {
      // This is not the first matching tuple
      _aggregateFn->process((*tuple)[_aggField]);
    }
  }
  
  // Handle the newly computed aggregate, if any.  If I have none, then
  // make sure I erase what I may remember. If I have some, if the new
  // one is the same, update no one. If I have some, but the new one is
  // different, update what I remember and send a notice to the listener.

  if (aMatchingTuple.get() == NULL) {
    // I got no aggregate for this value. Presumably this update was a
    // removal. Erase any remembered aggregate (for the group-by values
    // of the changed tuple) and notify no one.
    _currentAggregates.erase(changedTuple);
  } else {
    // We had at least one match so we must have some result.
    ValuePtr result = _aggregateFn->result();
    assert(result.get() != NULL);
    
    // Is this a new result?
    AggMap::iterator remembered = _currentAggregates.find(changedTuple);
    if (remembered == _currentAggregates.end()) {
      // I didn't remember anything for this group-by value set
      // No need to erase anything
    } else {
      // We have one. Is it the same?
      if (remembered->second->compareTo(result) == 0) {
        // Yup, no need to remember anything
        goto doneRemembering;
      } else {
        // Different. we need to forget the old one and remember the new
        // one
        _currentAggregates.erase(changedTuple);
      }
    }
    
    {
      // If we're here, we need to remember a result and, if necessary,
      // we've forgotten the old result.
      _currentAggregates.insert(std::make_pair(changedTuple, result));
      
      // Put together an update tuple for listeners
      TuplePtr updateTuple = Tuple::mk();
      for (Key::iterator k = _key.begin();
           k != _key.end();
           k++) {
        unsigned fieldNo = *k;
        updateTuple->append((*changedTuple)[fieldNo]);
      }
      updateTuple->append(result);
      updateTuple->freeze();
      
      // We also need to notify listeners for the change
      for (ListenerVector::iterator i = _listeners.begin();
           i != _listeners.end();
           i++) {
        Listener listener = *i;
        listener(updateTuple);
      }
    }
  doneRemembering:
    // Any closing remarks independent on whether I updated listeners
    // with a new result or not?
    ;
  }
  return;
}


Table2::Aggregate
Table2::aggregate(Table2::Key& groupBy,
                  unsigned aggFieldNo,
                  std::string functionName)
{
  // Find the aggregate function
  AggFunc* function = AggFactory::mk(functionName);
  if (function == NULL) {
    // Couldn't create one. Return no aggregate
    return NULL;
  } else {
    // Ensure we have a secondary index. If one already exists, this is
    // a no-op.
    secondaryIndex(groupBy);
    SecondaryIndex* index = findSecondaryIndex(groupBy);
    
    // Create the aggregate object
    Aggregate a = new AggregateObj(groupBy, index, aggFieldNo,
                                   function);
    
    // Store the aggregate
    _aggregates.push_back(a);
    return a;
  }
}


/** No expiration is represented as positive infinity. */
boost::posix_time::time_duration
Table2::NO_EXPIRATION(boost::date_time::pos_infin);


/** Default table expiration time is no expiration */
boost::posix_time::time_duration
Table2::DEFAULT_EXPIRATION(Table2::NO_EXPIRATION);


/** No size is represented as 0 size. */
size_t
Table2::NO_SIZE = 0;


/** Default table max size is no max size */
size_t
Table2::DEFAULT_SIZE(Table2::NO_SIZE);
