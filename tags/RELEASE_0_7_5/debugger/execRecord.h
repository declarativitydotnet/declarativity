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
 *
 * DESCRIPTION: This is not an element, it is a 
 * helper class to store the internals of the execution 
 * records. It is not visible to any other class outside
 * of ruleTracer class.
 *
 */


#ifndef __EXECRECORD_H__
#define __EXECRECORD_H__

#include <p2Time.h>
#include "boost/date_time/posix_time/posix_time.hpp"
#include "val_uint64.h"

class ExecRecord{
 public:
  // maximum number of "join" elements in a rule strand
  // possible (these define the stages in the pipe-lined
  // execution model)
  static const int MAX_STAGES = 5;

  static const int START = 0;
  static const int END = 1;
  static const int INTER = 2;

  string ruleId;

  /** ID of input tuple */
  uint32_t tupleIn;
  /** ID of output tuple */
  uint32_t tupleOut;
  /** preconditin for this record */
  uint32_t precond[MAX_STAGES];
  /** destination for the output tuple */
  string remoteNode;
  /** time at which input was recieved */
  boost::posix_time::ptime timeIn;
  /** time at which output was recieved */
  boost::posix_time::ptime timeOut;
  /** is this execution finished */
  bool finished;
  /** how many preconditions */
  int numEntries;

  /** range of elements in the rule strand that can 
      input to this record. */
  int range_start;
  int range_end;

  ExecRecord(string rule);
  ~ExecRecord();

  /** initializes the preconditions */
  void initializePreds();

  string toString();
  
  /** 0's the preconditions starting from 
      index till the end port */
  void flushTillIndex(int index);

  /** re-initializes everything */
  void flush();

  void resetTimer(boost::posix_time::ptime *t){};
  
};

#endif /** __EXECRECORD_H__ **/
