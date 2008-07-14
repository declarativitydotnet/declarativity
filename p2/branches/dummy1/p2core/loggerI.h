// -*- c-basic-offset: 2; related-file-name: "loggerI.h" -*-
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
 * DESCRIPTION: This is intended as an "interface" not to be
 * instantiated, to split the "elementness" of the logger element from
 * its logging interface.
 *
 */


#ifndef __LOGGERI_H__
#define __LOGGERI_H__

#include "reporting.h"
#include <string>


class LoggerI { 
public:
  virtual ~LoggerI() {};


  /** Create a log tuple. */
  virtual void log(std::string classname,
                   std::string instancename,
                   Reporting::Level severity,
                   int errnum,
                   std::string explanation) = 0;

 private:
};

#endif /* __LOGGERI_H_ */
