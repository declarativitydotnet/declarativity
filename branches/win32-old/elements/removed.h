// -*- c-basic-offset: 2; related-file-name: "removed.C" -*-
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
 * The removed element.  It listens for removals from a given table and
 * queues them up for pulls from its output port.  It has no input
 * ports.
 * 
 */

#ifndef __REMOVED_H__
#define __REMOVED_H__

#include "commonTable.h"
#include "element.h"
#include "tuple.h"
#include <list>

class Removed : public Element {
public:
  Removed(string name, 
          CommonTablePtr table);
  

  const char*
  class_name() const {return "Removed";}
  
  
  const char*
  processing() const {return "/l";}


  const char*
  flow_code() const {return "/-";}
  

  /** Return a removed tuple from the table, if available. */
  TuplePtr
  pull(int port, b_cbv cb);


  /** Listener for table removals. When a new tuple is removed from the
      table, the table calls this method for notification. */
  void
  listener(TuplePtr t);
  



 private:

  /** My queue of table removals */
  std::list< TuplePtr > _removedBuffer;


  /** Call back for downstream elements */
  b_cbv _pullCB;
};

#endif /* __REMOVED_H_ */
