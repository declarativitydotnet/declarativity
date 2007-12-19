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

#include <pullPush.h>
#include <element.h>
#include <math.h>

#include "loop.h"
#include "val_str.h"
#include "val_int64.h"
#include "reporting.h"
#include "scheduler.h"
#include "boost/bind.hpp"

DEFINE_ELEMENT_INITS(PullPush, "PullPush")

PullPush::PullPush(string name)
  : Element(name, 1, 1),
    _runnable(new PullPush::Runnable(boost::bind(&PullPush::run, this))),
    _unblockPull(boost::bind(&PullPush::pullWakeup, this)),
    _unblockPush(boost::bind(&PullPush::pushWakeup, this))
{
}

PullPush::PullPush(TuplePtr args)
  : Element(Val_Str::cast((*args)[2]),1,1),
    _runnable(new PullPush::Runnable(boost::bind(&PullPush::run, this))),
    _unblockPull(boost::bind(&PullPush::pullWakeup, this)),
    _unblockPush(boost::bind(&PullPush::pushWakeup, this))
{
}

int PullPush::initialize()
{
  ELEM_INFO("Initialising");
  Plumber::scheduler()->runnable(_runnable);
  return 0;
}

void PullPush::run() {
  ELEM_INFO("PullPush running..."); 

  //keep pushing until either blocked or quiesced
  while(true) {
    TuplePtr p = input(0)->pull(_unblockPull);
    if(p){
      int r = output(0)->push(p, _unblockPush);
      if(r==0){
	ELEM_INFO("PullPush BLOCKED!");
        _runnable->_pushPending = true;
	//as the element itself does not require such info
	//states are stored in the proxy object for that purpose
        _runnable->state(IRunnable::BLOCKED);
        return;
      }
    }else{
      ELEM_INFO("PullPush QUIESCED!");
      _runnable->_pullPending = true;
      _runnable->state(IRunnable::QUIECENE);
      return;
    }
  }
}

void PullPush::pullWakeup()
{
  _runnable->_pullPending = false;
  //new input and downstream clear, signal active
  if (!_runnable->_pushPending) {
    _runnable->state(IRunnable::ACTIVE);
  }
}

void PullPush::pushWakeup()
{
  _runnable->_pushPending = false;
  //downward available, input possible, signal active
  if (!_runnable->_pullPending) {
    _runnable->state(IRunnable::ACTIVE);
  }
}

PullPush::Runnable::Runnable(PullPush::RunCB cb) 
  : _pullPending(false),_pushPending(false), _runCB(cb)
{
}

void PullPush::Runnable::run()
{
  assert(IRunnable::state() == IRunnable::ACTIVE);
  _runCB(); 
}


//State code for the proxy object
void PullPush::Runnable::state(IRunnable::State state)
{
  if (state == IRunnable::OFF) {
    IRunnable::state(IRunnable::OFF);
  }
  else if( (state == IRunnable::ACTIVE) ){
    if(!(_pullPending || _pushPending)) {
      IRunnable::state(state);
    } else {
      assert(0);//for debug purposes
    }
  }
  else {
    IRunnable::state(IRunnable::BLOCKED);
  }
}
