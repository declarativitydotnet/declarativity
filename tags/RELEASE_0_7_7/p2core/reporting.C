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

std::map< std::string, Reporting::Level > Reporting::levelFromName;
std::map< Reporting::Level, std::string > Reporting::levelToName;
Reporting::Initializer Reporting::_initializer;


std::ostream* Reporting::wordy;


std::ostream* Reporting::info;


std::ostream* Reporting::warn;


std::ostream* Reporting::error;


std::ostream* Reporting::output;


Reporting::LeakyStreambuf Reporting::leakyStreambuf;


std::ostream Reporting::nullOStream(&Reporting::leakyStreambuf);


Reporting::Level Reporting::_level;


void
Reporting::setLevel(Level l)
{
  wordy = &nullOStream;
  info = &nullOStream;
  warn = &nullOStream;
  error = &nullOStream;
  output = &nullOStream;

  switch(l) {
  case ALL:
  case WORDY:
    wordy = &std::cerr;
  case INFO:
    info = &std::cerr;
  case WARN:
    warn = &std::cerr;
  case ERROR:
    error = &std::cerr;
  case OUTPUT:
    output = &std::cerr;
  case NONE:
  default:
    ;
  };

  _level = l;
}


Reporting::Level
Reporting::level()
{
  return _level;
}




Reporting::Initializer::Initializer()
{
  // Prepare maps
  levelFromName["ALL"] = ALL;
  levelFromName["WORDY"] = WORDY;
  levelFromName["INFO"] = INFO;
  levelFromName["WARN"] = WARN;
  levelFromName["ERROR"] = ERROR;
  levelFromName["OUTPUT"] = OUTPUT;
  levelFromName["NONE"] = NONE;
  
  levelToName[ALL] = "ALL";
  levelToName[INFO] = "INFO";
  levelToName[WARN] = "WARN";
  levelToName[ERROR] = "ERROR";
  levelToName[WORDY] = "WORDY";
  levelToName[OUTPUT] = "OUTPUT";
  levelToName[NONE] = "NONE";


  // Point streams. By default all are null
  wordy = info = warn = error = output = &nullOStream;
  setLevel(ERROR);
}


int
Reporting::LeakyStreambuf::overflow(int c)
{
  // Always succeed
  return 0;
}


