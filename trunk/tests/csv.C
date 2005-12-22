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
 * DESCRIPTION: Tests for CSV parser
 *
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */
#include <async.h>
#include <arpc.h>
#include <iostream>

#include "csvparser.h"
#include "val_str.h"

struct csv_test {
  char *in;
  char *out;
};

char *null_str = "__null_tuple";

const csv_test tests[] = {
  { "# Comment\r\n",	    null_str },
  { "\r\n",		    null_str },
  { "1,2,3\r\n",	    "<1, 2, 3>" },
  { " One, ",		    null_str },
  { " \"Two\" , ",	    null_str },
  { "Three,Four,Five\r\n",  "<One, Two, Three, Four, Five>"},

  { "\r\n",		    null_str }
};

#if 0
{ "This is half of a line. ... "
    "and this is the other half\"\n"
    "Another,line"
    "\r\n"
    NULL
    };
#endif 

int main(int argc, char **argv)
{
  std::cout << "\nCSV\n";
  
  CSVParser cp("CSV");
  
  
  for(unsigned i=0; i < (sizeof(tests) / sizeof(csv_test)); i++) {
    TupleRef t = Tuple::mk();
    t->append(Val_Str::mk(str(tests[i].in))); 
    t->freeze();
    cp.push(0, t, 0);
    TuplePtr t_out = cp.pull(0, 0);
    if (t_out != NULL) {
      // got a tuple
      std::cout << "Expected " << tests[i].out 
		<< ", got " << t_out->toString() << "\n";
      if (t_out->toString() != tests[i].out) {
	std::cerr << "** Unexpected tuple out: expected "
		  << tests[i].out << " and got "
		  << t_out->toString()
		  << "\n";
      }
    } else {
      // didn't get a tuple
      if ( tests[i].out != null_str) {
	std::cerr << "** Expected tuple " << tests[i].out 
		  << " but got none instead.\n";
      } 
    }
    std::cout.flush();
  }

  std::cout << "Done\n";

  return 0;
}
  

/*
 * End of file 
 */
 
