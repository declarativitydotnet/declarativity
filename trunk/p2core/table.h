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
 * DESCRIPTION: Simple table implementation: in memory.
 *
 */

#ifndef __TABLE_H__
#define __TABLE_H__

#include <deque>
#include <map>
#include <vector>

#include "tuple.h"

class Table;
typedef ref<Table> TableRef;
typedef ptr<Table> TablePtr;

/*
 OK, what does a table _do_?

0) Exactly one primary index, which may be unique but does not have to 
1) 0 or more unique field secondary indices.
2) 0 or more multiple-key field secondary indices.
3) max table size.
4) max tuple lifetime.

1) => map(field value -> tuple)
2) => map(field value -> [tuples])
3) => list of tuples in insertion order
4) => list of tuples in insertion order, plus timestamp.

Adding a tuple:
 - locate all tuples with matching unique fields, and delete them
 - add tuple to list head.
 - remove all tuples from list tail.
 - add tuple to each index.

Removing a tuple:
 - remove from the list (random access, by value)
 - remove from each map (random access, by field value)

Removal from maps is therefore _fast_. 

Removal from the list is therefore _slow_ (linear), since we have to
locate the tuple in the list.  And it can't be indexed, since then
we'd have to maintain the index across inserts and deletes. 

We could mark the tuple as erased, twiddle the tuple accounting (for
maximum table size), and lazily delete them as they reach the end.
But this leaves the problem of what to do when new tuples arrive very
quickly.  

*/

class Table {
public:

  // Create a new table.
  //  'name' is an identifying string.
  //  'max_size' is how many tuples it will hold before discarding (FIFO)
  //  'lifetime' is how long to keep tuples for before discarding
  Table(str tableName, size_t max_size, timespec *lifetime=NULL);

  ~Table();

  // Creating and removing indices.  
  // Note that an index can only be created on a single field.  
  // Note also that you should not create one of these _after_ any
  // tuple has been inserted, otherwise unpredicatable things will
  // happen (though this limitation is easily fixed and may be in the
  // future). 
  //
  void add_unique_index(unsigned fn);
  void del_unique_index(unsigned fn); 
  void add_multiple_index(unsigned fn);
  void del_multiple_index(unsigned fn);


  // Setting and removing the expiry time
  void set_tuple_lifetime(timespec &lifetime);
  void unset_tuple_lifetime() { expiry_lifetime = false; };

  // Insert a tuple
  void insert(TupleRef t);

  // How big is the table
  size_t size() { return els.size(); };

  //private:

  str name;
  size_t	max_tbl_size;
  timespec	max_lifetime;
  bool		expiry_lifetime;

  struct Entry {
    TupleRef t;
    timespec ts;
    Entry(TupleRef tp) : t(tp) { clock_gettime(CLOCK_REALTIME,&ts); };
  };
  std::deque<Entry *> els;

  /** My valueref comparison functor type for the indices. */
  struct valueRefCompare
  {
    bool operator()(const ValueRef first,
                    const ValueRef second) const
    {
      return first->compareTo(second) < 0;
    }
  };

  typedef std::multimap< ValueRef, Entry *, Table::valueRefCompare > MultIndex;
  typedef std::map< ValueRef, Entry *, Table::valueRefCompare >  UniqueIndex;

  class AggregateFunction
  {
  public:
    AggregateFunction() {};
    virtual ~AggregateFunction() {};

    /** Clear the state */
    virtual void reset() = 0;

    /** Start a new aggregate computation with a new tuple. */
    virtual void first(ValueRef) = 0;
    
    /** Process a tuple for this function */
    virtual void process(ValueRef) = 0;
    
    /** Retrieve the result for this function. If no tuples have been
        submitted, return NULL. */
    virtual ValuePtr result() = 0;
  };
  
  class AggregateFunctionMIN : public AggregateFunction
  {
  public:
    AggregateFunctionMIN();
    virtual ~AggregateFunctionMIN();

    void reset();

    void first(ValueRef);
    
    void process(ValueRef);
    
    ValuePtr result();

  private:
    ValuePtr _currentMin;
  };

  class AggregateFunctionMAX : public AggregateFunction
  {
  public:
    AggregateFunctionMAX();
    virtual ~AggregateFunctionMAX();

    void reset();

    void first(ValueRef);
    
    void process(ValueRef);
    
    ValuePtr result();

  private:
    ValuePtr _currentMax;
  };

  class AggregateFunctionCOUNT : public AggregateFunction
  {
  public:
    AggregateFunctionCOUNT();
    virtual ~AggregateFunctionCOUNT();

    void reset();

    void first(ValueRef);
    
    void process(ValueRef);
    
    ValuePtr result();

  private:
    uint64_t _current;
  };



  /** A listener */
  typedef callback< void, TupleRef >::ref Listener;


  /** An aggregate state record */
  template < typename _Index >
  class AggregateObj
  {
  public:
    AggregateObj(unsigned,
                 _Index*,
                 std::vector< unsigned >,
                 unsigned,
                 AggregateFunction*);
    
    void addListener(Listener);
    
    /** Update the aggregate given a newly inserted tuple */
    void update(TupleRef);

  private:
    typedef typename _Index::iterator _Iterator;

    /** My key field */
    unsigned _keyField;
    
    /** Which index? */
    _Index * _uIndex;
    
    /** Which group-by fields? XXX This could turn into a vector< bool >
        at some point. */
    std::vector< unsigned > _groupByFields;

    /** My field being aggregated */
    unsigned _aggField;

    /** Which aggregate function? */
    AggregateFunction* _aggregateFn;

    /** My listeners */
    std::vector< Listener > _listeners;

    /** My comparison functor type that compares two tuples for equality
        on the group-by fields. */
    struct CompareGroupByFields
    {
      CompareGroupByFields(std::vector< unsigned > groupBy)
        : _groupBy(groupBy)
      {
      }

      bool operator()(const TupleRef first,
                      const TupleRef second) const
      {
        int groupByMatch = 0;
        for (size_t f = 0;
             (f < _groupBy.size()) &&
               (groupByMatch == 0);
             f++) {
          unsigned fieldNo = _groupBy[f];
          groupByMatch +=
            (*first)[fieldNo]->compareTo((*second)[fieldNo]);
        }
        return (groupByMatch < 0);
      }

      /** On which fields should I compare tuples? */
      std::vector< unsigned > _groupBy;
    };

    /** The actual comparator object */
    CompareGroupByFields _groupByComparator;
    
    /** My map from group-by values to current aggregates.  Used to
        determine aggregate changes during updates. */
    typedef std::map< TupleRef,
                      ValueRef,
                      CompareGroupByFields > AggMap;
    
    /** My current aggregates */
    AggMap _currentAggregates;
  };

public:
  
  typedef AggregateObj< UniqueIndex > UniqueAggregateObj;
  typedef AggregateObj< MultIndex > MultAggregateObj;
  
  typedef ref< UniqueAggregateObj > UniqueAggregate;
  typedef ref< MultAggregateObj > MultAggregate;

  /** Create a group-by aggregation on a unique index.  Every time the
      table is updated, the aggregate is updated as well, creating
      notifications to all aggregate listeners. */
  UniqueAggregate add_unique_groupBy_agg(unsigned,
                                         std::vector< unsigned >,
                                         unsigned,
                                         AggregateFunction*);

  /** Create a group-by aggregation on a mult index.  Every time the
      table is updated, the aggregate is updated as well, creating
      notifications to all aggregate listeners. */
  MultAggregate add_mult_groupBy_agg(unsigned,
                                     std::vector< unsigned >,
                                     unsigned,
                                     AggregateFunction*);



  /** An opaque container for the iterator logic to be used with
      indices. */
  template < typename _Index >
  class IteratorObj {
  public:
    /** Fetch the next tuple pointer, or null if no next tuple exists */
    TuplePtr next();

    /** Is the iterator done? */
    bool done();

    /** The constructor just initializes the iterator */
    IteratorObj(_Index * index,
                ValueRef key);
    
  private:
    /** My index */
    _Index * _index;
    
    /** My lookup key */
    ValueRef _key;
    
    typedef typename _Index::iterator IndexIterator;

    /** My iterator */
    IndexIterator _iter;
  };
  
  /** An opaque container for the iterator logic to be used with
      fully scanned indices. */
  template < typename _Index >
  class ScanIteratorObj {
  public:
    /** Fetch the next tuple pointer, or null if no next tuple exists */
    TuplePtr next();

    /** Reset the iterator to its beginning */
    void reset();

    /** Is the iterator done? */
    bool done();

    /** The constructor just initializes the iterator */
    ScanIteratorObj(_Index * index);
    
  private:
    /** My index */
    _Index * _index;
    
    typedef typename _Index::iterator IndexIterator;

    /** My iterator */
    IndexIterator _iter;
  };
  
public:
  typedef IteratorObj < MultIndex > MultIteratorObj;
  typedef IteratorObj < UniqueIndex > UniqueIteratorObj;
  typedef ScanIteratorObj < MultIndex > MultScanIteratorObj;
  typedef ptr< MultIteratorObj > MultIterator;
  typedef ptr< UniqueIteratorObj > UniqueIterator;
  typedef ptr< MultScanIteratorObj > MultScanIterator;
  
  
  /** A lookup generator for mult indices */
  class MultLookupGenerator {
  public:
    static ptr< MultIteratorObj > lookup(TableRef, unsigned, ValueRef);
  };

  /** A lookup generator for mult indices */
  class UniqueLookupGenerator {
  public:
    static ptr< UniqueIteratorObj > lookup(TableRef, unsigned, ValueRef);
  };

private:
  std::vector<MultIndex *> mul_indices;
  std::vector<UniqueIndex *> uni_indices;

  /** My unique aggregators */
  std::vector< UniqueAggregate > _uniqueAggregates;
  
  /** My mult aggregators */
  std::vector< MultAggregate > _multAggregates;
  
  // Helper function to remove an Entry from all the indices in the system.
  void remove_from_indices(Entry *);

  /** Remove tuple from aggregates */
  void remove_from_aggregates(TupleRef);

  void garbage_collect();

public:
  /** Lookup in a unique index */
  UniqueIterator lookup(unsigned field, ValueRef key);

  /** Lookup in a multi index. */
  MultIterator lookupAll(unsigned field, ValueRef key);

  /** Get all entries in a multi index */
  MultScanIterator scanAll(unsigned field);

  /** Remove the entry found on the given unique index by the given key.
      The removal occurs from all indices. */
  void remove(unsigned field, ValueRef key);

  /** My aggregator functions */
  static AggregateFunctionMIN AGG_MIN;
  static AggregateFunctionMAX AGG_MAX;
  static AggregateFunctionCOUNT AGG_COUNT;
};

#include "iteratorObj.h"
#include "scanIteratorObj.h"
#include "aggregateObj.h"


#endif /* __TABLE_H_ */
