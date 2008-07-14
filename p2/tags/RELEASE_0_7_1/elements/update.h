// -*- c-basic-offset: 2; related-file-name: "update.C" -*-
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
 * The update element.  It listens for updates to a given table and
 * queues them up for pulls from its output port.  It has no input
 * ports.
 * 
 */

#ifndef __UPDATE_H__
#define __UPDATE_H__

#include "table2.h"
#include "element.h"
#include "tuple.h"
#include <list>

class Update : public Element {
public:
  Update(string name, 
         Table2Ptr table);
  

  const char*
  class_name() const {return "Update";}


  const char*
  processing() const {return "/l";}


  const char*
  flow_code() const {return "/-";}
  

  /** Return an update to the table if available. */
  TuplePtr
  pull(int port, b_cbv cb);


  /** Listener for table update. When a new tuple is added to the table,
      the table calls this method for notification. */
  void
  listener(TuplePtr t);
  



 private:

  /** My queue of table updates */
  std::list<TuplePtr> _updateBuffer;


  /** Call back for downstream elements */
  b_cbv _pullCB;
};

#endif /* __UPDATE_H_ */
