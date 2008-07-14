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

#include "tuple.h"
#include "p2Time.h"

#include "val_double.h"
#include "val_str.h"
#include "val_int64.h"
#include "val_time.h"
#include "reporting.h"

#include <errno.h>

DEFINE_ELEMENT_INITS(Logger, "Logger");

//
// One global sequence number
//
uint64_t Logger::seq=0;


// 
// Not much of a constructor
//
Logger::Logger(string name) : Element(name, 0, 1) { }

/**
 * Generic constructor.
 * Arguments:
 * 2. Val_Str:    Element Name.
 */
Logger::Logger(TuplePtr args)
  : Element(Val_Str::cast((*args)[2]), 0, 1)
{
}

void
Logger::log(string classname, 
            string instancename,
            Reporting::Level severity,
            int errnum,
            string explanation )
{
  if (severity >= Reporting::level()) {
    boost::posix_time::ptime now_ts;
    
    getTime(now_ts);
    TuplePtr t = Tuple::mk();
    t->append(Val_Time::mk(now_ts));
    t->append(Val_Int64::mk(seq++));
    t->append(Val_Str::mk(classname));
    t->append(Val_Str::mk(instancename));
    t->append(Val_Int64::mk(severity));
    t->append(Val_Int64::mk(errnum));
    t->append(Val_Str::mk(explanation));
    t->freeze();
    if (push(1, t, 0) == 0) {
      TELL_WARN << "Logger: possible tuple overrun next time\n";
    }
  }
}


