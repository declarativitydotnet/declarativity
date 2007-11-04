// -*- c-basic-offset: 2; related-file-name: "timedPullPush.h" -*-
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

#include "pullPush.h"

#include "eventLoop.h"
#include "val_str.h"
#include "val_int64.h"
#include "reporting.h"

DEFINE_ELEMENT_INITS(PullPush, "PullPush");

//
// We initialize the element with pull_ready = false (but with a DPC
// enqueued to set it to true - see initialize below), but with
// push_ready = true.  As soon as a tuple becomes available for
// pulling, the element will go off and push it. 
//
PullPush::PullPush(string name)
  : Element(name, 1, 1),
    pull_cb(boost::bind(&PullPush::pull_fn, this)),
    pull_ready(false),
    push_cb(boost::bind(&PullPush::push_fn, this)),
    push_ready(true),
    run_cb(boost::bind(&PullPush::run, this))
{
}

PullPush::PullPush(TuplePtr args)
  : Element(Val_Str::cast((*args)[2]),1,1),
    pull_cb(boost::bind(&PullPush::pull_fn, this)),
    pull_ready(false),
    push_cb(boost::bind(&PullPush::push_fn, this)),
    push_ready(true),
    run_cb(boost::bind(&PullPush::run, this))
{
}

int PullPush::initialize()
{
  ELEM_INFO("Initialising");
  // What's this?  Well, this causes a not-so-spurious pull wakeup at
  // start of day as soon as the element is activated and an event
  // arrives.  This ensures that the pull process can "get started",
  // thereafter the element is either waiting for a pull or push
  // callback which it has registered, or it's actually executing in
  // the run loop. 
  EventLoop::loop()->enqueue_dpc(pull_cb);
  return 0;
}

void PullPush::pull_fn()
{
  pull_ready = true;
  EventLoop::loop()->enqueue_dpc(run_cb);
}

void PullPush::push_fn()
{
  push_ready = true;
  EventLoop::loop()->enqueue_dpc(run_cb);
}

void PullPush::run() {
  ELEM_INFO("PullPush running..."); 

  while (pull_ready && push_ready) {
    TuplePtr p = input(0)->pull(pull_cb);
    if ( p ) {
      if ( (output(0)->push(p, push_cb)) == 0 ) {
	ELEM_INFO("PullPush blocked by push()...");
        push_ready = false;
	break;
      }
    } else {
      ELEM_INFO("PullPush blocked on pull");
      pull_ready = false;
      break;
    }
  }
}
