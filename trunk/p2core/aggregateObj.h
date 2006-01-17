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
 * DESCRIPTION: Aggregate object template
 *
 */

#ifndef __AGGREGATEOBJ_H__
#define __AGGREGATEOBJ_H__


template< typename _Index >
Table::AggregateObj< _Index >::AggregateObj(unsigned keyField,
                                            _Index* index,
                                            std::vector< unsigned > groupByFields,
                                            unsigned aggField,
                                            Table::AggregateFunction* aggregateFn)
  : _keyField(keyField),
    _uIndex(index),
    _groupByFields(groupByFields),
    _aggField(aggField),
    _aggregateFn(aggregateFn),
    _listeners(),
    _groupByComparator(groupByFields),
    _currentAggregates(_groupByComparator)
{
}

template< typename _Index >
void
Table::AggregateObj< _Index >::addListener(Table::Listener listenerCallback)
{
  _listeners.push_back(listenerCallback);
}

/** XXX For now, updates are done by recomputing the entire aggregate
    over the whole index. */
template < typename _Index >
void
Table::AggregateObj< _Index >::update(TuplePtr t)
{
  // Go through the portions of the index affected by this insertion
  ValuePtr key = (*t)[_keyField];
  bool started = false;
  TuplePtr aMatchingTuple = TuplePtr();
  _aggregateFn->reset();
  for (_Iterator i = _uIndex->lower_bound(key);
       i != _uIndex->upper_bound(key);
       i++) {
    // Fetch the next tuple
    TuplePtr tuple = i->second->t;

    // Does it match the group-by fields?
    bool groupByMatch = true;
    for (size_t f = 0;
         f < _groupByFields.size() && groupByMatch;
         f++) {
      unsigned fieldNo = _groupByFields[f];
      groupByMatch = groupByMatch &&
        ((fieldNo == _keyField) ||
         ((*t)[fieldNo]->compareTo((*tuple)[fieldNo]) == 0));
    }
    if (groupByMatch) {
      if (!started) {
        // Tuple matches group by fields.  Process it
        _aggregateFn->first((*tuple)[_aggField]);
        aMatchingTuple = tuple;
        started = true;
      } else {
        _aggregateFn->process((*tuple)[_aggField]);
      }
    }
  }

  if (!started) {
    // This was a removal that left no aggregate.
    assert(aMatchingTuple == NULL);

    // Remove the remembered aggregate for these group-by values
    _currentAggregates.erase(t);
    
    // And notify no one
  } else {
    ValuePtr result = _aggregateFn->result();
    assert(result != NULL);

    // Is this a new aggregate for these group-by values?
    typename AggMap::iterator remembered = _currentAggregates.find(t);
    if (remembered == _currentAggregates.end()) {
      // we don't have one
    } else {
      // We do have one. Is it the same?
      if (remembered->second->compareTo(result) == 0) {
        // it's the same.  Skip the update
        return;
      } else {
        // The new aggregate is different from the remembered one.
        // Remove the old one
        _currentAggregates.erase(remembered);
      }
    }

    // We're here because we need to remember this newly produced
    // aggregate

    // Put together the resulting tuple, containing the group-by fields
    // and the aggregate
    TuplePtr resultTuple = Tuple::mk();
    for (size_t f = 0;
         f < _groupByFields.size();
         f++) {
      unsigned fieldNo = _groupByFields[f];
      resultTuple->append((*aMatchingTuple)[fieldNo]);
    }
    resultTuple->append(result);
    resultTuple->freeze();
    
    // Remember this new aggregate
    _currentAggregates.insert(std::make_pair(t, result));
    
    // Notify listeners on the newly computed result
    for (size_t i = 0;
         i < _listeners.size();
         i++) {
      Listener listener = _listeners[i];
      listener(resultTuple);
    }
  }
}





#endif /* __AGGREGATEOBJ_H__ */
