// -*- c-basic-offset: 2; related-file-name: "" -*-
/*
 * @(#)$Id$
 *
 * Copyright (c) 2004 Intel Corporation. All rights reserved.
 *
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 * DESCRIPTION: Test harness for datalog....
 *
 */

#include "ol_lexer.h"
#include "ol_context.h"

extern int ol_parser_debug;
#include <iostream>
#include <fstream>

int main(int argc, char **argv)
{
  std::cout << "OVERLOG\n";

  OL_Context ctxt;
  
  for( int i=1; i<argc; i++) { 
    str arg(argv[i]);
    
    if (arg == "-d") { 
      ol_parser_debug = 1;
    } else if (arg == "-h") { 
      std::cerr << "Usage: " << argv[0] << "{option|filename}+\n"
		<< "\t-d: turn on parser debugging\n"
		<< "\t-h: print this help text\n"
		<< "\t- : read from stdin\n";
      exit(0);
    } else if (arg == "-") {
      ctxt.parse_stream(&std::cin);
    } else {
      std::ifstream istr(argv[i]);
      ctxt.parse_stream(&istr);
    }
  }
  return 0;
}


