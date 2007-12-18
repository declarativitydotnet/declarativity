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
#include "reporting.h"
#include "val_str.h"
#include "val_int32.h"
#include "boost/bind.hpp"
#include "scheduler.h"

DEFINE_ELEMENT_INITS(Switch, "Switch")

Switch::Switch(string name, int nTuple, bool reg)
  : Element(name,1,1),
    _runnable(new Switch::Runnable(boost::bind(&Switch::run, this))),
    mNTuple(nTuple),
    _register(reg),
    mPullUnblock(boost::bind(&Switch::pullWakeup,this)),
    mPushUnblock(boost::bind(&Switch::pushWakeup,this))
{
}

Switch::Switch(TuplePtr args)
  : Element(Val_Str::cast((*args)[2]),1,1),
    _runnable(new Switch::Runnable(boost::bind(&Switch::run, this))),
    mNTuple(0),
    _register(false),
    mPullUnblock(boost::bind(&Switch::pullWakeup,this)),
    mPushUnblock(boost::bind(&Switch::pushWakeup,this))
{
  if (args->size() > 3) {
    mNTuple = Val_Int32::cast((*args)[3]);
  }
  if (args->size() > 4) {
    _register = bool(Val_Int32::cast((*args)[4]));
  }
}


int Switch::initialize()
{
  //register myself with the scheduler.
  if (_register) {
    Plumber::scheduler()->registerSwitch(_runnable);
  }
  Plumber::scheduler()->runnable(_runnable);
  return 0;
}

void Switch::pullWakeup()
{
  _runnable->mPullPending = false;
  if(!_runnable->IRunnable::state() == IRunnable::OFF &&
     !_runnable->mPushPending) {
    _runnable->state(IRunnable::ACTIVE);
  }  
}

void Switch::pushWakeup()
{
  _runnable->mPushPending = false;
  if(!_runnable->IRunnable::state() == IRunnable::OFF &&
     !_runnable->mPullPending) {
    _runnable->state(IRunnable::ACTIVE);
  }
}


void Switch::run()
{
  ELEM_INFO("Running run()!");

  for (int i = 0; !mNTuple || i < mNTuple; i++) {
    TuplePtr t = input(0)->pull(mPullUnblock);
    if (t) { //OK, I've got something to push baby...
      ELEM_INFO("Switch Pulling...");
      int r = output(0)->push(t,mPushUnblock);
      if (r==0) {
	_runnable->mPushPending = true;
	_runnable->state(IRunnable::BLOCKED);
	ELEM_INFO("Switch BLOCKED!");
	return;
      }
    } else { //no pull nor pending tuple available
      _runnable->state(IRunnable::QUIECENE);
      _runnable->mPullPending = true;
      ELEM_INFO("Switch Quiesced");
      return;
    }
  }

  if (mNTuple) {
    _runnable->state(IRunnable::OFF); // We hit our max runnable tuples mark.
  }
}

Switch::Runnable::Runnable(Switch::RunCB cb)
  : mPullPending(false),
    mPushPending(false),
    _runCB(cb)
{
}

void Switch::Runnable::run()
{
  assert(IRunnable::state() == IRunnable::ACTIVE);
  _runCB(); 
}

void Switch::Runnable::state(IRunnable::State state)
{
  if (state == IRunnable::OFF) {
    IRunnable::state(IRunnable::OFF);
  }
  else if( state == IRunnable::ACTIVE ){
    if(!mPushPending) {
      IRunnable::state(state);
    }else assert(0); //If I am active, why blocked?
  }
  else {
    IRunnable::state(IRunnable::BLOCKED);
  }
}
