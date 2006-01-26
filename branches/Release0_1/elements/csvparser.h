// -*- c-basic-offset: 2; related-file-name: "element.C" -*-
/*
 * @(#)$Id$
 * 
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
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

#ifndef __CSVPARSER_H__
#define __CSVPARSER_H__

#include "element.h"
#include <queue>
#include <boost/regex.hpp>

class CSVParser : public Element { 
public:

  CSVParser(string);

  int push(int port, TuplePtr t, b_cbv cb);
  TuplePtr pull(int port, b_cbv);

  const char *class_name() const		{ return "CSVParser";}
  const char *processing() const		{ return PUSH_TO_PULL; }
  const char *flow_code() const			{ return "h/l"; }

private:
  int try_to_parse_line();
  
  static const size_t MIN_Q_SIZE;	// See csvparser.C...
  static const size_t MAX_Q_SIZE;

  std::queue<TuplePtr> _q;

  // Dataflow synchronization callbacks
  b_cbv	_push_cb;
  bool  _push_blocked;
  b_cbv	_pull_cb;
  
  // Regular expressions for matching the input
  boost::regex	_re_line;
  boost::regex	_re_comm;
  boost::regex	_re_qstr;
  boost::regex	_re_tokn;

  // The current string accumulator
  string	_acc;
};


#endif /* __CSVPARSER_H_ */
