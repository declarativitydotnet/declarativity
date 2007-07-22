// -*- c-basic-offset: 2; related-file-name: "CSVlex.C" -*-
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
 * DESCRIPTION: A CSV tokenizing stage.
 *
 */

#ifndef _CSVLEX_H_
#define _CSVLEX_H_

#include <string>
#include <queue>
#include <boost/regex.hpp>
#include "tuple.h"
#include "val_str.h"

class CSVlex {
	public:
		CSVlex();
  	~CSVlex() {};
	
	  int try_to_parse_line(string &, TuplePtr);
      static const int CSVGotLine = 1;
      static const int CSVGotComment = -1;

	private:
		// Regular expressions for matching the input
	  boost::regex	_re_line;
	  boost::regex	_re_comm;
	  boost::regex	_re_qstr;
	  boost::regex	_re_tokn;
		
	};

#endif // _CSVLEX_H
