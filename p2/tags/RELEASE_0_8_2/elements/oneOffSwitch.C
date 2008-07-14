// -*- c-basic-offset: 2; related-file-name: "slot.h" -*-
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

#include "oneOffSwitch.h"
#include "val_str.h"

OneOffSwitch::OneOffSwitch(string name)
  : Switch(name,1)
{
}

OneOffSwitch::OneOffSwitch(TuplePtr args)
  : Switch(Val_Str::cast((*args)[2]),1)
{
}

void OneOffSwitch::run()
{
  assert(getState() == IRunnable::ACTIVE);
  Switch::run();
  Switch::turnOff();
  assert(getState() == IRunnable::OFF);
}
