// -*- c-basic-offset: 2; related-file-name: "timedSource.h" -*-
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
 * DESCRIPTION: Creates logging tuples. 
 * 
 */

#include <logger.h>
#include <tuple.h>
#include <plumber.h>
#include <errno.h>

#include "val_double.h"
#include "val_uint64.h"
#include "val_str.h"
#include "val_int32.h"

//
// One global sequence number
//
uint64_t Logger::seq=0;


// 
// Not much of a constructor
//
Logger::Logger(string name) : Element(name, 0, 1) { }

void Logger::log( string classname, 
		  string instancename,
		  Level severity,
		  int errnum,
		  string explanation )
{
  if (severity >= plumber()->loggingLevel) {
    timespec now_ts;
    
    getTime(now_ts);
    TuplePtr t = Tuple::mk();
    t->append(Val_UInt64::mk(now_ts.tv_sec));
    t->append(Val_UInt64::mk(now_ts.tv_nsec));
    t->append(Val_UInt64::mk(seq++));
    t->append(Val_Str::mk(classname));
    t->append(Val_Str::mk(instancename));
    t->append(Val_Int32::mk(severity));
    t->append(Val_Int32::mk(errnum));
    t->append(Val_Str::mk(explanation));
    t->freeze();
    if (push(1, t, 0) == 0) {
      warn << "Logger: possible tuple overrun next time\n";
    }
  }
}


