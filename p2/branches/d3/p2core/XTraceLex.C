// -*- c-basic-offset: 2; related-file-name: "XTraceLex.h" -*-
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

//
// A little regex-based lexer for XTrace files.
//

#include "XTraceLex.h"

// Constructor
XTraceLex::XTraceLex()
  : _re_line("^([^\\r\\n]*)\\r?\\n"),
    _re_key("^\\s*([^:]+)\\s*:"),
    _re_value("^\\s*(.+)$")
   
{
}

// Given a string "acc", try to extract a full line of XTrace text to generate
// an outTup.  
// NOTE: may modify both acc and the contents of outTup!
// 
// Return conditions:
//    XTraceGotLine (1): a Xtrace line was found and parsed into outTup, and the acc
//       string was advanced 
//    XTraceComment (-1): a comment line was found and discarded, and the
//       acc string was advanced 
//    0: the acc string didn't contain a full line, and was returned
//       untouched 

int XTraceLex::try_to_parse_line(string &acc, string &key, string &value)
{
	TRACE_FUNCTION;
	TRACE_WORDY << "incoming parser...\n";

	// Do we have a complete line in the buffer?
	boost::smatch m;
	if (regex_search(acc,m,_re_line)) {
		TRACE_WORDY << "Got a line <" << m[1] << ">\n";
		string line = m[1];
		acc = acc.substr(m[0].length());
		while( line.length() > 0) {
			TRACE_WORDY << "Remaining line is <" << line << ">\n";
			{
				if (regex_search(line, m, _re_key)) {
					TRACE_WORDY << "Got a option key <" << m[1] << ">\n";
					//outTup->append(Val_Str::mk(m[1]));
					key = m[1];
					line = line.substr(m[0].length());

					regex_search(line, m, _re_value);
					TRACE_WORDY << "Got a option value <" << m[1] << ">\n";
					//outTup->append(Val_Str::mk(m[1]));
					value = m[1];
				 	line = "";
					
					return XTraceGotItem;	
				}
			}
	
			TRACE_WORDY << "ignore string <" << line << ">\n";
			line = "";
			return XTraceGotIgnoreLine;
		}
		// Push the tuple we have
		return XTraceGotBlankLine;
	} else {
		TRACE_WORDY << "Don't yet have a whole line <" << acc << ">\n";
		return 0;
	}
	return 0;
}
