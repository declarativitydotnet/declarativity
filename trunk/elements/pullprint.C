// -*- c-basic-offset: 2; related-file-name: "pullprint.h" -*-
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
  _run(0),
  _scheduled(false)
{
}

/** This is only called from another element's task.  It should not call
    run directly */
void PullPrint::wakeup()
{
  // I was signalled from my input that it's OK to pull some more.
  // Schedule myself.  This is only called from someone's callback so
  // it's OK to invoke fast reschedule.

  log(LoggerI::INFO, 0, "wakeup: pull print waking up");
  assert(_scheduled == false && _run == 0);
  _run = delaycb(0, 0, _run_cb);
  _scheduled = true;
}

int PullPrint::initialize()
{
  // Schedule my task to run
  log(LoggerI::INFO, 0, "init");
  wakeup();
  return 0;
}


void PullPrint::run()
{
  assert(_scheduled && (_run != 0));
  // Doesn't care if it receives pull failure.

  // Pull and print.  If pull fails, don't schedule yourself.  If pull
  // succeeds, schedule yourself again.
  log(LoggerI::INFO, 0, "run");
  TuplePtr t = input(0)->pull(_wakeup_cb);
  if (t != NULL) {
    // Print tuple
    std::cout << "PullPrint: " << (t->toString()) << "\n";

    // Keep running. Don't remove it.
    _run = delaycb(0, 0, _run_cb);
  } else {
    log(LoggerI::INFO, 0, "run: pull failed, sleeping.");

    // Didn't get anything
    _run = 0;
    _scheduled = false;
  }
}
