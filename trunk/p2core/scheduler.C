#include "scheduler.h"
#include <assert.h>
#include "reporting.h"
#include <vector>
#include <string>
#include <loop.h>

#include <iostream>

Scheduler::Scheduler(CommitManager* aMgr)
{
  mpCommitManager = aMgr;
  registerProcess(this);
  TELL_INFO<<"Scheduler initiated!";
}

Scheduler::~Scheduler()
{
}

void
Scheduler::registerQueue(IStatefulPtr qstate, string type)
{
  if (type == "internal") {
    mpIntQ = qstate;
  } 
  else if (type == "external") {
    mpExtQ = qstate;
  }
}

void
Scheduler::registerSwitch(IRunnablePtr r)
{
  assert(r);
  mpSwitch = r;
}



bool Scheduler::stateful(IStatefulPtr aStateful)
{
  assert(aStateful != NULL);
  mStatefuls.insert(aStateful);
  return true;
}

bool Scheduler::runnable(IRunnablePtr aRunnable, string type, int w)
{
  assert(aRunnable != NULL);
  mRunnables.insert(aRunnable);
  aRunnable->state(IRunnable::ACTIVE);
  if(type == "ExtQGateSwitch")
    mpSwitch = aRunnable;
  return true;
}

bool Scheduler::action(CommitManager::ActionPtr action)
{
  assert(action != NULL);
  mpCommitManager->addAction(action);
  return true;
}

bool Scheduler::runRunnables()
{
  bool hasNoRunnables = false;
  bool ranSomething = false;
  assert(!mRunnables.empty());
  /*Run until there is no runnables can be run*/
  while(!hasNoRunnables){
    hasNoRunnables = true;
    //a small opt: always get the switch to run first:
    //if(mp1OffSwitch->getState() == IRunnable::ACTIVE)
	//    mp1OffSwitch->run();
    /*we have to do this per iteration since elements could turn from
      blocked to active after some runnables are run*/
    for(tRunnables::iterator it = mRunnables.begin();
        it != mRunnables.end();
        it++)
    {
      if( (*it)->state() == IRunnable::ACTIVE ){
	//TELL_INFO<<"Running element
	//"<<boost::dynamic_pointer_cast<Element,IRunnable>(*it)->name()<<"\n";
	(*it)->run();
	// Remember if there was anything to run.  (mpSwitch will always be
	// ready the first time through, so ignore it).
	if(*it != mpSwitch) {
	  ranSomething = true;
	} else {
	  // A comment elsewhere in this file suggests that we should
	  // get *exactly one* tuple from mpSwitch per fixpoint.  If
	  // we see more than one, let's crash, and let the hapless
	  // person who hits this assert fix P2's atomicity semantics.
	  assert((*it)->state() != IRunnable::ACTIVE);
	}
      }
      /**Hmm, is it still active?*/
      if( (*it)->state() == IRunnable::ACTIVE  ) {
	hasNoRunnables = false;
      }
    }

    //check it up again in case some people got activated after others!
    for(tRunnables::iterator it = mRunnables.begin();
        it != mRunnables.end() && hasNoRunnables;
        it++) {
      if( (*it)->state() == IRunnable::ACTIVE  ) {
	hasNoRunnables = false;
      }
    }
  }
  // Returning hasNoRunnables was pointless (the loop ensures that
  // it's always "true").  Now we return ranSomething instead.
  //  return hasNoRunnables;
  return ranSomething;
}

void
Scheduler::proc(boost::posix_time::time_duration* waitDuration)
{
  TELL_WORDY << "Entering Scheduler::proc()\n";
  phase1(waitDuration);
  phase2(waitDuration);
  TELL_WORDY << "Leaving Scheduler::proc()\n";
}


void
Scheduler::phase1(boost::posix_time::time_duration* waitDuration)
{
}


void
Scheduler::phase2(boost::posix_time::time_duration* waitDuration)
{
  //run until quiescene
  /*Eric 2007-01-22:
    In current code base, only mRunnables.empty() would
    suffice to ensure quiescene condition.
    However, to incorporate future needs such as  on-disk
    blocks, we might run out of runnables and statefuls
    while the states are still being pulled out of the disk.
  */
  /*
    Rusty 2007-11-26
    The scheduler was blocking unnecessarily.  I've tried to
    remove some of the excess state, and clean this up a bit.

    I'm not convinced this code is correct, but it's faster. :)

    (See XXX below.)

   */
  bool ranSomething = false;
  if (mpExtQ != NULL) {
    bool first = true;
    while(mpExtQ->size() != 0 || first) {
      //turn on the gate for exactly 1 external event to flow
      //through
      mpSwitch->state(IRunnable::ACTIVE);

      first = false; //by turning on the 1off switch

      TELL_INFO << "\n===<FIXPOINT> A BIG MSG FOR A LOCAL FIXPOINT START! ExtQ size=" 
                << mpExtQ->size() << "===\n";
      //start processing it

      /*Run these guys until quiescene*/

      bool newRanSomething = runRunnables();
      if(newRanSomething) ranSomething = true;

      TELL_INFO << "\t ++++ Inner Q: "
		<< mpIntQ->size();
      //fixpoint is reached, commit all updates
      mpCommitManager->commit();
      TELL_INFO << "\t Leaving fixpoint check: IntQ.size() = " << mpIntQ->size() 
                << " ExtQ.size() = " << mpExtQ->size();
      TELL_INFO<<"\n===</FIXPOINT> A BIG & SHINY MSG FOR A LOCAL FIXPOINT END!!===\n";
    }
  }
  if(ranSomething) {
    // OK, we emptied the external event queue.  That (apparently)
    // doesn't mean that there's no more work to be done before the
    // next network event or callback.  Immediately run through the
    // event loop again.

    // XXX I don't understand why this is necessary.  The old code
    // blocks in the same (a similar) way as the old code.  Perhaps
    // we're getting events from somewhere other than the external
    // event queue, and those events aren't part of an atomic
    // fixpoint.
    //                         -Rusty
    TELL_INFO<<"ran something, and resetting wait time to zero\n"<< std::endl;
    *waitDuration = boost::posix_time::seconds(0);
  }

}
