// -*- c-basic-offset: 2; related-file-name: "element.C" -*-
/*
 * @(#)$Id$
 * 
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 * DESCRIPTION: Element which simply prints any tuple pulled by it
 */

#include "pullprint.h"

#include <iostream>

PullPrint::PullPrint() : 
  Element(1,0),
  _wakeup_cb(wrap(this, &PullPrint::wakeup)),
  _run_cb(wrap(this, &PullPrint::run)),
  _task(this)
{
}

void PullPrint::wakeup()
{
  // I was signalled from my input that it's OK to pull some more.
  // Schedule myself.  This is only called from someone's callback so
  // it's OK to invoke fast reschedule.

  _task.fast_reschedule();
  
  //  lazycb(0, _run_cb);
}

int PullPrint::initialize()
{
  // Schedule my task to run
  _task.initialize(this, true);
}


bool PullPrint::run_task()
{
  // Doesn't care if it receives pull failure.

  // Pull and print.  If pull fails, don't schedule yourself.  If pull
  // succeeds, schedule yourself again.
  TuplePtr t = input(0)->pull(_wakeup_cb);
  if (t != NULL) {
    // Print tuple
    std::cout << "PullPrint: " << (t->toString()) << "\n";

    // Reschedule
    _task.fast_reschedule();
    return true;
  } else {
    // Didn't get anything
    
    // Do nothing
    return false;
  }
}

void PullPrint::run()
{
  TuplePtr t = input(0)->pull(_wakeup_cb);
  if (t != NULL) {
    // Ensure element is still runnable
    std::cout << "PullPrint: " << (t->toString()) << "\n";
  } else {
    // Element is no longer runnable
  }
}

