// -*- c-basic-offset: 2; related-file-name: "plumber.C" -*-
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
 * Loosely inspired from the Click Router class, by Eddie Kohler
 *
 * DESCRIPTION: The plumber shell providing plumbing functionality among
 * elements.
 */

#ifndef __PLUMBER_H__
#define __PLUMBER_H__

#include <inlines.h>
#include <fstream>
#include <element.h>
#include <elementSpec.h>
#include <loggerI.h>
#include <dot.h>
#include <table2.h>
#include "dataflow.h"
#include "dataflowEdit.h"

class Scheduler;
class CommitManager;

/** A handy dandy type for plumber references */
class Plumber;
typedef boost::shared_ptr< Plumber > PlumberPtr;

class Plumber {
public:

  /** Create a new plumber given a configuration of constructed but not
      necessarily configured elements. */
  Plumber();

  /** Return a reference to a new dataflow object */
  static DataflowEditPtr
  edit(string name);


  /** Install the dataflow in this plumber. 
   *  Return: 0 on success, -1 on failure.  */
  static int install(DataflowPtr d);

  /** Return the scheduler */
  //currently, it is assumed we have exactly one scheduler
  //for multi-DF support, it would require some support
  //from the DF language, primarily concerning assignment
  //of elements to schedulers
  static Scheduler* scheduler();

  /** Output in .dot format a representation of this dataflow */
  static void toDot(string f);

  static int ndataflows() { return _dataflows.size(); }

  REMOVABLE_INLINE static DataflowPtr
  dataflow(string n);


  // LOGGING infrastructure
  
  /** The plumber-wide logger.  The pointer to the logger should not be
      cached or used at any time other than immediately after the
      invocation of this method. */
  REMOVABLE_INLINE LoggerI * logger() { return _logger; }

  /** Install a different logger for this plumber. This returns a pointer
      to the previous logger.  If non zero, it is the caller's
      responsibility to delete that logger if it's on the heap. */
  REMOVABLE_INLINE LoggerI * logger(LoggerI * newLogger);

  static CommonTable::ManagerPtr catalog() 
  { return _catalog; }
  
private:
  static CommonTable::ManagerPtr _catalog;

  /** The list of installed dataflows */
  static std::map< string, DataflowPtr > _dataflows;

  /** The scheduler */
  static Scheduler _scheduler;

  /** My local logger */
  LoggerI * _logger;
};

#endif
