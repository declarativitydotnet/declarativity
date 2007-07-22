// -*- c-basic-offset: 2; related-file-name: "stage.C" -*-
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
 * DESCRIPTION: Base class for external P2 stages.
 *
 * A stage is a single pull input, single pull output element. It
 * encapsulates a single tuple processor that generates 0, 1, or more
 * results for every input tuple it receives.
 *
 */

#ifndef _STAGE_H_
#define _STAGE_H_

#include "element.h"
#include "tuple.h"
#include <utility>

class Stage : public Element {
public:
  /** The status codes returned by newOutput() */
  enum Status {
    MORE,
    DONE
  };


  /** A tuple processor class. It should use the stage pointer sparing,
      only to invoke Stage::resume(). It should absolutely NOT call
      push, pull, etc. */
  class Processor {
  public:
    /** Construct a tuple processor by giving it a callback to the
        stage's resume method */
    Processor(Stage* myStage);


    /** Destroy the processor */
    virtual ~Processor();
    

    /** Start processing on a new input tuple.  This method should not
        call pull or push. It should just initialize the computation and
        let the actual production of tuples to the newOutput() method.*/
    virtual void
    newInput(TuplePtr inputTuple) = 0;


    /** Attempt to produce another output tuple along with its
        status. The result tuple may be null, in which case the puller
        will be blocked.  If the status is DONE, then the stage will
        proceed to its next input from its upstream element. */
    virtual std::pair< TuplePtr, Status >
    newOutput() = 0;



    
  protected:
    /** My containing stage */
    Stage* _stage;
  };


  /** Constructor takes only the name of this element and the name of
      the intended processor for the stage. */
  Stage(string name,
        string processorName);


  const char*
  processing() const;


  const char*
  class_name() const;


  /** The pull method of the stage. There's no push method. */
  TuplePtr
  pull(int port, b_cbv cb);


  /** Resume a pending computation that blocked inside newOutput(). This
      is typically used only by stage implementations that use private
      timers to wake up later, e.g., to do polling of a socket. */
  void
  resume();
  



private:
  /** My downstream puller's callback, to be called when I can generate
      more output */
  b_cbv _pullCallback;


  /** Do I have a pending computation? */
  TuplePtr _pendingInput;


  /** Is my upstream element blocked? */
  bool _upstreamBlocked;


  /** Is my processor blocked? */
  bool _processingBlocked;


  /** My callback for my upstream neighbor to call when I can pull
      another input */
  void
  unblock();


  /** My processor */
  Processor* _processor;
};

#define STAGE_LOG(_sev,_errnum,_rest) do { ostringstream _sb; _sb << _rest; _stage->log(_sev,_errnum,_sb.str()); } while (false)


#define STAGE_INFO(_rest) STAGE_LOG(Reporting::INFO, 0, _rest)
#define STAGE_WORDY(_rest) STAGE_LOG(Reporting::WORDY, 0, _rest)
#define STAGE_WARN(_rest) STAGE_LOG(Reporting::WARN, 0, _rest)
#define STAGE_ERROR(_rest) STAGE_LOG(Reporting::ERROR, 0, _rest)
#define STAGE_OUTPUT(_rest) STAGE_LOG(Reporting::OUTPUT, 0, _rest)


#endif // _STAGE_H_
