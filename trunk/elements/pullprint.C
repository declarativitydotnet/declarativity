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
  _run_cb(wrap(this, &PullPrint::run))
{
}

void PullPrint::run()
{
  TuplePtr t = input(0).pull(_wakeup_cb);
  if (t != NULL) {
    // Ensure element is still runnable
    std::cout << "PullPrint: " << (t->toString()) << "\n";
  } else {
    // Element is no longer runnable
  }
}

void PullPrint::wakeup()
{
  lazycb(0, _run_cb);
}
