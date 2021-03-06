// -*- c-basic-offset: 2; related-file-name: "csvparser.h" -*-
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
 * DESCRIPTION: Element which takes a push input consisting of strings
 *  (strictly speaking, tuples with a single string field) comprising
 *   a sequence of Comma-Separated Value lines, and exports a pull
 *   output which consists of tuples parsed from the input.  No
 *   assumptions are made as to where the line boundaries are relative
 *   to the string boundaries. 
 */

#include "csvparser.h"

#include "val_str.h"
#include <boost/regex.hpp>
#include "reporting.h"

//
// What are these queue parameters then?  Well...
//
//  - push-in will be disabled if there are more than MAX_Q_SIZE
//    tuples in the output queue. 
//  - push-in will be reenabled when there are less than MIN_Q_SIZE
//    tuples in the output queue. 
//
const uint32_t CSVParser::MIN_Q_SIZE = 100;
const uint32_t CSVParser::MAX_Q_SIZE = 100;

// 
// Constructor. 
//  
CSVParser::CSVParser(string name) 
  : Element(name, 1,1), 
    _push_cb(0), 
    _push_blocked(false), 
    _pull_cb(0),
    _re_line("^([^\\r\\n]*)\\r?\\n"),
    _re_comm("(^$|#.*)"),
    _re_qstr("^\\s*\\\"(([^\\n\\\"]*(\\\\(.|\\n))?)+)\\\"\\s*(,|$)"),
    _re_tokn("^\\s*([^,\\s\"\']+)\\s*(,|$)"),
    _acc("")
{
}  

//
// Pull a tuple from the parser
//
TuplePtr CSVParser::pull(int port, b_cbv cb)
{
  assert(port == 0);
  TuplePtr p;

  // Do we have a tuple to give back?
  if (_q.empty()) {
    p.reset();
    _pull_cb = cb;
  } else {
    p = _q.back();
    _q.pop();
    _pull_cb = 0;
  }

  // Should we restart pushes?  
  if (_push_blocked && _q.size() < MIN_Q_SIZE) {
    _push_blocked = false;
    if (_push_cb != 0) {
      _push_cb();
    }
  }
  return p;
}

//
// Push handler.  Accept a string from the input. 
//
int CSVParser::push(int port, TuplePtr t, b_cbv cb)
{
  //TRC_FN;
  assert(port == 0);
  _push_cb = cb;
  
  if (t->size() != 1) {
    TELL_WORDY << "Error: tuple is not of size 1\n";
  } else {
    _acc += Val_Str::cast((*t)[0]);
    while(try_to_parse_line());
  }
  if (_push_blocked) {
    TELL_WORDY << "Error: push after push has been blocked.\n";
    return 0;
  } else if ( _q.size() > MAX_Q_SIZE ) {
    _push_blocked = true;
    return 0;
  } else {
    return 1;
  }
}

//
// The actual lexer.
//
int CSVParser::try_to_parse_line()
{
  TRACE_FUNCTION;
  // Do we have a complete line in the buffer?
  boost::smatch m;
  if (regex_search(_acc,m,_re_line)) {
    TRACE_WORDY << "Got a line <" << m[1] << ">\n";
    TuplePtr t = Tuple::mk();
    string line = m[1];
    _acc = _acc.substr(m[0].length());
    if (regex_match(line, m, _re_comm)) {
      TRACE_WORDY << "Comment: discarding.\n";
      return 1;
    }
    while( line.length() > 0) {
      TRACE_WORDY << "Remaining line is <" << line << ">\n";
      { 
	if (regex_search(line,m,_re_qstr)) {
	  TRACE_WORDY << "Got a quoted string <" << m[1] << ">\n";
	  t->append(Val_Str::mk(m[1]));
	  line = line.substr(m[0].length());
	  continue;
	}
      }
      {
	if (regex_search(line,m,_re_tokn)) {
	  TRACE_WORDY << "Got a token <" << m[1] << ">\n";
	  t->append(Val_Str::mk(m[1]));
	  line = line.substr(m[0].length());
	  continue;
	}
      }
      TRACE_WORDY << "Don't understand string <" << line << ">\n";
      line = "";
    }
    // Push the tuple we have
    _q.push(t);
    // Restart pulls if we need to
    if (_pull_cb) {
      _pull_cb();
      _pull_cb = 0;
    }
    return 1;
  } else {
    TRACE_WORDY << "Don't yet have a whole line <" << _acc << ">\n";
    return 0;
  }
  return 0;
}
