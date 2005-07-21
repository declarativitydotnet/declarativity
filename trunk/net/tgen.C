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

#include <element.h>
#include <async.h>
#include <math.h>
#include "tgen.h"
#include "val_str.h"
#include "val_uint32.h"
#include "val_tuple.h"

TrafficGen::TrafficGen(str n, uint k, uint r, double s)
  : Element(n, 1, 1), 
    _wakeupCB(wrap(this, &TrafficGen::wakeup)),
    _runTimerCB(wrap(this, &TrafficGen::runTimer)),
    my_key_(k), key_range_(r), total_received_(0)
{
  _seconds = (uint) floor(s);
  s -= _seconds;
  _nseconds = (uint) (s * 1000000000);
}

int TrafficGen::push(int port, TupleRef tp, cbv cb) {
  uint key = getKey(tp);
  if (key == my_key_) {
    total_received_++;
  }
  else if (_timeCallback == NULL) {
    send_q_.push_back(tp);
  }
  else if (!output(0)->push(tp, _wakeupCB)) {
    timecb_remove(_timeCallback);
    _timeCallback = NULL;
  }
  return 1;
}      
    
int TrafficGen::initialize()
{
  log(LoggerI::INFO, 0, "initialize");
  // Schedule my timer
  _timeCallback = delaycb(_seconds, _nseconds, _runTimerCB);
  return 0;
}


void TrafficGen::runTimer()
{
  // remove the timer id
  _timeCallback = NULL;

  // Create a tuple
  TuplePtr tuple = Tuple::mk();
  tuple->append(Val_Str::mk("KEY"));
  tuple->append(Val_UInt32::mk(genLookupKey()));
  tuple->freeze();

  // Attempt to push it
  if (!output(0)->push(tuple, _wakeupCB)) {
    // We have been pushed back.  Don't reschedule wakeup
    log(LoggerI::INFO, 0, "runTimer: sleeping");
  } else {
    // Reschedule me into the future
    log(LoggerI::INFO, 0, "runTimer: rescheduling");
    _timeCallback = delaycb(_seconds, _nseconds, _runTimerCB);
  }
}

void TrafficGen::wakeup()
{
  // I'd better not be already scheduled
  assert(_timeCallback == NULL);

  log(LoggerI::INFO, 0, "wakeup");
  while (!send_q_.empty()) {
    TuplePtr tp = send_q_.front();
    send_q_.pop_front();
    if (!output(0)->push(tp, _wakeupCB)) return;
  }

  // Okey dokey.  Reschedule me into the future
  _timeCallback = delaycb(_seconds, _nseconds, _runTimerCB);
}

REMOVABLE_INLINE uint TrafficGen::genLookupKey() {
  uint k = my_key_;
  while (k != my_key_) k = key_range_ * rand();
  return k;
}

REMOVABLE_INLINE uint TrafficGen::getKey(TuplePtr tp) {
  for (uint i = 0; i < tp->size(); i++) {
    try {
      TupleRef t = Val_Tuple::cast((*tp)[i]); 
      if (Val_Str::cast((*t)[0]) == "KEY") {
        return Val_UInt32::cast((*t)[1]);
      }
    }
    catch (Value::TypeError& e) { } 
  }
  return 0;
}
