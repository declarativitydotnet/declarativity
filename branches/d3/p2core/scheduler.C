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

/*
void Scheduler::initialize(Plumber::DataflowPtr aDF)
{
  std::vector<ElementSpecPtr>::iterator it = aDF->elements_.begin();
  for(;it!= aDF->elements_.end();it++)
  {
    ElementPtr e( (*it)->element() );
    TELL_INFO<<"Checking Element: Class="<<e->class_name()<<" Instance="<<e->name()<<"\n";
    std::string eclass(e->class_name());

    //If this is a dataflow element, then dig in...
    if("Dataflow" == eclass){
      initialize(boost::dynamic_pointer_cast<Plumber::Dataflow, Element>(e));
    }
    else if("PullPush" == eclass){
      //PullPush* pp = dynamic_cast<PullPush*>(e.get());
      TELL_INFO<<"Adding Runnable "<<e->name()<<"\n";
      addRunnable(boost::any_cast<IRunnablePtr>(e->getProxy()));
    }
    else if( ("Queue" == eclass) || ("RangeLookup" == eclass) || ("Lookup2" == eclass)){
      addStateful(boost::any_cast<IStatefulPtr> (e->getProxy()));
      TELL_INFO<<"Adding Stateful "<<e->name()<<"\n";
      if(e->name() == "SEAInternalQ")
	      mpIntQ = boost::any_cast<IStatefulPtr> (e->getProxy());
      if(e->name() == "SEAExternalQ")
	      mpExtQ = boost::any_cast<IStatefulPtr> (e->getProxy());
    }else if( ("CommitBuf" == eclass) || ("Insert2" == eclass) || ("Delete2" == eclass)){
      TELL_INFO<<"Adding Action! Class="<<e->class_name()<< "Instance = "<<e->name()<<"\n";
      mpCommitManager->addAction(boost::any_cast<CommitManager::ActionPtr>(e->getProxy()));
    }
    else if("ExtQGateSwitch" == e->name()){
      TELL_INFO<<"Found the GATE!"<<e->name()<<"\n";
      mpSwitch = boost::any_cast<IRunnablePtr> (e->getProxy());
      addRunnable(mpSwitch);
    }

  }
}
*/



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

  assert(!mRunnables.empty());
  /*Run until there is no runnables can be run*/
  while(!hasNoRunnables){
    hasNoRunnables = true;
    //a small opt: always get the switch to run first:
    //if(mp1OffSwitch->getState() == IRunnable::ACTIVE)
	//    mp1OffSwitch->run();
    /*we have to do this per iteration since elements could turn from blocked to active after some runnables are runned*/
    for(tRunnables::iterator it = mRunnables.begin();it != mRunnables.end(); it++)
    {
      if( (*it)->state() == IRunnable::ACTIVE ){
	//TELL_INFO<<"Running element "<<boost::dynamic_pointer_cast<Element,IRunnable>(*it)->name()<<"\n";
	(*it)->run();
      }
      /**Hmm, is it still active?*/
      if( (*it)->state() == IRunnable::ACTIVE )
	hasNoRunnables = false;
    }

    //check it up again in case some people got activated after others!
    for(tRunnables::iterator it = mRunnables.begin();it != mRunnables.end();it++)
    {
      if( (*it)->state() == IRunnable::ACTIVE  )
	hasNoRunnables = false;
    }
  }
  return hasNoRunnables;
}

/*
bool Scheduler::runStatefuls()
{
  bool hasNoStatefuls = false;

  if(mStatefuls.empty())
    return true;

  while(!hasNoStatefuls){
    hasNoStatefuls = true;
    for(tStatefuls::iterator it = mStatefuls.begin();it != mStatefuls.end();it++)
    {
      if( (*it)->state() == IStateful::STATEFUL )
        (*it)->run();
      if( (*it)->state() == IStateful::STATEFUL)
        hasNoStatefuls = false;
    }
  }
  return hasNoStatefuls;
}

*/

void Scheduler::proc(boost::posix_time::time_duration* waitDuration)
{
  TELL_WORDY << "Entering Scheduler::proc()\n";
  phase1(waitDuration);
  phase2(waitDuration);
  TELL_WORDY << "Leaving Scheduler::proc()\n";
}

void Scheduler::phase1(boost::posix_time::time_duration* waitDuration)
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

  bool hasNoRunnables = false;
  bool hasNoStates = true;
  if (mpExtQ != NULL) {
    while(!(mpExtQ->size() == 0 &&
            hasNoRunnables &&
            hasNoStates)){
      //turn on the gate for exactly 1 external event to flow
      //through
      mpSwitch->state(IRunnable::ACTIVE);
      hasNoRunnables = false; //by turning on the 1off switch
      TELL_INFO << "\n===<FIXPOINT> A BIG MSG FOR A LOCAL FIXPOINT START! ExtQ size=" 
                << mpExtQ->size() << "===\n";
      //start processing it
      while(!hasNoRunnables){
        /*Run these guys until quiescene*/
        hasNoRunnables = runRunnables();
        TELL_INFO << "\t ++++ Inner Q: "
                  << mpIntQ->size() 
                  << " HasNoRunnables?" << hasNoRunnables << " ...\n";
      }
      //fixpoint is reached, commit all updates
      mpCommitManager->commit();
      TELL_INFO << "\t Leaving fixpoint check: IntQ.size() = " << mpIntQ->size() 
                << " ExtQ.size() = " << mpExtQ->size();
      TELL_INFO<<"\n===</FIXPOINT> A BIG & SHINY MSG FOR A LOCAL FIXPOINT END!!===\n";
    }
  }

}
