// -*- c-basic-offset: 2; related-file-name: "delete.C" -*-
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
 * The delete element.  It has only a single push input on which tuples
 * to be deleted from the table are sent.  XXX For now all deletes
 * succeed.
 * 
 */

#ifndef __DELETE2_H__
#define __DELETE2_H__

#include "element.h"
#include "elementRegistry.h"
#include "commonTable.h"
#include "eventLoop.h"

class Delete2 : public Element {
public:
  Delete2(string name, CommonTablePtr table);
  Delete2(TuplePtr);
  
  const char* class_name() const {return "Delete2";}
  const char* processing() const {return "h/";}
  const char* flow_code() const {return "-/";}

  /** Delete2 a pushed element */
  virtual int push(int port, TuplePtr, b_cbv cb);

  DECLARE_PUBLIC_ELEMENT_INITS

private:

  EventLoop::Closure	_action_cl;
  bool			_action_queued;
  void			_action();

  CommonTablePtr	_table;
  TupleSet		_buffer;

  DECLARE_PRIVATE_ELEMENT_INITS
};


#endif /* __DELETE_H_ */
