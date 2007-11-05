// -*- c-basic-offset: 2; related-file-name: "XTraceLex.C" -*-
/*
 * @(#)$$
 *
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * Or
 * UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
 * Berkeley, CA,  94707. Attention: P2 Group.
 * 
 * DESCRIPTION: A XTrace tokenizing stage.
 *
 */

#ifndef _XTRACELEX_H_
#define _XTRACELEX_H_

#include <string>
#include <queue>
#include <boost/regex.hpp>
#include "tuple.h"
#include "val_str.h"
#include "val_int32.h"

class XTraceLex {
	public:
		XTraceLex();
  	~XTraceLex() {};
	
	  int try_to_parse_line(string &, string&, string&);
      static const int XTraceGotItem = 1;
      static const int XTraceGotBlankLine = 2;
      static const int XTraceGotIgnoreLine = 3;

	private:
		// Regular expressions for matching the input
	  boost::regex	_re_line;
	  boost::regex	_re_key;
	  boost::regex	_re_value;
		
	};

#endif // _XTRACELEX_H
