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

//
// One global sequence number
//
uint64_t Logger::seq=0;


// 
// Not much of a constructor
//
Logger::Logger() : Element(0, 1) { }

void Logger::log( str classname, 
		  str instancename,
		  Level severity,
		  int errnum,
		  str explanation )
{
  timespec now_ts;
  double   now;

  if (clock_gettime(CLOCK_REALTIME,&now_ts)) {
    fatal << "clock_gettime:" << strerror(errno) << "\n";
  }
  now = now_ts.tv_sec + (1.0 / 1000 / 1000 / 1000 * now_ts.tv_nsec);

  TupleRef t = Tuple::mk();
  t->append(New refcounted<TupleField>(now));
  t->append(New refcounted<TupleField>(seq++));
  t->append(New refcounted<TupleField>(classname));
  t->append(New refcounted<TupleField>(instancename));
  t->append(New refcounted<TupleField>(severity));
  t->append(New refcounted<TupleField>(errnum));
  t->append(New refcounted<TupleField>(explanation));
  t->freeze();
  if (push(1,t,cbv_null) == 0) {
    warn << "Logger: possible tuple overrun next time\n";
  }
}


