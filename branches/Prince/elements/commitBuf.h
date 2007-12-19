// -*- c-basic-offset: 2; related-file-name: "insert.C" -*-
/*
 * @(#)$Id$
 * 
 * This file is distributed under the terms in the attached
 * LICENSE file.  If you do not find this file, copies can be
 * found by writing to: Intel Research Berkeley, 2150 Shattuck Avenue,
 * Suite 1300, Berkeley, CA, 94704.  Attention: Intel License Inquiry.
 * Or
 * UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
 * Berkeley, CA,  94707. Attention: P2 Group.
 *
 * The commitBuf element.  It has only a single push input on which tuples
 * to be inserted into the table are sent and a single push output which
 * takes effect only when commitManager gives the signal. .  
 * XXX
 * For now all inserts succeed.  This will change with transactional
 * tables or secondary storage.
 * 
 */

#ifndef __COMMIT_BUF_H__
#define __COMMIT_BUF_H__

#include "element.h"
#include "elementRegistry.h"
#include "commonTable.h"
#include "commitManager.h"
#include "iStateful.h"

class CommitBuf : public Element {
public:
  CommitBuf(string name);
  CommitBuf(TuplePtr);
  
  const char*
  class_name() const {return "CommitBuf";}

  const char*
  processing() const {return "h/h";}

  const char*
  flow_code() const {return "-/-";}
  

  int push(int, TuplePtr, b_cbv);

  DECLARE_PUBLIC_ELEMENT_INITS

private:
  typedef boost::function<void (TupleSet*)> FlushCB;
  class Action : public CommitManager::Action {
  public:
    Action(FlushCB);
  private:
    void commit();
    void abort();

    FlushCB _flush;
  };

  void flush(TupleSet*);
  int initialize();

  CommitManager::ActionPtr _action;

  DECLARE_PRIVATE_ELEMENT_INITS
};


#endif /* __INSERT_H_ */
