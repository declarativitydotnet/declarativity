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
 * DESCRIPTION: Tests for simple element plumbing
 *
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */
#include <async.h>
#include <arpc.h>
#include <iostream>

#include "tuple.h"
#include "pullprint.h"
#include "memoryPull.h"

TupleRef create_tuple(int i) {
  TupleRef t = New refcounted<Tuple>;
  t->append(*New TupleField());
  t->append(*New TupleField((int32_t)i));
  t->append(*New TupleField((uint64_t)i));
  t->append(*New TupleField(i));
  strbuf myStringBuf;
  myStringBuf << "This is string '" << i << "'";
  str myString = myStringBuf;
  t->append(*New TupleField(myString));
  t->freeze();
  return t;
}

int main(int argc, char **argv)
{
  TupleRef t = create_tuple(1);
  TupleRef tupleRefBuffer[1] = {t};

  std::cout << "BASIC_ELEMENT_PLUMBING\n";

  PullPrint *pullPrint = New PullPrint();
  MemoryPull *memoryPull = New MemoryPull(tupleRefBuffer, 1);
  if (memoryPull->connect_output(0, pullPrint, 0) == -1) {
    std::cerr << "Cannot connect elements\n";
    exit(-1);
  }

  std::cout << "PullPrint\n";
  pullPrint->run();


  return 0;
}
  

/*
 * End of file 
 */
