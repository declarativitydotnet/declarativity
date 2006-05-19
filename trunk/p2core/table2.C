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

/** My comparator for vectors of unsigned integers. */
bool
Table2::unsignedVectorLess::operator()(const Table2::Key first,
                                       const Table2::Key second) const
{
  if (first.size() < second.size()) {
    return true; // first < second
  } else if (first.size() > second.size()) {
    return false; // second < first
  } else {
    Table2::Key::const_iterator firstIt = first.begin();
    Table2::Key::const_iterator secondIt = second.begin();
    while (firstIt != first.end()) {
      if ((*firstIt) >= (*secondIt)) {
        // We're done, second >= first
        return false;
      }
    }
    // We're done, first < second
    return true;
  }
}


/** My comparator for value ptr vectors. */
bool
Table2::valuePtrVectorLess::
operator()(const Table2::ValuePtrVector first,
           const Table2::ValuePtrVector second) const
{
  if (first.size() < second.size()) {
    return true; // first < second
  } else if (first.size() > second.size()) {
    return false; // second < first
  } else {
    Table2::ValuePtrVector::const_iterator firstIt = first.begin();
    Table2::ValuePtrVector::const_iterator secondIt = second.begin();
    while (firstIt != first.end()) {
      if ((*firstIt)->compareTo(*secondIt) >= 0) {
        // We're done, second >= first
        return false;
      }
    }
    // We're done, first < second
    return true;
  }
}


/** A keyed comparator is only initialized with its key spec */
Table2::KeyedComparator::KeyedComparator(Key key)
  : _key(key)
{
}


/** The comparators checks the fields indicated by the key spec in the
    order of the key spec. It compares the fields lexicographically.
    Absence of a field is smaller than existence of a field */
bool
Table2::KeyedComparator::
operator()(const Table2::Entry* fEntry,
           const Table2::Entry* sEntry) const
{
  TuplePtr first = fEntry->tuple;
  TuplePtr second = sEntry->tuple;

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




////////////////////////////////////////////////////////////
// Constructors
////////////////////////////////////////////////////////////


Table2::Table2(std::string tableName,
               Key key,
               size_t maxSize,
               boost::posix_time::time_duration& lifetime)
  : _name(tableName),
    _key(key),
    _maxSize(maxSize),
    _maxLifetime(lifetime),
    _primaryIndex(KeyedComparator(key))
{
}




////////////////////////////////////////////////////////////
// Insert tuples
////////////////////////////////////////////////////////////

bool
Table2::insert(TuplePtr t)
{
  // Do I already have the identical tuple?
  if (tupleInTable(t)) {
    // I do. Do nothing and return false
    return false;
  } else {
    // I do not.

    // Remove any tuple by the same key
    removeTuple(t);

    // Insert new tuple
    insertTuple(t);

    // Return true
    return true;
  }
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

bool
Table2::tupleInTable(TuplePtr t)
{
  // Find tuple in primary index
  
  // If a tuple exists with same primary key

  //    Is the tuple identical?

  //       Return yes
  
  //    Otherwise
 
  //       Return no
  return false;
}
