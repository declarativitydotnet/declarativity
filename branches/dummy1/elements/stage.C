// -*- c-basic-offset: 2; related-file-name: "stage.h" -*-
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
 */

#include "stage.h"
#include "stageRegistry.h"
#include "elementRegistry.h"
#include <boost/bind.hpp>
#include "val_str.h"

DEFINE_ELEMENT_INITS(Stage, "Stage")
  
Stage::Stage(string name,
             string processorName)
  : Element(name, 1, 1),
    _pullCallback(0),
    _upstreamBlocked(false),
    _processingBlocked(false)
{
  _pendingInput.reset();
  _processor = 
    StageRegistry::mk(processorName,
                      this);
}


Stage::Stage(TuplePtr args)
  : Element(Val_Str::cast((*args)[2]), 1, 1),
    _pullCallback(0),
    _upstreamBlocked(false),
    _processingBlocked(false)
{
  _pendingInput.reset();
  _processor = 
    StageRegistry::mk(Val_Str::cast((*args)[3]),
                      this);
}


const char*
Stage::processing() const
{
  return "l/l";
}


const char*
Stage::class_name() const
{
  return "Stage";
}


TuplePtr
Stage::pull(int port,
            b_cbv cb) 
{
  // Is this the right port?
  assert(port == 0);

  // Am I blocked?
  if (_pullCallback) {
    // I am blocked. This is an underrun
    ELEM_WARN("pull: callback underrun");
    return TuplePtr();
  } else {
    // I am not blocked.
  
    // Keep trying until I block or return a result
    while (true) {
      // Do I have a pending input?
      if (_pendingInput == 0) {
        // Nope, no pending input. Attempt to fetch a new one.
        
        // Is my upstream element blocked?
        if (_upstreamBlocked) {
          // Yup, he's blocked. Register the downstream element's
          // callback and do nothing.
          _pullCallback = cb;
          return TuplePtr();
        } else {
          // No, he's not blocked. Get a new input tuple.
          _pendingInput = input(0)->pull(boost::bind(&Stage::unblock, this));
          if (_pendingInput != 0) {
            // Got an actual input
            _processor->newInput(_pendingInput);
          } else {
            // He has nothing to give me, and he has my callback. Block
            // my downstream puller as well.
            _upstreamBlocked = true;
            _pullCallback = cb;
            return TuplePtr();
          }
        }
      }
      
      // At this point, we have a pending computation and we are not
      // blocked
      
      // Now try to get an output
      std::pair< TuplePtr, Status > result =
        _processor->newOutput();
      
      if (result.first == 0) {
        // I got an empty result.  Am I done with this input?
        if (result.second == MORE) {
          // There's more outputs to come. Take a callback
          _pullCallback = cb;
          return TuplePtr();
        } else {
          // This is the end of this input. Try again with another input
          _pendingInput.reset();
        }
      } else {
        // This is a real output. Return it happily
        return result.first;
      }
    }
  }
}


void
Stage::unblock()
{
  if (!_upstreamBlocked) {
    // I shouldn't be called if I wasn't blocked.
    ELEM_WARN("unblock: called while not blocked");
  } else {
    // My neighbor was indeed blocked.
    _upstreamBlocked = false;
    
    // Unblock my downstream element. I must have blocked it.
    (_pullCallback)();
    _pullCallback = 0;
  }
}


void
Stage::resume()
{
  if (!_processingBlocked) {
    // I shouldn't be called if my processor wasn't blocked.
    ELEM_WARN("resume: called while not blocked");
  } else {
    // My processor was indeed blocked.
    _processingBlocked = false;
    
    // Unblock my downstream element. I must have blocked it.
    (_pullCallback)();
    _pullCallback = 0;
  }
}


Stage::Processor::Processor(Stage* myStage)
  : _stage(myStage)
{
}

Stage::Processor::~Processor()
{
}
