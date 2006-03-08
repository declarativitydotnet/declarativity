// -*- c-basic-offset: 2; related-file-name: "table.h" -*-
/*
 * @(#)$Id$
 *
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * Or
 * UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
 * Berkeley, CA,  94707. Attention: P2 Group.
 * 
 * DESCRIPTION: Table implementation
 *
 */

#include "table.h"
#include "loop.h"

#include "val_str.h"
#include "val_uint64.h"

#include "algorithm"

using namespace opr;

//
// Constructor
//
Table::Table(string table_name, size_t max_size,
             struct timespec& lifetime)
  : name(table_name),
    max_tbl_size(max_size),
    max_lifetime(lifetime),
    _uniqueAggregates(),
    _multAggregates()
{
}

Table::Table(string table_name, size_t max_size)
  : name(table_name),
    max_tbl_size(max_size),
    _uniqueAggregates(),
    _multAggregates()
{
  max_lifetime.tv_sec = -1;
  max_lifetime.tv_nsec = 0;
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
  uni_indices.at(fn) = new UniqueIndex();
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
  mul_indices.at(fn) = new MultIndex();
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
    Table::UniqueAggregate(new Table::UniqueAggregateObj(keyFieldNo,
                                                         uI,
                                                         groupByFieldNos,
                                                         aggFieldNo,
                                                         aggregate));

  
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
    Table::MultAggregate(new Table::MultAggregateObj(keyFieldNo,
                                                     mI,
                                                     groupByFieldNos,
                                                     aggFieldNo,
                                                     aggregate));

  
  // Store the aggregate
  _multAggregates.push_back(mA);
  return mA;
}




//
// Setting the expiry time for tuples
//
void Table::set_tuple_lifetime(struct timespec& lifetime)
{
  max_lifetime = lifetime;
  garbage_collect();
}

//
// Inserting a tuple..
//
void Table::insert(TuplePtr t)
{
  Entry *e = new Entry(t);

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
      TuplePtr removedEntry = remove(i, (*t)[i]);

      if (removedEntry == NULL || (max_lifetime.tv_sec >= 0) ||
	  t->compareTo(removedEntry) != 0) {
	// inform the scan listeners
	for (unsigned i = 0; i < _uni_scans.size(); i++) {
	  _uni_scans.at(i)->update(t);
	}
      }

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
TuplePtr Table::remove(unsigned fieldNo, ValuePtr key)
{
  UniqueIndex *ndx = uni_indices.at(fieldNo);
  if (!ndx) {
    // This index does not exist
    warn << "Requesting removal from non-existent unique index " << fieldNo
         << " in table " << name << "\n";
    assert(false);
    // We don't get here   
  }

  // Find the entry itself
  UniqueIndex::iterator iter = ndx->find(key);
  TuplePtr toRet = TuplePtr();
  if (iter == ndx->end()) {
    // No such key exists.  Do nothing
  } else {
    // Got it
    Entry* theEntry = iter->second;

    // Now eradicate
    remove_from_indices(theEntry);
    remove_from_aggregates(theEntry->t);
    toRet = theEntry->t;
  }

  // And clean up while we're at it...
  garbage_collect();
  return toRet;
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
    ValuePtr key = (*e->t)[i];
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
    ValuePtr key = (*e->t)[i];
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

void Table::remove_from_aggregates(TuplePtr t)
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
  if (max_lifetime.tv_sec >= 0) {
    // Remove everything that has timed out. 
    struct timespec now;
    getTime(now);
    
    struct timespec expiryTime;
    subtract_timespec(expiryTime, now, max_lifetime);
    while (els.size() > 0) {
      Entry * back = els.back();
      
      // is the last one to be dumped?
      if (compare_timespec(back->ts, expiryTime) < 0) {
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
  while(els.size() > max_tbl_size) {
    Entry * back = els.back();
    remove_from_indices(back);
    remove_from_aggregates(back->t);
    els.pop_back();
    delete back;
  }
}


Table::MultIterator
Table::lookupAll(unsigned field, ValuePtr key)
{
  garbage_collect();
  Table::MultIndex *ndx = mul_indices.at(field);
  if (ndx) {
    return Table::MultIterator(new Table::MultIteratorObj(ndx, key));
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
    return Table::MultScanIterator(new Table::MultScanIteratorObj(ndx));
  }
  warn << "Requesting multiple scan in table " << name << " on missing index " << field << "\n";
  assert(false);
}

Table::UniqueScanIterator
Table::uniqueScanAll(unsigned field, bool continuous)
{
  garbage_collect();
  Table::UniqueIndex *ndx = uni_indices.at(field);
  if (ndx) {
    Table::UniqueScanIterator scanIterator 
      = Table::UniqueScanIterator(new Table::UniqueScanIteratorObj(ndx));
    if (continuous) {
      _uni_scans.push_back(scanIterator);
    }
    return scanIterator;
  }
  warn << "Requesting unique scan in table " << name << " on missing index " << field << "\n";
  assert(false);
}

Table::UniqueIterator
Table::lookup(unsigned field, ValuePtr key)  
{
  garbage_collect();
  UniqueIndex *ndx = uni_indices.at(field);
  if (ndx) {
    return Table::UniqueIterator(new Table::UniqueIteratorObj(ndx, key));
  }
  warn << "Requesting unique lookup in table " << name << " on missing index " << field << "\n";
  assert(false);
}

Table::MultIterator
Table::MultLookupGenerator::lookup(TablePtr t, unsigned field, ValuePtr key)
{
  return t->lookupAll(field, key);
}

Table::UniqueIterator
Table::UniqueLookupGenerator::lookup(TablePtr t, unsigned field, ValuePtr key)
{
  return t->lookup(field, key);
}

Table::AggregateFunctionMIN Table::AGG_MIN;
Table::AggregateFunctionMAX Table::AGG_MAX;
Table::AggregateFunctionCOUNT Table::AGG_COUNT;












////////////////////////////////////////////////////////////
// Aggregation functions
////////////////////////////////////////////////////////////



// Simple MIN

Table::AggregateFunctionMIN::AggregateFunctionMIN()
{
}

Table::AggregateFunctionMIN::~AggregateFunctionMIN()
{
}
  
void
Table::AggregateFunctionMIN::reset()
{
  _currentMin.reset();
}
  
void
Table::AggregateFunctionMIN::first(ValuePtr v)
{
  _currentMin = v;
}
  
void
Table::AggregateFunctionMIN::process(ValuePtr v)
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




// k MIN

Table::AggregateFunctionK_MIN::AggregateFunctionK_MIN(uint k)
  : _k(k)
{
  assert(k >= 1);                // At least 1 min kept
}

Table::AggregateFunctionK_MIN::~AggregateFunctionK_MIN()
{
}
  
void
Table::AggregateFunctionK_MIN::reset()
{
  _currentMins.clear();
}
  
void
Table::AggregateFunctionK_MIN::first(ValuePtr v)
{
  _currentMins.push_back(v);
  std::push_heap(_currentMins.begin(), _currentMins.end());
}
  
void
Table::AggregateFunctionK_MIN::process(ValuePtr v)
{
  _currentMins.push_back(v);
  std::push_heap(_currentMins.begin(), _currentMins.end());

  // If I've exceeded size k, pop the smallest element
  if (_currentMins.size() > _k) {
    std::pop_heap(_currentMins.begin(), _currentMins.end());
    _currentMins.pop_back();
  }
}

ValuePtr 
Table::AggregateFunctionK_MIN::result()
{
  return ValuePtr();
}




// Simple MAX

Table::AggregateFunctionMAX::AggregateFunctionMAX()
{
}

Table::AggregateFunctionMAX::~AggregateFunctionMAX()
{
}
  
void
Table::AggregateFunctionMAX::reset()
{
  _currentMax.reset();
}
  
void
Table::AggregateFunctionMAX::first(ValuePtr v)
{
  _currentMax = v;
}
  
void
Table::AggregateFunctionMAX::process(ValuePtr v)
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



// Simple COUNT

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
Table::AggregateFunctionCOUNT::first(ValuePtr v)
{
  _current = 1;
}
  
void
Table::AggregateFunctionCOUNT::process(ValuePtr v)
{
  _current++;
}

ValuePtr 
Table::AggregateFunctionCOUNT::result()
{
  return Val_UInt64::mk(_current);
}




