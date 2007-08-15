// -*- c-basic-offset: 2; related-file-name: "aggwrap2.h" -*-
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
 * The state machine is as follows:
 *
 * State -1 is the configuration state (initial). During this state
 * group-by fields can be registered, and join callbacks can be
 * obtained.  Once an external tuple is pushed to the element, if in
 * state -1, it is switched away from state -1.  State -1 is never
 * reentered once it has been abandoned.
 *
 * State 0 is the welcoming state, during which the system is not
 * executing an aggregation and is listening for outer event
 * tuples. Once a tuple is pushed from the outer input, the element
 * transitions to state 1.  Inner inputs are not welcome in this state,
 * since no aggregate should be executing.
 *
 * State 1 is a processing state, during which an aggregation is
 * on-going. It no longer accepts pushes on its outer input, but only on
 * its inner input.  For every inner input received, the aggregate is
 * updated.  When inner inputs are concluded, the state reverts back to
 * 0.
 *
 * XXXX If the inner strand has no queues, causing the synchronous
 * production of output, a race may occur with regards to the push
 * method's return value.
 */

#ifdef WIN32
#include "p2_win32.h"
#endif // WIN32

#include "aggwrap2.h"
#include "val_int32.h"
#include "val_str.h"
#include "val_null.h"
#include "loop.h"
#include <boost/bind.hpp>

Aggwrap2::Aggwrap2(std::string name,
                   std::string aggregateFunctionName,
                   uint32_t aggfield,
                   bool starAgg,
                   std::string resultTupleName)
  : Element(name, 2, 2),
    // Config
    _aggField(aggfield),
    _starAgg(starAgg),
    _numJoins(0),
    _comparator(NULL),
    _outerGroupBy(),
    _innerGroupBy(),
    _resultTupleName(resultTupleName),
    // State
    _aggState(CONFIG),
    inner_accepting(true),
    ext_in_cb(0),
    _eventTuple()
{
  ELEM_WORDY("Creating aggwrap "
             << name
             << " on agg function "
             << aggregateFunctionName
             << " with aggregate field number "
             << aggfield
             << (starAgg ? "starred " : "not starred ")
             << "resulting in tuples of name "
             << resultTupleName);

  try {
    _aggregateFn = AggFactory::mk(aggregateFunctionName);
  } catch (AggFactory::AggregateNotFound a) {
    // Couldn't create one. Return no aggregate
    ELEM_ERROR("Could not find an aggregate with name '"
               << a.aggName
               << "'");
  }
}


Aggwrap2::~Aggwrap2()
{
  // Deallocate the comparator
  if (_comparator != NULL) {
    delete _comparator;
  }

  // Deallocate the agg function
  if (_aggregateFn != NULL) {
    delete _aggregateFn;
  }
}


int
Aggwrap2::push(int port, TuplePtr t, b_cbv cb)
{
  ELEM_INFO(" Push: " << port << "," << t->toString());
  // if receive the next one when previous has not finished, then keep
  // in queue?

  // Is this the right port?
  switch (port) {
  case 0:
    ext_in_cb = cb;
    switch(_aggState) {
    case CONFIG:
      // Switching from CONFIG to BUSY. Must finalize setup by turning
      // group-by fields into the appropriate comparator
      _comparator = new CommonTable::KeyedTupleComparator(_innerGroupBy);
      _aggState = IDLE;         // This is gratuitous and just for prim
                                // and proper reasons. Fall through to
                                // next case.
    case IDLE:  // Waiting
      ELEM_WORDY("I received a tuple from outside! " << t->toString());
      assert(inner_accepting);
      _eventTuple = t;
      agg_init();
      inner_accepting =
        output(1)->push(t, boost::bind(&Aggwrap2::int_push_cb, this));
      ELEM_WORDY("My inner_accepting at line "
                 << __LINE__
                 << " became "
                 << inner_accepting);
      break;
    case BUSY:
      ELEM_WARN("Received an overrun outer tuple "
                << t->toString()
                << " while in state "
                << _aggState
                << ".");
      break;
    default:
      ELEM_ERROR("FAIL: Unknown Aggwrap state " << _aggState);
    }     
    ELEM_WORDY(" Block downstream");
    return 0;
  case 1:
    switch(_aggState) {
    case BUSY:
      agg_accum(t);
      break;
    case IDLE:
    case CONFIG:
      ELEM_ERROR("Cannot handle inner tuples while in state "
                 << _aggState
                 << ". Dropping tuple "
                 << t->toString());
      break;
    default:
      ELEM_ERROR("FAIL: Unknown Aggwrap state " << _aggState);
      break;
    } 
    return 1;
  default:
    ELEM_ERROR("Received push of tuple "
               << t->toString()
               << " on unexpected port "
               << port);
    return  0;
  }
}


/** Callback from the inner graph when we can push more tuples.  */
void
Aggwrap2::int_push_cb()
{
  TRACE_FUNCTION;

  if (_aggState != BUSY) {
    // Can only receive callbacks while BUSY
    ELEM_ERROR("Received an inner callback while in state "
               << _aggState);
  } else {
    inner_accepting = true;
      ELEM_WORDY("My inner_accepting at line "
                 << __LINE__
                 << " became "
                 << inner_accepting);
    ELEM_WORDY("Callback from inner graph on successful push"
               << _aggState);
    if (ext_in_cb) {
      ELEM_WORDY("Invoke ext_in_cb");
      ext_in_cb();
      ext_in_cb = 0;
    }
  }
}


/** Completion callback by one of the inner joins */
void
Aggwrap2::comp_cb(int jnum)
{
  TRACE_FUNCTION;
  
  if (_aggState != BUSY) {
    // Can't receive join completions while not busy!
    ELEM_ERROR("Received a join completion callback while in state "
               << _aggState);
  } else {
    ELEM_WORDY("Join " << jnum << " completed.");
    if (jnum == 0) {
      // The first join is done, which means the whole aggregation is
      // done.
      agg_finalize();
    }
  }
}


/** Getting hold of a join completion closure based on comp_cb(). */
b_cbv
Aggwrap2::get_comp_cb()
{
  if (_aggState != CONFIG) {
    // Can't give out completion callbacks after configuration has finished.
    ELEM_ERROR("Received a join completion callback request while in state "
               << _aggState);
    throw ConfigAfterCommencement();
  } else {
    ELEM_WORDY("Joins so far: " << _numJoins + 1);
    return boost::bind(&Aggwrap2::comp_cb, this, _numJoins++);
  }
}


void
Aggwrap2::agg_init() {
  TRACE_FUNCTION;
  _aggState = BUSY;

  _seenTuples = false;
  _aggregateFn->reset();
}


void
Aggwrap2::agg_accum(TuplePtr t) {
  // Have I seen other tuples yet?
  if (!_seenTuples) {
    // This is the first tuple I am aggregating
    if (_starAgg) {
      _aggregateFn->first(Val_Null::mk());
    } else {
      _aggregateFn->first((*t)[_aggField]);
    }
    _seenTuples = true;
  } else {
    // This is not the first tuple I am aggregating
    if (_starAgg) {
      _aggregateFn->process(Val_Null::mk());
    } else {
      _aggregateFn->process((*t)[_aggField]);
    }
  }
}


void
Aggwrap2::agg_finalize() {
  // Fetch the result
  ValuePtr result = _aggregateFn->result();
  
  // No aggregate function should return null pointers. If it needs to
  // return null, it should return a Val_Null.
  if (result.get() == NULL) {
    result = Val_Null::mk();
    ELEM_ERROR("Aggregate Function "
               << _aggregateFn->name()
               << " returned null result. FIX IT!!!");
  }

  // Put together the result tuple.  Map the event tuple to the result
  // tuple via the external group-by map, and introduce the aggregate
  // result in the aggField position.
  TuplePtr resultTuple = Tuple::mk();

  // First should be the tuple name
  resultTuple->append(Val_Str::mk(_resultTupleName));

  // The group by fields that precede the agg field
  for (uint32_t i = 1;
       i < _aggField;
       i++) {
    resultTuple->append((*_eventTuple)[_outerGroupBy[i - 1]]); // the
                                // 0-th element of outer group-by is the
                                // 1st field to include. Since i is a
                                // field counter in the output tuple, we
                                // need to offset by one
  }
  // The agg field
  resultTuple->append(result);
  // The group by fields the follow the agg field
  for (uint32_t i = _aggField + 1;
       i < _outerGroupBy.size() + 2;
       i++) {
    resultTuple->append((*_eventTuple)[_outerGroupBy[i - 2]]);
    // outer group by starts with 0 whereas i started with 1 before the
    // agg field. Add to that the agg field itself, so we need to offset
    // by 2
  }
  resultTuple->freeze();

  // Push the result out
  output(0)->push(resultTuple, 0);

  _eventTuple.reset();
  result.reset();
  _aggState = IDLE;

  // Aggregation complete. Switch back to welcoming state.
  if (ext_in_cb) {
    ELEM_INFO("Invoke push callback for more tuples");
    // accept new tuples to be pushed in via outer regardless of any outputs
    ext_in_cb(); 
    ext_in_cb = 0;
  }
}

void
Aggwrap2::registerGroupbyField(uint32_t outerField)
{ 
  if (_aggState != CONFIG) {
    // Can't give out completion callbacks after configuration has finished.
    ELEM_ERROR("Received a join completion callback request while in state "
               << _aggState);
  } else {
    _outerGroupBy.push_back(outerField);

    uint32_t nextInnerFieldNo = 1 + // skip the tuple name
      _outerGroupBy.size();
    if (nextInnerFieldNo == _aggField) {
      nextInnerFieldNo++;
    }
    _innerGroupBy.push_back(nextInnerFieldNo);

    ELEM_WORDY("Register group by from the event's "
               << outerField
               << " field.");
  }
}
