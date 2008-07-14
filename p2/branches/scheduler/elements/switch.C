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

#include "switch.h"

#include "eventLoop.h"
#include "val_str.h"
#include "val_int64.h"
#include "reporting.h"

DEFINE_ELEMENT_INITS(Switch, "Switch")

Switch::Switch(string name, int nTuple)
  : Element(name,1,1),
    pull_cb(boost::bind(&Switch::pull_fn, this)),
    pull_ready(false),
    push_cb(boost::bind(&Switch::push_fn, this)),
    push_ready(true),
    running(true),
    run_cb(boost::bind(&Switch::run, this)),
    mNTuple(nTuple)
{
}

Switch::Switch(TuplePtr args)
  : Element(Val_Str::cast((*args)[2]),1,1),
    pull_cb(boost::bind(&Switch::pull_fn, this)),
    pull_ready(false),
    push_cb(boost::bind(&Switch::push_fn, this)),
    push_ready(true),
    run_cb(boost::bind(&Switch::run, this)),
    running(true),
    mNTuple(0)
{
  if (args->size() > 3) {
    mNTuple = Val_Int64::cast((*args)[3]);
  }
}


int Switch::initialize()
{
  EventLoop::loop()->enqueue_dpc(pull_cb);
  return 0;
}

void Switch::pull_fn()
{
  pull_ready = true;
  run();
}

void Switch::push_fn()
{
  push_ready = true;
  run();
}


void Switch::run()
{
  ELEM_INFO("Running run()!");

  for (int i = 0; running && ( (mNTuple == 0) || (i < mNTuple)); i++) {
    TuplePtr t = input(0)->pull(pull_cb);
    if ( t ) {
      if ( (output(0)->push(t,push_cb)) == 0 ) {
	ELEM_INFO("Switch blocked by push()...");
	push_ready = false;
	break;
      }
    } else { //no pull nor pending tuple available
      ELEM_INFO("Switch blocked on pull");
      pull_ready = false;
      break;
    }
  }
  if (mNTuple) {
    running = false;
  }
}

void Switch::set_state(bool torun)
{
  running = torun;
  if (running) {
    EventLoop::loop()->enqueue_dpc(run_cb);
  }
}
