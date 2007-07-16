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

#ifndef __DELETE_H__
#define __DELETE_H__

#include "element.h"
#include "commonTable.h"
#include "elementRegistry.h"

class Delete : public Element {
public:
  Delete(string name,
         CommonTablePtr table);
  
  Delete(TuplePtr args);

  const char*
  class_name() const {return "Delete";}


  const char*
  processing() const {return "h/";}


  const char*
  flow_code() const {return "-/";}

  
  /** Delete a pushed element */
  int
  push(int port, TuplePtr, b_cbv cb);

  
  DECLARE_PUBLIC_ELEMENT_INITS

private:
  /** My table */
  CommonTablePtr _table;

  DECLARE_PRIVATE_ELEMENT_INITS
};


#endif /* __DELETE_H_ */
