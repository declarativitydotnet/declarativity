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
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>

#include "loop.h"
#include "tuple.h"

class Table;
typedef boost::shared_ptr<Table> TablePtr;

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

  /**  Create a new table.  name is an identifying string.  max_size is
       how many tuples it will hold before discarding (FIFO).  lifetime
       is how long to keep tuples for before discarding; if it has
       negative seconds, it means tuples never expire. */
  Table(string tableName, size_t max_size,
        boost::posix_time::time_duration& lifetime);

  Table(string tableName, size_t max_size, string lifetime);
  
  /** Create a new table with tuples that do not expire. */
  Table(string tableName, size_t max_size);
  
  ~Table();

  // Creating and removing indices.  
  // Note that an index can only be created on a single field.  
  // Note also that you should not create one of these _after_ any
  // tuple has been inserted, otherwise unpredicatable things will
  // happen (though this limitation is easily fixed and may be in the
  // future). 
  //
  void add_unique_index(unsigned key);
  void add_unique_index(std::vector<unsigned> keys);
  void del_unique_index(unsigned key); 
  void del_unique_index(std::vector<unsigned> keys); 
  void add_multiple_index(unsigned key);
  void add_multiple_index(std::vector<unsigned> keys);
  void del_multiple_index(unsigned key);
  void del_multiple_index(std::vector<unsigned> keys);


  // Setting and removing the expiry time
  void set_tuple_lifetime(boost::posix_time::time_duration& newLifetime);
  void unset_tuple_lifetime() { max_lifetime = boost::posix_time::seconds(-1); };

  // Insert a tuple
  void insert(TuplePtr t);

  // How big is the table
  size_t size() { return els.size(); };

  //private:

  string name;
  size_t	max_tbl_size;

  /** For how long do I keep each entry? */
  boost::posix_time::time_duration max_lifetime;

  struct Entry {
    TuplePtr t;
    boost::posix_time::ptime ts;
    Entry(TuplePtr tp) : t(tp) { getTime(ts); };
  };
  std::deque<Entry *> els;

  /** 
   * My valueref comparison functor type for the indices. 
   * Returns true upon reaching a vector element in first that is strictly 
   * less than a vector element in second, in the respective position.
   */
  struct valuesPtrCompare
  {
    bool operator()(const std::vector<ValuePtr> first,
                    const std::vector<ValuePtr> second) const
    {
      assert(first.size() == second.size());
      for (unsigned i = 0; i < first.size(); i++) {
        if ((first[i])->compareTo(second[i]) < 0) {
	  return true;	// first < second
        }
        else if ((first[i])->compareTo(second[i]) > 0) {
          return false;	// second > first
        }
      }
      return false; 	// first == second
    }
  };

  typedef std::multimap< std::vector<ValuePtr>, Entry *, Table::valuesPtrCompare > MultIndex;
  typedef std::map< std::vector<ValuePtr>, Entry *, Table::valuesPtrCompare >  UniqueIndex;
  typedef std::multimap< std::vector<ValuePtr>, TuplePtr, Table::valuesPtrCompare > MultIndexFlat;
  typedef std::map< std::vector<ValuePtr>, TuplePtr, Table::valuesPtrCompare >  UniqueIndexFlat;

  class AggregateFunction
  {
  public:
    AggregateFunction() {};
    virtual ~AggregateFunction() {};

    /** Clear the state */
    virtual void reset() = 0;

    /** Start a new aggregate computation with a new tuple. */
    virtual void first(ValuePtr) = 0;
    
    /** Process a tuple for this function */
    virtual void process(ValuePtr) = 0;
    
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

    void first(ValuePtr);
    
    void process(ValuePtr);
    
    ValuePtr result();

  private:
    ValuePtr _currentMin;
  };


  // k-min

  class AggregateFunctionK_MIN : public AggregateFunction
  {
  public:
    AggregateFunctionK_MIN(uint k);
    virtual ~AggregateFunctionK_MIN();

    void reset();

    void first(ValuePtr);
    
    void process(ValuePtr);
    
    ValuePtr result();

  private:
    /** The number of mins I'm maintaining */
    uint _k;

    /** The vector of mins I'm maintaining */
    std::vector<ValuePtr> _currentMins; 
  };


  // max

  class AggregateFunctionMAX : public AggregateFunction
  {
  public:
    AggregateFunctionMAX();
    virtual ~AggregateFunctionMAX();

    void reset();

    void first(ValuePtr);
    
    void process(ValuePtr);
    
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

    void first(ValuePtr);
    
    void process(ValuePtr);
    
    ValuePtr result();

  private:
    uint64_t _current;
  };



  /** A listener */
  typedef boost::function<void (TuplePtr)> Listener;


  /** An aggregate state record */
  template < typename _Index >
  class AggregateObj
  {
  public:
    AggregateObj(std::vector< unsigned >,
                 _Index*,
                 std::vector< unsigned >,
                 unsigned,
                 AggregateFunction&);
    
    void addListener(Listener);
    
    /** Update the aggregate given a newly inserted tuple */
    void update(TuplePtr);

  private:
    typedef typename _Index::iterator _Iterator;

    /** My key field */
    std::vector<unsigned> _keyFields;
    
    /** Which index? */
    _Index * _uIndex;
    
    /** Which group-by fields? XXX This could turn into a vector< bool >
        at some point. */
    std::vector< unsigned > _groupByFields;

    /** My field being aggregated */
    unsigned _aggField;

    /** Which aggregate function? */
    AggregateFunction& _aggregateFn;

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

      bool operator()(const TuplePtr first,
                      const TuplePtr second) const
      {
        for (size_t f = 0; f < _groupBy.size(); f++) {
          unsigned fieldNo = _groupBy[f];
          if ( ((*first)[fieldNo])->compareTo((*second)[fieldNo]) < 0) {
	    return true;	// first < second
          }
          else if ( ((*first)[fieldNo])->compareTo((*second)[fieldNo]) > 0) {
            return false;	// second > first
          }
        }
        return false;		// first == second
      }

      /** On which fields should I compare tuples? */
      std::vector< unsigned > _groupBy;
    };

    /** The actual comparator object */
    CompareGroupByFields _groupByComparator;
    
    /** My map from group-by values to current aggregates.  Used to
        determine aggregate changes during updates. */
    typedef std::map< TuplePtr,
                      ValuePtr,
                      CompareGroupByFields > AggMap;
    
    /** My current aggregates */
    AggMap _currentAggregates;
  };

public:
  
  typedef AggregateObj< UniqueIndex > UniqueAggregateObj;
  typedef AggregateObj< MultIndex > MultAggregateObj;
  
  typedef boost::shared_ptr< UniqueAggregateObj > UniqueAggregate;
  typedef boost::shared_ptr< MultAggregateObj > MultAggregate;

  /** Create a group-by aggregation on a unique index.  Every time the
      table is updated, the aggregate is updated as well, creating
      notifications to all aggregate listeners. */
  UniqueAggregate add_unique_groupBy_agg(unsigned keyFieldNo,
                                         std::vector< unsigned > groupByFieldNos,
                                         unsigned aggFieldNo,
                                         AggregateFunction& aggregate);

  UniqueAggregate add_unique_groupBy_agg(std::vector< unsigned >,
                                         std::vector< unsigned >,
                                         unsigned,
                                         AggregateFunction&);

  /** Create a group-by aggregation on a mult index.  Every time the
      table is updated, the aggregate is updated as well, creating
      notifications to all aggregate listeners. */
  MultAggregate add_mult_groupBy_agg(unsigned,
                                     std::vector< unsigned >,
                                     unsigned,
                                     AggregateFunction&);

  MultAggregate add_mult_groupBy_agg(std::vector< unsigned >,
                                     std::vector< unsigned >,
                                     unsigned,
                                     AggregateFunction&);



  /** An opaque container for the iterator logic to be used with
      indices. */
  template < typename _Index, typename _FlatIndex >
  class IteratorObj {
  public:
    /** Fetch the next tuple pointer, or null if no next tuple exists */
    TuplePtr next();

    /** Is the iterator done? */
    bool done();

    /** The constructor just initializes the iterator */
    IteratorObj(_Index * index, ValuePtr key);
    IteratorObj(_Index * index, std::vector<ValuePtr> key);
    
  private:
    /** My index */
    _FlatIndex _index;
    
    /** My lookup key */
    std::vector<ValuePtr> _key;
    
    typedef typename _Index::iterator IndexIterator;
    typedef typename _FlatIndex::iterator FlatIndexIterator;

    /** My iterator */
    FlatIndexIterator _iter;
  };
  
  /** An opaque container for the iterator logic to be used with
      fully scanned indices. */
  template < typename _Index, typename _FlatIndex >
  class ScanIteratorObj {
  public:
    /** Fetch the next tuple pointer, or null if no next tuple exists */
    TuplePtr next();

    /** Is the iterator done? */
    bool done();

    /** The constructor just initializes the iterator */
    ScanIteratorObj(_Index * index);

    void update(TuplePtr t);

    void addListener(Table::Listener listenerCallback);

  private:
    /** My flattened index */
    _FlatIndex _index;
    
    typedef typename _Index::iterator IndexIterator;
    typedef typename _FlatIndex::iterator FlatIndexIterator;

    /** My iterator */
    FlatIndexIterator _iter;

    /** For continuous scanner */
    std::vector< Listener > _listeners;
  };
  
public:
  typedef IteratorObj < MultIndex, MultIndexFlat > MultIteratorObj;
  typedef IteratorObj < UniqueIndex, UniqueIndexFlat > UniqueIteratorObj;
  typedef ScanIteratorObj < UniqueIndex, UniqueIndexFlat > UniqueScanIteratorObj;
  typedef ScanIteratorObj < MultIndex, MultIndexFlat > MultScanIteratorObj;
  typedef boost::shared_ptr< MultIteratorObj > MultIterator;
  typedef boost::shared_ptr< UniqueIteratorObj > UniqueIterator;
  typedef boost::shared_ptr< MultScanIteratorObj > MultScanIterator;
  typedef boost::shared_ptr< UniqueScanIteratorObj > UniqueScanIterator;
  
  
  /** A lookup generator for mult indices */
  class MultLookupGenerator {
  public:
    static boost::shared_ptr< MultIteratorObj > lookup(TablePtr, unsigned, ValuePtr);
  };

  /** A lookup generator for mult indices */
  class UniqueLookupGenerator {
  public:
    static boost::shared_ptr< UniqueIteratorObj > lookup(TablePtr, unsigned, ValuePtr);
  };

private:
  std::vector<MultIndex *> mul_indices;
  std::vector<std::vector<unsigned> > mul_indices_keys;

  std::vector<UniqueIndex *> uni_indices;
  std::vector<std::vector<unsigned> > uni_indices_keys;

  /** My unique aggregators */
  std::vector< UniqueAggregate > _uniqueAggregates;
  
  /** My mult aggregators */
  std::vector< MultAggregate > _multAggregates;

  /** Continuous unique scanners */
  std::vector< UniqueScanIterator > _uni_scans;

  // Helper function to remove an Entry from all the indices in the system.
  void remove_from_indices(Entry *);

  /** Remove tuple from aggregates */
  void remove_from_aggregates(TuplePtr);

  void garbage_collect();

  /** Locate the index with the specific key fields 
   *  Return: the index or NULL if it does not exist
   */
  UniqueIndex* find_uni_index(std::vector<unsigned>&); 
  MultIndex*   find_mul_index(std::vector<unsigned>&); 

  /** My aggregator functions */
  static AggregateFunctionMIN AGG_MIN;
  static AggregateFunctionMAX AGG_MAX;
  static AggregateFunctionCOUNT AGG_COUNT;

public:
  /** Lookup in a unique index */
  UniqueIterator lookup(unsigned field, ValuePtr key);
  UniqueIterator lookup(std::vector<unsigned> fields, std::vector<ValuePtr> keys);

  /** Lookup in a multi index. */
  MultIterator lookupAll(unsigned field, ValuePtr key);
  MultIterator lookupAll(std::vector<unsigned> fields, std::vector<ValuePtr> keys);

  /** Get all entries in a unique index */
  MultScanIterator scanAll(unsigned field);
  MultScanIterator scanAll(std::vector<unsigned> fields);
  UniqueScanIterator uniqueScanAll(unsigned field, bool continuous);
  UniqueScanIterator uniqueScanAll(std::vector<unsigned> fields, bool continuous);

  /** Remove the entry found on the given unique index by the given key.
      The removal occurs from all indices. */
  TuplePtr remove(unsigned field, ValuePtr key);
  TuplePtr remove(std::vector<unsigned> fields, std::vector<ValuePtr> keys);

  static AggregateFunction& agg_min()   { return AGG_MIN; }
  static AggregateFunction& agg_max()   { return AGG_MAX; }
  static AggregateFunction& agg_count() { return AGG_COUNT; }
  
};

#include "iteratorObj.h"
#include "scanIteratorObj.h"
#include "aggregateObj.h"


#endif /* __TABLE_H_ */
