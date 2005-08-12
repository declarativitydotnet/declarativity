// -*- c-basic-offset: 2; related-file-name: "delete.C" -*-
/*
 * @(#)$Id$
 * 
 * This file is distributed under the terms in the attached
 * INTEL-LICENSE file.  If you do not find these files, copies can be
 * found by writing to: Intel Research Berkeley, 2150 Shattuck Avenue,
 * Suite 1300, Berkeley, CA, 94704.  Attention: Intel License Inquiry.
 *
 * The delete element.  It has only a single push input on which tuples
 * to be deleted from the table are sent.  XXX For now all deletes
 * succeed
 * 
 */

#ifndef __DELETE_H__
#define __DELETE_H__

#include "element.h"
#include "table.h"

class Delete : public Element {
 public:
  Delete(str name,
         TableRef table,
         unsigned indexFieldNo,
         unsigned keyFieldNo);
  
  const char *class_name() const		{ return "Delete";}
  const char *processing() const		{ return "h/"; }
  const char *flow_code() const			{ return "-/"; }
  
  /** Delete a pushed element */
  int push(int port, TupleRef, cbv cb);

  
 private:
  /** My table */
  TableRef _table;

  /** My index field into which I search for the entry to remove */
  unsigned _indexFieldNo;

  /** The field number of the input tuple field to use for searching */
  unsigned _keyFieldNo;
};


#endif /* __DELETE_H_ */
