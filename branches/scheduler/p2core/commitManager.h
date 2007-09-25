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


/* 
 * The run-command meta-object for all actions!
 * this includes insert,delete, and commit buffer
 *
*/

#ifndef _WORLD_WHEEL_H_
#define _WORLD_WHEEL_H_
#include "element.h"
#include "boost/shared_ptr.hpp"
#include "list"
using namespace std;


class CommitManager{
public:
  CommitManager();

  // This is in essence a stl set with
  // particular actions. See insert2 and delete2
  // for example.
  class Action{
  public:
    virtual ~Action();

    virtual void commit()=0;
    virtual void abort(){_buffer.clear();};

    void addTuple(TuplePtr);
    void removeTuple(TuplePtr);
  protected:
    TupleSet _buffer;
  };
  typedef boost::shared_ptr<Action> ActionPtr;

  void addAction(ActionPtr anAct);
  void removeAction(ActionPtr anAct);

  void commit();
  void commit(ActionPtr);
  void abort();
  void abort(ActionPtr);

protected:
  typedef list<ActionPtr> ActionList;
  ActionList mActionList;

};

#endif
