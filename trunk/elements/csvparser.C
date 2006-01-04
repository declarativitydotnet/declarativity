// -*- c-basic-offset: 2; related-file-name: "element.C" -*-
/*
 * @(#)$Id$
 * 
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
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
#include "trace.h"
#include <boost/regex.hpp>

//
// What are these queue parameters then?  Well...
//
//  - push-in will be disabled if there are more than MAX_Q_SIZE
//    tuples in the output queue. 
//  - push-in will be reenabled when there are less than MIN_Q_SIZE
//    tuples in the output queue. 
//
const size_t CSVParser::MIN_Q_SIZE = 100;
const size_t CSVParser::MAX_Q_SIZE = 100;

// 
// Constructor. 
//  
CSVParser::CSVParser(string name) 
  : Element(name, 1,1), 
    _push_cb(0), 
    _push_blocked(false), 
    _pull_cb(0),
/*
    _re_line("^([^\\r\\n]*)\\r?\\n"),
    _re_comm("(^$|#.*)"),
    _re_qstr("^\\s*\\\"(([^\\n\\\"]*(\\\\(.|\\n))?)+)\\\"\\s*(,|$)"),
    _re_tokn("^\\s*([^,\\s\"\']+)\\s*(,|$)"),
*/
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
    DBG("Error: tuple is not of size 1");
  } else {
    _acc += Val_Str::cast((*t)[0]);
    while(try_to_parse_line());
  }
  if (_push_blocked) {
    DBG("Error: push after push has been blocked.");
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
/*
  TRC_FN;
  // Do we have a complete line in the buffer?
  boost::smatch m;
  if (regex_search(_acc,m,_re_line)) {
    TRC("Got a line <" << m[1] << ">");
    TuplePtr t = Tuple::mk();
    string line = m[1];
    _acc = _acc.substr(m[0].length());
    if (regex_match(line, m, _re_comm)) {
      TRC("Comment: discarding.");
      return 1;
    }
    while( line.length() > 0) {
      TRC("Remaining line is <" << line << ">");
      { 
	if (regex_search(line,m,_re_qstr)) {
	  TRC("Got a quoted string <" << m[1] << ">");
	  t->append(Val_Str::mk(m[1]));
	  line = line.substr(m[0].length());
	  continue;
	}
      }
      {
	if (regex_search(line,m,_re_tokn)) {
	  TRC("Got a token <" << m[1] << ">");
	  t->append(Val_Str::mk(m[1]));
	  line = line.substr(m[0].length());
	  continue;
	}
      }
      TRC("Don't understand string <" << line << ">");
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
    TRC("Don't yet have a whole line <" << _acc << ">");
    return 0;
  }
*/
  return 0;
}
