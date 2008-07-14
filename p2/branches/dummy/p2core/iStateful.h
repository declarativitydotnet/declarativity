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
 * DESCRIPTION: An agent block in stateful elements managing their
 * existence of content or lack thereof. A stateful element that
 * currently holds no state (i.e., is empty) is STATELESS.
 */

#ifndef _I_STATEFUL_H_
#define _I_STATEFUL_H_
class IStateful {
public:
  enum State {STATEFUL,PENDING,STATELESS};

  IStateful() : _state(STATELESS), _s(0) {};
  virtual ~IStateful() {};

  State state() {return _state;};
  void state(State s) {_state=s;};

  unsigned long size(){return _s;};
  void size(unsigned long s) {_s = s;};

private:
  State _state;
  unsigned long _s;
};
typedef boost::shared_ptr<IStateful> IStatefulPtr;

#endif
