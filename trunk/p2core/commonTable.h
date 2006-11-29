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
 * DESCRIPTION: Common table base for both expiring tables and for
 * reference counted tables.
 *
 * It has a single primary key (single or multiple-field index).
 *
 * It may have any number of secondary indices.
 *
 * Deletions are always with regards to the primary key.
 *
 * Insertions and deletions do not require a copy of fields into/out of
 * tuples but can be performed leaving appropriate fields in place.
 *
 */

#ifndef __COMMONTABLE_H__
#define __COMMONTABLE_H__

#include "value.h"
#include "tuple.h"
#include <set>
#include "boost/date_time/posix_time/posix_time.hpp"
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include <deque>
#include "errno.h"


class CommonTable {
protected:
  ////////////////////////////////////////////////////////////
  // Tuple wrapper
  ////////////////////////////////////////////////////////////

  /** A tuple wrapper contains a tuple, its time of insertion, and a
      reference counter (each of which is interpreted by the sublass of
      CommonTable).  It is used to place tuples in all indices. */
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




public:
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


  /** A comparator of tuple pointers by a given key. */
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




protected:
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
       tuple ID is the primary key. */
  CommonTable(string tableName,
              Key& key);

  
  /** A destructor. Doesn't clean anything up. The subclass should deal
      with this. */
  virtual ~CommonTable();




protected:
  /** A convenience method for initializing the lookup search entry. It
      ensures the tuple in the entry can accommodate all fields in the
      given key.  XXXX Carry arity with table definition. */
  void
  lookupSearchEntry(Key& key);




public:  
  ////////////////////////////////////////////////////////////
  // Metadata checks
  ////////////////////////////////////////////////////////////

  /** Table size. It returns the number of (findable) tuples within the
      table. */
  uint32_t
  size() const;


  /** Table name. */
  std::string
  name() const;
  
  
  
  ////////////////////////////////////////////////////////////
  // Listeners
  ////////////////////////////////////////////////////////////

  /** A listener */
  typedef boost::function< void (TuplePtr) > Listener;


protected:
  /** Listener vector */
  typedef std::vector< Listener > ListenerVector;


public:
  /** Register update listeners. Every registered listener is called
      whenever a new tuple is inserted into the table.  */
  void
  updateListener(Listener);


  /** Register delete listeners. Every registered listener is called
      whenever a tuple is removed from the table, through explicit
      deletion or due to garbage collection.  */
  void
  removalListener(Listener);


  /** Register refresh listeners. Every registered listener is called
      whenever a tuple's lifetime (but not content) is updated in the
      table, through a reinsertion.  Refresh listeners are never called
      for tuples in tables with infinite size or lifetime */
  void
  refreshListener(Listener);
  

  /** Virtual methods inherited by table2 and refTable */
  virtual bool
  insert(TuplePtr t) = 0;

  virtual bool
  remove(TuplePtr t) = 0;
  


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

    
    /** Retrieve the result for this function. It should never return
        the null pointer. If the result of the aggregate is undefined
        (e.g., when running MIN over an empty set), this should return
        the NULL value (i.e., Val_Null::mk()). */
    virtual ValuePtr
    result() = 0;


    /** What's my name? */
    virtual std::string
    name() = 0;
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


    /** Send out updates conveying my state if I'm all empty */
    void
    evaluateEmpties();


  protected:
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
      is unknown, throws AggFactory::AggregateNotFound. */
  Aggregate
  aggregate(Key& groupBy,
            uint aggregateFieldNo,
            std::string functionName);


protected:
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
    
    
    
    
  protected:
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
  // To string
  ////////////////////////////////////////////////////////////

  /** Turn the table into a string of tuples along the primary key */
  std::string
  toString();




public:
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
  
  /** Create a fresh secondary index. It assume the index does not
      exist */
  void
  createSecondaryIndex(Key& key);


  /** Evaluates all zero-tuple-set aggregates before a table goes into
      operation, since those do not get triggered unless a tuple
      insertion/deletion occurs otherwise. */
  void
  evaluateEmptyAggregates();
  

protected:
  /** My name (human readable). */
  std::string _name;


  /** My primary key. If empty, use the tuple ID. */
  Key _key;


  /** My secondary indices, sorted by key spec. */
  SecondaryIndexIndex _indices;


  /** My aggregates, in a sequence. */
  AggregateVector _aggregates;


  /** A queue holding all secondary index comparator objects for
      elimination during destruction */
  std::deque< KeyedEntryComparator* > _keyedComparators;

  
  /** My primary index */
  PrimaryIndex _primaryIndex;


  /** My update listeners */
  ListenerVector _updateListeners;


  /** My remove listeners */
  ListenerVector _removalListeners;


  /** My refresh listeners */
  ListenerVector _refreshListeners;


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
      primary is not touched. */
  void
  removeDerivatives(TuplePtr t);


  /** Insert a brand new tuple (or its wrapper) into the database
      including all indices. This method *always* causes a new tuple to
      appear within the table and, therefore, always calls any insertion
      listeners. */
  void
  commonInsertTuple(Entry* newEntry,
                    TuplePtr t,
                    PrimaryIndex::iterator position);


  /** Return a secondary index if it exists, or NULL otherwise */
  SecondaryIndex*
  findSecondaryIndex(Key& key);


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
typedef boost::shared_ptr< CommonTable > CommonTablePtr;

#define TABLE_LOG(_table,_reportingLevel,_rest) "CommonTable, " \
  << _table->_name                                              \
  << ", "                                                       \
  << _reportingLevel                                            \
  << ", "                                                       \
  << errno                                                      \
  << ", "                                                       \
  << _rest

#define TABLE_WORDY(_rest) TELL_WORDY           \
  << TABLE_LOG(this, Reporting::WORDY, _rest)   \
    << "\n"

#endif
