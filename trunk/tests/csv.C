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

const char *input[] = {
  "# Comment\r\n",
  "\r\n",
  "1,2,3\r\n",
  "\"This is half of a line. ... ",
  "and this is the other half\"\n",
  "Another,line",
  "\r\n",
  NULL
};

int main(int argc, char **argv)
{
  std::cout << "\nCSV\n";
  
  CSVParser cp;

  for(int i=0; input[i] != NULL; i++) {
    TupleRef t = Tuple::mk();
    t->append(New refcounted<TupleField>(str(input[i]))); 
    t->freeze();
    std::cout << "Tuple: <" << t->toString() << ">\n";
    cp.push(0,t,cbv_null);
    std::cout.flush();
  }

  std::cout << "\nDone\n";

  return 0;
}
  

/*
 * End of file 
 */
 
