// -*- c-basic-offset: 2; related-file-name: "CSVlex.h" -*-
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

//
// A little regex-based lexer for CSV files.
//

#include "CSVlex.h"

// Constructor
CSVlex::CSVlex()
  : _re_line("^([^\\r\\n]*)\\r?\\n"),
    _re_comm("(^$|#.*)"),
    _re_qstr("^\\s*\\\"(([^\\n\\\"]*(\\\\(.|\\n))?)+)\\\"\\s*(,|$)"),
		_re_tokn("^\\s*([^,\\s\"\']+)\\s*(,|$)")
{
}

// Given a string "acc", try to extract a full line of CSV text to generate
// an outTup.  
// NOTE: may modify both acc and the contents of outTup!
// 
// Return conditions:
//    CSVGotLine (1): a CSV line was found and parsed into outTup, and the acc
//       string was advanced 
//    CSVComment (-1): a comment line was found and discarded, and the
//       acc string was advanced 
//    0: the acc string didn't contain a full line, and was returned
//       untouched 

int CSVlex::try_to_parse_line(string &acc, TuplePtr outTup)
{
	TRACE_FUNCTION;
	// Do we have a complete line in the buffer?
	boost::smatch m;
	if (regex_search(acc,m,_re_line)) {
		TRACE_WORDY << "Got a line <" << m[1] << ">\n";
		string line = m[1];
		acc = acc.substr(m[0].length());
		if (regex_match(line, m, _re_comm)) {
			TRACE_WORDY << "Comment: discarding.\n";
			return CSVGotComment;
		}
		while( line.length() > 0) {
			TRACE_WORDY << "Remaining line is <" << line << ">\n";
			{ 
				if (regex_search(line,m,_re_qstr)) {
					TRACE_WORDY << "Got a quoted string <" << m[1] << ">\n";
					outTup->append(Val_Str::mk(m[1]));
					line = line.substr(m[0].length());
					continue;
				}
			}
			{
				if (regex_search(line,m,_re_tokn)) {
					TRACE_WORDY << "Got a token <" << m[1] << ">\n";
					outTup->append(Val_Str::mk(m[1]));
					line = line.substr(m[0].length());
					continue;
				}
			}
			TRACE_WORDY << "Don't understand string <" << line << ">\n";
			line = "";
		}
		// Push the tuple we have
		return CSVGotLine;
	} else {
		TRACE_WORDY << "Don't yet have a whole line <" << acc << ">\n";
		return 0;
	}
	return 0;
}
