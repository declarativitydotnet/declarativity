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


#include "trace.h"

const size_t CSVParser::MIN_Q_SIZE = 100;
  
CSVParser::CSVParser() : Element(1,1), 
			 _push_cb(cbv_null), 
			 _pull_cb(cbv_null),
                         _re_line("^([^\\r\\n]*)\\r?\\n"),
			 _re_comm("(^$|#.*)"),
			 _re_qstr("\\s*\\\"(([^\\n\\\"]*(\\\\(.|\\n))?)+)\\\"\\s*(,|$)"),
			 _re_tokn("\\s*([^,\\s\"\']+)\\s*(,|$)"),
			 _acc(""),
			 _tout(Tuple::mk())
{

}  


//
// Pull a tuple from the parser
//
TuplePtr CSVParser::pull(int port, cbv cb)
{
  assert(port == 0);
  TuplePtr p;
  _pull_cb = cb;

  // Do we have a tuple to give back?
    if (_q.empty()) {
    p = NULL;
  } else {
    p = _q.back();
    _q.pop();
  }

  // Should we restart pushes?  
  if (_push_blocked && _q.size() < MIN_Q_SIZE) {
    _push_blocked = false;
    if (_push_cb != cbv_null) {
      _push_cb();
    }
  }
  return p;
}

//
// Push handler.  Accept a string from the input. 
//
int CSVParser::push(int port, TupleRef t, cbv cb)
{
  //TRC_FN;
  assert(port == 0);
  _push_cb = cb;
  
  if (t->size() != 1) {
    DBG("Error: tuple is not of size 1");
  } else if ((*t)[0]->t != TupleField::STRING) {
    // Error: field is not a string
    DBG("Error: field is not a string");
  } else {
    str tf = (*t)[0]->as_s();
    strbuf newacc;
    newacc << _acc;
    newacc << tf;
    _acc = newacc;
    parse();
  }
}

//
// The actual lexer.
//
void CSVParser::parse()
{
  TRC_FN;
  // Do we have a complete line in the buffer?
  rxx::matchresult m = _re_line.search(_acc);
  if (m) {
    TRC("Got a line <" << m[1] << ">");
    TupleRef t = Tuple::mk();
    str line = m[1];
    _acc = substr(_acc,m[0].len());
    if (_re_comm.match(line)) {
      TRC("Comment: discarding.");
      return;
    }
    while( line.len() > 0) {
      { 
	rxx::matchresult m = _re_qstr.search(line);
	if (m) {
	  TRC("Got a quoted string <" << m[1] << ">");
	  t->append(New refcounted<TupleField>(m[1]));
	  line = substr(line,m[0].len());
	  continue;
	}
      }
      {
	rxx::matchresult m = _re_tokn.search(line);
	if (m) {
	  TRC("Got a token <" << m[1] << ">");
	  t->append(New refcounted<TupleField>(m[1]));
	  line = substr(line,m[0].len());
	  continue;
	}
      }
      TRC("Don't understand string <" << line << ">");
      line = "";
    }
    TRC("OUTPUT TUPLE: " << t->toString());
    _q.push(t);
  } else {
    TRC("_acc doesn't yet make a line <" << _acc << ">");
  }
}
