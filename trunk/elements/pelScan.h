// -*- c-basic-offset: 2; related-file-name: "pelScan.C" -*-
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
 * The PEL scan element.  It has a push input supplying a scanning
 * attribute. It has a single pull output on which computed results are
 * returned.  The last element returned is tagged with END_OF_SCAN.  The
 * element takes three PEL scripts, an initialization script, a scanning
 * script, and a cleanup script. The initialization script is run when
 * a push input is received (and pushes subsequently blocked).  Scanning
 * scripts are run one per table element, and they may or may not create
 * a result tuple, which is subsequently returned to pullers.  When the
 * table elements are exhausted, the last returned element is tagged
 * with END_OF_SCAN.  The cleanup script is then run and either produces
 * a tuple or not, thereafter releasing the lock on pushers and blocking
 * pulls.
 * 
 */

#ifndef __PELSCAN_H__
#define __PELSCAN_H__

#include "table.h"
#include "element.h"
#include "pel_program.h"
#include "pel_vm.h"

class PelScan : public Element {
 public:
  PelScan(string name,
          TablePtr table,
          unsigned field,
          string startup,
          string scan,
          string cleanup);
  
  const char *class_name() const		{ return "PelScan";}
  const char *processing() const		{ return "h/l"; }
  const char *flow_code() const			{ return "-/-"; }
  
  /** Receive a new lookup key */
  int push(int port, TuplePtr, b_cbv cb);
  
  /** Return a match to the current lookup */
  TuplePtr pull(int port, b_cbv cb);
  
  /** The END_OF_SCAN tuple tag. */
  static string END_OF_SCAN;
  
 private:
  /** My parent's table */
  TablePtr _table;
  
  /** My current iterator */
  Table::MultScanIterator _iterator;

  /** My pusher's callback */
  b_cbv _pushCallback;
  
  /** My puller's callback */
  b_cbv _pullCallback;
  
  /** My index field */
  unsigned _indexFieldNo;

  /** My current scan tuple */
  TuplePtr _scanTuple;

  /** My startup program.  */
  boost::shared_ptr< Pel_Program > _startup;

  /** My scan program */
  boost::shared_ptr< Pel_Program > _scan;

  /** My cleanup program */
  boost::shared_ptr< Pel_Program > _cleanup;

  /** The virtual machine within which to execute the transform.  Any
      need to share this? */
  Pel_VM _vm;
};

#endif /* __PELSCAN_H_ */
