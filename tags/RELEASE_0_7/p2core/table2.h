/*
 * @(#)$Id$
 *
 * Copyright (c) 2005 Intel Corporation. All rights reserved.
 *
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * Or
 * UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
 * Berkeley, CA,  94707. Attention: P2 Group.
 * 
 * DESCRIPTION: Second-generation in-memory table implementation.
 *
 * It has a single primary key (single or multiple-field index).
 *
 * It may have any number of secondary indices.
 *
 * It has a maximum lifetime of tuples and a maximum size.
 *
 * Insertions of existing tuples change no content, but do update the
 * insertion time of the existing tuple.
 *
 * Deletions are always with regards to the primary key.
 *
 * Insertions and deletions do not require a copy of fields into/out of
 * tuples but can be performed leaving appropriate fields in place.
 *
 */

#ifndef __TABLE2_H__
#define __TABLE2_H__

#include "value.h"
#include "tuple.h"
#include <set>
#include "boost/date_time/posix_time/posix_time.hpp"
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include <deque>


class Table2 {
private:
  ////////////////////////////////////////////////////////////
  // Tuple wrapper
  ////////////////////////////////////////////////////////////

  /** A tuple wrapper contains a tuple and its time of insertion.  It is
      used to place tuples in all indices and in the flushing queue,
      when one is maintained.  It may appear multiple times in the
      flushing queue, once for every reinsertion of the contained
      tuple. */
  struct Entry {
    /** Which tuple am I wrapping? */
    TuplePtr tuple;
    
    
    /** What is the latest time this tuple was inserted (or reinserted)
        into the table? */
    boost::posix_time::ptime time;


    /** How many times does this entry appear in the table queue?  Until
        this count is 0, the entry should not be deleted from the
        heap. */
    uint32_t refCount;
    
    
    /** Create a new entry with the current time */
    Entry(TuplePtr tp);

    
    ~Entry();
  };




public:
  ////////////////////////////////////////////////////////////
  // special vectors
  ////////////////////////////////////////////////////////////

  /** A key is a vector of unsigned field numbers */
  typedef std::vector< unsigned > Key;


  /** Some default keys */
  static Key KEYID;
  static Key KEY0;
  static Key KEY1;
  static Key KEY2;
  static Key KEY3;
  static Key KEY4;
  static Key KEY01;
  static Key KEY12;
  static Key KEY23;
  static Key KEY13;
  static Key KEY012;
  static Key KEY123;
  static Key KEY01234;


  /** The non-expiring expiration time */
  static boost::posix_time::time_duration NO_EXPIRATION;


  /** The default table expiration time */
  static boost::posix_time::time_duration DEFAULT_EXPIRATION;


  /** The non-size-limited size */
  static size_t NO_SIZE;


  /** The default table size */
  static size_t DEFAULT_SIZE;




private:
  ////////////////////////////////////////////////////////////
  // Comparators
  ////////////////////////////////////////////////////////////

  /** A comparator object for key specs */
  struct KeyComparator
  {
    bool
    operator()(const Key& first,
               const Key& second) const;
  };


  /** A comparator of table entry pointers by a given key */
  struct KeyedEntryComparator
  {
    /** What's my key spec? */
    const Key _key;
    
    /** Construct me */
    KeyedEntryComparator(const Key key);
    
    
    /** My comparison operator */
    bool
    operator()(const Entry *,
               const Entry *) const;
  };


  /** A comparator of tuple pointers by a given key. Operates exactly
      like the KeyedEntryComparator, except on tuple pointers without
      the Entry structure indirection. */
  struct KeyedTupleComparator
  {
    /** What's my key spec? */
    const Key _key;
    
    
    /** Construct me */
    KeyedTupleComparator(const Key key);
    
    
    /** My comparison operator */
    bool
    operator()(const TuplePtr,
               const TuplePtr) const;
  };




  ////////////////////////////////////////////////////////////
  // Indices
  ////////////////////////////////////////////////////////////
  
  /** A primary index is a set of Entries sorted by those tuple values
      indicated by a key designation. The key designation will appear
      within the particular KeyedComparator object passed during
      construction */
  typedef std::set< Entry*, KeyedEntryComparator > PrimaryIndex;


  /** A secondary index is a multiset of Entries sorted by those tuple
      values indicated by a key designation. The key designation will
      appear within the particular KeyedComparator object passed during
      construction */
  typedef std::multiset< Entry*, KeyedEntryComparator > SecondaryIndex;


  /** An index of indices secondary indices is a map from key
      designations to secondary indices. */
  typedef std::map< Key, SecondaryIndex*, KeyComparator >
  SecondaryIndexIndex;
  


public:
  ////////////////////////////////////////////////////////////
  // Secondary indices
  ////////////////////////////////////////////////////////////

  /** Create a secondary index on the given key spec. The key spec must
      not be empty. If such an index exists already nothing is done and
      false is returned. Otherwise, true is returned. */
  bool
  secondaryIndex(Key& key);




  


  ////////////////////////////////////////////////////////////
  // Constructors
  ////////////////////////////////////////////////////////////

  /**  Create a new table.  name is an identifying string.  key is a
       vector containing a sequence of field numbers, which make up the
       primary key of the table; an empty key vector means the implicit
       tuple ID is the primary key.  maxSize is how many tuples it will
       hold before discarding (FIFO) and must be non-negative.  Max size
       of 0 means unlimited table size.  lifetime is how long to keep
       tuples for before discarding and must be a positive time
       duration; a lifetime may be positive infinite, indicating no
       expiration. */
  Table2(string tableName,
         Key& key,
         size_t maxSize,
         boost::posix_time::time_duration& lifetime);

  /** A convenience constructor that allows the use of string
      representations for maximum tuple lifetime. */
  Table2(string tableName,
         Key& key,
         size_t maxSize,
         string lifetime);
  

  /** A convenience constructor that does not expire tuples. */
  Table2(string tableName,
         Key& key,
         size_t maxSize);
  

  /** A convenience constructor with no size or time limits. */
  Table2(string tableName,
         Key& key);
  
  
  /** A destructor. It empties out the table and then destroys it. */
  ~Table2();


private:
  /** A convenience method for initializing the lookup search entry. It
      ensures the tuple in the entry can accommodate all fields in the
      given key.  XXXX Carry arity with table definition. */
  void
  lookupSearchEntry(Key& key);




public:
  ////////////////////////////////////////////////////////////
  // Metadata checks
  ////////////////////////////////////////////////////////////

  /** Table size. It returns the number of tuples within the table
      (excluding those that may have been logically deleted but not yet
      physically removed). */
  size_t
  size() const;


  /** Table name. */
  std::string
  name() const;
  
  
  
  
  ////////////////////////////////////////////////////////////
  // Listeners
  ////////////////////////////////////////////////////////////

  /** A listener */
  typedef boost::function< void (TuplePtr) > Listener;


private:
  /** Listener vector */
  typedef std::vector< Listener > ListenerVector;


public:
  /** Register update listeners. Every registered listener is called
      whenever a new tuple is inserted into the table.  */
  void
  updateListener(Listener);
  




  ////////////////////////////////////////////////////////////
  // Aggregation
  ////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////

  /** A container for aggregation functions.  It only contains
      information about the computation of a single aggregate from all
      relevant tuples. */
  class AggFunc
  {
  public:
    AggFunc();
    
    
    virtual ~AggFunc();
    
    
    /** Clear the state of the aggregate function. */
    virtual void
    reset() = 0;

    
    /** Start a new aggregate computation with a new value. */
    virtual void
    first(ValuePtr value) = 0;
    

    /** Process a value for this function (not the first one). */
    virtual void
    process(ValuePtr value) = 0;

    
    /** Retrieve the result for this function. If no tuples have been
        submitted, return NULL. */
    virtual ValuePtr
    result() = 0;
  };
  



  /** An aggregate state record. It contains all relevant state for all
      aggregated groups within a table (i.e., all seen group-by value
      sets).  Whenever it sees an update to the table, it scans the
      relevant secondary index for all tuples with the same group-by
      values as the updated tuple and uses an AggFunc object
      to compute the new value for the aggregate. If that value is
      different from the last value computed for this group-by value
      set, the object remembers it, and sends an update to all
      listeners. */
  class AggregateObj
  {
  public:
    /** Create an aggregate given its key (i.e., the group-by fields)
        a pointer to its table, the aggregated field number (single
        field for now), and the aggregate function object. */
    AggregateObj(Key& key,
                 SecondaryIndex* index,
                 unsigned aggField,
                 AggFunc* function);


    /** Destroy the aggregate object.  This just means its aggregate
        function object */
    ~AggregateObj();
    
    
    /** Add a listener for aggregate updates. */
    void
    listener(Listener listener);

    
    /** Update the aggregate given a newly change tuple.  Specifically,
        the given tuple was either just removed from or just inserted
        into the table. */
    void
    update(TuplePtr changedTuple);


  private:
    /** My index (and group-by fields). */
    Key _key;
    
    
    /** My secondary index. */
    SecondaryIndex* _index;
    

    /** My aggregate (single!) field. */
    unsigned _aggField;
    

    /** Which aggregate function? */
    AggFunc* _aggregateFn;


    /** My listeners */
    ListenerVector _listeners;


    /** My map from a tuple (with a given combination of group-by
        values) to a value containing the latest result for the
        aggregate on those group-by values.  Used to determine aggregate
        changes during updates. */
    typedef std::map< TuplePtr,
                      ValuePtr,
                      KeyedTupleComparator > AggMap;


    /** My comparator object, used with my aggregate value map */
    KeyedTupleComparator _comparator;
    

    /** My current aggregates */
    AggMap _currentAggregates;
  };

  
  /** An externally visible aggregate is a pointer to an aggregate
      object */
  typedef AggregateObj* Aggregate;


  /** Install a new aggregate returning a handle to it. If the aggregate
      is unknown, return NULL. */
  Aggregate
  aggregate(Key& groupBy,
            uint aggregateFieldNo,
            std::string functionName);


private:
  /** My aggregates vector */
  typedef std::vector< Aggregate > AggregateVector;


  



public:
  ////////////////////////////////////////////////////////////
  // Lookup Iterator
  ////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////

  /** An opaque container for the iterator logic to be used with
      indices.  It hides details about the underlying STL index, i.e.,
      whether it's a set or a multiset.  As far as the interface is
      concerned, an index is an index. */
  class IteratorObj {
  public:
    /** Fetch the next tuple pointer, or null if no next tuple exists */
    TuplePtr next();
    
    
    /** Is the iterator done? */
    bool done();
    
    
    /** The constructor just initializes the iterator */
    IteratorObj(std::deque< TuplePtr >* spool);


    /** The destructor kills the spool queue */
    ~IteratorObj();
    
    
    
    
  private:
    /** The iterator's data. This is spooled, instead of read on-line
        from the table, to deal with the following STL interface
        concurrency shortcoming: if an iterator still has elements to
        traverse while the underlying container is concurrently modified
        (e.g., shrunk), there's no way to tell that the iterator is
        viewing an inconsistent view of the container (e.g., pointing at
        a removed element) but instead segfaults. */
    std::deque< TuplePtr >* _spool;
  };


  /** A pointer to lookup iterators */
  typedef boost::shared_ptr< IteratorObj > Iterator;
  
  
  
  
  ////////////////////////////////////////////////////////////
  // Lookups
  ////////////////////////////////////////////////////////////

  /** Looks up tuple t in the index defined by indexKey.  If no such
      index exists, a null result is returned.  The lookup finds all
      elements in the table (searched in the order of the given index)
      whose index field values match the values of t on the lookup field
      values.  For example, lookup(<1, 3>, <2, 4>, t) searches the index
      on key <2, 4>, for all tuples s such that s[2] == t[1] and s[4] ==
      t[3].  The lookup and index keys are assumed to have the same
      size.  If the lookup and index keys are known to be identical,
      lookup(Key, TuplePtr) should be used instead for peformance
      reasons.  */
  Iterator
  lookup(Key& lookupKey, Key& indexKey, TuplePtr t);


  /** Looks up tuple t in the index defined by indexKey.  If no such
      index exists, a null result is returned.  The lookup finds all
      elements in the table (searched in the order of the given index)
      whose index field values match the values of t on the same fields.
      For example, lookup(<1, 3>, t) searches the index on key <1, 3>,
      for all tuples s such that s[1] == t[1] and s[3] == t[3].  This is
      equivalent to lookup(indexKey, indexKey, t), skipping the
      projection implied in the more complex method, however it is
      slightly faster than that method since it does not perform the
      projection. */
  Iterator
  lookup(Key& indexKey, TuplePtr t);


  /** Returns a pointer to a lookup iterator on all elements ordered by
      the given index.  If no such index exists, a null pointer is
      returned.  */
  Iterator
  scan(Key& key);


  /** Returns a pointer to a lookup iterator on all elements ordered by
      the primary index.*/
  Iterator
  scan();
  




  ////////////////////////////////////////////////////////////
  // Updates
  ////////////////////////////////////////////////////////////

  /** Insert a tuple into the table. Return true if the insertion
      modified the table, by growing by a tuple or replacing an existing
      tuple. Return false if the insertion did not affect the table at
      all, e.g., it corresponded to a tuple equal (as per tuple
      comparison) to an existing tuple. However, reinsertion of an
      existing tuple does update the insertion time of that tuple with
      the time of reinsertion.  No update is generated due to insertion
      time updates though.

      The semantics is as follows: find the tuple by the same primary
      key. If it's equal (as per compareTo) do, update the insertion
      time but otherwise do nothing (i.e., update no aggregates or
      listeners, and return false). If it's different, replace it. If it
      doesn't exist, insert the new one in. */
  bool
  insert(TuplePtr t);

  
  /** Remove a tuple from the table. Return true if the removal modified
      the table by shrinking it by one tuple. Return false if the
      removal did not affect the table, e.g., because the given tuple
      was not there to be removed.  Removal happens with regards to the
      primary key of the table.  Therefore, the tuple given the remove
      method and the tuple actually removed from the table may not be
      the same or equal in terms of Tuple::compareTo. */
  bool
  remove(TuplePtr t);


  ////////////////////////////////////////////////////////////
  // To string
  ////////////////////////////////////////////////////////////

  /** Turn the table into a string of tuples along the primary key */
  std::string
  toString();




private:
  /** My name (human readable). */
  std::string _name;


  /** My primary key. If empty, use the tuple ID. */
  Key _key;


  /** My maximum size in tuples. If 0, size is unlimited. */
  size_t _maxSize;

  
  /** My maximum lifetime. It must be positive, and may be positive
      infinity, meaning table entries do not expire. */
  boost::posix_time::time_duration _maxLifetime;


  /** My secondary indices, sorted by key spec. */
  SecondaryIndexIndex _indices;


  /** My aggregates, in a sequence. */
  AggregateVector _aggregates;


  /** A queue holding all secondary index comparator objects for
      elimination during destruction */
  std::deque< KeyedEntryComparator* > _keyedComparators;

  
  /** My primary index */
  PrimaryIndex _primaryIndex;


  /** The time-order queue of entries, for fast garbage collection. This
      is only initialized if the table has a finite expiration time.  A
      given entry may appear multiple times in the queue, once for every
      reinsertion of the tuple.  Specifically, if an existing tuple is
      inserted again, the entry containing that tuple is reinserted in
      the beginning of this queue, with its time updated and its
      refcount incremented by one.  Any other existing instances of the
      entry in the queue remained untouched until they appear in the
      back during flushing.  During flushing, if an entry has a 0
      refcount, it is removed from the queue, from the indices, and from
      the memory heap.  If it has a non-zero refcount, its refcount is
      removed, its instance found is flushed from the queue, but the
      entry and its contained tuple are left alone.*/
  std::deque< Entry * > _queue;


  /** Are we garbage collecting?  We are only garbage collecting if
      entries can be auto-flushed, either due to expiration or due to
      table size limitations. */
  bool _flushing;


  /** My update listeners */
  ListenerVector _updateListeners;


  /** My lookup search entry, used with projecting lookups. It must have
      as many fields as my largest index field number */
  Entry _lookupSearchEntry;




  ////////////////////////////////////////////////////////////
  // Static Initializer
  ////////////////////////////////////////////////////////////

  /** A static initializer object to initialize static class objects */
  class Initializer {
  public:
    Initializer();
  };
  static Initializer
  _INITIALIZER;

  


  ////////////////////////////////////////////////////////////
  // Queue management
  ////////////////////////////////////////////////////////////

  /** Flush expired or supernumerary entries */
  void
  flush();




  ////////////////////////////////////////////////////////////
  // Convenience Functions
  ////////////////////////////////////////////////////////////

  /** Performs a primary key lookup but no flushing. */
  Iterator
  lookupPrimary(Entry* searchEntry);


  /** Performs a secondary key lookup but no flushing. */
  Iterator
  lookupSecondary(Entry* searchEntry,
                  SecondaryIndex& index);


  /** Performs a primary key scan but no flushing. */
  Iterator
  scanPrimary();


  /** Remove an existing tuple (in the primary index position given)
      from the database including all indices. This tuple always causes
      a tuple to be removed from the table and therefore always calls
      any deletion listeners. */
  void
  removeTuple(PrimaryIndex::iterator primaryPosition);


  /** Flush an entry with an existing tuple from the table and all
      indices. */
  void
  flushEntry(Entry* e);

  
  /** Remove a tuple from derivative constructs, including secondary
      indices and aggregates, and update any delete listeners. The
      primary or queue are not touched. */
  void
  removeDerivatives(TuplePtr t);


  /** Updates the insertion time of an existing tuple in the table. */
  void
  updateTime(Entry* e);


  /** Insert a brand new tuple into the database including all
      indices. This method *always* causes a new tuple to appear within
      the table and, therefore, always calls any insertion listeners. */
  void
  insertTuple(TuplePtr t,
              PrimaryIndex::iterator position);


  /** Return a secondary index if it exists, or NULL otherwise */
  SecondaryIndex*
  findSecondaryIndex(Key& key);


  /** Create a fresh secondary index. It assume the index does not
      exist */
  void
  createSecondaryIndex(Key& key);

  
  /** Project a (potentially frozen) source tuple to an unfrozen
      destination tuple, by placing all the fields of the source
      described by the source key into the corresponding fields of the
      destination described by the destination key.  The method assumes
      that the source and destination keys have the same size and that
      the destination tuple is not frozen. */
  void
  project(TuplePtr source,
          Key& sourceKey,
          TuplePtr destination,
          Key& destinationKey);
};


/** A pointer to tables */
typedef boost::shared_ptr< Table2 > Table2Ptr;

#endif
