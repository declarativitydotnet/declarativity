// -*- c-basic-offset: 2;
/*
 * @(#)$Id$
 * 
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 * DESCRIPTION: This is intended as an "interface" not to be
 * instantiated, to split the "elementness" of the logger element from
 * its logging interface.
 *
 */


#ifndef __LOGGERI_H__
#define __LOGGERI_H__

#include <async.h>

class LoggerI { 
public:
  
  enum Level { INFO, WARN, ERROR };
  
  /** Create a log tuple. */
  virtual void log( str classname,
                    str instancename,
                    Level severity,
                    int errnum,
                    str explanation ) = 0;
};

#endif /* __LOGGERI_H_ */
