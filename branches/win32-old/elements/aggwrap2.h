// -*- c-basic-offset: 2; related-file-name: "aggwrap2.C" -*-
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
 * DESCRIPTION: Element that "wraps" a dataflow graph and performs one
 * aggegate calculation for each input tuple.
 *
 * Inputs:  0, push: incoming event tuples
 *          1, push: output from the inner graph
 * Outputs: 0, push: aggregates (external output)
 *          1, push: push events to the inner graph
 *
 * XXX Must remove dependence on external information from Join elements
 * connected via function calls!
 */

#ifndef __AGGWRAP2_H__
#define __AGGWRAP2_H__

#include "element.h"
#include "aggFactory.h"

class Aggwrap2 : public Element { 
public:

  /** The constructor takes the name of the element instance, the name
      of the aggregation function applied, the field number on which to
      apply the aggregation function with regards to the expected input
      schema received from the inner graph on input 1.  When starAgg is
      true, then the aggregation is not done on a field but on the whole
      tuple coming in from the input. */
  Aggwrap2(std::string name,
           std::string aggregateFunctionName,
           uint32_t aggfield,
           bool starAgg,
           std::string resultTupleName);


  /** The destructor, deallocating internal heap state */
  ~Aggwrap2();


  /** What's my class? */
  const char*
  class_name() const {return "Aggwrap2";}


  /** What's my processing personality? Push all around. */
  const char*
  processing() const {return "hh/hh";};


  /** Any flow recommendations? None since no ports are agnostic. */
  const char*
  flow_code() const {return "--/--";}


  /** Register the next event tuple group-by field. The order of
      registration corresponds to the order of appearance in the inner
      tuples, skipping the aggregate field.  For example, if the outer
      event tuple is event(A, B, C, D), and the inner tuple is inner(C,
      B, agg, D), the registration order should be 3, 2, 4. */
  void
  registerGroupbyField(uint32_t eventField); 


  /** Receive a new fixed tuple */
  int
  push(int port, TuplePtr, b_cbv cb);


  /** My completion callback, called by join number jnum when it is done
      producing results */
  void
  comp_cb(int jnum);


  /** Create anew completion callback for the next join */
  b_cbv
  get_comp_cb();


  /** My exception returned whenever an operation that is only available
      during CONFIG is invoked outside CONFIG. */
  struct ConfigAfterCommencement {
  };




private:
  ////////////////////////////////////////////////////////////
  // Configuration
  ////////////////////////////////////////////////////////////

  /** The aggregated field number of my internal inputs */
  uint32_t _aggField;


  /** Is my aggregate expression a DONT_CARE? */
  bool _starAgg;


  /** My number of joins registered so far */
  int _numJoins;
    

  /** A map from a tuple (with a given combination of group-by values)
      to a value containing the latest result for the aggregate on those
      group-by values.  Used to collect results for each group-by value
      combination. */
  typedef std::map< TuplePtr,
                    ValuePtr,
                    CommonTable::KeyedTupleComparator > AggMap;
  
  
  /** My comparator object, used with my aggregate value map */
  CommonTable::KeyedTupleComparator* _comparator;


  /** My outer group-by fields.  Each field number in order corresponds
      to a field of the event tuple to appear in the output tuple
      (skipping the aggregate field, whose position is given
      elsewhere).*/
  CommonTable::Key _outerGroupBy;


  /** My inner group-by fields.  They must be in ascending order.  They
      must not contain the aggregate field.*/
  CommonTable::Key _innerGroupBy;


  /** The name of result tuples. This should be what the inner graph
      delivers in our inner input port as well. */
  std::string _resultTupleName;




  ////////////////////////////////////////////////////////////
  // State
  ////////////////////////////////////////////////////////////

  /** My state identifier enum */
  typedef enum {
    CONFIG,
    IDLE,
    BUSY
  } State;


  /** My aggregation function. */
  CommonTable::AggFunc* _aggregateFn;

  
  /** My aggregation state identifier for the aggwrap state machine. */
  State _aggState;


  /** Am I still accepting inner tuples? */
  bool inner_accepting;


  /** External callbacks from my external input. Call it when I'm ready
      to take more push.  */
  b_cbv ext_in_cb;




  ////////////////////////////////////////////////////////////
  // Subordinate state, reset by state transitions
  ////////////////////////////////////////////////////////////

  /** Have I seen any matching tuples for the current aggregate? */
  bool _seenTuples;


  /** My event tuple that triggered the current aggregation */
  TuplePtr _eventTuple;




  ////////////////////////////////////////////////////////////
  // Aggregate computation
  ////////////////////////////////////////////////////////////

  /** Start a new aggregation. */
  void agg_init();


  /** Process an internal input with the current partial aggregation
      results. */
  void agg_accum(TuplePtr t);


  /** Finalize my aggregation */
  void agg_finalize();




  ////////////////////////////////////////////////////////////
  // Callbacks
  ////////////////////////////////////////////////////////////

  /** My internal push callback */
  void
  int_push_cb();
};

#endif /* __AGGWRAP2_H_ */
