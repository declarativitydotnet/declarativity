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
#include "loop.h"
#include "loggerI.h"

#include "val_str.h"
#include "val_uint64.h"

#include "algorithm"

using namespace opr;

//
// Constructor
//
Table::Table(string table_name, size_t max_size,
             boost::posix_time::time_duration& lifetime)
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
  max_lifetime = boost::posix_time::seconds(-1);
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
  std::vector<unsigned> key;
  key.push_back(fn);
  add_unique_index(key);
}
void Table::add_unique_index(std::vector<unsigned> keys)
{
  del_unique_index(keys);
  uni_indices.push_back(new UniqueIndex());
  uni_indices_keys.push_back(keys); 
}

void Table::del_unique_index(unsigned fn)
{
  std::vector<unsigned> key;
  key.push_back(fn);
  del_unique_index(key);
}

void Table::del_unique_index(std::vector<unsigned> keys)
{
  std::vector<UniqueIndex *>::iterator          i_iter = uni_indices.begin();
  std::vector<std::vector<unsigned> >::iterator k_iter = uni_indices_keys.begin();

  for ( ;i_iter != uni_indices.end() && *k_iter != keys; i_iter++, k_iter++)
	;
  if (i_iter != uni_indices.end()) {
    delete *i_iter;
    uni_indices.erase(i_iter);
    uni_indices_keys.erase(k_iter);
  }
}

void Table::add_multiple_index(unsigned fn)
{
  std::vector<unsigned> key;
  key.push_back(fn);
  add_multiple_index(key);
}
void Table::del_multiple_index(unsigned fn)
{
  std::vector<unsigned> key;
  key.push_back(fn);
  del_multiple_index(key);
}

void Table::add_multiple_index(std::vector<unsigned> keys)
{
  del_multiple_index(keys);
  mul_indices.push_back(new MultIndex());
  mul_indices_keys.push_back(keys);
}
void Table::del_multiple_index(std::vector<unsigned> keys)
{
  std::vector<MultIndex *>::iterator            i_iter = mul_indices.begin();
  std::vector<std::vector<unsigned> >::iterator k_iter = mul_indices_keys.begin();

  for ( ;i_iter != mul_indices.end() && *k_iter != keys; i_iter++, k_iter++)
	;
  if (i_iter != mul_indices.end()) {
    delete *i_iter;
    mul_indices.erase(i_iter);
    mul_indices_keys.erase(k_iter);
  }
}

Table::UniqueAggregate
Table::add_unique_groupBy_agg(unsigned keyFieldNo,
                              std::vector< unsigned > groupByFieldNos,
                              unsigned aggFieldNo,
                              Table::AggregateFunction* aggregate)
{
  std::vector< unsigned > keyFields;
  keyFields.push_back(keyFieldNo);
  return add_unique_groupBy_agg(keyFields, groupByFieldNos, aggFieldNo, aggregate);
}

Table::UniqueAggregate
Table::add_unique_groupBy_agg(std::vector< unsigned > keyFields,
                              std::vector< unsigned > groupByFieldNos,
                              unsigned aggFieldNo,
                              Table::AggregateFunction* aggregate)
{
  // Get the index
  UniqueIndex* uI = find_uni_index(keyFields);
  assert(uI != NULL);
  Table::UniqueAggregate uA =
    Table::UniqueAggregate(new Table::UniqueAggregateObj(keyFields,
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
  std::vector< unsigned > keyFields;
  keyFields.push_back(keyFieldNo);
  return add_mult_groupBy_agg(keyFields, groupByFieldNos, aggFieldNo, aggregate);
}

Table::MultAggregate
Table::add_mult_groupBy_agg(std::vector< unsigned > keyFields,
                            std::vector< unsigned > groupByFieldNos,
                            unsigned aggFieldNo,
                            Table::AggregateFunction* aggregate)
{
  // Get the index
  MultIndex* mI = find_mul_index(keyFields);
  assert(mI != NULL);
  Table::MultAggregate mA =
    Table::MultAggregate(new Table::MultAggregateObj(keyFields,
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
void Table::set_tuple_lifetime(boost::posix_time::time_duration& lifetime)
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
    UniqueIndex*          ndx    = uni_indices[i];
    std::vector<unsigned> fields = uni_indices_keys[i];
    std::vector<ValuePtr> keys;
    for (std::vector<unsigned>::iterator i = fields.begin(); 
         i != fields.end(); i++) {
      if (*i <= t->size()) {
        keys.push_back((*t)[*i]);
      }
      else {
        std::cerr << "ERROR: Table insert range error: tuple does not match table\n";
        return;
      }
    }
    // Remove any tuple with the same unique key from all indices and
    // aggregates
    TuplePtr removedEntry = remove(fields, keys);

    if (removedEntry.get() == NULL || (max_lifetime >= boost::posix_time::seconds(0)) ||
        t->compareTo(removedEntry) != 0) {
      // inform the scan listeners
      for (unsigned i = 0; i < _uni_scans.size(); i++) {
        _uni_scans.at(i)->update(t);
      }
    }

    // Now store the new tuple.  Make sure it's stored
    std::pair< UniqueIndex::iterator, bool > result =
        ndx->insert(std::make_pair(keys, e));
    assert(result.second);
  }

  // Add to all multiple indices
  for(size_t i = 0;
      i < mul_indices.size();
      i++) {
    MultIndex*            ndx    = mul_indices[i];
    std::vector<unsigned> fields = mul_indices_keys[i];
    std::vector<ValuePtr> keys;
    for (std::vector<unsigned>::iterator i = fields.begin(); 
         i != fields.end(); i++) {
      if (*i <= t->size()) {
        keys.push_back((*t)[*i]);
      }
      else {
        std::cerr << "ERROR: Table insert range error: tuple does not match table\n";
        return;
      }
    }

    ndx->insert(std::make_pair(keys, e));
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


/** see Table::remove(std::vector<unsigned>, std::vector<ValuePtr>) */
TuplePtr Table::remove(unsigned field, ValuePtr key) 
{
  std::vector<unsigned> fields;
  std::vector<ValuePtr> keys;
  fields.push_back(field);
  keys.push_back(key);
  return remove(fields, keys);
}

/**
 * Removing a tuple from an index.  It finds the entry in the index and,
 * if it exists, removes it from all indices and aggregates.  XXX This
 * doesn't remove an entry from the queue; the entry will expire in
 * time.
 */
TuplePtr Table::remove(std::vector<unsigned> fields, std::vector<ValuePtr> keys)
{
  // Locate the index based on supported fields (order matters)
  UniqueIndex *ndx = find_uni_index(fields);	
  if (ndx == NULL) {
    // This index does not exist
    warn << "Requesting removal from non-existent unique index "
         << " in table " << name << "\n";
    assert(false);
    // We don't get here   
  }

  UniqueIndex::iterator iter = ndx->find(keys);
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
    UniqueIndex*          ndx  = uni_indices[i];
    std::vector<unsigned> keys = uni_indices_keys[i];
    std::vector<ValuePtr> vkey;
    for (std::vector<unsigned>::iterator iter = keys.begin();
         iter != keys.end(); iter++) 
      vkey.push_back((*e->t)[*iter]);

    // Removal must be identical to how it happens for multiple
    // indices, because an entry in the queue might not in fact still
    // be in this index.  As a result, if the queue contains two
    // entries with the same primary key (as far as this index is
    // concerned), and one of them is removed from this unique index,
    // we must compare not only the keys, but also the entries
    // themselves.
    for (UniqueIndex::iterator pos = ndx->find(vkey); 
         (pos != ndx->end()) && (pos->first == vkey); 
         pos++) {
      if (pos->second == e) {
        ndx->erase(pos);
        break;
      }
    }
  }

  for (size_t i = 0;
       i < mul_indices.size();
       i++) {
    MultIndex*            ndx  = mul_indices[i];
    std::vector<unsigned> keys = mul_indices_keys[i];
    std::vector<ValuePtr> vkey;
    for (std::vector<unsigned>::iterator iter = keys.begin();
         iter != keys.end(); iter++) 
      vkey.push_back((*e->t)[*iter]);

    for (MultIndex::iterator pos = ndx->find(vkey); 
         (pos != ndx->end()) && (pos->first == vkey); 
         pos++) {
      if (pos->second == e) {
        ndx->erase(pos);
        break;
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
  if (max_lifetime.seconds() >= 0) {
    // Remove everything that has timed out. 
    boost::posix_time::ptime now;
    getTime(now);
    
    boost::posix_time::ptime expiryTime;
    expiryTime = now - max_lifetime;
    while (els.size() > 0) {
      Entry * back = els.back();
      
      // is the last one to be dumped?
      if (back->ts < expiryTime) {
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

Table::MultIndex* Table::find_mul_index(std::vector<unsigned>& fields)
{
  for (unsigned i = 0; i < mul_indices_keys.size(); i++) {
    if (mul_indices_keys[i] == fields) { 
      return mul_indices[i];
    }
  }
  return NULL;
}

Table::UniqueIndex* Table::find_uni_index(std::vector<unsigned>& fields) 
{
  for (unsigned i = 0; i < uni_indices_keys.size(); i++) {
    if (uni_indices_keys[i] == fields) {
      return uni_indices[i];
    }
  }
  return NULL;
}


Table::MultIterator
Table::lookupAll(unsigned field, ValuePtr key)
{
  std::vector<unsigned> fields;
  std::vector<ValuePtr> keys;
  fields.push_back(field);
  keys.push_back(key);
  return lookupAll(fields, keys);
}

Table::MultIterator
Table::lookupAll(std::vector<unsigned> fields, std::vector<ValuePtr> keys)
{
  garbage_collect();
  Table::MultIndex *ndx = find_mul_index(fields);

  if (ndx) {
    return Table::MultIterator(new Table::MultIteratorObj(ndx, keys));
  }
  warn << "Requesting multi lookup in table " << name << " on missing index." << "\n";
  assert(false);
}


Table::MultScanIterator
Table::scanAll(unsigned field)
{
  std::vector<unsigned> fields;
  fields.push_back(field);
  return scanAll(fields);
}

Table::MultScanIterator
Table::scanAll(std::vector<unsigned> fields)
{
  garbage_collect();
  Table::MultIndex *ndx = find_mul_index(fields);

  if (ndx) {
    return Table::MultScanIterator(new Table::MultScanIteratorObj(ndx));
  }
  warn << "Requesting multiple scan in table " << name << " on missing index." << "\n";
  assert(false);
}

Table::UniqueScanIterator
Table::uniqueScanAll(unsigned field, bool continuous)
{
  std::vector<unsigned> fields;
  fields.push_back(field);
  return uniqueScanAll(fields, continuous);
}

Table::UniqueScanIterator
Table::uniqueScanAll(std::vector<unsigned> fields, bool continuous)
{
  garbage_collect();
  UniqueIndex *ndx = find_uni_index(fields);

  if (ndx) {
    Table::UniqueScanIterator scanIterator 
      = Table::UniqueScanIterator(new Table::UniqueScanIteratorObj(ndx));
    if (continuous) {
      _uni_scans.push_back(scanIterator);
    }
    return scanIterator;
  }
  warn << "Requesting unique scan in table " << name << " on missing index." << "\n";
  assert(false);
}

Table::UniqueIterator
Table::lookup(unsigned field, ValuePtr key)  
{
  std::vector<unsigned> fields;
  std::vector<ValuePtr> keys;
  fields.push_back(field);
  keys.push_back(key);
  return lookup(fields, keys);
}

Table::UniqueIterator
Table::lookup(std::vector<unsigned> fields, std::vector<ValuePtr> keys)  
{
  garbage_collect();
  UniqueIndex *ndx = find_uni_index(fields);

  if (ndx) {
    return Table::UniqueIterator(new Table::UniqueIteratorObj(ndx, keys));
  }
  warn << "Requesting unique lookup in table " << name << " on missing index." << "\n";
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




