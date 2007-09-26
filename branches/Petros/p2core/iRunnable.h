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

#ifndef _I_RUNNABLE_H_
#define _I_RUNNABLE_H_
#include <string>


/**
 * Runnable interface for all elements to be runned by the scheduler
 * run() obviously must be implemented as this is the point where
 * scheduler invokes.
 *
 * A runnable must reflect its state into one of the four categories
 * defined before run() is terminated to ensure correct scheduler
 * operation.
 */

class IRunnable {
public:
  /**
   * Quiecene means no pending tuples can be generated
   * Active means still pending for pull or it is...
   * Blocked, when it is active, but cannot push tuples onwards.
   * OFF states that the element is not to be considered
   * for schedulingo at the moment.
   */
  enum State {QUIECENE=0, ACTIVE=1, BLOCKED=2, OFF=3};

  IRunnable() : _state(ACTIVE), _weight(0) {};
  virtual ~IRunnable() {};

  virtual void run() = 0;
  virtual void  weight(int w)  {_weight = w;};
  virtual int   weight()       {return _weight;};
  virtual State state() const  {return _state;};
  virtual void  state(State s) { _state = s; };

private:
  State _state;
  int   _weight;
};

typedef boost::shared_ptr<IRunnable> IRunnablePtr;

#endif
