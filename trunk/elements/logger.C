// -*- c-basic-offset: 2; related-file-name: "timedSource.h" -*-
/*
 * @(#)$Id$
 * 
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 * DESCRIPTION: Creates logging tuples. 
 * 
 */

#include <logger.h>
#include <tuple.h>
#include <router.h>

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
Logger::Logger(str name) : Element(name, 0, 1) { }

void Logger::log( str classname, 
		  str instancename,
		  Level severity,
		  int errnum,
		  str explanation )
{
  if (severity >= router()->loggingLevel) {
    timespec now_ts;
    
    if (clock_gettime(CLOCK_REALTIME,&now_ts)) {
      fatal << "clock_gettime:" << strerror(errno) << "\n";
    }
    TupleRef t = Tuple::mk();
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


