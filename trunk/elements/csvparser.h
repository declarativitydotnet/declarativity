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

#ifndef __CSVPARSER_H__
#define __CSVPARSER_H__

#include "element.h"
#include <queue>
#include <rxx.h>

class CSVParser : public Element { 
public:

  CSVParser();

  int push(int port, TupleRef t, cbv cb);
  TuplePtr pull(int port, cbv);

  const char *class_name() const		{ return "CSVParser";}
  const char *processing() const		{ return PUSH_TO_PULL; }
  const char *flow_code() const			{ return "x/x"; }

private:
  void parse();


  static const size_t MIN_Q_SIZE;

  std::queue<TupleRef> _q;
  bool _push_blocked;

  // Dataflow synchronization callbacks
  cbv	_push_cb;
  cbv	_pull_cb;
  
  // Regular expressions for matching the input
  rxx	_re_line;
  rxx	_re_comm;
  rxx	_re_qstr;
  rxx	_re_tokn;

  // The current string accumulator
  str	_acc;
  // The current tuple accumulator
  TupleRef _tout;
};


#endif /* __CSVPARSER_H_ */
