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
 * The insert element.  It has only a single push input on which tuples
 * to be inserted into the table are sent.  Inserted tuples that cause a
 * change in the underlying table are output on the output port.  XXX
 * For now all inserts succeed.
 * 
 */

#ifndef __INSERT_H__
#define __INSERT_H__

#include "element.h"
#include "table.h"

class Insert : public Element {
 public:
  Insert(string name,
         TablePtr table);
  
  const char *class_name() const		{ return "Insert";}
  const char *processing() const		{ return "a/a"; }
  const char *flow_code() const			{ return "x/x"; }
  
  /** Overridden to perform the insertion operation on passing
      tuples. */
  TuplePtr simple_action(TuplePtr p);
  
 private:
  /** My table */
  TablePtr _table;

};


#endif /* __INSERT_H_ */
