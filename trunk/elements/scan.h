// -*- c-basic-offset: 2; related-file-name: "scan.C" -*-
/*
 * @(#)$Id$
 * 
 * This file is distributed under the terms in the attached
 * INTEL-LICENSE file.  If you do not find these files, copies can be
 * found by writing to: Intel Research Berkeley, 2150 Shattuck Avenue,
 * Suite 1300, Berkeley, CA, 94704.  Attention: Intel License Inquiry.
 *
 * The scan element.  It has a single pull output.  It continuously
 * iterates over elemenets in the table returning them when pulled.
 * XXX. It does not block.  If the table is empty, it returns empty
 * tuples. When storage is abstracted to the table object, we could
 * interconnect inserts into the table with blocking/unblocking the scan
 * side.
 * 
 */

#ifndef __SCAN_H__
#define __SCAN_H__

#include "table.h"
#include "element.h"

class Scan : public Element {
 public:
  Scan(str name,
       TableRef table,
       unsigned field);
  
  const char *class_name() const		{ return "Scan";}
  const char *processing() const		{ return "/l"; }
  const char *flow_code() const			{ return "/-"; }
  
  /** Return a match to the current lookup */
  TuplePtr pull(int port, cbv cb);
  
 private:
  /** My parent's table */
  TableRef _table;
  
  /** My current iterator */
  Table::MultScanIterator _iterator;
};

#endif /* __SCAN_H_ */
