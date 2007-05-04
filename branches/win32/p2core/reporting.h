// -*- c-basic-offset: 2; related-file-name: "reporting.h" -*-
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
 * DESCRIPTION: (Simple) error reporting facility for P2.  Reporting
 * level is encoded in the output stream name.  When the reporting
 * facility is not enabled, output on the stream is a no-op.
 *
 */


#ifndef __REPORTING_H__
#define __REPORTING_H__

#ifdef VISUAL_LEAK_DETECTOR
// SINCE THIS FILE GETS INCLUDED EVERYWHERE, ADD
// THE VISUAL LEAK DETECTOR INCLUDE HERE
#include "vld.h"
#endif // VISUAL_LEAK_DETECTOR

#include <map>
#include <string>
#include <ostream>


class Reporting { 
public:
  virtual ~Reporting() {};


  /** What are the different reporting levels? */
  enum
  Level { ALL = 0,              // Print everything
          WORDY,                // Print excruciatingly detailed info
          INFO,                 // Print informational messages
          WARN,                 // Print warnings
          P2_ERROR,                // Print outright errors
          OUTPUT,               // Print application output
          NONE                  // Print nothing
  };


  
private:
  static std::map< std::string, Reporting::Level >* _levelFromName;

  static std::map< Reporting::Level, std::string >* _levelToName;

  /** The wordy stream */
  static std::ostream* _wordy;


  /** The info stream */
  static std::ostream* _info;


  /** The warn stream */
  static std::ostream* _warn;


  /** The error stream */
  static std::ostream* _error;


  /** The error stream */
  static std::ostream* _output;


  /** Convenience function to share level setting functionality with
      initializers and external clients. Only to be used internally.  */
  static void
  innerSetLevel(Level l);


public:
  static std::map< std::string, Reporting::Level >&
  levelFromName();
 

  static std::map< Reporting::Level, std::string >&
  levelToName();
  

  /** Set the logging level. Everything at and above it is
      enabled. Everything below it is disabled. */
  static void
  setLevel(Level l);


  /** Obtain the logging level. */
  static Level
  level();


  /** The wordy stream */
  static std::ostream* wordy();


  /** The info stream */
  static std::ostream* info();


  /** The warn stream */
  static std::ostream* warn();


  /** The error stream */
  static std::ostream* error();


  /** The error stream */
  static std::ostream* output();





  /** The leaky stream buffer. */
  class LeakyStreambuf : public std::streambuf {
    /** Override the overflow method to do nothing */
    int
    overflow(int c = EOF);
  };

  
  class Initializer {
  public:
    Initializer();
  };


  /** Fetch the initializer to construct it */
  static Initializer*
  theInitializer();


  
private:
  /** Can't create objects */
  Reporting() {}


  /** The leaky stream buffer */
  static LeakyStreambuf* _leakyStreambuf;


  /** The null stream */
  static std::ostream* _nullOStream;


  /** The Logging Level */
  static Level _level;
};




////////////////////////////////////////////////////////////
// MACROS
////////////////////////////////////////////////////////////

#define TELL_WORDY (*Reporting::wordy()) << "REPORTING: "
#define TELL_INFO (*Reporting::info()) << "REPORTING: "
#define TELL_WARN (*Reporting::warn()) << "REPORTING: "
#define TELL_ERROR (*Reporting::error()) << "REPORTING: "
#define TELL_OUTPUT (*Reporting::output()) << "REPORTING: "

#define TRACE_WORDY ((*Reporting::wordy()) <<  __FUNCSIG__)
#define TRACE_INFO (*Reporting::info() <<  __FUNCSIG__)
#define TRACE_WARN (*Reporting::warn() <<  __FUNCSIG__)
#define TRACE_ERROR (*Reporting::error() <<  __FUNCSIG__)
#define TRACE_OUTPUT (*Reporting::output() <<  __FUNCSIG__)



////////////////////////////////////////////////////////////
// Tracing
////////////////////////////////////////////////////////////

/** A trace object is allocated in the stack of a function, so that its
    constructor and destructor can report entering into and exiting from
    the function */
class TraceObj {
private:
  /** The name of the function to trace */
  const char *fn;


public:
  /** The constructor reports entering a scope. */
  TraceObj(const char *s) 
    : fn(s) 
  {
    TELL_INFO << "Entering " << fn << "\n";
  }


  /** The destructor reports exiting from the scope */
  ~TraceObj() 
  { 
    TELL_INFO << "Exiting " << fn << "\n";
  }
};


#ifndef TRACE_OFF
// Place this in a function that must be traced
#define TRACE_FUNCTION TraceObj _t(__FUNCSIG__)

#else

#define TRACE_FUNCTION

#endif /* TRACE_OFF */



#endif /* __REPORTING_H_ */
