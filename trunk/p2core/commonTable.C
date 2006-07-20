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
 * includes the tuple itself and its time of insertion (may be unused)
 * and reference counter.  Tuple entries are indexed by the primary key
 * in a sorted container.  Secondary indices also point to the tuple
 * wrapper.  Since tuples are immutable, there are no update consistency
 * issues here.
 */

#include "commonTable.h"
#include "p2Time.h"
#include "aggFactory.h"
#include "iostream"

////////////////////////////////////////////////////////////
// Sorters
////////////////////////////////////////////////////////////

/** This comparator performs first less second for two key specs. */
bool
CommonTable::KeyComparator::
operator()(const CommonTable::Key& first,
           const CommonTable::Key& second) const
{
  if (first.size() < second.size()) {
    return true; // first < second
  } else if (first.size() > second.size()) {
    return false; // second < first
  } else {
    // As long as they're equal, keep checking.
    CommonTable::Key::const_iterator firstIt = first.begin();
    CommonTable::Key::const_iterator secondIt = second.begin();
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
CommonTable::KeyedEntryComparator::KeyedEntryComparator(const Key key)
  : _key(key)
{
}


/** The comparators checks the fields indicated by the key spec in the
    order of the key spec. It compares the fields lexicographically.
    Absence of a field is smaller than existence of a field.  If the key
    spec is empty, then the comparator compares tuple IDs. */
bool
CommonTable::KeyedEntryComparator::
operator()(const CommonTable::Entry* fEntry,
           const CommonTable::Entry* sEntry) const
{
  TuplePtr first(fEntry->tuple);
  TuplePtr second(sEntry->tuple);

  if (_key.empty()) {
    // Compare tuple IDs only
    return (first->ID() < second->ID());
  } else {
    CommonTable::Key::const_iterator fieldNos = _key.begin();
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
CommonTable::KeyedTupleComparator::KeyedTupleComparator(const Key key)
  : _key(key)
{
}


/** The comparators checks the fields indicated by the key spec in the
    order of the key spec. It compares the fields lexicographically.
    Absence of a field is smaller than existence of a field.  If the key
    spec is empty, then the comparator compares tuple IDs. */
bool
CommonTable::KeyedTupleComparator::
operator()(const TuplePtr first,
           const TuplePtr second) const
{
  if (_key.empty()) {
    // Compare tuple IDs only
    return (first->ID() < second->ID());
  } else {
    CommonTable::Key::const_iterator fieldNos = _key.begin();
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


CommonTable::CommonTable(std::string tableName,
                         Key& key)
  : _name(tableName),
    _key(key),
    _primaryIndex(KeyedEntryComparator(key)),
    _lookupSearchEntry(Tuple::mk())
{
  lookupSearchEntry(key);
}


void
CommonTable::lookupSearchEntry(CommonTable::Key& key)
{
  for (CommonTable::Key::iterator i = key.begin();
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

CommonTable::~CommonTable()
{
}




////////////////////////////////////////////////////////////
// Secondary Indices
////////////////////////////////////////////////////////////

bool
CommonTable::secondaryIndex(CommonTable::Key& key)
{
  if (findSecondaryIndex(key) == NULL) {
    createSecondaryIndex(key);
    return true;
  } else {
    return false;
  }
}


void
CommonTable::createSecondaryIndex(CommonTable::Key& key)
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


CommonTable::SecondaryIndex*
CommonTable::findSecondaryIndex(CommonTable::Key& key)
{
  SecondaryIndexIndex::iterator iter =
    _indices.find(key);

  if (iter == _indices.end()) {
    return NULL;
  } else {
    return (*iter).second;
  }
}




std::string
CommonTable::toString()
{
  ostringstream oss;
  oss << "Table '" << _name << "':";
  if (_primaryIndex.size() > 0) {
    PrimaryIndex::iterator i = _primaryIndex.begin();
    oss << (*i)->tuple->toString();
    for (i++;
         i != _primaryIndex.end();
         i++) {
      oss << ", " << (*i)->tuple->toString();
    }
    oss << ".";
  } else {
    oss << "Empty.";
  }
  return oss.str();
}




////////////////////////////////////////////////////////////
// Tuple wrapper
////////////////////////////////////////////////////////////

CommonTable::Entry::Entry(TuplePtr tp)
  : tuple(tp)
{
  //  std::cout << "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@Creating entry at address " << this << "\n";
  getTime(time);
  refCount = 0;
}


CommonTable::Entry::~Entry()
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
CommonTable::commonInsertTuple(Entry* newEntry,
                               TuplePtr t,
                               PrimaryIndex::iterator position)
{
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
}


void
CommonTable::removeDerivatives(TuplePtr t)
{
  // Secondary indices
  SecondaryIndexIndex::iterator iter = _indices.begin();
  while (iter != _indices.end()) {
    SecondaryIndex& index = *((iter++)->second);
    
    // Now find the tuple removed from the primary index in this
    // secondary index and erase it
    static Entry searchEntry(Tuple::EMPTY);
    searchEntry.tuple = t;
    SecondaryIndex::iterator secIter = index.lower_bound(&searchEntry);
    SecondaryIndex::iterator secIterEnd = index.upper_bound(&searchEntry);
    while (secIter != secIterEnd) {
      // Is this tuple identical to the one removed from the primary
      // index?
      if ((*secIter)->tuple->ID() == t->ID()) {
        // It's the exact tuple. erase it
        index.erase(secIter);
        break;
      }
      secIter++;
    }
  }

  // Aggregates
  for (AggregateVector::iterator i = _aggregates.begin();
       i != _aggregates.end();
       i++) {
    (*i)->update(t);
  }

  // Delete Listeners
  for (ListenerVector::iterator i = _removalListeners.begin();
       i != _removalListeners.end();
       i++) {
    (*i)(t);
  }
}


void
CommonTable::removeTuple(PrimaryIndex::iterator position)
{
  // Set the search entry to the tuple from the primary index. This is
  // the tuple we want to remove from all secondary indices below.
  TuplePtr toRemove = (*position)->tuple;

  // Remove from derivatives
  removeDerivatives(toRemove);

  // The subclass finishes it off from the primary index
}




CommonTable::Key CommonTable::KEYID = CommonTable::Key();
CommonTable::Key CommonTable::KEY0 = CommonTable::Key();
CommonTable::Key CommonTable::KEY1 = CommonTable::Key();
CommonTable::Key CommonTable::KEY2 = CommonTable::Key();
CommonTable::Key CommonTable::KEY3 = CommonTable::Key();
CommonTable::Key CommonTable::KEY4 = CommonTable::Key();
CommonTable::Key CommonTable::KEY01 = CommonTable::Key();
CommonTable::Key CommonTable::KEY12 = CommonTable::Key();
CommonTable::Key CommonTable::KEY23 = CommonTable::Key();
CommonTable::Key CommonTable::KEY13 = CommonTable::Key();
CommonTable::Key CommonTable::KEY012 = CommonTable::Key();
CommonTable::Key CommonTable::KEY123 = CommonTable::Key();
CommonTable::Key CommonTable::KEY01234 = CommonTable::Key();


CommonTable::Initializer::Initializer()
{
  // No need to initialize KEYID. It's already empty.
  CommonTable::KEY0.push_back(0);

  CommonTable::KEY1.push_back(1);

  CommonTable::KEY2.push_back(2);

  CommonTable::KEY3.push_back(3);

  CommonTable::KEY4.push_back(4);

  CommonTable::KEY01.push_back(0);
  CommonTable::KEY01.push_back(1);

  CommonTable::KEY12.push_back(1);
  CommonTable::KEY12.push_back(2);

  CommonTable::KEY23.push_back(2);
  CommonTable::KEY23.push_back(3);

  CommonTable::KEY13.push_back(1);
  CommonTable::KEY13.push_back(3);

  CommonTable::KEY012.push_back(0);
  CommonTable::KEY012.push_back(1);
  CommonTable::KEY012.push_back(2);

  CommonTable::KEY123.push_back(1);
  CommonTable::KEY123.push_back(2);
  CommonTable::KEY123.push_back(3);

  CommonTable::KEY01234.push_back(0);
  CommonTable::KEY01234.push_back(1);
  CommonTable::KEY01234.push_back(2);
  CommonTable::KEY01234.push_back(3);
  CommonTable::KEY01234.push_back(4);
}

/** Run the static initialization */
CommonTable::Initializer
CommonTable::_INITIALIZER;



////////////////////////////////////////////////////////////
// Lookup IteratorObjs
////////////////////////////////////////////////////////////

/** An iterator contains only a queue of tuple pointers, which it
    dispenses when asked. XXX Replace by just returning the queue to the
    caller. */
CommonTable::IteratorObj::IteratorObj(std::deque< TuplePtr >* spool)
  : _spool(spool)
{
}


TuplePtr
CommonTable::IteratorObj::next()
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
CommonTable::IteratorObj::done()
{
  return (_spool->size() == 0);
}


CommonTable::IteratorObj::~IteratorObj()
{
  // Delete the queue. Its contents are shared pointers to tuples, so no
  // need to worry about them
  _spool->clear();
  delete _spool;
}


uint32_t
CommonTable::size() const
{
  uint32_t s = _primaryIndex.size();
  return s;
}


std::string
CommonTable::name() const
{
  return _name;
}




////////////////////////////////////////////////////////////
// Lookups
////////////////////////////////////////////////////////////

CommonTable::Iterator
CommonTable::lookup(CommonTable::Key& lookupKey,
                    CommonTable::Key& indexKey,
                    TuplePtr t)  
{
  // It is essential that the two keys have the exact number of field
  // numbers
  assert(lookupKey.size() == indexKey.size());

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
    CommonTable::SecondaryIndexIndex::iterator indexIter =
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


CommonTable::Iterator
CommonTable::lookup(CommonTable::Key& indexKey,
                    TuplePtr t)  
{
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
    CommonTable::SecondaryIndexIndex::iterator indexIter =
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


CommonTable::Iterator
CommonTable::lookupSecondary(CommonTable::Entry* searchEntry,
                        CommonTable::SecondaryIndex& index)  
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


CommonTable::Iterator
CommonTable::lookupPrimary(CommonTable::Entry* searchEntry)  
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
CommonTable::project(TuplePtr source,
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

CommonTable::Iterator
CommonTable::scan(CommonTable::Key& key)  
{
  // Find the appropriate index. Is it the primary index?
  if (key == _key) {
    return scanPrimary();
  } 
  // If not, is it a secondary index?
  else {
    CommonTable::SecondaryIndexIndex::iterator indexIter =
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


CommonTable::Iterator
CommonTable::scan()  
{
  return scanPrimary();
} 


CommonTable::Iterator
CommonTable::scanPrimary()  
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
CommonTable::updateListener(Listener listener)
{
  _updateListeners.push_back(listener);
}


void
CommonTable::removalListener(Listener listener)
{
  _removalListeners.push_back(listener);
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

CommonTable::AggFunc::AggFunc()
{
}


CommonTable::AggFunc::~AggFunc()
{
}


CommonTable::AggregateObj::AggregateObj(CommonTable::Key& key,
                                   CommonTable::SecondaryIndex* index,
                                   unsigned aggField,
                                   CommonTable::AggFunc* function)
  : _key(key),
    _index(index),
    _aggField(aggField),
    _aggregateFn(function),
    _listeners(),
    _comparator(key),
    _currentAggregates(_comparator)
{
}


CommonTable::AggregateObj::~AggregateObj()
{
  delete _aggregateFn;
}


/** This method does not check for duplication. If a caller places
    itself into the listener queue twice, it will receive two calls for
    every update. */
void
CommonTable::AggregateObj::listener(CommonTable::Listener listener)
{
  _listeners.push_back(listener);
}


void
CommonTable::AggregateObj::update(TuplePtr changedTuple)
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


CommonTable::Aggregate
CommonTable::aggregate(CommonTable::Key& groupBy,
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


