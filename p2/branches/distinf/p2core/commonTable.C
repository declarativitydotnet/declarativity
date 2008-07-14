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
#include "val_null.h"
#include "val_str.h"
#include "plumber.h"
#include "systemTable.h"
#include "tuple.h"
#include "val_int64.h"

////////////////////////////////////////////////////////////
// Constants
////////////////////////////////////////////////////////////


/** No expiration is represented as positive infinity. */
boost::posix_time::time_duration
CommonTable::NO_EXPIRATION(boost::date_time::pos_infin);


/** Default table expiration time is no expiration */
boost::posix_time::time_duration
CommonTable::DEFAULT_EXPIRATION(CommonTable::NO_EXPIRATION);


/** No size is represented as 0 size. */
uint32_t
CommonTable::NO_SIZE = 0;


/** Default table max size is no max size */
uint32_t
CommonTable::DEFAULT_SIZE(CommonTable::NO_SIZE);


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

  // Register table
  theRegistry()->insert(std::make_pair(tableName, this));
}


void
CommonTable::lookupSearchEntry(CommonTable::Key& key)
{
  for (CommonTable::Key::iterator i = key.begin();
       i != key.end();
       i++) {
    unsigned fieldNo = *i;
    while (fieldNo + 1 > _lookupSearchEntry.tuple->size()) {
      _lookupSearchEntry.tuple->append(Val_Null::mk());
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
  TABLE_WORDY("Creating new secondary index.");
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

  // And enlarge the lookup entry
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






////////////////////////////////////////////////////////////
// Tuple wrapper
////////////////////////////////////////////////////////////

CommonTable::Entry::Entry(TuplePtr tp)
  : tuple(tp)
{
  assert(tp.get() != NULL);
  //  TELL_WORDY << "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@Creating entry at address " << this << "\n";
  getTime(time);
  refCount = 0;
}


CommonTable::Entry::~Entry()
{
  //  TELL_WORDY << "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@Destroying entry at address " << this << "\n";
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

  // Update table information
  CommonTable::ManagerPtr catalog = Plumber::catalog();
  if (catalog) {
    CommonTablePtr table = catalog->table(TABLE);
    if (table) {
      static unsigned cardPos = catalog->attribute(TABLE, "CARD");
      TuplePtr tableNameTp = Tuple::mk();
      tableNameTp->append(Val_Str::mk(name()));
      tableNameTp->freeze();
      CommonTable::Iterator iter = table->lookup(CommonTable::theKey(CommonTable::KEY0),
                                                 CommonTable::theKey(CommonTable::KEY3),
                                                 tableNameTp); 
      if (!iter->done()) {
        TuplePtr tp = iter->next()->clone();
        tp->set(cardPos, Val_Int64::mk(size()));
      }
    }
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
    static Entry searchEntry(Tuple::EMPTY());
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
CommonTable::removeTupleOfEntry(Entry* theEntry)
{
  // Set the search entry to the tuple from the primary index. This is
  // the tuple we want to remove from all secondary indices below.
  TuplePtr toRemove = theEntry->tuple;

  // Remove from derivatives
  removeDerivatives(toRemove);

  // The subclass finishes it off from the primary index

  // Update table statistics
  CommonTable::ManagerPtr catalog = Plumber::catalog();
  if (catalog) {
    CommonTablePtr table = catalog->table(TABLE);
    if (table) {
      static unsigned cardPos = catalog->attribute(TABLE, "CARD");
      TuplePtr tableNameTp = Tuple::mk();
      tableNameTp->append(Val_Str::mk(name()));
      tableNameTp->freeze();
      CommonTable::Iterator iter = table->lookup(CommonTable::theKey(CommonTable::KEY0),
                                                 CommonTable::theKey(CommonTable::KEY3),
                                                 tableNameTp); 
      if (!iter->done()) {
        TuplePtr tp = iter->next()->clone();
        tp->set(cardPos, Val_Int64::mk(size()));
      }
    }
  }
}



CommonTable::Initializer::Initializer()
{
  // No need to initialize KEYID. It's already empty.
  theKEY0.push_back(0);

  theKEY1.push_back(1);

  theKEY2.push_back(2);

  theKEY3.push_back(3);

  theKEY4.push_back(4);

  theKEY5.push_back(5);

  theKEY6.push_back(6);

  theKEY9.push_back(9);

  theKEY01.push_back(0);
  theKEY01.push_back(1);

  theKEY12.push_back(1);
  theKEY12.push_back(2);

  theKEY23.push_back(2);
  theKEY23.push_back(3);

  theKEY25.push_back(2);
  theKEY25.push_back(5);

  theKEY13.push_back(1);
  theKEY13.push_back(3);

  theKEY34.push_back(3);
  theKEY34.push_back(4);

  theKEY35.push_back(3);
  theKEY35.push_back(5);

  theKEY36.push_back(3);
  theKEY36.push_back(6);

  theKEY38.push_back(3);
  theKEY38.push_back(8);

  theKEY39.push_back(3);
  theKEY39.push_back(9);

  theKEY45.push_back(4);
  theKEY45.push_back(5);

  theKEY012.push_back(0);
  theKEY012.push_back(1);
  theKEY012.push_back(2);

  theKEY123.push_back(1);
  theKEY123.push_back(2);
  theKEY123.push_back(3);

  theKEY345.push_back(3);
  theKEY345.push_back(4);
  theKEY345.push_back(5);

  theKEY01234.push_back(0);
  theKEY01234.push_back(1);
  theKEY01234.push_back(2);
  theKEY01234.push_back(3);
  theKEY01234.push_back(4);
}


CommonTable::Initializer*
CommonTable::theInitializer()
{
  static Initializer* _initializer =
    new CommonTable::Initializer();
  return _initializer;
}
  
/** Fetch an existing key object.  This method uses the "construct at
    first use" pattern. See below at theRegistry for details. */
CommonTable::Key&
CommonTable::theKey(CommonTable::KeyName keyID)
{
  Initializer* initializer;
  initializer = theInitializer(); // To ensure keys have been initialized


  switch (keyID) {
  case KEYID:
    return initializer->theKEYID;
    break;
  case KEY0:
    return initializer->theKEY0;
    break;
  case KEY1:
    return initializer->theKEY1;
    break;
  case KEY2:
    return initializer->theKEY2;
    break;
  case KEY3:
    return initializer->theKEY3;
    break;
  case KEY4:
    return initializer->theKEY4;
    break;
  case KEY5:
    return initializer->theKEY5;
    break;
  case KEY6:
    return initializer->theKEY6;
    break;
  case KEY9:
    return initializer->theKEY9;
    break;
  case KEY01:
    return initializer->theKEY01;
    break;
  case KEY12:
    return initializer->theKEY12;
    break;
  case KEY23:
    return initializer->theKEY23;
    break;
  case KEY25:
    return initializer->theKEY25;
    break;
  case KEY13:
    return initializer->theKEY13;
    break;
  case KEY34:
    return initializer->theKEY34;
    break;
  case KEY35:
    return initializer->theKEY35;
    break;
  case KEY36:
    return initializer->theKEY36;
    break;
  case KEY38:
    return initializer->theKEY38;
    break;
  case KEY39:
    return initializer->theKEY39;
    break;
  case KEY45:
    return initializer->theKEY45;
    break;
  case KEY012:
    return initializer->theKEY012;
    break;
  case KEY123:
    return initializer->theKEY123;
    break;
  case KEY345:
    return initializer->theKEY345;
    break;
  case KEY01234:
    return initializer->theKEY01234;
    break;
  default:
    throw UnknownKeyNameException();
    break;
  };
}



/** Find a table in the registry */
CommonTable*
CommonTable::lookupTable(string tableName)
{
  TableRegistry::iterator t = theRegistry()->find(tableName);
  if (t == theRegistry()->end()) {
    // Didn't find anything
    return NULL;
  } else {
    return t->second;
  }
}

/** This implements the "construct at first use" pattern, which is the
    best known method for dealing with static initializer ordering in
    C++. This way of initializing the registry "leaks" the registry at
    exit time, since the object is never freed from the queue. However,
    this is an acceptable shortcoming (and recommended by most FAQs such
    as the C++ FAQ Lite). Using the alternative, a stack-allocated
    registry object, still fixes the initialization ordering problem but
    does not fix the deinitialization ordering problem which is just as
    annoying... */
CommonTable::TableRegistry*
CommonTable::theRegistry()
{
  static TableRegistry* _registry =
    new TableRegistry();

  return _registry;
}




////////////////////////////////////////////////////////////
// Lookup IteratorObjs
////////////////////////////////////////////////////////////

/** An iterator contains only a queue of tuple pointers, which it
    dispenses when asked. XXX Replace by just returning the queue to the
    caller. */
/*
	Eric: modified to have this iterator rewindable 
	Modified to have the spool resuable without exposing
	STL API
*/
	
CommonTable::IteratorObj::IteratorObj(std::deque< TuplePtr >* spool)
  : _spool(spool)
{
  _it = _spool->rbegin();
}


TuplePtr
CommonTable::IteratorObj::next()
{
  if (!done()) {
    TuplePtr back = *_it;
    _it++;
    return back;
  } else {
    // We've run out of elements, period.
    return TuplePtr();
  }
}


bool
CommonTable::IteratorObj::done()
{
  if(_spool == NULL || _spool->empty())
    return true;
  return (_it == _spool->rend());
}

void
CommonTable::IteratorObj::rewind()
{
  _it = _spool->rbegin();
  /**XXX: somehow using temp obj causes seg fault...
	  DO NOT USE return Iterator(this); !!!!
  */
  /*Iterator itObj(this);
  return itObj;*/
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

const CommonTable::Key&
CommonTable::primaryKey() const
{
  return _key;
}



////////////////////////////////////////////////////////////
// Lookups
////////////////////////////////////////////////////////////

CommonTable::Iterator
CommonTable::lookup(const CommonTable::Key& lookupKey,
                    const CommonTable::Key& indexKey,
                    TuplePtr t)  
{
  TABLE_WORDY("Lookup with projection of tuple "
              << t->toString());

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
      /**How could this be possible? no secondary Index?!*/
      TELL_ERROR<<"Table "<<_name<<" does not have a secondary index for "
                << t->toString() <<"!!!??\n";
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
CommonTable::lookup(const CommonTable::Key& indexKey,
                    TuplePtr t)  
{
  TABLE_WORDY("Lookup without projection of tuple "
              << t->toString());

  // The search entry.  We don't need to project for this one so the
  // search entry can be initialized with a known tuple.
  static Entry searchEntry(Tuple::EMPTY());

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
      assert(0);
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
       (tupleIter != tupleIterEnd) && (tupleIter != index.end());
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
  PrimaryIndex::iterator endIter = _primaryIndex.upper_bound(searchEntry);

  // Create the lookup iterator by spooling all results found into a
  // queue and passing them into the iterator.
  std::deque< TuplePtr >* spool = new std::deque< TuplePtr >();
  while ((tupleIter != endIter) &&
         (tupleIter != _primaryIndex.end())) {
    spool->push_front((*(tupleIter++))->tuple);
  }
  Iterator iterPtr = Iterator(new IteratorObj(spool));
  return iterPtr;
} 


void
initTupleEntry(TuplePtr tuple, CommonTable::Key& key)
{
  for (CommonTable::Key::iterator i = key.begin();
       i != key.end();
       i++) {
    unsigned fieldNo = *i;
    while (fieldNo + 1 > tuple->size()) {
      tuple->append(ValuePtr());
    }
  }
}


CommonTable::Iterator
CommonTable::range1DLookup(CommonTable::Key& lKey,
                           CommonTable::Key& rKey,
                           CommonTable::Key& indexKey,
                           bool openL, bool openR, TuplePtr t)
{
  assert(indexKey.size() == rKey.size() || indexKey.size() == lKey.size());
  
  Entry eU(Tuple::mk());
  Entry eL(Tuple::mk());
  
  if (!lKey.empty()) {
    initTupleEntry(eL.tuple,indexKey);
    project(t,lKey,eL.tuple,indexKey);
    ///if(eL.tuple->size()!=0) TELL_INFO<<"\nCommonTable::RangeLookup: eL= "<<eL.tuple->toString()<<"\n";
  }
  if (!rKey.empty()) {
    initTupleEntry(eU.tuple,indexKey);
    project(t,rKey,eU.tuple,indexKey);
    //if(eU.tuple->size() != 0) TELL_INFO<<"\nCommonTable::RangeLookup: eU= "<<eU.tuple->toString()<<"\n";
  }
  
  //is it a secondary index?
  CommonTable::SecondaryIndexIndex::iterator it = _indices.find(indexKey);
  if (it == _indices.end()) {
    TELL_ERROR<<"XXX Range query without secondary index?!\n";
    assert(0);	
    return Iterator();
  } else { 
    SecondaryIndex& index = *(*it).second;
    return range1DLookupSecondary(openL, &eL, openR, &eU, index);
  }
}

CommonTable::Iterator
CommonTable::range1DLookupSecondary(bool openL, CommonTable::Entry* lb, 
                                    bool openR, CommonTable::Entry* rb, 
                                    SecondaryIndex& index)
{
  SecondaryIndex::iterator itL, itR;

  //if(index.size() !=0)
  //  TELL_INFO<< "\nSAMPLE: "<<(*index.begin())->tuple->toString()<<"\n";

  //no equal for the left boudary?
  if(lb->tuple->size() !=0 && openL == false) //close
	itL = index.lower_bound(lb); // things >=lb
  else if(lb->tuple->size() != 0) //open
	itL = index.upper_bound(lb); //things > lb
  else
	itL = index.begin();//infty

  //contains equal for right boundary?
  if(rb->tuple->size() != 0 && openR == false) //close RHS
	itR = index.upper_bound(rb); // stop at things > rb
  else if(rb->tuple->size() != 0) //open 	
	itR = index.lower_bound(rb); // stop at things >= rb
  else
	itR = index.end();

  int count =0;

  std::deque< TuplePtr >* spool = new std::deque< TuplePtr >();
  for(;itL != itR;itL++){
	//std::cout<<"\nRange Lookup: result added: "<<(*itL)->tuple->toString();
	//std::cout.flush();
	spool->push_front( (*itL)->tuple );
	count++;
  }
  //TELL_INFO<<"\nRangeLookup Results found: "<<count<<"\n";

  return Iterator(new IteratorObj(spool));
}



void
CommonTable::project(TuplePtr source,
                     const Key& sourceKey,
                     TuplePtr destination,
                     const Key& destinationKey)
{
  Key::const_iterator s;
  Key::const_iterator d;
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
// Listeners
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


void
CommonTable::refreshListener(Listener listener)
{
  _refreshListeners.push_back(listener);
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

  // Have I seen tuples before?
  bool seenTuples = false;

  // Scan the index on the group-by fields
  static Entry searchEntry(Tuple::EMPTY());
  searchEntry.tuple = changedTuple;
  for (SecondaryIndex::iterator i = _index->lower_bound(&searchEntry);
       i != _index->upper_bound(&searchEntry);
       i++) {
    // Fetch the next tuple. Recall the secondary index lies over Entry
    // pointers 
    TuplePtr tuple = (*i)->tuple;

    // It is bound to match the group-by fields since we're still
    // between the lower bound and the upper bound
    if (!seenTuples) {
      // This is the first matching tuple
      _aggregateFn->first((*tuple)[_aggField]);
      seenTuples = true;
    } else {
      // This is not the first matching tuple
      _aggregateFn->process((*tuple)[_aggField]);
    }
  }
  
  // Fetch the result
  ValuePtr result = _aggregateFn->result();

  // No aggregate function should return null pointers. If it needs to
  // return null, it should return a Val_Null.
  if (result.get() == NULL) {
    result = Val_Null::mk();
    TELL_ERROR << "Aggregate Function "
               << _aggregateFn->name()
               << " returned null result. FIX IT!!!";
  }

  // Do I remember a prior result for this group-by combination?
  AggMap::iterator remembered = _currentAggregates.find(changedTuple);
  if (remembered == _currentAggregates.end()) {
    // I don't remember anything for this group-by value set
    
    // Put together the result tuple
    TuplePtr updateTuple = Tuple::mk();
    for (Key::iterator k = _key.begin();
         k != _key.end();
         k++) {
      unsigned fieldNo = *k;
      updateTuple->append((*changedTuple)[fieldNo]);
    }
    updateTuple->append(result);
    updateTuple->freeze();

    // Remember the tuple
    _currentAggregates.insert(std::make_pair(changedTuple, result));

    // Update listeners
    for (ListenerVector::iterator i = _listeners.begin();
         i != _listeners.end();
         i++) {
      Listener listener = *i;
      listener(updateTuple);
    }
  } else {
    // We have one. Is it the same?
    if (remembered->second->compareTo(result) == 0) {
      // No need to update anyone
    } else {
      // Different. we need to forget the old one and remember the new
      // one
      _currentAggregates.erase(changedTuple);

      // Put together the new result tuple
      TuplePtr updateTuple = Tuple::mk();
      for (Key::iterator k = _key.begin();
           k != _key.end();
           k++) {
        unsigned fieldNo = *k;
        updateTuple->append((*changedTuple)[fieldNo]);
      }
      updateTuple->append(result);
      updateTuple->freeze();
      
      // Remember the tuple
      _currentAggregates.insert(std::make_pair(changedTuple, result));

      // Update listeners.
      for (ListenerVector::iterator i = _listeners.begin();
           i != _listeners.end();
           i++) {
        Listener listener = *i;
        listener(updateTuple);
      }
    }
  }
}


void
CommonTable::AggregateObj::evaluateEmpties()
{
  // Only empty aggregate indices make sense for this and then only for
  // the empty key (i.e., empty group-by).  Non-empty group-bys cannot
  // be expecting a result without any elements in the table.

  if (_index->empty() && _key.empty()) {
    // Make the aggregate output a result tuple

    // Start a new computation of the aggregate function
    _aggregateFn->reset();

    ValuePtr result = _aggregateFn->result();

    // No aggregate function should return null pointers. If it needs to
    // return null, it should return a Val_Null.
    if (result.get() == NULL) {
      result = Val_Null::mk();
      TELL_ERROR << "Aggregate Function "
                 << _aggregateFn->name()
                 << " returned null result. FIX IT!!!";
    }
    
    // Put together the result tuple
    TuplePtr updateTuple = Tuple::mk();
    updateTuple->append(result);
    updateTuple->freeze();
    
    // Update listeners
    for (ListenerVector::iterator i = _listeners.begin();
         i != _listeners.end();
         i++) {
      Listener listener = *i;
      listener(updateTuple);
    }
  }
}


CommonTable::Aggregate
CommonTable::aggregate(CommonTable::Key& groupBy,
                       unsigned aggFieldNo,
                       std::string functionName)
{
  // Find the aggregate function
  AggFunc* function = AggFactory::mk(functionName);

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


void
CommonTable::evaluateEmptyAggregates()
{
  // Go through all the aggregates
  for (AggregateVector::iterator i = _aggregates.begin();
       i != _aggregates.end();
       i++) {
    (*i)->evaluateEmpties();
  }
}
