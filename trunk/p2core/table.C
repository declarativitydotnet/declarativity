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


#include "trace.h"

//
// Constructor
//
Table::Table(str table_name, size_t max_size, timespec *lifetime)
  : name(table_name), max_tbl_size(max_size)
{
  if (lifetime != NULL) {
    expiry_lifetime = true;
    max_lifetime = *lifetime;
  } else {
    expiry_lifetime = false;
  }
}

//
// Create and destroy indices.  
//
void Table::add_unique_index(int fn)
{
  del_unique_index(fn);
  uni_indices.at(fn) = New UniqueIndex();
}
void Table::del_unique_index(int fn)
{
  uni_indices.reserve(fn);
  if (uni_indices.at(fn) != NULL) {
    delete uni_indices.at(fn);
  }
}
void Table::add_multiple_index(int fn)
{
  del_multiple_index(fn);
  mul_indices.at(fn) = New MultIndex();
}
void Table::del_multiple_index(int fn)
{
  mul_indices.reserve(fn);
  if (mul_indices.at(fn) != NULL) {
    delete mul_indices.at(fn);
  }
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
  for( size_t i=0; i<uni_indices.size(); i++) {
    UniqueIndex *ndx = uni_indices.at(i);
    str key = e->t[i].toString();
    if (ndx) {
      (*ndx)[key] = e;
    }
  }
  // Add to all multiple indices
  for( size_t i=0; i<mul_indices.size(); i++) {
    MultIndex *ndx = mul_indices.at(i);
    str key = e->t[i].toString();
    ndx->insert(std::make_pair(key,e));
  }
  // And clear up...
  garbage_collect();
}

//
// Lookup a tuple
//
TuplePtr Table::lookup(int field, ValueRef key)  
{
  garbage_collect();
  UniqueIndex *ndx = uni_indices.at(field);
  if (ndx) {
    return (*ndx)[key->toString()]->t;
  } else {
    return NULL;
  }
}

//
// Remove an entry from all the indices
//
void Table::remove_from_indices(Entry *e)
{
  for( size_t i=0; i<uni_indices.size(); i++) {
    UniqueIndex *ndx = uni_indices.at(i);
    str key = e->t[i].toString();
    if (ndx) {
      TRC("Removing from index field " << i );
      ndx->erase(key);
    }
  }
  for( size_t i=0; i<mul_indices.size(); i++) {
    MultIndex *ndx = mul_indices.at(i);
    str key = e->t[i].toString();
    for( MultIndex::iterator pos = ndx->find(key); 
	 pos != ndx->end() && pos->second->t[i].toString() != key; 
	 pos++) {
      if ( pos->second == e ) {
	ndx->erase(pos);
	break;
      }
    }
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
    while( els.back()->ts < (now-max_lifetime) ) {
      remove_from_indices(els.back());
      els.pop_back();
    }
  } 
  // Trim the size of the table
  while( els.size() > max_tbl_size ) {
    remove_from_indices(els.back());
    els.pop_back();
  }
}
