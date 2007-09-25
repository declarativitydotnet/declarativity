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

#ifndef _SSS_SCHEDULER_H_
#define _SSS_SCHEDULER_H_
#include "loop.h"
#include "plumber.h"
#include "iRunnable.h"
#include "iStateful.h"
#include "commitManager.h"
#include "set"


/*
 * A scheduler is a process directly hooked
   to the main event loop.

   It will exhaust all runnables registered to
   it at which point the control is passed onto
   the next code in the loop.C.
   *** It is assumed that the DF will terminate
       and the DF does not deadlock.
       Currently, (2007-06-22), this is guaranteed
       by having a internal event queue of
       infinite size.

   For the moment, it does not handle statefuls
   as we do not have states in the DF that could
   be pending such as on-disk tables.
 */

/*
 * Further work might be required if the Plumber
 * decides to have multi-DFs running in isolation
 * with each other. In which case, the way each
 * element now registers itself must be made
 * explicit of DF names which should also be
 * Scheduler names.
 */


class Scheduler : public IProcess{
public:
  Scheduler(CommitManager*);

  virtual ~Scheduler();

  //registers a queue noted with identifier
  //string: "internal" or "external"
  //Currently, this is invoked when a Queue
  //is initialised
  void registerQueue(IStatefulPtr, string);

  //registers the element switch for external
  //internal DF separation
  void registerSwitch(IRunnablePtr r);

  bool stateful(IStatefulPtr aStateful);
  bool runnable(IRunnablePtr aRunnable, string type = "nil", int w =10);

  bool action(CommitManager::ActionPtr a);

  //External methods for direct manipulation of
  //Runnable and Stateful states
  //bool setRunnable(IRunnablePtr aRunnable, IRunnable::State aState);
  //bool setStateful(IStatefulPtr aStateful, IStateful::State aState);

  //IProcess interface
  void proc(boost::posix_time::time_duration*);

private:
  typedef set<IRunnablePtr>  tRunnables;
  typedef set<IStatefulPtr>  tStatefuls;

  tRunnables mRunnables;
  tStatefuls mStatefuls;

  IStatefulPtr mpExtQ;
  IStatefulPtr mpIntQ;
  IRunnablePtr mpSwitch;

  // OneOffSwitch* mp1OffSwitch;
  CommitManager* mpCommitManager;

  bool runRunnables();
  //bool runStatefuls();

  //reserved for statefuls
  void phase1(boost::posix_time::time_duration*);
  //main exhausion code for runnables
  void phase2(boost::posix_time::time_duration*);
};

#endif
