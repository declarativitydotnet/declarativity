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
 */


#include <reporting.h>
#include <iostream>

std::ostream* Reporting::_wordy;


std::ostream* Reporting::_info;


std::ostream* Reporting::_warn;


std::ostream* Reporting::_error;


std::ostream* Reporting::_output;


Reporting::LeakyStreambuf* Reporting::_leakyStreambuf;


std::ostream* Reporting::_nullOStream;


Reporting::Level Reporting::_level;


std::map< std::string, Reporting::Level >* Reporting::_levelFromName;


std::map< Reporting::Level, std::string >* Reporting::_levelToName;


/** This method assumes the initializations have happened. It is only
    called internally.  External clients should use setLevel()
    instead. */
void
Reporting::innerSetLevel(Level l)
{
  _wordy = _nullOStream;
  _info = _nullOStream;
  _warn = _nullOStream;
  _error = _nullOStream;
  _output = _nullOStream;

  switch(l) {
  case ALL:
  case WORDY:
    _wordy = &std::cerr;
  case INFO:
    _info = &std::cerr;
  case WARN:
    _warn = &std::cerr;
  case ERROR:
    _error = &std::cerr;
  case OUTPUT:
    _output = &std::cerr;
  case NONE:
  default:
    ;
  };

  _level = l;
}


void
Reporting::setLevel(Level l)
{
  Initializer* initializer = theInitializer(); // ensure the initializer
                                               // has run
  initializer = initializer;    // avoid "unused" warnings

  innerSetLevel(l);
}


Reporting::Level
Reporting::level()
{
  Initializer* initializer = theInitializer(); // ensure the initializer
                                               // has run
  initializer = initializer;    // avoid "unused" warnings

  return _level;
}


std::map< std::string, Reporting::Level >&
Reporting::levelFromName()
{
  Initializer* initializer = theInitializer(); // ensure the initializer
                                               // has run
  initializer = initializer;    // avoid "unused" warnings

  return (*_levelFromName);
}
 

std::map< Reporting::Level, std::string >&
Reporting::levelToName()
{
  Initializer* initializer = theInitializer(); // ensure the initializer
                                               // has run
  initializer = initializer;    // avoid "unused" warnings

  return (*_levelToName);
}



std::ostream*
Reporting::wordy()
{
  Initializer* initializer = theInitializer();
  initializer = initializer;
  return _wordy;
}
std::ostream*
Reporting::info()
{
  Initializer* initializer = theInitializer();
  initializer = initializer;
  return _info;
}
std::ostream*
Reporting::warn()
{
  Initializer* initializer = theInitializer();
  initializer = initializer;
  return _warn;
}
std::ostream*
Reporting::error()
{
  Initializer* initializer = theInitializer();
  initializer = initializer;
  return _error;
}
std::ostream*
Reporting::output()
{
  Initializer* initializer = theInitializer();
  initializer = initializer;
  return _output;
}




Reporting::Initializer::Initializer()
{
  _leakyStreambuf = new Reporting::LeakyStreambuf();
  _nullOStream = new std::ostream(Reporting::_leakyStreambuf);
  _levelFromName = new std::map< std::string, Reporting::Level >();
  _levelToName = new std::map< Reporting::Level, std::string >();

  // Prepare maps
  (*_levelFromName)["ALL"] = ALL;
  (*_levelFromName)["WORDY"] = WORDY;
  (*_levelFromName)["INFO"] = INFO;
  (*_levelFromName)["WARN"] = WARN;
  (*_levelFromName)["ERROR"] = ERROR;
  (*_levelFromName)["OUTPUT"] = OUTPUT;
  (*_levelFromName)["NONE"] = NONE;
  
  (*_levelToName)[ALL] = "ALL";
  (*_levelToName)[INFO] = "INFO";
  (*_levelToName)[WARN] = "WARN";
  (*_levelToName)[ERROR] = "ERROR";
  (*_levelToName)[WORDY] = "WORDY";
  (*_levelToName)[OUTPUT] = "OUTPUT";
  (*_levelToName)[NONE] = "NONE";


  // Point streams. By default all are null
  _wordy = _info = _warn = _error = _output = _nullOStream;
  innerSetLevel(ERROR);
}


/** Implements the construct before use pattern */
Reporting::Initializer*
Reporting::theInitializer()
{
  static Initializer* _initializer =
    new Initializer();
  return _initializer;
}


int
Reporting::LeakyStreambuf::overflow(int c)
{
  // Always succeed
  return 0;
}
