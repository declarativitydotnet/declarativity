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
#include <map>

class LoggerI { 
public:
  
  enum Level { ALL = 0, WORDY, INFO, WARN, ERROR, NONE };

  
  /** Create a log tuple. */
  virtual void log( str classname,
                    str instancename,
                    Level severity,
                    int errnum,
                    str explanation ) = 0;

  static std::map< str, LoggerI::Level > levelFromName;
  static std::map< LoggerI::Level, str > levelToName;

  class Initializer {
  public:
    Initializer() {
      levelFromName["ALL"] = ALL;
      levelFromName["WORDY"] = WORDY;
      levelFromName["INFO"] = INFO;
      levelFromName["WARN"] = WARN;
      levelFromName["ERROR"] = ERROR;
      levelFromName["NONE"] = NONE;

      levelToName[ALL] = "ALL";
      levelToName[INFO] = "INFO";
      levelToName[WARN] = "WARN";
      levelToName[ERROR] = "ERROR";
      levelToName[WORDY] = "WORDY";
      levelToName[NONE] = "NONE";
    }
  };
  
 private:
  static Initializer _initializer;

};

#endif /* __LOGGERI_H_ */
