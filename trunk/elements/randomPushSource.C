// -*- c-basic-offset: 2; related-file-name: "timedPushSource.h" -*-
/*
 * @(#)$Id$
 * 
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 */
 
#include <randomPushSource.h>
#include <element.h>
#include <async.h>
#include <math.h>
#include <cstdlib>

#include "val_str.h"
#include "val_double.h"

RandomPushSource::RandomPushSource(str name, double seconds, int randSeed, int max)
  : Element(name, 0, 1),
    _wakeupCB(wrap(this, &RandomPushSource::wakeup)),
    _runTimerCB(wrap(this, &RandomPushSource::runTimer))
{
  _seconds = (uint) floor(seconds);
  seconds -= _seconds;
  _nseconds = (uint) (seconds * 1000000000);
  srand(randSeed);
  _max = max;
}
    
int RandomPushSource::initialize()
{
  log(LoggerI::INFO, 0, "initialize");
  // Schedule my timer
  _timeCallback = delaycb(_seconds,
                          _nseconds, _runTimerCB);

  return 0;
}


void RandomPushSource::runTimer()
{
  // remove the timer id
  _timeCallback = 0;

  // Create a tuple
  TuplePtr tuple = Tuple::mk();
  if (tuple == 0) {
    // Couldn't create a new tuple.  Bzzzt
    log(LoggerI::ERROR, 0, "runTimer: Failed to create new tuple");
    return;
  }

  //log(LoggerI::INFO, 0, "runTimer: Creating new tuple");
  // Fill it up with the current timeval
  struct timespec t;
  clock_gettime(CLOCK_REALTIME, &t);
  
  tuple->append(Val_Str::mk("Random"));
  tuple->append(Val_Double::mk(rand() % _max));
  tuple->append(Val_Double::mk(rand() % _max));
  tuple->append(Val_Double::mk(rand() % _max));
  tuple->freeze();

  // Attempt to push it
  int result = output(0)->push(tuple, _wakeupCB);
  if (result == 0) {
    // We have been pushed back.  Don't reschedule wakeup
    log(LoggerI::INFO, 0, "runTimer: sleeping");
  } else {
    // Reschedule me into the future
    log(LoggerI::INFO, 0, "runTimer: rescheduling");
    _timeCallback = delaycb(_seconds,
                            _nseconds,
                            _runTimerCB);
  }
}

void RandomPushSource::wakeup()
{
  // I'd better not be already scheduled
  assert(_timeCallback == 0);

  log(LoggerI::INFO, 0, "wakeup");

  // Okey dokey.  Reschedule me into the future
  _timeCallback = delaycb(_seconds,
                          _nseconds,
                          _runTimerCB);
}
