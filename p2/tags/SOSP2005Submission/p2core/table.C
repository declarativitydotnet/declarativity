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
 * DESCRIPTION: Table implementation
 *
 */

#include "table.h"

#include "val_str.h"
#include "val_uint64.h"

//
// Constructor
//
Table::Table(str table_name, size_t max_size, timespec *lifetime)
  : name(table_name),
    max_tbl_size(max_size),
    _uniqueAggregates(),
    _multAggregates()
{
  if (lifetime != NULL) {
    expiry_lifetime = true;
    max_lifetime = *lifetime;
  } else {
    expiry_lifetime = false;
  }
}

Table::~Table()
{
  // Remove all entries from the deque
  for (size_t i = 0;
       i < els.size();
       i++) {
    Entry * e = els[i];
    els[i] = NULL;
    delete e;
  }

  // And remove all indices
  for (size_t i = 0;
       i < uni_indices.size();
       i++) {
    if (uni_indices[i]) {
      delete uni_indices[i];
    }
  }
  for (size_t i = 0;
       i < mul_indices.size();
       i++) {
    if (mul_indices[i]) {
      delete mul_indices[i];
    }
  }

  // The the expiration
}

//
// Create and destroy indices.  
//
void Table::add_unique_index(unsigned fn)
{
  del_unique_index(fn);
  uni_indices.at(fn) = New UniqueIndex();
}
void Table::del_unique_index(unsigned fn)
{
  uni_indices.reserve(fn);
  while( uni_indices.size() <= fn) {
    uni_indices.push_back(NULL);
  }
  if (uni_indices.at(fn) != NULL) {
    delete uni_indices.at(fn);
  }
}
void Table::add_multiple_index(unsigned fn)
{
  del_multiple_index(fn);
  mul_indices.at(fn) = New MultIndex();
}
void Table::del_multiple_index(unsigned fn)
{
  mul_indices.reserve(fn);
  while( mul_indices.size() <= fn) {
    mul_indices.push_back(NULL);
  }
  if (mul_indices.at(fn) != NULL) {
    delete mul_indices.at(fn);
  }
}

Table::UniqueAggregate
Table::add_unique_groupBy_agg(unsigned keyFieldNo,
                              std::vector< unsigned > groupByFieldNos,
                              unsigned aggFieldNo,
                              Table::AggregateFunction* aggregate)
{
  // Get the index
  UniqueIndex* uI = uni_indices.at(keyFieldNo);
  assert(uI != NULL);
  Table::UniqueAggregate uA =
    New refcounted< Table::UniqueAggregateObj >(keyFieldNo,
                                                uI,
                                                groupByFieldNos,
                                                aggFieldNo,
                                                aggregate);

  
  // Store the aggregate
  _uniqueAggregates.push_back(uA);
  return uA;
}

Table::MultAggregate
Table::add_mult_groupBy_agg(unsigned keyFieldNo,
                            std::vector< unsigned > groupByFieldNos,
                            unsigned aggFieldNo,
                            Table::AggregateFunction* aggregate)
{
  // Get the index
  MultIndex* mI = mul_indices.at(keyFieldNo);
  assert(mI != NULL);
  Table::MultAggregate mA =
    New refcounted< Table::MultAggregateObj >(keyFieldNo,
                                              mI,
                                              groupByFieldNos,
                                              aggFieldNo,
                                              aggregate);

  
  // Store the aggregate
  _multAggregates.push_back(mA);
  return mA;
}




//
// Setting the expiry time for tuples
//
void Table::set_tuple_lifetime(timespec &lifetime)
{
  max_lifetime = lifetime;
  expiry_lifetime = true;
  garbage_collect();
}

//
// Inserting a tuple..
//
void Table::insert(TupleRef t)
{
  Entry *e = New Entry(t);

  // Add to the queue
  els.push_front(e);
  
  // Add to all unique indices.
  for(size_t i = 0;
      i < uni_indices.size();
      i++) {
    UniqueIndex *ndx = uni_indices.at(i);
    if (ndx) {
      // Remove any tuple with the same unique key from all indices and
      // aggregates
      remove(i, (*t)[i]);
      
      // Now store the new tuple.  Make sure it's stored
      std::pair< UniqueIndex::iterator, bool > result =
        ndx->insert(std::make_pair((*e->t)[i], e));
      assert(result.second);
    }
  }

  // Add to all multiple indices
  for(size_t i = 0;
      i < mul_indices.size();
      i++) {
    MultIndex *ndx = mul_indices.at(i);
    if (ndx) {
      ndx->insert(std::make_pair((*e->t)[i], e));
    }
  }

  // And update the aggregates
  for (size_t i = 0;
       i < _uniqueAggregates.size();
       i++) {
    UniqueAggregate uA = _uniqueAggregates[i];
    uA->update(t);
  }

  for (size_t i = 0;
       i < _multAggregates.size();
       i++) {
    MultAggregate mA = _multAggregates[i];
    mA->update(t);
  }

  // And clear up...
  garbage_collect();
}

/**
   Removing a tuple from an index.  It finds the entry in the index and,
   if it exists, removes it from all indices and aggregates.  XXX This
   doesn't remove an entry from the queue; the entry will expire in
   time.
*/
void
Table::remove(unsigned fieldNo, ValueRef key)
{
  UniqueIndex *ndx = uni_indices.at(fieldNo);
  if (!ndx) {
    // This index does not exist
    warn << "Requesting removal from non-existent unique index " << fieldNo
         << " in table " << name << "\n";
    assert(false);
    // We don't get here
    return;
  }

  // Find the entry itself
  UniqueIndex::iterator iter = ndx->find(key);
  if (iter == ndx->end()) {
    // No such key exists.  Do nothing
  } else {
    // Got it
    Entry* theEntry = iter->second;

    // Now eradicate
    remove_from_indices(theEntry);
    remove_from_aggregates(theEntry->t);
  }

  // And clean up while we're at it...
  garbage_collect();
}

//
// Remove an entry from all the indices
//
void Table::remove_from_indices(Entry *e)
{
  for (size_t i = 0;
       i < uni_indices.size();
       i++) {
    UniqueIndex* ndx = uni_indices.at(i);
    ValueRef key = (*e->t)[i];
    if (ndx) {
      // Removal must be identical to how it happens for multiple
      // indices, because an entry in the queue might not in fact still
      // be in this index.  As a result, if the queue contains two
      // entries with the same primary key (as far as this index is
      // concerned), and one of them is removed from this unique index,
      // we must compare not only the keys, but also the entries
      // themselves.
      for (UniqueIndex::iterator pos = ndx->find(key); 
           (pos != ndx->end()) && (pos->first->compareTo(key) == 0); 
           pos++) {
        if (pos->second == e) {
          ndx->erase(pos);
          break;
        }
      }
    }
  }
  for (size_t i = 0;
       i < mul_indices.size();
       i++) {
    MultIndex* ndx = mul_indices.at(i);
    ValueRef key = (*e->t)[i];
    if (ndx) {
      for (MultIndex::iterator pos = ndx->find(key); 
           (pos != ndx->end()) && (pos->first->compareTo(key) == 0); 
           pos++) {
        if (pos->second == e) {
          ndx->erase(pos);
          break;
        }
      }
    }
  }
}

void Table::remove_from_aggregates(TupleRef t)
{
  // And update the unique aggregates
  for (size_t i = 0;
       i < _uniqueAggregates.size();
       i++) {
    UniqueAggregate uA = _uniqueAggregates[i];
    uA->update(t);
  }
  // And update the mult aggregates
  for (size_t i = 0;
       i < _multAggregates.size();
       i++) {
    MultAggregate mA = _multAggregates[i];
    mA->update(t);
  }
}

//
// Garbage collect: remove everything past the expiry time, and/or
// everything beyond the maximum size of the table.
//
void Table::garbage_collect()
{
  if (expiry_lifetime) {
    // Remove everything that has timed out. 
    timespec now;
    clock_gettime(CLOCK_REALTIME, &now);
    
    timespec expiry_time = now - max_lifetime;
    while (els.size() > 0) {
      Entry * back = els.back();

      // is the last one to be dumped?
      if (back->ts < (now-max_lifetime)) {
        // Kick it.
        remove_from_indices(back);
        remove_from_aggregates(back->t);
        els.pop_back();
        delete back;
        back = els.back();
      } else {
        // If the last one is not to be dumped, none will be.  Get out
        // of the while loop.
        break;
      }
    }
  } 
  // Trim the size of the table
  while( els.size() > max_tbl_size ) {
    Entry * back = els.back();
    remove_from_indices(back);
    remove_from_aggregates(back->t);
    els.pop_back();
    delete back;
  }
}


Table::MultIterator
Table::lookupAll(unsigned field, ValueRef key)
{
  garbage_collect();
  Table::MultIndex *ndx = mul_indices.at(field);
  if (ndx) {
    return New refcounted< Table::MultIteratorObj >(ndx, key);
  }
  warn << "Requesting multi lookup in table " << name << " on missing index " << field << "\n";
  assert(false);
}

Table::MultScanIterator
Table::scanAll(unsigned field)
{
  garbage_collect();
  Table::MultIndex *ndx = mul_indices.at(field);
  if (ndx) {
    return New refcounted< Table::MultScanIteratorObj >(ndx);
  }
  warn << "Requesting multi scan in table " << name << " on missing index " << field << "\n";
  assert(false);
}

Table::UniqueIterator
Table::lookup(unsigned field, ValueRef key)  
{
  garbage_collect();
  UniqueIndex *ndx = uni_indices.at(field);
  if (ndx) {
    return New refcounted< Table::UniqueIteratorObj >(ndx, key);
  }
  warn << "Requesting unique lookup in table " << name << " on missing index " << field << "\n";
  assert(false);
}

Table::MultIterator
Table::MultLookupGenerator::lookup(TableRef t, unsigned field, ValueRef key)
{
  return t->lookupAll(field, key);
}

Table::UniqueIterator
Table::UniqueLookupGenerator::lookup(TableRef t, unsigned field, ValueRef key)
{
  return t->lookup(field, key);
}

Table::AggregateFunctionMIN Table::AGG_MIN;
Table::AggregateFunctionMAX Table::AGG_MAX;
Table::AggregateFunctionCOUNT Table::AGG_COUNT;












////////////////////////////////////////////////////////////
// Aggregation functions
////////////////////////////////////////////////////////////



Table::AggregateFunctionMIN::AggregateFunctionMIN()
  : _currentMin(NULL)
{
}

Table::AggregateFunctionMIN::~AggregateFunctionMIN()
{
}
  
void
Table::AggregateFunctionMIN::reset()
{
  _currentMin = NULL;
}
  
void
Table::AggregateFunctionMIN::first(ValueRef v)
{
  _currentMin = v;
}
  
void
Table::AggregateFunctionMIN::process(ValueRef v)
{
  assert(_currentMin != 0);
  if (v->compareTo(_currentMin) < 0) {
    _currentMin = v;
  }
}

ValuePtr 
Table::AggregateFunctionMIN::result()
{
  return _currentMin;
}


Table::AggregateFunctionMAX::AggregateFunctionMAX()
  : _currentMax(NULL)
{
}

Table::AggregateFunctionMAX::~AggregateFunctionMAX()
{
}
  
void
Table::AggregateFunctionMAX::reset()
{
  _currentMax = NULL;
}
  
void
Table::AggregateFunctionMAX::first(ValueRef v)
{
  _currentMax = v;
}
  
void
Table::AggregateFunctionMAX::process(ValueRef v)
{
  if (v->compareTo(_currentMax) > 0) {
    _currentMax = v;
  }
}

ValuePtr 
Table::AggregateFunctionMAX::result()
{
  return _currentMax;
}



Table::AggregateFunctionCOUNT::AggregateFunctionCOUNT()
  : _current(0)
{
}

Table::AggregateFunctionCOUNT::~AggregateFunctionCOUNT()
{
}
  
void
Table::AggregateFunctionCOUNT::reset()
{
  _current = 0;
}
  
void
Table::AggregateFunctionCOUNT::first(ValueRef v)
{
  _current = 1;
}
  
void
Table::AggregateFunctionCOUNT::process(ValueRef v)
{
  _current++;
}

ValuePtr 
Table::AggregateFunctionCOUNT::result()
{
  return Val_UInt64::mk(_current);
}




